#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <cstring>
#include <atomic>
#include "../Utils/Logging.h"

#define MA_NO_ENCODING
#define STB_VORBIS_HEADER_ONLY
#include <miniaudio/stb_vorbis.c>    /* Enables Vorbis decoding. */
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>
/* stb_vorbis implementation must come after the implementation of miniaudio. */
#undef STB_VORBIS_HEADER_ONLY
#include <miniaudio/stb_vorbis.c>    /* Enables Vorbis decoding. */


namespace miniaudio
{

struct DeviceDesc {
	ma_uint32 phyChannels;
	ma_uint32 sampleRate;
	static const ma_format format {ma_format_f32};
};

template<typename TStream>
class Decoder {
public:
	Decoder(TStream&& s)
	  :m_stream(std::move(s)) {}
	~Decoder() {
		ma_decoder_uninit(&m_decoder);
	}
	Decoder(const Decoder&) = delete;
	Decoder& operator =(const Decoder&) = delete;
	Decoder(Decoder&& o):m_decoder(o.m_decoder),m_stream(std::move(m_stream)) {
		o.m_decoder = ma_decoder();
	}
	Decoder& operator=(Decoder&& o) {
		m_decoder = o.m_decoder;
		m_stream = std::move(m_stream);
		o.m_decoder = ma_decoder();
		return *this;
	}
	  
	bool Init(const DeviceDesc& d) {
		ma_decoder_config config = ma_decoder_config_init(DeviceDesc::format, d.phyChannels, d.sampleRate);
		ma_result result = ma_decoder_init(Read, Seek, this, &config, &m_decoder);
		m_inited = result == MA_SUCCESS;
		if(!m_inited) {
			LOG_ERROR("init decoder failed");
		}
		return m_inited;
	}
	ma_uint32 NextPcmData(void* pData, ma_uint32 frameCount) {
		if(!m_inited) return 0;
		return ma_decoder_read_pcm_frames(&m_decoder, pData, frameCount);
	}
	bool IsInited() { return m_inited; }
private:
	static size_t Read(ma_decoder* pMaDecoder, void* pBufferOut, size_t bytesToRead) {
		auto* pDecoder = static_cast<Decoder<TStream>*>(pMaDecoder->pUserData);
		return pDecoder->m_stream.Read(pBufferOut, bytesToRead);
	}
	static ma_bool32 Seek(ma_decoder* pMaDecoder, ma_int64 byteOffset, ma_seek_origin origin) {
		auto* pDecoder = static_cast<Decoder<TStream>*>(pMaDecoder->pUserData);
		return pDecoder->m_stream.Seek(byteOffset, origin);
	}
	bool m_inited {false};
	ma_decoder m_decoder {};
	TStream m_stream;
};



class Channel {
public:
	Channel() = default;
	virtual ~Channel() = default;
	Channel(const Channel&) = delete;
	Channel& operator=(const Channel&) = delete;

