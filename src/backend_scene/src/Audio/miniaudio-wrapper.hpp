#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <cstring>
#include <atomic>
#include <utility>
#include "Utils/Logging.h"
#include "Utils/NoCopyMove.hpp"

#define MA_NO_WASAPI
#define MA_NO_DSOUND
#define MA_NO_WINMM
#define MA_NO_COREAUDIO
#define MA_NO_ENCODING
#define STB_VORBIS_HEADER_ONLY
#include <miniaudio/stb_vorbis.c> /* Enables Vorbis decoding. */
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>
/* stb_vorbis implementation must come after the implementation of miniaudio. */
#undef STB_VORBIS_HEADER_ONLY
#include <miniaudio/stb_vorbis.c> /* Enables Vorbis decoding. */

namespace miniaudio
{

struct DeviceDesc {
    ma_uint32              phyChannels;
    ma_uint32              sampleRate;
    static const ma_format format { ma_format_f32 };
};

template<typename TStream>
class Decoder : NoCopy {
public:
    Decoder(TStream&& s): m_stream(std::move(s)) {}
    ~Decoder() { ma_decoder_uninit(&m_decoder); }
    Decoder(Decoder&& o) noexcept
        : m_decoder(std::exchange(o.m_decoder, ma_decoder()), m_stream(std::move(m_stream))) {}
    Decoder& operator=(Decoder&& o) noexcept {
        m_decoder = std::exchange(o.m_decoder, ma_decoder());
        m_stream  = std::move(m_stream);
        return *this;
    }

    bool Init(const DeviceDesc& d) {
        ma_decoder_config config =
            ma_decoder_config_init(DeviceDesc::format, d.phyChannels, d.sampleRate);
        ma_result result = ma_decoder_init(Read, Seek, this, &config, &m_decoder);
        m_inited         = result == MA_SUCCESS;
        if (! m_inited) {
            LOG_ERROR("init decoder failed");
        }
        return m_inited;
    }
    ma_uint64 NextPcmData(void* pData, ma_uint64 frameCount) {
        if (! m_inited) return 0;
        decltype(frameCount) readed { 0 };
        ma_result result = ma_decoder_read_pcm_frames(&m_decoder, pData, frameCount, &readed);
        return result == MA_SUCCESS ? readed : 0;
    }
    bool IsInited() { return m_inited; }

private:
    static ma_result Read(ma_decoder* pMaDecoder, void* pBufferOut, size_t bytesToRead,
                          size_t* pBytesRead) {
        auto* pDecoder = static_cast<Decoder<TStream>*>(pMaDecoder->pUserData);
        *pBytesRead    = pDecoder->m_stream.Read(pBufferOut, bytesToRead);
        return MA_SUCCESS;
    }
    static ma_result Seek(ma_decoder* pMaDecoder, ma_int64 byteOffset, ma_seek_origin origin) {
        auto* pDecoder = static_cast<Decoder<TStream>*>(pMaDecoder->pUserData);
        bool  ok       = pDecoder->m_stream.Seek(byteOffset, origin);
        return ok ? MA_SUCCESS : MA_ERROR;
    }
    bool       m_inited { false };
    ma_decoder m_decoder {};
    TStream    m_stream;
};

class Channel : NoCopy {
public:
    Channel()          = default;
    virtual ~Channel() = default;

