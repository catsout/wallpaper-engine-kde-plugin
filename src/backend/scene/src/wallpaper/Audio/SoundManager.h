#pragma once
#include <memory>
#include <cstdint>
#include <functional>
#include "../Utils/Logging.h"

namespace wallpaper
{

namespace fs { class IBinaryStream; }
namespace audio
{

class SoundStream {
public:
	struct Desc {
		uint32_t channels;
		uint32_t sampleRate;
	};
public:
	SoundStream() = default;
	virtual ~SoundStream() = default;
	SoundStream(const SoundStream&) = delete;
	SoundStream& operator=(const SoundStream&) = delete;
	SoundStream(SoundStream&&) = default;
	SoundStream& operator=(SoundStream&&) = default;

	virtual uint32_t NextPcmData(void* pData, uint32_t frameCount) = 0;
};
std::unique_ptr<SoundStream> CreateSoundStream(std::shared_ptr<fs::IBinaryStream>, const SoundStream::Desc&);

typedef std::function<bool(const SoundStream::Desc&)> MountCallbackOp;

class SoundManager {
public:
	SoundManager();
	~SoundManager();
	void MountStream(std::unique_ptr<SoundStream>&&, const MountCallbackOp&);
	void UnMountAll();
	void Test(std::shared_ptr<fs::IBinaryStream>);
	bool Init();
	bool IsInited() const;
	void Play();
	void Pause();

	float Volume() const;
	bool Muted() const;
	void SetMuted(bool);
	void SetVolume(float);

private:
	class impl;
	std::unique_ptr<impl> pImpl;
};
}
}
