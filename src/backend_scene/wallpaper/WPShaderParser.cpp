#include "WPShaderParser.h"

#include "WPJson.h"

#include "wpscene/WPUniform.h"
#include "Fs/VFS.h"
#include "Utils/span.hpp"
#include "Vulkan/Shader.hpp"

#include <regex>
#include <stack>
#include <charconv>

static constexpr std::string_view SHADER_PLACEHOLD {"__SHADER_PLACEHOLD__"};

using namespace wallpaper;

static constexpr const char* pre_shader_code = R"(#version 150
#define GLSL 1
#define HLSL 1
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

__SHADER_PLACEHOLD__

)";

static constexpr const char* pre_shader_code_vert = R"(
#define attribute in
#define varying out

)";
static constexpr const char* pre_shader_code_frag = R"(
#define varying in
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
					std::string name = defines.back();
					bool istex = name.compare(0, 9, "g_Texture") == 0;
					if(istex) {
						wpscene::WPUniformTex wput;
						wput.FromJson(sv_json);
						int32_t index {0};
						STRTONUM(name.substr(9), index);
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
							name = defines.back();
							if(value.is_string()) {
								std::vector<float> v;
								GET_JSON_VALUE(value, v);
								sv = Span<float>(v);
							}
							else if(value.is_number()) {
								sv.setSize(1);
								GET_JSON_VALUE(value, sv[0]);
							}
							shadervalues[name] = sv;
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

static EShLanguage ToGLSL(ShaderType type) {
	switch (type)
	{
	case ShaderType::VERTEX: return EShLangVertex;
	case ShaderType::FRAGMENT: return EShLangFragment;
	}
	return EShLangVertex;
} 


void WPShaderParser::InitGlslang() {
	glslang::InitializeProcess();
}
void WPShaderParser::FinalGlslang() {
	glslang::FinalizeProcess();
}

void WPShaderParser::Preprocessor(const std::string& in_src, ShaderType type, const Combos& combos, WPPreprocessorInfo& process_info) {
	std::string res;

	std::string src = PreShaderHeader(in_src, combos, type);

	glslang::TShader::ForbidIncluder includer;
	glslang::TShader shader(ToGLSL(type));
	const EShMessages emsg { (EShMessages)(EShMsgDefault 
		| EShMsgSpvRules
		| EShMsgRelaxedErrors
		| EShMsgSuppressWarnings
		| EShMsgVulkanRules
	)};
	auto* data = src.c_str();
	shader.setStrings(&data, 1);
	shader.preprocess(&vulkan::DefaultTBuiltInResource, 100, EProfile::ECoreProfile, false, false, emsg, &res, includer);

	std::regex re_io (R"(.+\s(in|out)\s[\s\w]+\s(\w+)\s*;)", std::regex::ECMAScript);
	for(auto it = std::sregex_iterator(res.begin(), res.end(), re_io);
		it != std::sregex_iterator(); it++) {
		std::smatch mc = *it;
		if(mc[1] == "in") {
			process_info.input[mc[2]] = mc[0].str();
		} else {
			process_info.output[mc[2]] = mc[0].str();
		}
	}

	std::regex re_tex(R"(uniform\s+sampler2D\s+g_Texture(\d+))", std::regex::ECMAScript);
	for(auto it = std::sregex_iterator(res.begin(), res.end(), re_tex);
		it != std::sregex_iterator(); it++) {
		std::smatch mc = *it;
		auto str = mc[1].str();
		uint slot;
		auto [ptr, ec] { std::from_chars(str.c_str(), str.c_str() + str.size(), slot) };
		if(ec != std::errc()) continue;
		process_info.active_tex_slots.insert(slot);
	}
	process_info.result = std::move(res);
}

std::string WPShaderParser::Finalprocessor(const WPPreprocessorInfo& cur, const WPPreprocessorInfo* pre, const WPPreprocessorInfo* next) {
	std::string insert_str;
	if(pre != nullptr) {
		for(auto& [k,v]:pre->output) {
			if(!exists(cur.input, k)) {
				auto n = std::regex_replace(v, std::regex(R"(\s*out\s)"), " in ");
				insert_str += n + '\n';
			}
		}
	}
	if(next != nullptr) {
		for(auto& [k,v]:next->input) {
			if(!exists(cur.output, k)) {
				auto n = std::regex_replace(v, std::regex(R"(\s*in\s)"), " out ");
				insert_str += n + '\n';
			}
		}
	}
	std::regex re_hold(SHADER_PLACEHOLD.data());
	//LOG_INFO("insert: %s", insert_str.c_str());
	return std::regex_replace(
		std::regex_replace(cur.result, re_hold, insert_str),
		std::regex(R"(\s+\n)"),
		"\n"
	);
}
