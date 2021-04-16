#include "WPSceneParser.h"
#include "WPJson.h"
#include "pkg.h"
#include "common.h"
#include "wallpaper.h"
#include "WPTexImageParser.h"
#include "Type.h"

#include "WPShaderValueUpdater.h"
#include "wpscene/WPImageObject.h"
#include "wpscene/WPScene.h"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>


using namespace wallpaper;

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

const std::string pre_shader_code = "#version 120\n"
									  "#define highp\n"
									  "#define mediump\n"
									  "#define lowp\n"
									  "#define mul(x, y) (y * x)\n"
									  "#define frac fract\n"
									  "#define CAST2(x) (vec2(x))\n"
									  "#define CAST3(x) (vec3(x))\n"
									  "#define CAST4(x) (vec4(x))\n"
									  "#define CAST3X3(x) (mat3(x))\n"
									  "#define saturate(x) (clamp(x, 0.0, 1.0))\n"
									  "#define texSample2D texture2D\n"
									  "#define texSample2DLod texture2DLod\n"
									  "#define texture2DLod texture2D\n"
									  "#define atan2 atan\n"
									  "#define ddx dFdx\n"
									  "#define max(x, y) max(y, x)\n"
									  "#define ddy(x) dFdy(-(x))\n\n";

std::string LoadGlslInclude(const std::string& input) {
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
		auto includeSrc = fs::GetContent(WallpaperGL::GetPkgfs(),"shaders/"+includeName);
		output.append(LoadGlslInclude(includeSrc));

		pos = lineEnd;
	}
	output.append(input.substr(pos));
	return output;
}

