#include "WPSoundParser.h"
#include "Audio/SoundManager.h"
#include "Fs/VFS.h"
#include "wpscene/WPSoundObject.h"
#include "Utils/Logging.h"

#include <string>
#include <string_view>

using namespace wallpaper;

enum class PlaybackMode {
	Random,
	Loop
};

static PlaybackMode ToPlaybackMode(std::string_view s) {
	if(s == "loop")
		return PlaybackMode::Loop;
	else if(s == "random")
		return PlaybackMode::Random;
	return PlaybackMode::Loop;
};

class WPSoundStream : public audio::SoundStream {
public:
	struct Config {
		float maxtime {10.0f};
		float mintime {0.0f};
		float volume {1.0f};
		PlaybackMode mode {PlaybackMode::Loop};
	};
	WPSoundStream(const std::vector<std::string>& paths, fs::VFS& vfs, Config c):m_soundPaths(paths),vfs(vfs),m_config(c) {};
	virtual ~WPSoundStream() = default;

	uint32_t NextPcmData(void* pData, uint32_t frameCount) override {
		// first
		if(!m_curActive) { Switch() ;}

		// loop
		uint32_t frameReads = m_curActive->NextPcmData(pData, frameCount);
		if(frameReads == 0) {
			Switch();
			frameReads = m_curActive->NextPcmData(pData, frameCount);
		}
		// volume 
		{
			float* pData_float = static_cast<float*>(pData);
			const auto num = frameReads * m_desc.channels;
			for(uint i=0;i<num;i++,pData_float++) {
				(*pData_float) *= m_config.volume;
			}
		}
		return frameReads;
	};
	void SetDesc(const Desc& d) { m_desc = d; }
	void Switch() {
		std::string path = m_soundPaths[LoopIndex()];
		LOG_INFO("Switch to audio file: %s", path.c_str());
		m_curActive = audio::CreateSoundStream(vfs.Open("/assets/" + path), m_desc);
	}
	uint32_t LoopIndex() {
		m_curIndex++;
		if(m_curIndex == m_soundPaths.size()) m_curIndex = 0;
		return m_curIndex;
	}
private:
	const std::vector<std::string> m_soundPaths;
	fs::VFS& vfs;
	Config m_config;
	Desc m_desc;
	std::unique_ptr<SoundStream> m_curActive;
	uint32_t m_curIndex {0};
};

void WPSoundParser::Parse(const wpscene::WPSoundObject& obj, fs::VFS& vfs, audio::SoundManager& sm) {
	WPSoundStream::Config config{
		.maxtime = obj.maxtime,
		.mintime = obj.mintime,
		.volume = obj.volume > 1.0f	? 1.0f : obj.volume,
		.mode = ToPlaybackMode(obj.playbackmode)
	};
	auto ss = std::make_unique<WPSoundStream>(obj.sound, vfs, config);
	auto ss_raw = ss.get();
	sm.MountStream(std::move(ss), [ss_raw](const audio::SoundStream::Desc& d) {
		ss_raw->SetDesc(d);
		return true;
	});
}