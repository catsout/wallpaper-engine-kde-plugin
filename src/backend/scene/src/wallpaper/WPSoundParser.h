#pragma once

namespace wallpaper 
{

namespace audio { class SoundManager; }
namespace fs { class VFS; }
namespace wpscene { class WPSoundObject; }
class WPSoundParser {
public:
	static void Parse(const wpscene::WPSoundObject&, fs::VFS&, audio::SoundManager&);
};
}