void ParseWPShader(const std::string& src, int32_t texcount, WPShaderInfo* pWPShaderInfo, std::string::size_type& includeInsertPos) {
	auto& combos = pWPShaderInfo->combos;
	auto& wpAliasDict = pWPShaderInfo->alias;
	auto& shadervalues = pWPShaderInfo->svs;
	auto& defTexs = pWPShaderInfo->defTexs;
	// pos start of line
	std::string::size_type pos = 0, lineEnd = std::string::npos;
	while((lineEnd = src.find_first_of(('\n'), pos)), true) {
		const auto clineEnd = lineEnd;
		const auto line = src.substr(pos, lineEnd - pos);

		// no continue
		bool update_pos = false;
        if(line.find("attribute ") != std::string::npos) {
			update_pos = true;
		}
		else if(line.find("varying ") != std::string::npos) {
			update_pos = true;
		}
		else if(line.find("// [COMBO]") != std::string::npos) {
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
			update_pos = true;
			if(line.find("// {") != std::string::npos) {
				nlohmann::json sv_json;
				if(PARSE_JSON(line.substr(line.find_first_of('{')), sv_json)) {
					std::vector<std::string> defines = SpliteString(line.substr(0, line.find_first_of(';')), " ");

					std::string material;
					GET_JSON_NAME_VALUE(sv_json, "material", material);
					if(!material.empty())
						wpAliasDict[material] = defines.back();	

					ShaderValue sv;	
					sv.name = defines.back();
					if(defines.back()[0] != 'g') {
						LOG_INFO("PreShaderSrc User shadervalue not supported");
					}
					if(sv_json.contains("default")){
						auto value = sv_json.at("default");
						if(sv.name.compare(0, 9, "g_Texture") == 0) {
							int32_t index {0};
							std::string strValue;
							STRCONV(sv.name.substr(9, sv.name.size()-9), index);
							GET_JSON_VALUE(value, strValue);
							defTexs.push_back({index, strValue});
						} else {
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
					}
					if(sv_json.contains("combo")) {
						std::string name;
						int32_t value = 1;
						GET_JSON_NAME_VALUE(sv_json, "combo", name);
						if(sv.name.compare(0, 9, "g_Texture") == 0) {
							int32_t t;
							STRCONV(sv.name.substr(9), t);
							if(t >= texcount)
								value = 0;
						}
						combos[name] = value;
					}
				}
			}
		}
		if(update_pos)
			includeInsertPos = clineEnd;

		// end
		if(clineEnd == std::string::npos) {
			break;
		}
		if(line.find("void main()") != std::string::npos) {
			break;
		}
		pos = lineEnd + 1;	
	} 
	if(includeInsertPos == std::string::npos) 
		includeInsertPos = 0; 
	else includeInsertPos++;
	if(src.substr(includeInsertPos, 6) == "#endif") {
		includeInsertPos += 7;
	}
}

std::string PreShaderSrc(const std::string& src, int32_t texcount, WPShaderInfo* pWPShaderInfo) {
	std::string newsrc(src);
	std::string::size_type pos = 0;
	std::string include;
	while(pos = src.find("#include", pos), pos != std::string::npos) {
		auto begin = pos;
		pos = src.find_first_of('\n', pos);	
		newsrc.replace(begin, pos-begin, pos-begin, ' ');
		include.append(src.substr(begin, pos - begin) + "\n");
	}
	include = LoadGlslInclude(include);
	ParseWPShader(include, texcount, pWPShaderInfo, pos);
	pos = 0;
	ParseWPShader(newsrc, texcount, pWPShaderInfo, pos);

	newsrc.insert(pos, include); 
	return newsrc;
}

std::string PreShaderHeader(const std::string& src, const Combos& combos) {
	std::string header(pre_shader_code);
	for(const auto& c:combos) {
		std::string cup(c.first);
		std::transform(c.first.begin(), c.first.end(), cup.begin(), ::toupper);
		header.append("#define " + cup + " " + std::to_string(c.second) + "\n");
	}
	return header + src;
}

void LoadMaterial(const wpscene::WPMaterial& wpmat, Scene* pScene, SceneNode* pNode, SceneMaterial* pMaterial, WPShaderValueData* pSvData, WPShaderInfo* pWPShaderInfo=nullptr) {
	auto& svData = *pSvData;
	auto& material = *pMaterial;

	std::unique_ptr<WPShaderInfo> upWPShaderInfo(nullptr);
	if(pWPShaderInfo == nullptr) {
		upWPShaderInfo = std::make_unique<WPShaderInfo>();
		pWPShaderInfo = upWPShaderInfo.get();
	}

	SceneMaterialCustomShader materialShader;

	auto& shader = materialShader.shader;
	shader = std::make_shared<SceneShader>();

	std::string svCode = fs::GetContent(WallpaperGL::GetPkgfs(),"shaders/"+wpmat.shader+".vert");
	std::string fgCode = fs::GetContent(WallpaperGL::GetPkgfs(),"shaders/"+wpmat.shader+".frag");
	int32_t texcount = wpmat.textures.size();
	svCode = PreShaderSrc(svCode, texcount, pWPShaderInfo);
	fgCode = PreShaderSrc(fgCode, texcount, pWPShaderInfo);
	shader->uniforms = pWPShaderInfo->svs;

	for(const auto& el:wpmat.combos) {
		pWPShaderInfo->combos[el.first] = el.second; 
	}

	svCode = PreShaderHeader(svCode, pWPShaderInfo->combos);
	fgCode = PreShaderHeader(fgCode, pWPShaderInfo->combos);

	shader->vertexCode = svCode;
	shader->fragmentCode = fgCode;
	shader->attrs.push_back({"a_Position", 0});
	shader->attrs.push_back({"a_TexCoord", 1});

	auto textures = wpmat.textures;
	if(pWPShaderInfo->defTexs.size() > 0) {
		for(auto& t:pWPShaderInfo->defTexs) {
			if(textures.size() > t.first) {
				if(!textures.at(t.first).empty())
					continue;
			} else {
				textures.resize(t.first+1);
			}
			textures[t.first] = t.second;
		}
	}
//		svData.resolutions.resize(content.at("textures").size());
	WPTexImageParser texParser;
	for(int32_t i=0;i<textures.size();i++) {
		std::string name = textures.at(i);
		if(name == "_rt_FullFrameBuffer")
			name = "_rt_default";
		if(name.compare(0, 6, "_rt_im") == 0) {
			name = "";
			LOG_ERROR("unsupported layer texture");
		}
		material.textures.push_back(name);
		material.defines.push_back("g_Texture" + std::to_string(i));
		if(name.empty()) {
			LOG_ERROR("empty texture name");
			continue;
		}
		std::vector<float> resolution;
		if(name.compare(0, 4,"_rt_") == 0) {
			if(pScene->renderTargets.count(name) == 0) {
				LOG_INFO(name);
			} else {
				svData.renderTargetResolution.push_back({i, name});
				const auto& rt = pScene->renderTargets.at(name);
				resolution = {
					(float)rt.width, 
					(float)rt.height, 
					(float)rt.width, 
					(float)rt.height
				};
			}
		} else {
			auto texh = pScene->imageParser->ParseHeader(name);
			if(texh->type == ImageType::UNKNOWN)
				resolution = {
					(float)texh->width, 
					(float)texh->height, 
					(float)texh->mapWidth, 
					(float)texh->mapHeight
				};
			else
				resolution = {
					(float)texh->mapWidth, 
					(float)texh->mapHeight, 
					(float)texh->mapWidth, 
					(float)texh->mapHeight
				};
			if(pScene->textures.count(name) == 0) {
				SceneTexture stex;
				stex.sample = texh->sample;
				stex.url = name;
				if(texh->isSprite) {
					stex.isSprite = texh->isSprite;
					stex.spriteAnim = std::move(texh->spriteAnim);
				}
				pScene->textures[name] = std::make_shared<SceneTexture>(stex);
			}
		}
		if(!resolution.empty()) {
			const std::string gResolution = "g_Texture" + std::to_string(i) + "Resolution";
			materialShader.constValues[gResolution] = {gResolution, resolution};
		}
	}
	if(wpmat.blending == "translucent" || wpmat.blending == "normal") {
		material.blenmode = BlendMode::Translucent;
	} else if(wpmat.blending == "additive") {
		material.blenmode = BlendMode::Additive;
	} else if(wpmat.blending == "disabled") {
		material.blenmode = BlendMode::Disable;
	} else {
		LOG_ERROR("unknown blending "+wpmat.blending);
	}
	for(const auto& el:pWPShaderInfo->baseConstSvs) {
		materialShader.constValues[el.first] = el.second;
	}
	material.customShader = materialShader;
}

std::unique_ptr<Scene> WPSceneParser::Parse(const std::string& buf) {
	nlohmann::json json;
	if(!PARSE_JSON(buf, json)) 
		return nullptr;
	wpscene::WPScene sc;
	sc.FromJson(json);
//	LOG_INFO(nlohmann::json(sc).dump(4));

	auto upScene = std::make_unique<Scene>();
	upScene->sceneGraph = std::make_shared<SceneNode>();
	upScene->imageParser = std::make_unique<WPTexImageParser>();
	auto shaderValueUpdater = std::make_unique<WPShaderValueUpdater>(upScene.get());

	shaderValueUpdater->SetOrtho(sc.general.orthogonalprojection.width, sc.general.orthogonalprojection.height);

	upScene->clearColor = sc.general.clearcolor;
	
	WPCameraParallax cameraParallax;
	cameraParallax.enable = sc.general.cameraparallax;
	cameraParallax.amount = sc.general.cameraparallaxamount;
	cameraParallax.delay = sc.general.cameraparallaxdelay;
	cameraParallax.mouseinfluence = sc.general.cameraparallaxmouseinfluence;
	shaderValueUpdater->SetCameraParallax(cameraParallax);

	const auto& ortho = sc.general.orthogonalprojection; 
	upScene->renderTargets["_rt_default"] = {(uint32_t)ortho.width, (uint32_t)ortho.height};

	// effect camera 
	upScene->cameras["effect"] = std::make_shared<SceneCamera>(2, 2, -1.0f, 1.0f);
	auto spEffCamNode = std::make_shared<SceneNode>(); // at 0,0,0
	upScene->cameras.at("effect")->AttatchNode(spEffCamNode);
	upScene->sceneGraph->AppendChild(spEffCamNode);

	// global camera
	upScene->cameras["global"] = std::make_shared<SceneCamera>(
		int32_t(ortho.width / sc.general.zoom), 
		int32_t(ortho.height / sc.general.zoom), 
		-1.0f, 1.0f
	);
	upScene->activeCamera = upScene->cameras.at("global").get();
    auto& objects = json.at("objects");
	std::vector<float> cori{ortho.width/2.0f,ortho.height/2.0f,0},cscale{1.0f,1.0f,1.0f},cangle(3);
	auto spCamNode = std::make_shared<SceneNode>(cori, cscale, cangle);
	upScene->activeCamera->AttatchNode(spCamNode);
	upScene->sceneGraph->AppendChild(spCamNode);

    for(auto& obj:objects) {
		if(!obj.contains("image")) continue;
		if(obj.at("image").is_null()) continue;
		wpscene::WPImageObject wpimgobj;
		wpimgobj.FromJson(obj);
		//LOG_INFO(nlohmann::json(wpimgobj).dump(4));

		if(!wpimgobj.visible)
			continue;
		if(wpimgobj.colorBlendMode != 0) {
			wpscene::WPImageEffect colorEffect;
			wpscene::WPMaterial colorMat;
			nlohmann::json json;
			if(!PARSE_JSON(fs::GetContent(WallpaperGL::GetPkgfs(), "materials/util/effectpassthrough.json"), json)) 
				return nullptr;
			colorMat.FromJson(json);
			colorMat.combos["BONECOUNT"] = 1;
			colorMat.combos["BLENDMODE"] = wpimgobj.colorBlendMode;
			colorMat.blending = "disabled";
			colorEffect.materials.push_back(colorMat);
			wpimgobj.effects.push_back(colorEffect);
		}	
		int32_t count_eff = 0;
		for(const auto& wpeffobj:wpimgobj.effects) {
			if(wpeffobj.visible)
				count_eff++;
		}
		bool hasEffect = count_eff > 0;
		// skip no effect fullscreen layer
		if(!hasEffect && wpimgobj.fullscreen)
			continue;

		bool isCompose = wpimgobj.image == "models/util/composelayer.json";
		// skip no effect compose layer
		// it's no the correct behaviour, but do it for now
		if(!hasEffect && isCompose)
			continue;

		wpimgobj.origin[1] = ortho.height - wpimgobj.origin[1];
		auto spNode = std::make_shared<SceneNode>(wpimgobj.origin, wpimgobj.scale, wpimgobj.angles);

		SceneMaterial material;
		WPShaderValueData svData;
		if(!hasEffect)
			svData.parallaxDepth = wpimgobj.parallaxDepth;

		ShaderValues baseConstSvs;
		baseConstSvs["g_Alpha"] = {"g_Alpha", {wpimgobj.alpha}};
		baseConstSvs["g_Color"] = {"g_Color", wpimgobj.color};
		baseConstSvs["g_UserAlpha"] = {"g_UserAlpha", {wpimgobj.alpha}};
		baseConstSvs["g_Brightness"] = {"g_Brightness", {wpimgobj.brightness}};

		WPShaderInfo shaderInfo;
		shaderInfo.baseConstSvs = baseConstSvs;
		LoadMaterial(wpimgobj.material, upScene.get(), spNode.get(), &material, &svData, &shaderInfo);
		for(const auto& cs:wpimgobj.material.constantshadervalues) {
			const auto& name = cs.first;
			const std::vector<float>& value = cs.second;
			std::string glname;
			if(shaderInfo.alias.count(name) != 0) {
				glname = shaderInfo.alias.at(name);
			} else {
				for(const auto& el:shaderInfo.alias) {
					if(el.second.substr(2) == name) {
						glname = el.second;
						break;
					}
				}
			}
			if(glname.empty()) {
				LOG_ERROR("ShaderValue: " +name+ " not found in glsl");
			} else {
				material.customShader.constValues[glname] = {glname, value}; 
			}
		}

		// mesh
		auto spMesh = std::make_shared<SceneMesh>();
		auto& mesh = *spMesh;
		bool pow2Split = false;

		if(material.customShader.constValues.count("g_Texture0Resolution") != 0) {
			const auto& resolution = material.customShader.constValues.at("g_Texture0Resolution").value;
			pow2Split = (int32_t)resolution[0] != (int32_t)resolution[2];
			pow2Split = pow2Split || (int32_t)resolution[1] != (int32_t)resolution[3];
		}

		SceneMesh::GenCardMesh(mesh, std::vector<int32_t>(wpimgobj.size.begin(), wpimgobj.size.end()), pow2Split);
		// disable img material blend, as it's the first effect node now
		if(hasEffect)
			material.blenmode = BlendMode::Disable;
		mesh.AddMaterial(std::move(material));
		spNode->AddMesh(spMesh);

		shaderValueUpdater->SetNodeData(spNode.get(), svData);
		if(hasEffect) {
			// currently use addr for unique
			std::string nodeAddr = std::to_string(reinterpret_cast<intptr_t>(spNode.get()));
			// set camera to attatch effect
			if(isCompose) {
				upScene->cameras[nodeAddr] = std::make_shared<SceneCamera>(
					(int32_t)upScene->activeCamera->Width(), 
					(int32_t)upScene->activeCamera->Height(),
					-1.0f, 1.0f
				);
				upScene->cameras.at(nodeAddr)->AttatchNode(upScene->activeCamera->GetAttachedNode());
			}
			else {
				// applly scale to crop
				int32_t w = wpimgobj.size[0] * wpimgobj.scale[0];
				int32_t h = wpimgobj.size[1] * wpimgobj.scale[1];
				upScene->cameras[nodeAddr] = std::make_shared<SceneCamera>(w, h, -1.0f, 1.0f);
				upScene->cameras.at(nodeAddr)->AttatchNode(spNode);
			}
			spNode->SetCamera(nodeAddr);
			// set image effect
			auto imgEffectLayer = std::make_shared<SceneImageEffectLayer>(spNode.get(), wpimgobj.size[0], wpimgobj.size[1]);
			upScene->cameras.at(nodeAddr)->AttatchImgEffect(imgEffectLayer);
			// set renderTarget for ping-pong operate
			std::string effectRTs[2];
			effectRTs[0] = "_rt_" + nodeAddr;
			effectRTs[1] = "_rt_" + nodeAddr + "1";
			upScene->renderTargets[effectRTs[0]] = {
				.width = (uint32_t)wpimgobj.size[0], 
				.height = (uint32_t)wpimgobj.size[1], 
				.allowReuse = true
			};
			upScene->renderTargets[effectRTs[1]] = upScene->renderTargets.at(effectRTs[0]);
			if(wpimgobj.fullscreen) {
				upScene->renderTargetBindMap[effectRTs[0]] = {
					.name = "_rt_default"
				};
				upScene->renderTargetBindMap[effectRTs[1]] = {
					.name = "_rt_default"
				};
			}
			imgEffectLayer->SetFirstTarget(effectRTs[0]);
			int32_t i_eff = -1;
			for(const auto& wpeffobj:wpimgobj.effects) {
				i_eff++;
				if(!wpeffobj.visible) {
					i_eff--;
					continue;
				}
				std::shared_ptr<SceneImageEffect> imgEffect = std::make_shared<SceneImageEffect>();
				imgEffectLayer->AddEffect(imgEffect);

				const std::string& inRT = effectRTs[(i_eff)%2];
				std::string outRT;
				if(i_eff + 1 == count_eff) {
					outRT = "_rt_default";
				} else {
					outRT = effectRTs[(i_eff+1)%2];
				}

				// fbo name map
				std::string effaddr = std::to_string(reinterpret_cast<intptr_t>(imgEffectLayer.get()));
				std::unordered_map<std::string, std::string> fboMap;
				fboMap["previous"] = inRT;
				for(int32_t i=0;i < wpeffobj.fbos.size();i++) {
					const auto& wpfbo = wpeffobj.fbos.at(i);
					std::string rtname = "_rt_" + effaddr + std::to_string(i);
					if(wpimgobj.fullscreen) {
						upScene->renderTargetBindMap[rtname] = {
							.name = "_rt_default",
							.copy = false,
							.scale = 1.0f/wpfbo.scale
						};
						upScene->renderTargets[rtname] = {2, 2, true};
					} else {
						// i+2 for not override object's rt
						upScene->renderTargets[rtname] = {
							.width = (uint32_t)(wpimgobj.size[0]/wpfbo.scale),
							.height = (uint32_t)(wpimgobj.size[1]/wpfbo.scale), 
							.allowReuse = true
						};
					}
					fboMap[wpfbo.name] = rtname;
				}
				for(int32_t i_mat=0;i_mat < wpeffobj.materials.size();i_mat++) {
					wpscene::WPMaterial wpmat = wpeffobj.materials.at(i_mat);
					std::string matOutRT = outRT;
					if(wpeffobj.passes.size() > i_mat) {
						const auto& wppass = wpeffobj.passes.at(i_mat);
						wpmat.MergePass(wppass);
						// Set rendertarget, in and out
						for(const auto& el:wppass.bind) {
							if(fboMap.count(el.name) == 0) {
								LOG_ERROR("fbo " +el.name+ " not found");
								continue;
							}
							if(wpmat.textures.size() <= el.index)
								wpmat.textures.resize(el.index+1);
							wpmat.textures[el.index] = fboMap[el.name];
						}
						if(!wppass.target.empty()) {
							if(fboMap.count(wppass.target) == 0) {
								LOG_ERROR("fbo " +wppass.target+ " not found");
							}
							else {
								matOutRT = fboMap.at(wppass.target);
							}
						}
					}
					if(wpmat.textures.size() == 0)
						wpmat.textures.resize(1);
					if(wpmat.textures.at(0).empty()) {
						wpmat.textures[0] = inRT;
					}
					auto spEffNode = std::make_shared<SceneNode>();
					std::string effmataddr = std::to_string(reinterpret_cast<intptr_t>(spEffNode.get()));
					WPShaderInfo wpShaderInfo;
					shaderInfo.baseConstSvs = baseConstSvs;
					shaderInfo.baseConstSvs["g_EffectTextureProjectionMatrix"] = {
						"g_EffectTextureProjectionMatrix", 
						ShaderValue::ValueOf(glm::mat4(1.0f))
					};
					shaderInfo.baseConstSvs["g_EffectTextureProjectionMatrixInverse"] = {
						"g_EffectTextureProjectionMatrixInverse", 
						ShaderValue::ValueOf(glm::mat4(1.0f))
					};
					SceneMaterial material;
					WPShaderValueData svData;
					LoadMaterial(wpmat, upScene.get(), spEffNode.get(), &material, &svData, &wpShaderInfo);
					// load glname from alias and load to constvalue
					for(const auto& cs:wpmat.constantshadervalues) {
						const auto& name = cs.first;
						const std::vector<float>& value = cs.second;
						std::string glname;
						if(wpShaderInfo.alias.count(name) != 0) {
							glname = wpShaderInfo.alias.at(name);
						} else {
							for(const auto& el:wpShaderInfo.alias) {
								if(el.second.substr(2) == name) {
									glname = el.second;
									break;
								}
							}
						}
						if(glname.empty()) {
							LOG_ERROR("ShaderValue: " +name+ " not found in glsl");
						} else {
							material.customShader.constValues[glname] = {glname, value}; 
						}
					}
					for(int32_t i=0;i<material.textures.size();i++) {
						auto& t = material.textures.at(i);
						if(t == matOutRT) {
							t = "_rt_" + effmataddr + std::to_string(i);
							upScene->renderTargetBindMap[t] = {
								.name = matOutRT,
								.copy = true
							};
							upScene->renderTargets[t] = upScene->renderTargets.at(matOutRT);
							for(auto& el:svData.renderTargetResolution) {
								if(el.first == i)
									el = {i,t};
							}
						}
					}
					auto spMesh = std::make_shared<SceneMesh>();
					auto& mesh = *spMesh;
					// the last effect and last material
					if((i_eff + 1 == count_eff && i_mat + 1 == wpeffobj.materials.size()) && !wpimgobj.fullscreen) {
						SceneMesh::GenCardMesh(mesh, {(int32_t)wpimgobj.size[0], (int32_t)wpimgobj.size[1]}, false);
						spEffNode->CopyTrans(*spNode);
						svData.parallaxDepth = wpimgobj.parallaxDepth;
					} else {
						SceneMesh::GenCardMesh(mesh, {2, 2}, false);
						// disable blend for effect node, as it seems blend manually
						material.blenmode = BlendMode::Disable;
						spEffNode->SetCamera("effect");
					}
					mesh.AddMaterial(std::move(material));
					spEffNode->AddMesh(spMesh);

					shaderValueUpdater->SetNodeData(spEffNode.get(), svData);
					imgEffect->nodes.push_back({matOutRT, spEffNode});
				}
			}
		}
		upScene->sceneGraph->AppendChild(spNode);
	}
	upScene->shaderValueUpdater = std::move(shaderValueUpdater);
	return upScene;	
}