    virtual ma_uint64 NextPcmData(void* pData, ma_uint32 frameCount) = 0;
    virtual void      PassDeviceDesc(const DeviceDesc&)              = 0;
};

class Device : NoCopy {
public:
    Device() {}
    ~Device() { UnInit(); }
    Device(Device&& o) noexcept: m_device(std::exchange(o.m_device, ma_device())) {}
    Device& operator=(Device&& o) noexcept {
        m_device = std::exchange(o.m_device, ma_device());
        return *this;
    }

public:
    bool Init(const DeviceDesc& d) {
        if (IsInited()) return true; // already inited
        ma_result result;
        auto      config = GenMaDeviceConfig(d);
        Stop();
        result = ma_device_init(NULL, &config, &m_device);
        if (result == MA_SUCCESS) {
            LOG_INFO("sound device inited");
        }
        if (result != MA_SUCCESS || ! IsInited()) {
            LOG_ERROR("can't init sound device");
            UnInit();
            return false;
        }
        if (m_device.playback.format != ma_format_f32) {
            LOG_ERROR("wrong playback format");
            UnInit();
            return false;
        }
        if (ma_device_start(&m_device) != MA_SUCCESS) {
            LOG_ERROR("can't start sound device");
            UnInit();
            return false;
        }
        {
            std::unique_lock<std::mutex> lock { m_mutex };
            for (auto& el : m_channels) {
                el.chn->PassDeviceDesc(GetDesc());
            }
        }
        Start();
        return true;
    }
    bool IsInited() const { return m_device.state != ma_device_state_uninitialized; }
    void UnInit() {
        if (IsInited()) {
            LOG_INFO("uninit sound device");
        }
        UnmountAll();
        ma_device_uninit(&m_device); // always do it
    }
    // bool IsStarted() const { return ma_device_is_started(&m_device); }
    // bool IsStopped() const { return ma_device_get_state(&m_device) == MA_STATE_STOPPED; }
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
    bool  Muted() const { return m_muted; }
    void  SetMuted(bool v) { m_muted = v; }

    void SetVolume(float v) { m_volume = v; };
    void MountChannel(std::shared_ptr<Channel> chn) {
        ChannelWrap chnw;
        chnw.chn = chn;
        chnw.chn->PassDeviceDesc(GetDesc());
        {
            std::unique_lock<std::mutex> lock { m_mutex };
            m_channels.push_back(chnw);
        }
    }
    void UnmountAll() {
        {
            std::unique_lock<std::mutex> lock { m_mutex };
            m_channels.clear();
        }
    }
    DeviceDesc GetDesc() const {
        return DeviceDesc { .phyChannels = m_device.playback.channels,
                            .sampleRate  = m_device.sampleRate };
    }

private:
    static void data_callback(ma_device* pMaDevice, void* pOutput, const void* pInput,
                              ma_uint32 frameCount) {
        Device* pDevice = static_cast<Device*>(pMaDevice->pUserData);
        if (! pDevice->IsInited()) return;
        pDevice->data_callback(pOutput, pInput, frameCount);
    }
    void data_callback(void* pOutput, const void* pInput, ma_uint32 frameCount) {
        (void)pInput;
        if (! m_running || m_muted) return;
        const auto phyChannels    = m_device.playback.channels;
        const auto framesSize     = frameCount * phyChannels;
        const auto framesByteSize = framesSize * sizeof(float);
        {
            if (m_frameBuffer.size() < framesByteSize) m_frameBuffer.resize(framesByteSize);
            // std::memset(pOutput, 0, framesByteSize);
        }
        {
            std::unique_lock<std::mutex> lock { m_mutex };

            float* pOutput_float = static_cast<float*>(pOutput);
            float* pBuffer_float = reinterpret_cast<float*>(m_frameBuffer.data());
            for (ma_uint32 i = 0; i < m_channels.size(); i++) {
                ma_uint64 framesReaded =
                    m_channels[i].chn->NextPcmData(m_frameBuffer.data(), frameCount);
                if (framesReaded == 0) {
                    m_channels[i].end = true;
                } else {
                    for (size_t i = 0; i < framesSize; i++)
                        pOutput_float[i] += m_volume * pBuffer_float[i];
                }
            }
            m_channels.erase(std::remove_if(m_channels.begin(),
                                            m_channels.end(),
                                            [](auto& c) {
                                                return c.end;
                                            }),
                             m_channels.end());
        }
    }
    ma_device_config GenMaDeviceConfig(const DeviceDesc& d) {
        ma_device_config config  = ma_device_config_init(ma_device_type_playback);
        config.sampleRate        = d.sampleRate;
        config.playback.format   = ma_format_f32;
        config.playback.channels = d.phyChannels;
        config.dataCallback      = data_callback;
        config.pUserData         = (void*)this;
        return config;
    }

private:
    struct ChannelWrap {
        bool                     end { false };
        std::shared_ptr<Channel> chn;
    };
    ma_device         m_device {}; // must init c struct
    std::mutex        m_mutex;     // for operating channel vector
    std::atomic<bool> m_running { false };

    float m_volume { 1.0f };
    bool  m_muted { false };

    std::vector<ChannelWrap> m_channels;
    std::vector<uint8_t>     m_frameBuffer;
};

} // namespace miniaudio
