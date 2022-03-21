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
	ShaderValueMap svs;
	ShaderValueMap baseConstSvs;
	WPAliasValueDict alias;
	WPDefaultTexs defTexs;
};

struct WPPreprocessorInfo {
	Map<std::string, std::string> input; // name to line
	Map<std::string, std::string> output;

	Set<uint> active_tex_slots;
	std::string result;
};

struct WPShaderTexInfo {
	bool enabled {false};
	std::array<bool, 3> composEnabled {false, false, false};
};

class WPShaderParser {
public:
	static std::string PreShaderSrc(fs::VFS&, const std::string& src, WPShaderInfo* pWPShaderInfo, const std::vector<WPShaderTexInfo>& texs);

	static std::string PreShaderHeader(const std::string& src, const Combos& combos, ShaderType); 

	static void InitGlslang();
	static void FinalGlslang();

	static void Preprocessor(const std::string& src, ShaderType, const Combos&, WPPreprocessorInfo&);
	static std::string Finalprocessor(const WPPreprocessorInfo& cur, const WPPreprocessorInfo* pre=nullptr, const WPPreprocessorInfo* next=nullptr);
};
}
