#include "WPShaderParser.h"

#include "WPJson.h"

#include "wpscene/WPUniform.h"
#include "Fs/VFS.h"

#include <regex>
#include <stack>

using namespace wallpaper;

static constexpr const char* pre_shader_code = R"(#version 140
#define GLSL 1
#define highp
#define mediump
#define lowp

#define CAST2(x) (vec2(x))
#define CAST3(x) (vec3(x))
#define CAST4(x) (vec4(x))
#define CAST3X3(x) (mat3(x))

#define texSample2D texture
#define texSample2DLod textureLod
#define mul(x, y) ((y) * (x))
#define frac fract
#define atan2 atan
#define fmod(x, y) (x-y*trunc(x/y))
#define ddx dFdx
#define ddy(x) dFdy(-(x))
#define saturate(x) (clamp(x, 0.0, 1.0))

#define max(x, y) max(y, x)

#define float1 float
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define lerp mix
)";

static constexpr const char* pre_shader_code_vert = R"(
)";
static constexpr const char* pre_shader_code_frag = R"(
#define gl_FragColor glOutColor
out vec4 glOutColor;
)";

std::string LoadGlslInclude(fs::VFS& vfs, const std::string& input) {
	std::string::size_type pos = 0;
	std::string output;
	std::string::size_type linePos = std::string::npos;

	while(linePos = input.find("#include", pos), linePos != std::string::npos) {
		auto lineEnd = input.find_first_of('\n', linePos);
		auto lineSize = lineEnd - linePos;
		auto lineStr = input.substr(linePos, lineSize);
		output.append(input.substr(pos, linePos-pos));

		auto inP = lineStr.find_first_of('\"') + 1;
		auto inE = lineStr.find_last_of('\"');
		auto includeName = lineStr.substr(inP, inE - inP);
		auto includeSrc = fs::GetFileContent(vfs, "/assets/shaders/"+includeName);
		output.append("\n//-----include " + includeName + "\n");
		output.append(LoadGlslInclude(vfs, includeSrc));
		output.append("\n//-----include end\n");

		pos = lineEnd;
	}
	output.append(input.substr(pos));
	return output;
}


void ParseWPShader(const std::string& src, WPShaderInfo* pWPShaderInfo, const std::vector<WPShaderTexInfo>& texinfos) {
	auto& combos = pWPShaderInfo->combos;
	auto& wpAliasDict = pWPShaderInfo->alias;
	auto& shadervalues = pWPShaderInfo->svs;
	auto& defTexs = pWPShaderInfo->defTexs;
	int32_t texcount = texinfos.size();

	// pos start of line
	std::string::size_type pos = 0, lineEnd = std::string::npos;
	while((lineEnd = src.find_first_of(('\n'), pos)), true) {
		const auto clineEnd = lineEnd;
		const auto line = src.substr(pos, lineEnd - pos);

		/*
		if(line.find("attribute ") != std::string::npos || line.find("in ") != std::string::npos) {
			update_pos = true;
		}
		else if(line.find("varying ") != std::string::npos || line.find("out ") != std::string::npos) {
			update_pos = true;
		}
		*/
		if(line.find("// [COMBO]") != std::string::npos) {
			nlohmann::json combo_json;
			if(PARSE_JSON(line.substr(line.find_first_of('{')), combo_json)) {
				if(combo_json.contains("combo")) {
					std::string name;
					int32_t value = 0;
					GET_JSON_NAME_VALUE(combo_json, "combo", name);
					GET_JSON_NAME_VALUE(combo_json, "default", value);
					combos[name] = value;
				}
			}
		}
		else if(line.find("uniform ") != std::string::npos) {
			if(line.find("// {") != std::string::npos) {
				nlohmann::json sv_json;
				if(PARSE_JSON(line.substr(line.find_first_of('{')), sv_json)) {
					std::vector<std::string> defines = utils::SpliteString(line.substr(0, line.find_first_of(';')), ' ');

					std::string material;
					GET_JSON_NAME_VALUE_NOWARN(sv_json, "material", material);
					if(!material.empty())
						wpAliasDict[material] = defines.back();	

					ShaderValue sv;	
					sv.name = defines.back();
					bool istex = sv.name.compare(0, 9, "g_Texture") == 0;
					if(istex) {
						wpscene::WPUniformTex wput;
						wput.FromJson(sv_json);
						int32_t index {0};
						STRTONUM(sv.name.substr(9), index);
						if(!wput.default_.empty())
							defTexs.push_back({index,wput.default_});
						if(!wput.combo.empty()) {
							int32_t value {1};
							if(index >= texcount)
								value = 0;
							combos[wput.combo] = value;
						}
						if(index < texcount && texinfos[index].enabled) {
							auto& compos = texinfos[index].composEnabled;
							int num = std::min(compos.size(), wput.components.size());
							for(int i=0;i<num;i++) {
								if(compos[i])
									combos[wput.components[i].combo] = 1;
							}
						}

					} else {
						if(sv_json.contains("default")){
							auto value = sv_json.at("default");
							ShaderValue sv;	
							sv.name = defines.back();
							if(value.is_string())
								GET_JSON_VALUE(value, sv.value);
							if(value.is_number()) {
								sv.value.resize(1);
								GET_JSON_VALUE(value, sv.value[0]);
							}
								//sv.value = {value.get<float>()};
							shadervalues[sv.name] = sv;
						}
						if(sv_json.contains("combo")) {
							std::string name;
							int32_t value = 1;
							GET_JSON_NAME_VALUE(sv_json, "combo", name);
							combos[name] = value;
						}
					}
					if(defines.back()[0] != 'g') {
						LOG_INFO("PreShaderSrc User shadervalue not supported");
					}
				}
			}
		}

		// end
		if(line.find("void main()") != std::string::npos || clineEnd == std::string::npos) {
			break;
		}
		pos = lineEnd + 1;	
	} 
}

