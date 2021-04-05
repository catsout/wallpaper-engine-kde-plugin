#include "WPSceneParser.h"
#include "WPJson.h"
#include "pkg.h"
#include "common.h"
#include "wallpaper.h"
#include "WPTexImageParser.h"
#include "Type.h"

#include "WPShaderValueUpdater.h"
#include "wpscene/WPImageObject.h"

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
			auto combo_json = nlohmann::json::parse(line.substr(line.find_first_of('{')));
			if(combo_json.contains("combo")) {
				std::string name;
				int32_t value = 0;
				GET_JSON_NAME_VALUE(combo_json, "combo", name);
				GET_JSON_NAME_VALUE(combo_json, "default", value);
				combos[name] = value;
				//line = "";
			}
		}
		else if(line.find("uniform ") != std::string::npos) {
			update_pos = true;
			if(line.find("// {") != std::string::npos) {
				auto sv_json = nlohmann::json::parse(line.substr(line.find_first_of('{')));
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

void ParseMaterial(nlohmann::json& json, Scene* pScene, SceneNode* pNode, SceneMaterial* pMaterial, WPShaderValueData* pSvData, WPShaderInfo* pWPShaderInfo=nullptr) {
	auto& svData = *pSvData;
	auto& material = *pMaterial;

	std::unique_ptr<WPShaderInfo> upWPShaderInfo(nullptr);
	if(pWPShaderInfo == nullptr) {
		upWPShaderInfo = std::make_unique<WPShaderInfo>();
		pWPShaderInfo = upWPShaderInfo.get();
	}


	auto& mat_j = json;
	auto& content = mat_j.at("passes")[0];
	std::string shadername;
	GET_JSON_NAME_VALUE(content, "shader", shadername);
//	if(shadername != "genericimage2") return;

	if(content.contains("combos")) {
		auto& jC = content.at("combos");
		for(auto& je:jC.items()) {
			std::string name;
			int32_t value;
			GET_JSON_VALUE(je.value(), value);
			GET_JSON_VALUE(je.key(), name);
			if(pWPShaderInfo->combos.count(name) == 0)
				pWPShaderInfo->combos[name] = value; 
		}
	}
	SceneMaterialCustomShader materialShader;
	if(content.contains("constantshadervalues")) {
		LOG_ERROR("constantshadervalues in material");
	}
	auto& shader = materialShader.shader;
	shader = std::make_shared<SceneShader>();

	std::string svCode = fs::GetContent(WallpaperGL::GetPkgfs(),"shaders/"+shadername+".vert");
	std::string fgCode = fs::GetContent(WallpaperGL::GetPkgfs(),"shaders/"+shadername+".frag");
	int32_t texcount = content.contains("textures")?content.at("textures").size():0;
	svCode = PreShaderSrc(svCode, texcount, pWPShaderInfo);
	fgCode = PreShaderSrc(fgCode, texcount, pWPShaderInfo);
	shader->uniforms = pWPShaderInfo->svs;

	svCode = PreShaderHeader(svCode, pWPShaderInfo->combos);
	fgCode = PreShaderHeader(fgCode, pWPShaderInfo->combos);

	shader->vertexCode = svCode;
	shader->fragmentCode = fgCode;
	shader->attrs.push_back({"a_Position", 0});
	shader->attrs.push_back({"a_TexCoord", 1});

	if(pWPShaderInfo->defTexs.size() > 0) {
		if(!content.contains("textures"))
			content["textures"] = nlohmann::json::array();
		for(auto& t:pWPShaderInfo->defTexs) {
			auto& textures = content.at("textures");
			if(textures.size() > t.first) {
				if(!textures.at(t.first).is_null())
					continue;
			}
			textures[t.first] = nlohmann::json(t.second);
		}
	}
	if(content.contains("textures")) {
//		svData.resolutions.resize(content.at("textures").size());
		WPTexImageParser texParser;
		int32_t i = -1;
		for(auto& t:content.at("textures")) {
			i++;
			std::string name = t.is_null()?"":t;
			if(name == "_rt_FullFrameBuffer")
				name = "_rt_default";

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
	}
	std::string blendmode;
	GET_JSON_NAME_VALUE(content, "blending", blendmode);
	if(blendmode == "translucent" || blendmode == "normal") {
		material.blenmode = BlendMode::Normal;
	} else if(blendmode == "additive") {
		material.blenmode = BlendMode::Additive;
	} else {
		LOG_ERROR("unknown blending "+blendmode);
	}

	materialShader.constValues = pWPShaderInfo->baseConstSvs;
	material.customShader = materialShader;
}

std::unique_ptr<Scene> WPSceneParser::Parse(const std::string& buf) {
	auto json = nlohmann::json::parse(buf);
	auto& general_j = json.at("general");

	auto upScene = std::make_unique<Scene>();
	upScene->sceneGraph = std::make_shared<SceneNode>();
	upScene->imageParser = std::make_unique<WPTexImageParser>();
	auto shaderValueUpdater = std::make_unique<WPShaderValueUpdater>(upScene.get());

	GET_JSON_NAME_VALUE(general_j, "clearcolor", upScene->clearColor);
	
	WPCameraParallax cameraParallax;
	GET_JSON_NAME_VALUE_NOWARN(general_j, "cameraparallax", cameraParallax.enable);
	if(cameraParallax.enable) {
		GET_JSON_NAME_VALUE(general_j, "cameraparallaxamount", cameraParallax.amount);
		GET_JSON_NAME_VALUE(general_j, "cameraparallaxdelay", cameraParallax.delay);
		GET_JSON_NAME_VALUE(general_j, "cameraparallaxmouseinfluence", cameraParallax.mouseinfluence);
	}
	shaderValueUpdater->SetCameraParallax(cameraParallax);

	auto& ortho_j = general_j.at("orthogonalprojection");
	std::vector<uint32_t> ortho(2);
	GET_JSON_NAME_VALUE(ortho_j, "width", ortho.at(0));
	GET_JSON_NAME_VALUE(ortho_j, "height", ortho.at(1));
	// effect camera 
	upScene->cameras["effect"] = std::make_shared<SceneCamera>(2, 2, -1.0f, 1.0f);
	auto spEffCamNode = std::make_shared<SceneNode>(); // at 0,0,0
	upScene->cameras.at("effect")->AttatchNode(spEffCamNode.get());
	upScene->sceneGraph->AppendChild(std::move(spEffCamNode));
	// global camera
	upScene->cameras["global"] = std::make_shared<SceneCamera>((int32_t)ortho[0], (int32_t)ortho[1], -1.0f, 1.0f);
	upScene->activeCamera = upScene->cameras.at("global").get();
	std::vector<float> ori,scale,angle;
    auto& objects = json.at("objects");
	std::vector<float> cori{ortho[0]/2.0f,ortho[1]/2.0f,0},cscale{1.0f,1.0f,1.0f},cangle(3);
	auto spCamNode = std::make_shared<SceneNode>(cori, cscale, cangle);
	upScene->activeCamera->AttatchNode(spCamNode.get());
	upScene->sceneGraph->AppendChild(std::move(spCamNode));

    for(auto& obj:objects) {
		if(!obj.contains("image")) continue;
		if(obj.at("image").is_null()) continue;
		wpscene::WPImageObject wpimgobj;
		wpimgobj.FromJson(obj);
		LOG_INFO(nlohmann::json(wpimgobj).dump(4));
		bool visible {true};
		GET_JSON_NAME_VALUE_NOWARN(obj, "visible", visible);
		if(!visible)
			continue;

		auto image = nlohmann::json::parse(fs::GetContent(WallpaperGL::GetPkgfs(), obj.at("image")));
		bool hasEffect = obj.contains("effects");
		bool fullscreen {false};
		GET_JSON_NAME_VALUE_NOWARN(image, "fullscreen", fullscreen);
		// skip no used fullscreen layer
		if(!hasEffect && fullscreen)
			continue;

		std::string name;
		GET_JSON_NAME_VALUE_NOWARN(obj, "name", name);
		bool isCompose = name == "Compose";
		if(obj.contains("name"))
			std::cerr << obj.at("name") << std::endl;
		std::vector<float> ori(3),scale({1.0f,1.0f,1.0f}),angle(3);
		std::vector<float> size({2, 2}),pDepth(2),color({1,1,1});
		if(!fullscreen) {
			GET_JSON_NAME_VALUE(obj, "origin", ori);	
			GET_JSON_NAME_VALUE(obj, "angles", angle);	
			GET_JSON_NAME_VALUE(obj, "scale", scale);	
			if(image.contains("width")) {
				int32_t w,h;
				GET_JSON_NAME_VALUE(image, "width", w);	
				GET_JSON_NAME_VALUE(image, "height", h);	
				size = {(float)w, (float)h};
			} else
				GET_JSON_NAME_VALUE(obj, "size", size);	
			GET_JSON_NAME_VALUE_NOWARN(obj, "parallaxDepth", pDepth);
		}
		GET_JSON_NAME_VALUE_NOWARN(obj, "color", color);
		float alpha = 1.0f, brightness = 1.0f;
		GET_JSON_NAME_VALUE_NOWARN(obj, "alpha", alpha);
		GET_JSON_NAME_VALUE_NOWARN(obj, "brightness", brightness);
		
		ori[1] = ortho[1] - ori[1];
		auto spNode = std::make_shared<SceneNode>(ori, scale, angle);

		bool autosize = false;
		GET_JSON_NAME_VALUE_NOWARN(image, "autosize", autosize);	


		SceneMaterial material;
		WPShaderValueData svData;
		if(!hasEffect)
			svData.parallaxDepth = pDepth;
		ShaderValues baseConstSvs;
		baseConstSvs["g_Alpha"] = {"g_Alpha", {alpha}};
		baseConstSvs["g_Color"] = {"g_Color", color};
		baseConstSvs["g_UserAlpha"] = {"g_UserAlpha", {alpha}};
		baseConstSvs["g_Brightness"] = {"g_Brightness", {brightness}};

		WPShaderInfo shaderInfo;
		shaderInfo.baseConstSvs = baseConstSvs;

		std::string material_str = fs::GetContent(WallpaperGL::GetPkgfs(), image.at("material"));
		auto mat_j = nlohmann::json::parse(material_str);
		ParseMaterial(mat_j, upScene.get(), spNode.get(), &material, &svData, &shaderInfo);

		// mesh
		auto mesh = SceneMesh();
		bool pow2Split = false;

		std::string shadername;
		GET_JSON_NAME_VALUE(mat_j.at("passes")[0], "shader", shadername);
		if(shadername.compare(0, 12, "genericimage") == 0) {
			if(svData.resolutions.size() == 1) {
				const auto& rel = svData.resolutions[0];
				pow2Split = (int32_t)rel.width != (int32_t)rel.mapWidth;
				pow2Split = pow2Split || (int32_t)rel.height != (int32_t)rel.mapHeight;
			}
		}
		SceneMesh::GenCardMesh(mesh, std::vector<int>(size.begin(), size.end()), pow2Split);
		material.blenmode = BlendMode::Normal;
		// disable img material blend, as it's the first effect node now
		if(hasEffect)
			material.blenmode = BlendMode::Disable;
		mesh.AddMaterial(std::move(material));
		spNode->AddMesh(std::move(mesh));

		shaderValueUpdater->SetNodeData(spNode.get(), svData);

		if(obj.contains("effects")) {
			// currently use addr for unique
			std::string nodeAddr = std::to_string(reinterpret_cast<intptr_t>(spNode.get()));
			// set camera to attatch effect
			if(isCompose) {
				upScene->cameras[nodeAddr] = std::make_shared<SceneCamera>((int32_t)ortho[0], (int32_t)ortho[1], -1.0f, 1.0f);
				upScene->cameras.at(nodeAddr)->AttatchNode(upScene->activeCamera->GetAttachedNode());
			}
			else {
				// applly scale to crop
				upScene->cameras[nodeAddr] = std::make_shared<SceneCamera>((int32_t)(size[0]*scale[0]), (int32_t)(size[1]*scale[1]), -1.0f, 1.0f);
				upScene->cameras.at(nodeAddr)->AttatchNode(spNode.get());
			}
			spNode->SetCamera(nodeAddr);
			// set image effect
			auto imgEffectLayer = std::make_shared<SceneImageEffectLayer>(spNode.get(), size[0], size[1]);
			upScene->cameras.at(nodeAddr)->AttatchImgEffect(imgEffectLayer);
			// set renderTarget for ping-pong operate
			std::string effectRTs[2];
			effectRTs[0] = "_rt_" + nodeAddr;
			effectRTs[1] = "_rt_" + nodeAddr + "1";
			upScene->renderTargets[effectRTs[0]] = {(uint32_t)size[0], (uint32_t)size[1], 0, true};
			upScene->renderTargets[effectRTs[1]] = {(uint32_t)size[0], (uint32_t)size[1], 1, true};

			imgEffectLayer->SetFirstTarget(effectRTs[0]);

			auto& jEffs = obj.at("effects");
			int32_t i_eff = -1;
			int32_t count_eff = jEffs.size();
			for(auto& jEff:jEffs) {
				i_eff++;

				auto imgEffect = std::make_shared<SceneImageEffect>();
				imgEffectLayer->AddEffect(imgEffect);
				const std::string& inRT = effectRTs[i_eff%2];
				std::string outRT;
				if(i_eff + 1 == count_eff) {
					outRT = "_rt_default";
				} else {
					outRT = effectRTs[(i_eff+1)%2];
				}

				std::string sEContent = fs::GetContent(WallpaperGL::GetPkgfs(), jEff.at("file"));
				auto jEContent = nlohmann::json::parse(sEContent);
				nlohmann::json* pjEPass = nullptr;
				if(jEff.contains("passes") && jEff.at("passes").size() > 0) {
					pjEPass = &jEff.at("passes"); 
				}
				auto& jECPass = jEContent.at("passes");
				// fbo name map
				std::string effaddr = std::to_string(reinterpret_cast<intptr_t>(imgEffectLayer.get()));
				std::unordered_map<std::string, std::string> fboMap;
				if(jEContent.contains("fbos")) {
					int32_t i = -1;
					for(auto& f:jEContent.at("fbos")) {
						i++;
						int32_t scale;
						std::string name;
						GET_JSON_NAME_VALUE(f, "scale", scale);
						GET_JSON_NAME_VALUE(f, "name", name);
						std::string rtname = "_rt_" + effaddr + std::to_string(i);
						// i+2 for not override object's rt
						upScene->renderTargets[rtname] = {(uint32_t)(size[0]/scale), (uint32_t)(size[1]/scale), i+2, true};
						fboMap[name] = rtname;
					}
					fboMap["previous"] = inRT;
				}
				bool effCompose {false};
				std::string effComposeRT;
				int32_t i_node = -1;
				size_t count_node = jECPass.size();
				for(auto& jP:jECPass) {
					i_node++;

					auto spEffNode = std::make_shared<SceneNode>();
					std::string nodeOutRT = outRT;
					
					WPShaderInfo wpShaderInfo;
					// put passes in effect to effect content 
					auto jETexs = nlohmann::json::array();
					if(pjEPass != nullptr && pjEPass->size() >= i_node+1) {
						auto& jEP = pjEPass->at(i_node);
						if(jEP.contains("combos")) {
							auto& jCobs = jEP.at("combos");
							for(auto& jCob:jCobs.items()) {
								std::string name;
								int32_t value;
								GET_JSON_VALUE(jCob.value(), value);
								GET_JSON_VALUE(jCob.key(), name);
								wpShaderInfo.combos[name] = value; 
							}
						}
						if(jEP.contains("textures")) {
							jETexs = jEP.at("textures");
						}
					}
					// Set rendertarget, in and out
					if(jP.contains("bind")) { 
						for(auto& jBind:jP.at("bind")){
							int32_t index;
							std::string name;
							GET_JSON_NAME_VALUE(jBind, "index", index);
							GET_JSON_NAME_VALUE(jBind, "name", name);
							if(fboMap.count(name) == 0) {
								LOG_ERROR("fbo " +name+ " not found");
								continue;
							}
							jETexs[index] = nlohmann::json(fboMap[name]);
						}
					// set after in for compose
					} else if(effCompose) {
						jETexs[0] = nlohmann::json(effComposeRT);
					} else {
						jETexs[0] = nlohmann::json(inRT);
					}
					// compose, mean two effect node, gen after target
					if(jP.contains("compose")) {
						GET_JSON_NAME_VALUE(jP, "compose", effCompose);
						if(effCompose) {
							effComposeRT = "_rt_" + effaddr + std::to_string(0);
							upScene->renderTargets[effComposeRT] = {(uint32_t)size[0], (uint32_t)size[1], 2, true};
						} 
					}
					//LOG_INFO("bind: " + jETexs.dump());
					if(jP.contains("target")) {
						std::string target;
						GET_JSON_NAME_VALUE(jP, "target", target);
						if(fboMap.count(target) == 0)
							LOG_ERROR("fbo " +target+ " not found")
						else
							nodeOutRT = fboMap.at(target);
					} else if(effCompose) {
						nodeOutRT = effComposeRT;
					}
					
					//LOG_INFO("target: " + nodeOutRT);

					SceneMaterial material;
					WPShaderValueData svData;
					if(!jP.contains("material")) {
						LOG_ERROR(jP.dump());
						continue;
					}
					std::string material_str = fs::GetContent(WallpaperGL::GetPkgfs(), jP.at("material"));
					auto jMat = nlohmann::json::parse(material_str);
					
					if(jETexs.size() > 0) {
						auto& jMC = jMat.at("passes")[0];
						if(!jMC.contains("textures"))
							jMC["textures"] = nlohmann::json::array();
						auto& jMTexs = jMC.at("textures");
						int32_t i = -1;
						for(auto& jT:jETexs) {
							i++;
							// skip if jMtexs own and jT is null
							if(jMTexs.size() > i && jT.is_null())
								continue;
							jMTexs[i] = jT;
						}
					}
					ParseMaterial(jMat, upScene.get(), spEffNode.get(), &material, &svData, &wpShaderInfo);
					if(pjEPass != nullptr && pjEPass->size() >= i_node+1) {
						if(pjEPass->at(i_node).contains("constantshadervalues")) {
							auto& jConstShaderValues = pjEPass->at(i_node).at("constantshadervalues");
							for(auto& jCsv:jConstShaderValues.items()) {
								std::string name;
								std::vector<float> value(1);
								nlohmann::json jvalue = jCsv.value();
								if(jvalue.is_object()) {
									if(jvalue.contains("value")) {
										jvalue = jvalue.at("value");
									} else {
										LOG_ERROR(jvalue.dump());
									}
								}
								GET_JSON_VALUE(jCsv.key(), name);
								if(jvalue.is_number())
									GET_JSON_VALUE(jvalue, value[0]);
								else
									GET_JSON_VALUE(jvalue, value);
								if(wpShaderInfo.alias.count(name) == 0) {
									LOG_ERROR("ShaderValue: " +name+ " not found in glsl");
								} else {
									const auto& glname = wpShaderInfo.alias.at(name);
									material.customShader.constValues[glname] = {glname, value}; 
								}
							}
						}
					}

					auto mesh = SceneMesh();
					if(i_eff + 1 == count_eff && i_node + 1 == count_node) {
						SceneMesh::GenCardMesh(mesh, {(int32_t)size[0], (int32_t)size[1]}, false);
						spEffNode->CopyTrans(*spNode);
						svData.parallaxDepth = pDepth;
					} else {
						SceneMesh::GenCardMesh(mesh, {2, 2}, false);
						// disable blend for effect node, as it seems blend manually
						material.blenmode = BlendMode::Disable;
						spEffNode->SetCamera("effect");
					}
					mesh.AddMaterial(std::move(material));
					spEffNode->AddMesh(std::move(mesh));

					shaderValueUpdater->SetNodeData(spEffNode.get(), svData);
					imgEffect->nodes.push_back({nodeOutRT, spEffNode});
				}
			}
		}
		upScene->sceneGraph->AppendChild(std::move(spNode));
	}
	upScene->shaderValueUpdater = std::move(shaderValueUpdater);
	return upScene;	
}
