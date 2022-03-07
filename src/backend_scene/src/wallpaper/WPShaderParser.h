#pragma once
#include "Scene/SceneShader.h"
#include "Type.h"

namespace wallpaper
{
namespace fs { class VFS; }
typedef std::unordered_map<std::string, int32_t> Combos;

// ui material name to gl uniform name
typedef std::unordered_map<std::string, std::string> WPAliasValueDict;

typedef std::vector<std::pair<int32_t, std::string>> WPDefaultTexs;

struct WPShaderInfo {
	Combos combos;
	ShaderValues svs;
	ShaderValues baseConstSvs;
	WPAliasValueDict alias;
	WPDefaultTexs defTexs;

	Map<std::string, std::string> innerInOut; // name to line
};

struct WPShaderTexInfo {
	bool enabled {false};
	std::array<bool, 3> composEnabled {false, false, false};
};

class WPShaderParser {
public:
	static std::string PreShaderSrc(fs::VFS&, const std::string& src, WPShaderInfo* pWPShaderInfo, const std::vector<WPShaderTexInfo>& texs);

	static std::string PreShaderHeader(const std::string& src, const Combos& combos, ShaderType); 
};
}