std::size_t FindIncludeInsertPos(const std::string& src, std::size_t startPos) {
	/* rule:
	after attribute/varying/uniform/struct
	befor any func
	not in {}
	not in #if #endif
	*/
	auto NposToZero = [](std::size_t p) { return p == std::string::npos ? 0 : p; };
	auto search = [](const std::string& p, std::size_t pos, const auto& re) {
		std::smatch match;
		if(std::regex_search(p.begin() + pos, p.end(), match, re)) {
			return pos + match.position();
		}
		return std::string::npos;
	};		
	auto searchLast = [](const std::string& p, const auto& re) { 
		auto startPos = p.begin();
		std::smatch match;
		while(std::regex_search(startPos+1, p.end(), match, re)) {
			startPos++;
			startPos += match.position();
		}
		return startPos == p.end() ? std::string::npos : startPos - p.begin();
	};
	auto nextLinePos = [](const std::string& p, std::size_t pos) {
		return p.find_first_of('\n', pos) + 1;
	};

	std::size_t mainPos = src.find("void main(");
	std::size_t pos;
	{
		const std::regex reAfters(R"(\n(attribute|varying|uniform|struct) )");
		std::size_t afterPos = searchLast(src, reAfters);
		if(afterPos != std::string::npos) {
			afterPos = nextLinePos(src, afterPos+1);
		}
		pos = std::min({NposToZero(afterPos), mainPos});
	}
	{
		std::stack<std::size_t> ifStack;
		std::size_t nowPos {0};
		const std::regex reIfs(R"((#if|#endif))");
		while(true) {
			auto p = search(src, nowPos + 1, reIfs);
			if(p > mainPos || p == std::string::npos) break;
			if(src.substr(p, 3) == "#if") {
				ifStack.push(p);
			} else {
				if(ifStack.empty()) break;
				std::size_t ifp = ifStack.top();
				ifStack.pop();
				std::size_t endp = p;
				if(pos > ifp && pos <= endp) {
					pos = nextLinePos(src, endp + 1); 
				}
			}
			nowPos = p;
		}
		pos = std::min({pos, mainPos});
	}

	return NposToZero(pos);
}

std::string WPShaderParser::PreShaderSrc(fs::VFS& vfs, const std::string& src, WPShaderInfo* pWPShaderInfo, const std::vector<WPShaderTexInfo>& texinfos) {
	std::string newsrc(src);
	std::string::size_type pos = 0;
	std::string include;
	while(pos = src.find("#include", pos), pos != std::string::npos) {
		auto begin = pos;
		pos = src.find_first_of('\n', pos);	
		newsrc.replace(begin, pos-begin, pos-begin, ' ');
		include.append(src.substr(begin, pos - begin) + "\n");
	}
	include = LoadGlslInclude(vfs, include);

	ParseWPShader(include, pWPShaderInfo, texinfos);
	ParseWPShader(newsrc,pWPShaderInfo, texinfos);

	newsrc.insert(FindIncludeInsertPos(newsrc, 0), include); 
	return newsrc;
}

std::string WPShaderParser::PreShaderHeader(const std::string& src, const Combos& combos, ShaderType type) {
	std::string pre(pre_shader_code);
	if(type == ShaderType::VERTEX) pre += pre_shader_code_vert;
	if(type == ShaderType::FRAGMENT) pre += pre_shader_code_frag;
	std::string header(pre);
	for(const auto& c:combos) {
		std::string cup(c.first);
		std::transform(c.first.begin(), c.first.end(), cup.begin(), ::toupper);
		header.append("#define " + cup + " " + std::to_string(c.second) + "\n");
	}
	return header + src;
}
