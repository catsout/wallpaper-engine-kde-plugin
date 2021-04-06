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

const std::string pre_shader_code = "#version 130\n"
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
									  //"#define VERSION\n"
									  "#define max(x, y) max(y, x)\n"
									  "#define ddy(x) dFdy(-(x))\n\n";

void LoadShaderWithInclude2(std::string& source) 
{
	LineStr line;
	std::string::size_type pos = 0;
	std::string find_str = "#include"; 
    while(line = GetLineWith(source, find_str, pos),line.pos != std::string::npos)
    { 
		std::string new_src;
		std::stringstream line_stream(line.value);
		line_stream >> new_src; // drop #include
		line_stream >> new_src;
		new_src = new_src.substr(1, new_src.size()-2);  // drop "

		new_src = fs::GetContent(WallpaperGL::GetPkgfs(),"shaders/"+new_src);
		if(new_src.empty()) return;

		LoadShaderWithInclude2(new_src);

		DeleteLine(source, line);
		source.insert(line.pos, new_src);
		pos = line.pos + new_src.size();
    }
}

std::string PreShaderSrc(const std::string& src, int32_t texcount, WPShaderInfo* pWPShaderInfo) {
	std::string new_src = "";
	std::string include;
    std::string line;
    std::istringstream content(src);
	size_t last_var_pos = 0;

	auto& combos = pWPShaderInfo->combos;
	auto& wpAliasDict = pWPShaderInfo->alias;
	auto& shadervalues = pWPShaderInfo->svs;
	auto& defTexs = pWPShaderInfo->defTexs;

	
    while(std::getline(content,line)) {
		bool update_pos = false;
		if(line.find("#include") != std::string::npos) {
			include.append(line + "\n");
			line = "";
		}
        else if(line.find("attribute ") != std::string::npos || line.find("varying ") != std::string::npos) {
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
					//line = "";
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
        new_src += line + '\n';
		if(update_pos)
			last_var_pos = new_src.size();
		if(line.find("void main()") != std::string::npos) {
			new_src += src.substr(content.tellg());
			break;
		}
	}
	if(new_src.substr(last_var_pos, 6) == "#endif")
		last_var_pos += 7;
	LoadShaderWithInclude2(include);
    new_src.insert(last_var_pos, include);
	return new_src;
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
		if(name.compare(0, 4,"_rt_") == 0) {
			if(pScene->renderTargets.count(name) == 0) {
				LOG_INFO(name);
			} else {
				const auto& rt = pScene->renderTargets.at(name);
				svData.resolutions.push_back({
					i,
					(float)rt.width, 
					(float)rt.height, 
					(float)rt.width, 
					(float)rt.height
				});
			}
		} else {
			auto texh = pScene->imageParser->ParseHeader(name);
			if(texh->type == ImageType::UNKNOWN)
				svData.resolutions.push_back({
					i,
					(float)texh->width, 
					(float)texh->height, 
					(float)texh->mapWidth, 
					(float)texh->mapHeight
				});
			else
				svData.resolutions.push_back({
					i,
					(float)texh->mapWidth, 
					(float)texh->mapHeight, 
					(float)texh->mapWidth, 
					(float)texh->mapHeight
				});
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
	}
	if(wpmat.blending == "translucent" || wpmat.blending == "normal") {
		material.blenmode = BlendMode::Normal;
	} else if(wpmat.blending == "additive") {
		material.blenmode = BlendMode::Additive;
	} else {
		LOG_ERROR("unknown blending "+wpmat.blending);
	}

	materialShader.constValues = pWPShaderInfo->baseConstSvs;
	material.customShader = materialShader;
}

std::unique_ptr<Scene> WPSceneParser::Parse(const std::string& buf) {
	nlohmann::json json;
	if(!PARSE_JSON(buf, json)) 
		return nullptr;
	wpscene::WPScene sc;
	sc.FromJson(json);
	LOG_INFO(nlohmann::json(sc).dump(4));

	auto upScene = std::make_unique<Scene>();
	upScene->sceneGraph = std::make_shared<SceneNode>();
	upScene->imageParser = std::make_unique<WPTexImageParser>();
	auto shaderValueUpdater = std::make_unique<WPShaderValueUpdater>(upScene.get());

	upScene->clearColor = sc.general.clearcolor;
	
	WPCameraParallax cameraParallax;
	cameraParallax.enable = sc.general.cameraparallax;
	cameraParallax.amount = sc.general.cameraparallaxamount;
	cameraParallax.delay = sc.general.cameraparallaxdelay;
	cameraParallax.mouseinfluence = sc.general.cameraparallaxmouseinfluence;
	shaderValueUpdater->SetCameraParallax(cameraParallax);

	const auto& ortho = sc.general.orthogonalprojection; 

	// effect camera 
	upScene->cameras["effect"] = std::make_shared<SceneCamera>(2, 2, -1.0f, 1.0f);
	auto spEffCamNode = std::make_shared<SceneNode>(); // at 0,0,0
	upScene->cameras.at("effect")->AttatchNode(spEffCamNode.get());
	upScene->sceneGraph->AppendChild(std::move(spEffCamNode));

	// global camera
	upScene->cameras["global"] = std::make_shared<SceneCamera>(ortho.width, ortho.height, -1.0f, 1.0f);
	upScene->activeCamera = upScene->cameras.at("global").get();
    auto& objects = json.at("objects");
	std::vector<float> cori{ortho.width/2.0f,ortho.height/2.0f,0},cscale{1.0f,1.0f,1.0f},cangle(3);
	auto spCamNode = std::make_shared<SceneNode>(cori, cscale, cangle);
	upScene->activeCamera->AttatchNode(spCamNode.get());
	upScene->sceneGraph->AppendChild(std::move(spCamNode));

    for(auto& obj:objects) {
		if(!obj.contains("image")) continue;
		if(obj.at("image").is_null()) continue;
		wpscene::WPImageObject wpimgobj;
		wpimgobj.FromJson(obj);
		LOG_INFO(nlohmann::json(wpimgobj).dump(4));

		if(!wpimgobj.visible)
			continue;

		int32_t count_eff = 0;
		for(const auto& wpeffobj:wpimgobj.effects) {
				if(wpeffobj.visible)
					count_eff++;
		}
		bool hasEffect = count_eff > 0;
		// skip no effect fullscreen layer
		if(!hasEffect && wpimgobj.fullscreen)
			continue;

		bool isCompose = wpimgobj.name == "Compose";
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

		// mesh
		auto mesh = SceneMesh();
		bool pow2Split = false;

		std::string shadername = wpimgobj.material.shader;
		if(shadername.compare(0, 12, "genericimage") == 0) {
			if(svData.resolutions.size() == 1) {
				const auto& rel = svData.resolutions[0];
				pow2Split = (int32_t)rel.width != (int32_t)rel.mapWidth;
				pow2Split = pow2Split || (int32_t)rel.height != (int32_t)rel.mapHeight;
			}
		}

		SceneMesh::GenCardMesh(mesh, std::vector<int32_t>(wpimgobj.size.begin(), wpimgobj.size.end()), pow2Split);
		// disable img material blend, as it's the first effect node now
		if(hasEffect)
			material.blenmode = BlendMode::Disable;
		mesh.AddMaterial(std::move(material));
		spNode->AddMesh(std::move(mesh));

		shaderValueUpdater->SetNodeData(spNode.get(), svData);
		if(hasEffect) {
			// currently use addr for unique
			std::string nodeAddr = std::to_string(reinterpret_cast<intptr_t>(spNode.get()));
			// set camera to attatch effect
			if(isCompose) {
				upScene->cameras[nodeAddr] = std::make_shared<SceneCamera>(ortho.width, ortho.height, -1.0f, 1.0f);
				upScene->cameras.at(nodeAddr)->AttatchNode(upScene->activeCamera->GetAttachedNode());
			}
			else {
				// applly scale to crop
				int32_t w = wpimgobj.size[0] * wpimgobj.scale[0];
				int32_t h = wpimgobj.size[1] * wpimgobj.scale[1];
				upScene->cameras[nodeAddr] = std::make_shared<SceneCamera>(w, h, -1.0f, 1.0f);
				upScene->cameras.at(nodeAddr)->AttatchNode(spNode.get());
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
				(uint32_t)wpimgobj.size[0], 
				(uint32_t)wpimgobj.size[1], 
				0, 
				true
			};
			upScene->renderTargets[effectRTs[1]] = {
				(uint32_t)wpimgobj.size[0],
				(uint32_t)wpimgobj.size[1],
				1, 
				true
			};
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
					// i+2 for not override object's rt
					upScene->renderTargets[rtname] = {
						(uint32_t)(wpimgobj.size[0]/wpfbo.scale), 
						(uint32_t)(wpimgobj.size[1]/wpfbo.scale), 
						i+2, 
						true
					};
					fboMap[wpfbo.name] = rtname;
				}
				for(int32_t i_mat=0;i_mat < wpeffobj.materials.size();i_mat++) {
					wpscene::WPMaterial wpmat = wpeffobj.materials.at(i_mat);
					std::string matOutRT = outRT;
					if(wpeffobj.passes.size() > i_mat) {
						const auto& wppass = wpeffobj.passes.at(i_mat);
						wpmat.MergePass(wppass);
						if(wpmat.textures.size() == 0)
							wpmat.textures.resize(1);

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
						if(wppass.bind.size() == 0) {
							wpmat.textures[0] = inRT;
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
					auto spEffNode = std::make_shared<SceneNode>();
					WPShaderInfo wpShaderInfo;
					shaderInfo.baseConstSvs = baseConstSvs;
					SceneMaterial material;
					WPShaderValueData svData;
					// fix 2304304373, disable it for now
//					if(wpmat.combos.count("COMPOSITE") == 0)
//						wpmat.combos["COMPOSITE"] = 1; 
					LoadMaterial(wpmat, upScene.get(), spEffNode.get(), &material, &svData, &wpShaderInfo);
					// load glname from alias and load to constvalue
					for(const auto& cs:wpmat.constantshadervalues) {
						const auto& name = cs.first;
						const std::vector<float>& value = cs.second;
						if(wpShaderInfo.alias.count(name) == 0) {
							LOG_ERROR("ShaderValue: " +name+ " not found in glsl");
						} else {
							const auto& glname = wpShaderInfo.alias.at(name);
							material.customShader.constValues[glname] = {glname, value}; 
						}
					}
					auto mesh = SceneMesh();
					// the last effect and last material
					if(i_eff + 1 == count_eff && i_mat + 1 == wpeffobj.materials.size()) {
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
					spEffNode->AddMesh(std::move(mesh));

					shaderValueUpdater->SetNodeData(spEffNode.get(), svData);
					imgEffect->nodes.push_back({matOutRT, spEffNode});
				}
			}
		}
		upScene->sceneGraph->AppendChild(std::move(spNode));
	}
	upScene->shaderValueUpdater = std::move(shaderValueUpdater);
	return upScene;	
}
