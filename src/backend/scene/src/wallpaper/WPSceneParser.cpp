#include "WPSceneParser.h"
#include "WPJson.h"
#include "pkg.h"
#include "Util.h"
#include "Log.h"
#include "wallpaper.h"
#include "Type.h"
#include "Algorism.h"
#include "SpecTexs.h"

#include "WPShaderParser.h"
#include "WPTexImageParser.h"
#include "Particle/WPParticleRawGener.h"
#include "WPParticleParser.h"


#include "WPShaderValueUpdater.h"
#include "wpscene/WPImageObject.h"
#include "wpscene/WPParticleObject.h"
#include "wpscene/WPScene.h"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <random>
#include <cmath>
#include <functional>
#include <Eigen/Dense>

using namespace wallpaper;
using namespace Eigen;


typedef std::function<float()> RandomFn;

std::string getAddr(void *p) {
	return std::to_string(reinterpret_cast<intptr_t>(p));
}
void GenCardMesh(SceneMesh& mesh, const std::vector<int> size, bool autosize=false) {
	float left = -(size[0]/2.0f);
	float right = size[0]/2.0f;
	float bottom = -(size[1]/2.0f);
	float top = size[1]/2.0f;
	float z = 0.0f;
	std::vector<float> pos = {
			left, bottom, z,
			right, bottom, z,
			right,  top, z,
			left,  top, z,
	};
	std::vector<float> texCoord;
	float tw = 1.0f,th = 1.0f;
	if(autosize) {
		uint32_t x = 1,y = 1;
		while(x < size[0]) x*=2;	
		while(y < size[1]) y*=2;	
		tw = size[0] / (float)x;
		th = size[1] / (float)y;
	}
	texCoord = {
			0.0f, 0.0f,
			tw, 0.0f,
			tw, th,
			0.0f, th,
	};
	std::vector<uint32_t> indices = { 
		0, 1, 3,
		1, 2, 3
	};
	SceneVertexArray vertex({
		{"a_Position", VertexType::FLOAT3},
		{"a_TexCoord", VertexType::FLOAT2},
	}, 4);
	vertex.SetVertex("a_Position", pos);
	vertex.SetVertex("a_TexCoord", texCoord);
	mesh.AddVertexArray(std::move(vertex));
	mesh.AddIndexArray(SceneIndexArray(indices));
}

void SetParticleMesh(SceneMesh& mesh, const wpscene::Particle& particle, uint32_t count, bool sprite=false) {
	std::vector<SceneVertexArray::SceneVertexAttribute> attrs {
		{"a_Position", VertexType::FLOAT3},
		{"a_TexCoordVec4", VertexType::FLOAT4},
		{"a_Color", VertexType::FLOAT4},
	};
	if(sprite) {
		attrs.push_back({"a_TexCoordVec4C1", VertexType::FLOAT4});
	}
	attrs.push_back({"a_TexCoordC2", VertexType::FLOAT2});
	mesh.AddVertexArray(SceneVertexArray(attrs, count*4));
	mesh.AddIndexArray(SceneIndexArray(count*2));
}

ParticleAnimationMode ToAnimMode(const std::string& str) {
	if(str == "randomframe")
		return ParticleAnimationMode::RANDOMONE;
	else if(str == "sequence")
		return ParticleAnimationMode::SEQUENCE;
	else {
		return ParticleAnimationMode::SEQUENCE;
	}
}

void LoadInitializer(ParticleSubSystem& pSys, const wpscene::Particle& wp, const wpscene::ParticleInstanceoverride& over, RandomFn& randomFn) {
	for(const auto& ini:wp.initializers) {
		pSys.AddInitializer(WPParticleParser::genParticleInitOp(ini, randomFn));
	}
	if(over.enabled)
		pSys.AddInitializer(WPParticleParser::genOverrideInitOp(over));
}
void LoadOperator(ParticleSubSystem& pSys, const wpscene::Particle& wp, RandomFn& randomFn) {
	for(const auto& op:wp.operators) {
		pSys.AddOperator(WPParticleParser::genParticleOperatorOp(op, randomFn));
	}
}
void LoadEmitter(ParticleSubSystem& pSys, const wpscene::Particle& wp, float count, RandomFn& randomFn) {
	for(const auto& em:wp.emitters) {
		auto newEm = em;
		newEm.rate *= count;
		//newEm.origin[2] -= perspectiveZ;
		pSys.AddEmitter(WPParticleParser::genParticleEmittOp(newEm, randomFn));
	}
}

