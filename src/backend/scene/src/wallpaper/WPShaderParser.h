#pragma once
#include "Scene/SceneShader.h"

namespace wallpaper
{

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
};

class WPShaderParser {
public:
	static std::string LoadGlslInclude(const std::string& input);	

	static void ParseWPShader(const std::string& src, int32_t texcount, WPShaderInfo* pWPShaderInfo);

	static std::size_t FindIncludeInsertPos(const std::string& src, std::size_t startPos);

	static std::string PreShaderSrc(const std::string& src, int32_t texcount, WPShaderInfo* pWPShaderInfo);

	static std::string PreShaderHeader(const std::string& src, const Combos& combos); 
};
}
