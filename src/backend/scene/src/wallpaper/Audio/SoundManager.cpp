#include "SoundManager.h"
#include "miniaudio-wrapper.hpp"
#include "../Fs/IBinaryStream.h"
#include "../Utils/Logging.h"

using namespace wallpaper::audio;

static SoundStream::Desc ToSSDesc(const miniaudio::DeviceDesc& d) {
	return {
		.channels = d.phyChannels,
		.sampleRate = d.sampleRate
	};
}
static miniaudio::DeviceDesc ToSSDesc(const SoundStream::Desc& d) {
	return {
		.phyChannels = d.channels,
		.sampleRate = d.sampleRate
	};
}

class Channel_Impl: public miniaudio::Channel {
public:
	Channel_Impl(std::unique_ptr<SoundStream>&& ss):m_ss(std::move(ss)) {}
	virtual ~Channel_Impl() = default;

	ma_uint32 NextPcmData(void* pData, ma_uint32 frameCount) override {
		return m_ss->NextPcmData(pData, frameCount);
	}
	void PassDeviceDesc(const miniaudio::DeviceDesc& desc) override {
		m_ss->PassDesc(ToSSDesc(desc));
	}
private:
	miniaudio::DeviceDesc m_desc;
	std::unique_ptr<SoundStream> m_ss;
};

struct BStreamWrapper {
	std::shared_ptr<wallpaper::fs::IBinaryStream> stream;
	size_t Read(void* pBufferOut, size_t bytesToRead) {
		size_t reads =  stream->Read(pBufferOut, bytesToRead);
		//LOG_INFO("r:%u, %u",bytesToRead, reads);
		return reads;
	}	
	bool Seek(size_t offset, ma_seek_origin origin) {
		bool result {false};
		switch (origin)
		{
		case ma_seek_origin_start:
			result = stream->SeekSet(offset);
			break;
		case ma_seek_origin_current:
			result = stream->SeekCur(offset);
			break;
		case ma_seek_origin_end:
			result = stream->SeekEnd(offset);
			break;
		}
		//LOG_INFO("s:%u, %d",offset, result);
		return false;
	}
};

template<typename T>
class SoundStream_impl : public SoundStream {
public:
	SoundStream_impl(std::unique_ptr<T>&& ss):m_ss(std::move(ss)) {}
	virtual ~SoundStream_impl() {}

	uint32_t NextPcmData(void* pData, uint32_t frameCount) override {
		return m_ss->NextPcmData(pData, frameCount);
	}
	void PassDesc(const Desc&) override {}
private:
	std::unique_ptr<T> m_ss;
};

std::unique_ptr<SoundStream> wallpaper::audio::CreateSoundStream(std::shared_ptr<wallpaper::fs::IBinaryStream> stream, const SoundStream::Desc& desc) {
	BStreamWrapper sw{stream};
	auto decoder = std::make_unique<miniaudio::Decoder<BStreamWrapper>>(std::move(sw));
	decoder->Init(ToSSDesc(desc));
	return std::make_unique<SoundStream_impl<miniaudio::Decoder<BStreamWrapper>>>(std::move(decoder));
}



class SoundManager::impl : NoCopy,NoMove {
public:
	impl():device() {};
	~impl() = default;
	miniaudio::Device device {};
};

SoundManager::SoundManager():pImpl(std::make_unique<impl>()) {

}
SoundManager::~SoundManager() {}

void SoundManager::MountStream(std::unique_ptr<SoundStream>&& ss) {
	// if(!IsInited()) return;
	pImpl->device.MountChannel(std::make_unique<Channel_Impl>(std::move(ss)));
}

void SoundManager::Test(std::shared_ptr<fs::IBinaryStream> stream) {
	BStreamWrapper sw{stream};
	auto decoder = std::make_unique<miniaudio::Decoder<BStreamWrapper>>(std::move(sw));
}
bool SoundManager::Init() {
	if(Muted()) { 
		LOG_INFO("muted, not init sound device");
		return false;
	}
	return pImpl->device.Init({});
}
bool SoundManager::IsInited() const {
	return pImpl->device.IsInited();
}
void SoundManager::Play() {
	pImpl->device.Start();
}
void SoundManager::Pause() {
	pImpl->device.Stop();
}

void SoundManager::UnMountAll() {
	pImpl->device.UnmountAll();
}
float SoundManager::Volume() const {
	return pImpl->device.Volume();
}

bool SoundManager::Muted() const {
	return pImpl->device.Muted();
}
void SoundManager::SetMuted(bool v) {
	pImpl->device.SetMuted(v);
	if(!Muted()) {
		Init();
	}	
	else {
		pImpl->device.UnInit();
	} 
}
void SoundManager::SetVolume(float v) {
	pImpl->device.SetVolume(v);
}