	virtual ma_uint32 NextPcmData(void* pData, ma_uint32 frameCount) = 0;
};

class Device {
public:
	Device() {}
	~Device() {
		Uninit();
	}
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	Device(Device&& o):m_device(m_device) {
		o.m_device = ma_device();
	}
	Device& operator=(Device&& o) {
		m_device = o.m_device;
		o.m_device = ma_device();
		return *this;
	}
public:
	bool Init(const DeviceDesc& d) {
		ma_result result;
		auto config = GenMaDeviceConfig(d);
		result = ma_device_init(NULL, &config, &m_device);
		assert(result == MA_SUCCESS);
		assert(IsInited());
		if(m_device.playback.format != ma_format_f32) {
			LOG_ERROR("Wrong playback format");
			return false;
		}
		if (ma_device_start(&m_device) != MA_SUCCESS) {
			LOG_ERROR("can't start sound device");
			return false;
		}
		Start();
		return true;
	}
	bool IsInited() const { return m_device.state != MA_STATE_UNINITIALIZED; }
	void Uninit() {
    	ma_device_uninit(&m_device);
	}
	//bool IsStarted() const { return ma_device_is_started(&m_device); }
	//bool IsStopped() const { return ma_device_get_state(&m_device) == MA_STATE_STOPPED; }
	void Start() {
		m_running = true;
		/*
		if(!IsStopped()) return;
		LOG_INFO("state: %d", ma_device_get_state(&m_device));
		if (ma_device_start(&m_device) != MA_SUCCESS) {
			LOG_ERROR("can't start sound device");
			//ma_device_uninit(&m_device);
		}
		*/
	}
	void Stop() {
		m_running = false;
		/*
		if(!IsStarted()) return;
		LOG_INFO("state: %d", ma_device_get_state(&m_device));
		if(ma_device_stop(&m_device) != MA_SUCCESS){
			LOG_ERROR("can't stop sound device");
		}*/
	}
	float Volume() const { return m_volume; }
	bool Muted() const { return m_muted; }
	void SetMuted(bool v) { m_muted = v; }
	void SetVolume(float v) { m_volume = v; };
	void MountChannel(std::shared_ptr<Channel> chn) {
		ChannelWrap chnw;
		chnw.chn = chn;
		{
			std::unique_lock<std::mutex> lock {m_mutex};
			m_channels.push_back(chnw);
		}
	}
	void UnmountAll() {
		{
			std::unique_lock<std::mutex> lock {m_mutex};
			m_channels.clear();
		}
	}
	DeviceDesc GetDesc() const {
		return DeviceDesc{
			.phyChannels = m_device.playback.channels,
			.sampleRate = m_device.sampleRate
		};
	}
private:
    static void data_callback(ma_device* pMaDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)	{
		Device* pDevice = static_cast<Device*>(pMaDevice->pUserData);
		if(!pDevice->IsInited()) return;
		pDevice->data_callback(pOutput, pInput, frameCount);
	}
	void data_callback(void* pOutput, const void* pInput, ma_uint32 frameCount) {
		(void)pInput;
		if(!m_running || m_muted) return;
		const auto phyChannels = m_device.playback.channels;
		const auto framesSize = frameCount * phyChannels;
		const auto framesByteSize = framesSize * sizeof(float);
		{
			if(m_frameBuffer.size() < framesByteSize) 
				m_frameBuffer.resize(framesByteSize);
			//std::memset(pOutput, 0, framesByteSize);
		}
		{
			std::unique_lock<std::mutex> lock {m_mutex};

			float* pOutput_float = static_cast<float*>(pOutput);
			float* pBuffer_float = reinterpret_cast<float*>(m_frameBuffer.data());
			for(int i=0;i<m_channels.size();i++) {
				ma_uint32 framesReaded = m_channels[i].chn->NextPcmData(m_frameBuffer.data(), frameCount);
				if(framesReaded == 0) {
					m_channels[i].end = true;
				} else {
					for (size_t i=0; i < framesSize; i++)
						pOutput_float[i] += m_volume*pBuffer_float[i];

				}
			}
			m_channels.erase(std::remove_if(
				m_channels.begin(),
				m_channels.end(),
				[](auto& c) { return c.end; }
			),m_channels.end());
		}
	}
	ma_device_config GenMaDeviceConfig(const DeviceDesc& d) {
		ma_device_config config = ma_device_config_init(ma_device_type_playback);
		config.sampleRate = d.sampleRate;
		config.playback.format = ma_format_f32;
		config.playback.channels = d.phyChannels;
		config.dataCallback = data_callback;
		config.pUserData = (void*)this;
		return config;
	}
private:
	struct ChannelWrap {
		bool end {false};
		std::shared_ptr<Channel> chn;
	};
	ma_device m_device {};
	std::mutex m_mutex; // for operating channel vector
	std::atomic<bool> m_running {false};

	float m_volume {1.0f};
	bool m_muted {false};
	std::vector<ChannelWrap> m_channels;
	std::vector<uint8_t> m_frameBuffer;
};

}