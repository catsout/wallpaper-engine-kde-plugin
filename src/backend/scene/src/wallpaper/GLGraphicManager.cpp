#include "GLGraphicManager.h"
#include "Log.h"
#include "SpriteAnimation.h"
#include "Algorism.h"

#include <functional>
#include <iostream>

using namespace wallpaper;

// render target

void GLRenderTargetManager::Clear() {
	for(const auto& el:m_unuse) {
		m_pGlw->DeleteFramebuffer(el.second);
	}
	m_unuse.clear();
	for(const auto& el:m_inuse) {
		m_pGlw->DeleteFramebuffer(el.second);
	}	
	m_inuse.clear();
}

uint64_t GLRenderTargetManager::GetID(const SceneRenderTarget& rt) const {
	uint64_t id = 0;	
	id += rt.width;
	id <<= 14;
	id += rt.height;
	id <<= 1;
	id += rt.withDepth;
	return id;
}

gl::GLFramebuffer* GLRenderTargetManager::GetFrameBuffer(const std::string& name, const SceneRenderTarget& rt) {
	auto id = GetID(rt);
	std::string keystr = name + std::to_string(id);
	if(m_inuse.count(keystr) == 0) {
		auto unuseEl = m_unuse.end();
		for(auto it=m_unuse.begin();it != m_unuse.end();it++) {
			if(id == it->first) {
				unuseEl = it;	
			}
		}
		if(unuseEl == m_unuse.end()) {
			auto* fb = m_pGlw->CreateFramebuffer(rt.width, rt.height, rt.sample);
			m_inuse[keystr] = fb;
		} else {
			m_inuse[keystr] = unuseEl->second;
			m_unuse.erase(unuseEl);
		}
	}
	return m_inuse.at(keystr);
}

void GLRenderTargetManager::ReleaseFrameBuffer(const std::string& name, const SceneRenderTarget& rt) {
	auto id = GetID(rt);
	std::string keystr = name + std::to_string(id);
	if(m_inuse.count(keystr) != 0) {
		m_unuse.push_back({id, m_inuse.at(keystr)});
		m_inuse.erase(keystr);
	}
}

void GLRenderTargetManager::ReleaseAndDeleteFrameBuffer(const std::string& name, const SceneRenderTarget& rt) {
	auto id = GetID(rt);
	std::string keystr = name + std::to_string(id);
	if(m_inuse.count(keystr) != 0) {
		m_pGlw->DeleteFramebuffer(m_inuse.at(keystr));
		m_inuse.erase(keystr);
	}
}


void UpdateDefaultRenderTargetBind(Scene& scene, GLRenderTargetManager& rtm, uint32_t w, uint32_t h) {

	if(scene.renderTargets.count("_rt_default") != 0)
		rtm.ReleaseAndDeleteFrameBuffer("_rt_default", scene.renderTargets.at("_rt_default"));
	scene.renderTargets["_rt_default"] = {w, h};

	for(const auto& el:scene.renderTargetBindMap) {
		if(!el.second.copy && el.second.name == "_rt_default") {
			uint32_t sw = w * el.second.scale;
			uint32_t sh = h * el.second.scale;
			scene.renderTargets[el.first] = {sw, sh};
		}
	}
}



std::string OutImageType(const Image& img) {
	if(img.type == ImageType::UNKNOWN)
		return ToString(img.format);	
	else 
		return ToString(img.type);
}

std::vector<gl::GLTexture*> LoadImage(gl::GLWrapper* pglw, const SceneTexture& tex, const Image& img) {
    auto& glw = *pglw;
	LOG_INFO(std::string("Load tex ") + OutImageType(img) + " " + tex.url);
	std::vector<gl::GLTexture*> texs;
	for(int i_img=0;i_img < img.count;i_img++) {
		auto& mipmaps = img.imageDatas.at(i_img);
		if(mipmaps.size() == 0) {
			LOG_ERROR("no tex data");
			continue;
		}
		auto texture = glw.CreateTexture(glw.ToGLType(TextureType::IMG_2D), img.width, img.height, mipmaps.size()-1, tex.sample);
		// mipmaps
		for(int i_mip=0;i_mip < mipmaps.size();i_mip++){
			auto& imgData = mipmaps.at(i_mip);
			glw.TextureImagePbo(texture, i_mip, imgData.width, imgData.height, img.format, imgData.data.get(), imgData.size);
		}
		texs.push_back(texture);
	}
	return texs;
}