BlendMode ParseBlendMode(std::string_view str) {
	BlendMode bm;
	if(str == "translucent") {
		bm = BlendMode::Translucent;
	} else if(str == "additive") {
		bm = BlendMode::Additive;
	} else if(str == "normal") {
		bm = BlendMode::Normal;
	} else if(str == "disabled") {
		bm = BlendMode::Disable;
	} else {
		LOG_ERROR("unknown blending: "+std::string(str));
	}
	return bm;
}

void ParseSpecTexName(std::string& name, const wpscene::WPMaterial& wpmat, const WPShaderInfo& sinfo) {
	if(IsSpecTex(name)) {
		if(name == "_rt_FullFrameBuffer") {
			name = SpecTex_Default;
			if(wpmat.shader == "genericimage2" && (sinfo.combos.count("BLENDMODE") && sinfo.combos.at("BLENDMODE") != 1))
				name = "";
			/*
			if(wpmat.shader == "genericparticle") {
				name = "_rt_ParticleRefract";
			}
			*/
		}
		if(name.compare(0, 6, "_rt_im") == 0) {
			name = "";
			LOG_ERROR("unsupported layer texture");
		}
		if(name == "_rt_MipMappedFrameBuffer") {
			name = "";
		}
	}
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
	std::string shaderPath("shaders/"+wpmat.shader);
	std::string svCode = fs::GetContent(WallpaperGL::GetPkgfs(),shaderPath+".vert");
	std::string fgCode = fs::GetContent(WallpaperGL::GetPkgfs(),shaderPath+".frag");
	int32_t texcount = wpmat.textures.size();
	svCode = WPShaderParser::PreShaderSrc(svCode, texcount, pWPShaderInfo);
	fgCode = WPShaderParser::PreShaderSrc(fgCode, texcount, pWPShaderInfo);
	shader->uniforms = pWPShaderInfo->svs;

	for(const auto& el:wpmat.combos) {
		pWPShaderInfo->combos[el.first] = el.second; 
	}

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
	for(int32_t i=0;i<textures.size();i++) {
		std::string name = textures.at(i);
		ParseSpecTexName(name, wpmat, *pWPShaderInfo);
		material.textures.push_back(name);
		material.defines.push_back("g_Texture" + std::to_string(i));
		if(name.empty()) {
			//LOG_ERROR("empty texture name");
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
			if(i == 0) {
				if(texh->format == TextureFormat::R8)
					fgCode = "#define TEX0FORMAT FORMAT_R8\n" + fgCode;
				else if (texh->format == TextureFormat::RG8)
					fgCode = "#define TEX0FORMAT FORMAT_RG88\n" + fgCode;
			}
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
					stex.spriteAnim = texh->spriteAnim;
				}
				pScene->textures[name] = std::make_shared<SceneTexture>(stex);
			}
			if((pScene->textures.at(name))->isSprite) {
				material.hasSprite = true;
				const auto& f1 = texh->spriteAnim.GetCurFrame();
				if(wpmat.shader == "genericparticle") {
					pWPShaderInfo->combos["SPRITESHEET"] = 1;
					pWPShaderInfo->combos["THICKFORMAT"] = 1;
					materialShader.constValues["g_RenderVar1"] = { "g_RenderVar1", {
						f1.width,
						f1.height,
						(float)(texh->spriteAnim.numFrames()),
						f1.rate
					}};
				}
			}
		}
		if(!resolution.empty()) {
			const std::string gResolution = "g_Texture" + std::to_string(i) + "Resolution";
			materialShader.constValues[gResolution] = {gResolution, resolution};
		}
	}

	svCode = WPShaderParser::PreShaderHeader(svCode, pWPShaderInfo->combos);
	fgCode = WPShaderParser::PreShaderHeader(fgCode, pWPShaderInfo->combos);

	shader->vertexCode = svCode;
	shader->fragmentCode = fgCode;
	shader->attrs.push_back({"a_Position", 0});
	shader->attrs.push_back({"a_TexCoord", 1});
	shader->attrs.push_back({"a_TexCoordVec4", 1});
	shader->attrs.push_back({"a_Color", 2});

	material.blenmode = ParseBlendMode(wpmat.blending);

	for(const auto& el:pWPShaderInfo->baseConstSvs) {
		materialShader.constValues[el.first] = el.second;
	}
	material.customShader = materialShader;
	material.name = wpmat.shader;
}