void TraverseNode(GLGraphicManager* pMgr, void (GLGraphicManager::*func)(SceneNode*), SceneNode* node) {
	(pMgr->*func)(node);
	for(auto& child:node->GetChildren())
		TraverseNode(pMgr, func, child.get());
}

void GLGraphicManager::LoadNode(SceneNode* node) {
    auto& glw = *m_glw;
	if(node->Mesh() == nullptr) return;
	auto* mesh = node->Mesh();
	glw.LoadMesh(*mesh);
	if(mesh->Material() == nullptr) return;

	auto* material = mesh->Material();
	for(const auto& url:material->textures) {
		if(url.empty() || url.compare(0, 4, "_rt_") == 0) continue;
		if(m_textureMap.count(url) > 0) continue;
		auto img = m_scene->imageParser->Parse(url);;
		m_textureMap[url] = LoadImage(m_glw.get(), *m_scene->textures[url].get(), *img.get());
	}
	auto& materialShader = material->customShader;
	auto* shader = materialShader.shader.get();

	std::vector<gl::GLShader*> glshaders;
	glshaders.push_back(glw.CreateShader(glw.ToGLType(ShaderType::VERTEX), shader->vertexCode));
	glshaders.push_back(glw.CreateShader(glw.ToGLType(ShaderType::FRAGMENT), shader->fragmentCode));
	if(!shader->geometryCode.empty()) {
		glshaders.push_back(glw.CreateShader(glw.ToGLType(ShaderType::GEOMETRY), shader->geometryCode));
	}
	m_programMap[shader] = glw.CreateProgram(glshaders, shader->attrs);
	auto* program = m_programMap.at(shader);
	glw.BindProgram(program);
	glw.QueryProUniforms(program);
	for(auto& el:shader->uniforms)
		glw.UpdateUniform(program, el.second);
	for(auto& el:materialShader.constValues) {
		glw.UpdateUniform(program, el.second);
	}
	int32_t i = 0;
	for(const auto& def:material->defines)
		glw.SetTexSlot(program, def, i++);
}

void GLGraphicManager::RenderNode(SceneNode* node) {
    auto& glw = *m_glw;
	if(node->Mesh() == nullptr) return;
	auto* mesh = node->Mesh();
	if(mesh->Material() == nullptr) return;
	gl::GLFramebuffer* target(nullptr);
	// Bind target first, this make first frame show correct
	if(!node->Camera().empty()) {
		auto& cam = m_scene->cameras.at(node->Camera());
		if(cam->HasImgEffect()) {
			const auto& name = cam->GetImgEffect()->FirstTarget();
			if(m_scene->renderTargets.count(name) != 0) {
				target = m_rtm.GetFrameBuffer(name, m_scene->renderTargets.at(name));
				m_glw->BindFramebufferViewport(target);
			}
		}
	}
	auto* material = mesh->Material();

	auto& materialShader = material->customShader;
	auto* shader = materialShader.shader.get();
	m_scene->shaderValueUpdater->UpdateShaderValues(node, shader);

	int32_t i_tex = -1;
	for(const auto& name:material->textures) {
		i_tex++;
		glw.ActiveTexture(i_tex);
		int32_t imageId = 0;
		if(name.compare(0, 4, "_rt_") == 0) {
			if(m_scene->renderTargets.count(name) == 0) continue;
			auto& rt = m_scene->renderTargets.at(name);
			if(m_scene->renderTargetBindMap.count(name) != 0) {
				const auto& copy = m_scene->renderTargetBindMap.at(name);
				if(copy.copy == true) {
					const auto& rtcopy = m_scene->renderTargets.at(copy.name);
					rt = rtcopy;
					auto* gltex = &m_rtm.GetFrameBuffer(name, rt)->color_texture;
					auto* glcopy = m_rtm.GetFrameBuffer(copy.name, rtcopy);
					glw.CopyTexture(glcopy, gltex);
				}
			}
			glw.BindFramebufferTex(m_rtm.GetFrameBuffer(name, rt));
		}
		else if(m_textureMap.count(name) != 0) {
			// deal sprite
			if(m_scene->textures.count(name) != 0) {
				const auto& stex = m_scene->textures.at(name);
				if(stex->isSprite)
					imageId = stex->spriteAnim.GetCurFrame().imageId;
			}
			auto& texs = m_textureMap.at(name);
			if(texs.size() > 0)
				glw.BindTexture(texs[imageId]);
		}
    }
	auto program = m_programMap.at(shader);
	glw.BindProgram(program);
	for(auto& el:materialShader.updateValueList)
		glw.UpdateUniform(program, el);
	materialShader.updateValueList.clear();
	glw.SetBlend(material->blenmode);
	if(target != nullptr)
		m_glw->BindFramebufferViewport(target);
	glw.RenderMesh(*mesh);

	if(!node->Camera().empty()) {
		auto& cam = m_scene->cameras.at(node->Camera());
		if(cam->HasImgEffect()) {
			auto& effs = cam->GetImgEffect();
			for(int32_t i=0;i<effs->EffectCount();i++) {
				auto& eff = effs->GetEffect(i);
				for(auto& n:eff->nodes) {
					auto& name = n.output;
					m_glw->BindFramebufferViewport(m_rtm.GetFrameBuffer(name, m_scene->renderTargets.at(name)));
					if(name != "_rt_default") {
						//m_glw.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					}
					RenderNode(n.sceneNode.get());
					m_glw->BindFramebufferViewport(m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default")));
				}
			}

			for(int32_t i=0;i<effs->EffectCount();i++) {
				auto& eff = effs->GetEffect(i);
				for(auto& n:eff->nodes) {
					if(n.sceneNode->Mesh() == nullptr) continue;
					const auto& mesh = n.sceneNode->Mesh();
					if(mesh->Material() == nullptr) continue;
					const auto& mat = mesh->Material();
					for(const auto& t:mat->textures) {
						if(t.compare(0, 4, "_rt_") == 0 && m_scene->renderTargets.count(t) != 0) {
							const auto& rt = m_scene->renderTargets.at(t);
							if(t != "_rt_default" && rt.allowReuse) {
								m_rtm.ReleaseFrameBuffer(t, rt);
							}
						}
					}
				}
			}
		}
	}
}

void GLGraphicManager::InitializeScene(Scene* scene) {
	m_scene = scene;
	for(auto& cam:m_scene->cameras) {
		if(cam.second->HasImgEffect()) {
			auto& imgEffctLayer = cam.second->GetImgEffect();
			for(int32_t i=0;i < imgEffctLayer->EffectCount();i++) {
				for(auto& node:imgEffctLayer->GetEffect(i)->nodes) {
					LoadNode(node.sceneNode.get());
				}
			}
		}
	}
	TraverseNode(this, &GLGraphicManager::LoadNode, scene->sceneGraph.get());
	if(!m_fboNode) {
		std::string vsCode = "#version 120\n"
			"attribute vec3 a_position;\n"
			"attribute vec2 a_texCoord;\n"
			"varying vec2 TexCoord;\n"
			"void main()\n"
			"{gl_Position = vec4(a_position, 1.0f);TexCoord = a_texCoord;}";
		std::string fgCode = "#version 120\n"
			"varying vec2 TexCoord;\n"
			"uniform sampler2D g_Texture0;\n"
			"void main() {gl_FragColor = texture2D(g_Texture0, TexCoord);}";
		m_fboNode = std::make_shared<SceneNode>();
		auto mesh = std::make_shared<SceneMesh>();
		SceneMesh::GenCardMesh(*mesh, {2, 2}, false);
		SceneMaterial material;
		material.textures.push_back("_rt_default");
		material.defines.push_back("g_Texture0");
		material.customShader.shader = std::make_shared<SceneShader>();
		material.customShader.shader->vertexCode = vsCode;
		material.customShader.shader->fragmentCode = fgCode;
		mesh->AddMaterial(std::move(material));
		m_fboNode->AddMesh(mesh);
		LoadNode(m_fboNode.get());
	}

	if(m_defaultFbo.width != 0) {
		UpdateDefaultRenderTargetBind(*m_scene, m_rtm, m_defaultFbo.width, m_defaultFbo.height);
	}
}

void GLGraphicManager::Draw() {
	if(m_scene == nullptr) return;
	m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default"));

	m_scene->paritileSys.Emitt();
	m_scene->shaderValueUpdater->FrameBegin();

	const auto& cc = m_scene->clearColor;
	m_glw->BindFramebufferViewport(m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default")));
	m_glw->ClearColor(cc[0], cc[1], cc[2], 1.0f);
	TraverseNode(this, &GLGraphicManager::RenderNode, m_scene->sceneGraph.get());

	m_scene->shaderValueUpdater->FrameEnd();

	m_glw->BindFramebufferViewport(&m_defaultFbo);
	m_glw->SetBlend(BlendMode::Disable);
	m_glw->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	if(m_fboNode) RenderNode(m_fboNode.get());
}

bool GLGraphicManager::Initialize(void *get_proc_addr(const char*)) {
	bool ok = m_glw->Init(get_proc_addr);
	return ok;
}

void UpdateCameraForFbo(Scene& scene, uint32_t fbow, uint32_t fboh, FillMode fillmode) {
	if(fboh == 0) return;
	double sw = scene.ortho[0],sh = scene.ortho[1];
	double fboAspect = fbow/(double)fboh, sAspect = sw/sh;
	double nw = 0.0f,nh = 0.0f;
	auto& gCam = *scene.cameras.at("global");
	auto& gPerCam = *scene.cameras.at("global_perspective");
	// assum cam 
	switch (fillmode)
	{
	case FillMode::STRETCH:
		gCam.SetWidth(sw);
		gCam.SetHeight(sh);
		gPerCam.SetAspect(sAspect);
		gPerCam.SetFov(algorism::CalculatePersperctiveFov(1000.0f, gCam.Height()));
		break;
	case FillMode::ASPECTFIT:
		if(fboAspect < sAspect) {
			// scale height
			gCam.SetWidth(sw);
			gCam.SetHeight(sw / fboAspect);
		} else {
			gCam.SetWidth(sh * fboAspect);
			gCam.SetHeight(sh);
		}
		gPerCam.SetAspect(fboAspect);
		gPerCam.SetFov(algorism::CalculatePersperctiveFov(1000.0f, gCam.Height()));
		break;
	case FillMode::ASPECTCROP:
	default:
		if(fboAspect > sAspect) {
			// scale height
			gCam.SetWidth(sw);
			gCam.SetHeight(sw / fboAspect);
		} else {
			gCam.SetWidth(sh * fboAspect);
			gCam.SetHeight(sh);
		}
		gPerCam.SetAspect(fboAspect);
		gPerCam.SetFov(algorism::CalculatePersperctiveFov(1000.0f, gCam.Height()));
		break;
	}
	gCam.Update();
	gPerCam.Update();
	scene.UpdateLinkedCamera("global");
}

void GLGraphicManager::SetDefaultFbo(uint fbo, uint32_t w, uint32_t h, FillMode fillMode) {
	if(m_scene != nullptr) {
		if(m_scene->renderTargets.count("_rt_default") == 0)
			m_scene->renderTargets["_rt_default"] = {w, h};
	} else return;

	m_defaultFbo = {w, h};
	m_defaultFbo.framebuffer = fbo;

	UpdateDefaultRenderTargetBind(*m_scene, m_rtm, w, h);
	UpdateCameraForFbo(*m_scene, w, h, fillMode);
}

void GLGraphicManager::ChangeFillMode(FillMode fillMode) {
	if(m_scene == nullptr) return;
	UpdateCameraForFbo(*m_scene, m_defaultFbo.width, m_defaultFbo.height, fillMode);
}

void GLGraphicManager::Destroy() {
	m_scene = nullptr;
	for(const auto& el:m_programMap) {
		m_glw->DeleteProgram(el.second);
	}
	m_programMap.clear();
	for(const auto& el:m_textureMap) {
		for(const auto& t:el.second) {
			m_glw->DeleteTexture(t);
		}
	}
	m_textureMap.clear();
	m_glw->CleanMeshBuf();
	m_rtm.Clear();
	m_fboNode = nullptr;
}

GLGraphicManager::~GLGraphicManager() {
	Destroy();
}