std::unique_ptr<Scene> WPSceneParser::Parse(const std::string& buf) {
	nlohmann::json json;
	if(!PARSE_JSON(buf, json)) 
		return nullptr;
	wpscene::WPScene sc;
	sc.FromJson(json);
//	LOG_INFO(nlohmann::json(sc).dump(4));

	auto ur = std::make_shared<std::uniform_real_distribution<float>>(0.0f, 1.0f);
	auto randomSeed = std::make_shared<std::default_random_engine>();
	RandomFn randomFn = [randomSeed, ur]() { return (*ur)(*randomSeed); };

	std::vector<wpscene::WPImageObject> wpimgobjs;
	std::vector<wpscene::WPParticleObject> wppartobjs;
	std::vector<std::pair<std::string, uint32_t>> indexTable;

    for(auto& obj:json.at("objects")) {
		if(obj.contains("image") && !obj.at("image").is_null()) {
			wpscene::WPImageObject wpimgobj;
			if(!wpimgobj.FromJson(obj)) continue;
			if(!wpimgobj.visible) continue;
			wpimgobjs.push_back(wpimgobj);
			indexTable.push_back({"image", wpimgobjs.size() - 1});
			//LOG_INFO(nlohmann::json(wpimgobj).dump(4));
		} else if(obj.contains("particle") && !obj.at("particle").is_null()) {
			//continue;
			wpscene::WPParticleObject wppartobj;
			if(!wppartobj.FromJson(obj)) continue;
			if(!wppartobj.visible) continue;
			wppartobjs.push_back(wppartobj);
			indexTable.push_back({"particle", wppartobjs.size() - 1});
			//LOG_INFO(nlohmann::json(wppartobj).dump(4));
		}
	}
	if(sc.general.orthogonalprojection.auto_) {
		uint32_t w = 0, h = 0;
		for(auto& obj:wpimgobjs) {
			uint32_t size = obj.size.at(0) * obj.size.at(1);
			if(size > w*h) {
				w = obj.size.at(0);
				h = obj.size.at(1);
			}
		}
		sc.general.orthogonalprojection.width = w;
		sc.general.orthogonalprojection.height = h;
	}


	auto upScene = std::make_unique<Scene>();
	upScene->sceneGraph = std::make_shared<SceneNode>();
	upScene->imageParser = std::make_unique<WPTexImageParser>();
	upScene->paritileSys.gener = std::make_unique<WPParticleRawGener>();
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
	upScene->ortho[0] = (uint32_t)ortho.width;
	upScene->ortho[1] = (uint32_t)ortho.height;
	{
		std::string def {SpecTex_Default};
		upScene->renderTargets[def] = {
			.width = (uint16_t)ortho.width, 
			.height = (uint16_t)ortho.height,
			.bind = {
				.enable = true,
				.screen = true
			}
		};
	}

	// effect camera 
	upScene->cameras["effect"] = std::make_shared<SceneCamera>(2, 2, -1.0f, 1.0f);
	auto spEffCamNode = std::make_shared<SceneNode>(); // at 0,0,0
	upScene->cameras.at("effect")->AttatchNode(spEffCamNode);
	upScene->sceneGraph->AppendChild(spEffCamNode);

	// global camera
	upScene->cameras["global"] = std::make_shared<SceneCamera>(
		int32_t(ortho.width / sc.general.zoom), 
		int32_t(ortho.height / sc.general.zoom), 
		-5000.0f, 5000.0f
	);

	upScene->activeCamera = upScene->cameras.at("global").get();

	ShaderValues globalBaseConstSvs;
	globalBaseConstSvs["g_ViewUp"] = {"g_ViewUp", {0.0f, 1.0f, 0}};
	globalBaseConstSvs["g_ViewRight"]= {"g_ViewRight", {1.0f, 0, 0}};
	globalBaseConstSvs["g_ViewForward"]= {"g_ViewForward", {0, 0, -1.0f}};
	globalBaseConstSvs["g_EyePosition"]= {"g_EyePosition", {0, 0, 0}};
	globalBaseConstSvs["g_TexelSize"]= {"g_TexelSize", {1.0f/1920.0f, 1.0f/1080.0f}};
	globalBaseConstSvs["g_TexelSizeHalf"]= {"g_TexelSizeHalf", {1.0f/1920.0f/2.0f, 1.0f/1080.0f/2.0f}};

	Vector3f cori{ortho.width/2.0f,ortho.height/2.0f,0},cscale{1.0f,1.0f,1.0f},cangle(Vector3f::Zero());
	auto spCamNode = std::make_shared<SceneNode>(cori, cscale, cangle);
	upScene->activeCamera->AttatchNode(spCamNode);
	upScene->sceneGraph->AppendChild(spCamNode);

	// perspective cam
	upScene->cameras["global_perspective"] = std::make_shared<SceneCamera>(
		ortho.width/(float)ortho.height,
		sc.general.nearz,
		sc.general.farz,
		algorism::CalculatePersperctiveFov(1000.0f, ortho.height)
	);
	{
		Vector3f cperori = cori; cperori[2] = 1000.0f;
		auto spPerCamNode = std::make_shared<SceneNode>(cperori, cscale, cangle);
		upScene->cameras["global_perspective"]->AttatchNode(spPerCamNode);
		upScene->sceneGraph->AppendChild(spPerCamNode);
	}

    for(const auto& indexT:indexTable) {
		if(indexT.first == "image") {	
			auto& wpimgobj = wpimgobjs.at(indexT.second);
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

			bool isCompose = (wpimgobj.image == "models/util/composelayer.json");
			// skip no effect compose layer
			// it's not the correct behaviour, but do it for now
			if(!hasEffect && isCompose)
				continue;

			wpimgobj.origin[1] = ortho.height - wpimgobj.origin[1];
			auto spImgNode = std::make_shared<SceneNode>(
				Vector3f(&wpimgobj.origin[0]), 
				Vector3f(&wpimgobj.scale[0]), 
				Vector3f(&wpimgobj.angles[0]) 
			);

			SceneMaterial material;
			WPShaderValueData svData;
			if(!hasEffect)
				svData.parallaxDepth = wpimgobj.parallaxDepth;

			ShaderValues baseConstSvs = globalBaseConstSvs;
			baseConstSvs["g_Alpha"] = {"g_Alpha", {wpimgobj.alpha}};
			baseConstSvs["g_Color"] = {"g_Color", wpimgobj.color};
			baseConstSvs["g_UserAlpha"] = {"g_UserAlpha", {wpimgobj.alpha}};
			baseConstSvs["g_Brightness"] = {"g_Brightness", {wpimgobj.brightness}};

			WPShaderInfo shaderInfo;
			shaderInfo.baseConstSvs = baseConstSvs;
			LoadMaterial(wpimgobj.material, upScene.get(), spImgNode.get(), &material, &svData, &shaderInfo);
			
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

			if(!wpimgobj.nopadding && material.customShader.constValues.count("g_Texture0Resolution") != 0) {
				const auto& resolution = material.customShader.constValues.at("g_Texture0Resolution").value;
				pow2Split = (int32_t)resolution[0] != (int32_t)resolution[2];
				pow2Split = pow2Split || (int32_t)resolution[1] != (int32_t)resolution[3];
			}

			GenCardMesh(mesh, std::vector<int32_t>(wpimgobj.size.begin(), wpimgobj.size.end()), pow2Split);

			// material blendmode for last step to use
			auto imgBlendMode = material.blenmode;
			// disable img material blend, as it's the first effect node now
			if(hasEffect) {
				material.blenmode = BlendMode::Normal;
			}
			mesh.AddMaterial(std::move(material));
			spImgNode->AddMesh(spMesh);

			shaderValueUpdater->SetNodeData(spImgNode.get(), svData);
			if(hasEffect) {
				// currently use addr for unique
				std::string nodeAddr = getAddr(spImgNode.get());
				// set camera to attatch effect
				if(isCompose) {
					upScene->cameras[nodeAddr] = std::make_shared<SceneCamera>(
						(int32_t)upScene->activeCamera->Width(), 
						(int32_t)upScene->activeCamera->Height(),
						-1.0f, 1.0f
					);
					upScene->cameras.at(nodeAddr)->AttatchNode(upScene->activeCamera->GetAttachedNode());
					if(upScene->linkedCameras.count("global") == 0)
						upScene->linkedCameras["global"] = {};
					upScene->linkedCameras.at("global").push_back(nodeAddr);
				}
				else {
					// applly scale to crop
					int32_t w = wpimgobj.size[0];
					int32_t h = wpimgobj.size[1];
					upScene->cameras[nodeAddr] = std::make_shared<SceneCamera>(w, h, -1.0f, 1.0f);
					upScene->cameras.at(nodeAddr)->AttatchNode(spEffCamNode);
				}
				spImgNode->SetCamera(nodeAddr);
				// set image effect
				auto imgEffectLayer = std::make_shared<SceneImageEffectLayer>(spImgNode.get(), wpimgobj.size[0], wpimgobj.size[1]);
				upScene->cameras.at(nodeAddr)->AttatchImgEffect(imgEffectLayer);
				// set renderTarget for ping-pong operate
				std::string effectRTs[2];
				effectRTs[0] = "_rt_" + nodeAddr;
				effectRTs[1] = "_rt_" + nodeAddr + "1";
				upScene->renderTargets[effectRTs[0]] = {
					.width = (uint16_t)wpimgobj.size[0], 
					.height = (uint16_t)wpimgobj.size[1], 
					.allowReuse = true
				};
				upScene->renderTargets[effectRTs[1]] = upScene->renderTargets.at(effectRTs[0]);
				if(wpimgobj.fullscreen) {
					upScene->renderTargets[effectRTs[0]].bind = {
						.enable = true,
						.screen = true
					};
					upScene->renderTargets[effectRTs[1]].bind = {
						.enable = true,
						.screen = true
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
					std::string effaddr = getAddr(imgEffectLayer.get());
					std::unordered_map<std::string, std::string> fboMap;
					fboMap["previous"] = inRT;
					for(int32_t i=0;i < wpeffobj.fbos.size();i++) {
						const auto& wpfbo = wpeffobj.fbos.at(i);
						std::string rtname = "_rt_" + effaddr + std::to_string(i);
						if(wpimgobj.fullscreen) {
							upScene->renderTargets[rtname] = {2, 2, true};
							upScene->renderTargets[rtname].bind = {
								.enable = true,
								.screen = true,
								.scale = 1.0f/wpfbo.scale
							};
						} else {
							// i+2 for not override object's rt
							upScene->renderTargets[rtname] = {
								.width = (uint16_t)(wpimgobj.size[0]/wpfbo.scale),
								.height = (uint16_t)(wpimgobj.size[1]/wpfbo.scale), 
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
						std::string effmataddr = getAddr(spEffNode.get());
						WPShaderInfo wpShaderInfo;
						shaderInfo.baseConstSvs = baseConstSvs;
						shaderInfo.baseConstSvs["g_EffectTextureProjectionMatrix"] = {
							"g_EffectTextureProjectionMatrix", 
							ShaderValue::ValueOf(Eigen::Matrix4f::Identity())
						};
						shaderInfo.baseConstSvs["g_EffectTextureProjectionMatrixInverse"] = {
							"g_EffectTextureProjectionMatrixInverse", 
							ShaderValue::ValueOf(Eigen::Matrix4f::Identity())
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
						auto spMesh = std::make_shared<SceneMesh>();
						auto& mesh = *spMesh;
						// the last effect and last material
						if((i_eff + 1 == count_eff && i_mat + 1 == wpeffobj.materials.size()) && !wpimgobj.fullscreen) {
							GenCardMesh(mesh, {(int32_t)wpimgobj.size[0], (int32_t)wpimgobj.size[1]}, false);
							spEffNode->CopyTrans(*spImgNode);
							if(!isCompose)
								spImgNode->CopyTrans(SceneNode());
							svData.parallaxDepth = wpimgobj.parallaxDepth;
							material.blenmode = imgBlendMode;
						} else {
							GenCardMesh(mesh, {2, 2}, false);
							// disable blend for effect node, as it seems blend manually
							material.blenmode = BlendMode::Normal;
							spEffNode->SetCamera("effect");
						}
						mesh.AddMaterial(std::move(material));
						spEffNode->AddMesh(spMesh);

						shaderValueUpdater->SetNodeData(spEffNode.get(), svData);
						imgEffect->nodes.push_back({matOutRT, spEffNode});
					}
				}
			}
			upScene->sceneGraph->AppendChild(spImgNode);
		} else if(indexT.first == "particle") {
			auto& wppartobj = wppartobjs.at(indexT.second);
			wppartobj.origin[1] = ortho.height - wppartobj.origin[1];

			auto spNode = std::make_shared<SceneNode>(
				Vector3f(&wppartobj.origin[0]), 
				Vector3f(&wppartobj.scale[0]), 
				Vector3f(&wppartobj.angles[0]) 
			);
			if(wppartobj.particleObj.flags.perspective) {
				spNode->SetCamera("global_perspective");
			}

			SceneMaterial material;
			WPShaderValueData svData;
			WPShaderInfo shaderInfo;
			shaderInfo.baseConstSvs = globalBaseConstSvs;
			shaderInfo.baseConstSvs["g_OrientationUp"] = {"g_OrientationUp", {0.0f, -1.0f, 0}};
			shaderInfo.baseConstSvs["g_OrientationRight"]= {"g_OrientationRight", {1.0f, 0, 0}};
			shaderInfo.baseConstSvs["g_OrientationForward"]= {"g_OrientationForward", {0, 0, 1.0f}};
			bool hastrail {false};
			if(wppartobj.particleObj.renderers.size() > 0) {
				auto wppartRenderer = wppartobj.particleObj.renderers.at(0);
				if(wppartRenderer.name == "spritetrail") {
					shaderInfo.baseConstSvs["g_RenderVar0"]= {"g_RenderVar0", {
						wppartRenderer.length, wppartRenderer.maxlength, 0, 0
					}};
					shaderInfo.combos["THICKFORMAT"] = 1;
					shaderInfo.combos["TRAILRENDERER"] = 1;
					hastrail = true;
				}
			}
			if(material.hasSprite && !wppartobj.particleObj.flags.spritenoframeblending) {
				shaderInfo.combos["SPRITESHEETBLEND"] = 1;
				shaderInfo.combos["SPRITESHEETBLENDNPOT"] = 1;
			}

			LoadMaterial(wppartobj.material, upScene.get(), spNode.get(), &material, &svData, &shaderInfo);
			auto spMesh = std::make_shared<SceneMesh>(true);
			auto& mesh = *spMesh;
			uint32_t maxcount = wppartobj.particleObj.maxcount;
			auto animationmode = ToAnimMode(wppartobj.particleObj.animationmode);
			auto sequencemultiplier = wppartobj.particleObj.sequencemultiplier;
			bool hasSprite = material.hasSprite;
			maxcount = maxcount > 4000 ? 4000 : maxcount;
			SetParticleMesh(mesh, wppartobj.particleObj, maxcount, material.hasSprite || hastrail);
			const auto& wpemitter = wppartobj.particleObj.emitters[0];
			auto particleSub = std::make_unique<ParticleSubSystem>(
				upScene->paritileSys, 
				spMesh, 
				maxcount,
				wppartobj.instanceoverride.rate,
				[=](const Particle& p, const ParticleRawGenSpec& spec) {
					auto& lifetime = *(spec.lifetime);
					if(lifetime < 0.0f) return;
					switch(animationmode) {
					case ParticleAnimationMode::RANDOMONE:
						lifetime = std::floor(p.lifetimeInit);
						break;
					case ParticleAnimationMode::SEQUENCE:
						lifetime = (1.0f - (p.lifetime / p.lifetimeInit)) * sequencemultiplier;
						break;
					}
				}
			);

			LoadEmitter(*particleSub, wppartobj.particleObj, wppartobj.instanceoverride.count, randomFn);
			LoadInitializer(*particleSub, wppartobj.particleObj, wppartobj.instanceoverride, randomFn);
			LoadOperator(*particleSub, wppartobj.particleObj, randomFn);

			upScene->paritileSys.subsystems.emplace_back(std::move(particleSub));
			mesh.AddMaterial(std::move(material));
			spNode->AddMesh(spMesh);
			upScene->sceneGraph->AppendChild(spNode);
		}
	}
	upScene->shaderValueUpdater = std::move(shaderValueUpdater);
	return upScene;	
}
