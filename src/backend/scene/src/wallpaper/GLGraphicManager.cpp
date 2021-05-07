#include "GLGraphicManager.h"
#include "common.h"
#include "SpriteAnimation.h"

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


// graphic manager
static Scene* curScene;
static gl::GLWrapper* pGlw;

std::string OutImageType(const Image& img) {
	if(img.type == ImageType::UNKNOWN)
		return ToString(img.format);	
	else 
		return ToString(img.type);
}

std::vector<gl::GLTexture*> LoadImage(const SceneTexture& tex, const Image& img) {
    auto& glw = *pGlw;
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
    auto& glw = *pGlw;
	if(node->Mesh() == nullptr) return;
	auto* mesh = node->Mesh();
	glw.LoadMesh(*mesh);
	if(mesh->Material() == nullptr) return;

	auto* material = mesh->Material();
	for(const auto& url:material->textures) {
		if(url.empty() || url.compare(0, 4, "_rt_") == 0) continue;
		if(m_textureMap.count(url) > 0) continue;
		auto img = curScene->imageParser->Parse(url);;
		m_textureMap[url] = LoadImage(*curScene->textures[url].get(), *img.get());
	}
	auto& materialShader = material->customShader;
	auto* shader = materialShader.shader.get();
	
	auto sv = glw.CreateShader(glw.ToGLType(ShaderType::VERTEX), shader->vertexCode);
	auto fg = glw.CreateShader(glw.ToGLType(ShaderType::FRAGMENT), shader->fragmentCode);
	m_programMap[shader] = glw.CreateProgram({sv, fg}, shader->attrs);
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
    auto& glw = *pGlw;
	if(node->Mesh() == nullptr) return;
	auto* mesh = node->Mesh();
	if(mesh->Material() == nullptr) return;
	gl::GLFramebuffer* target(nullptr);
	// Bind target first, this make first frame show correct
	if(!node->Camera().empty()) {
		auto& cam = m_scene->cameras.at(node->Camera());
		if(cam->HasImgEffect()) {
			const auto& name = cam->GetImgEffect()->FirstTarget();
			target = m_rtm.GetFrameBuffer(name, m_scene->renderTargets.at(name));
			m_glw->BindFramebufferViewport(target);
		}
	}
	auto* material = mesh->Material();

	auto& materialShader = material->customShader;
	auto* shader = materialShader.shader.get();
	curScene->shaderValueUpdater->UpdateShaderValues(node, shader);

	int32_t i_tex = -1;
	for(const auto& name:material->textures) {
		i_tex++;
		glw.ActiveTexture(i_tex);
		int32_t imageId = 0;
		if(name.compare(0, 4, "_rt_") == 0) {
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
			if(curScene->textures.count(name) != 0) {
				const auto& stex = curScene->textures.at(name);
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
						if(t.compare(0, 4, "_rt_") == 0) {
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
	curScene = scene;
	pGlw = m_glw.get();
	m_scene = scene;
	m_aspect = -1.0f;
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
}

void GLGraphicManager::Draw() {
	if(m_scene == nullptr) return;
	m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default"));
	curScene->shaderValueUpdater->FrameBegin();
	const auto& cc = m_scene->clearColor;
	m_glw->BindFramebufferViewport(m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default")));
	m_glw->ClearColor(cc[0], cc[1], cc[2], 1.0f);
	TraverseNode(this, &GLGraphicManager::RenderNode, m_scene->sceneGraph.get());
	curScene->shaderValueUpdater->FrameEnd();
	m_glw->BindFramebufferViewport(&m_defaultFbo);
	m_glw->SetBlend(BlendMode::Disable);
	m_glw->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	if(m_fboNode)
		RenderNode(m_fboNode.get());
}

bool GLGraphicManager::Initialize(void *get_proc_addr(const char*)) {
	bool ok = m_glw->Init(get_proc_addr);
	return ok;
}

void GLGraphicManager::SetDefaultFbo(uint fbo, uint32_t w, uint32_t h) {
	if(m_scene != nullptr) {
		if(m_scene->renderTargets.count("_rt_default") == 0)
			m_scene->renderTargets["_rt_default"] = {w, h};
	} else return;
	if(w == m_defaultFbo.width && h == m_defaultFbo.height && m_aspect > 0) return;
	if(m_aspect < 0)
		m_aspect = m_scene->activeCamera->Aspect();

	m_defaultFbo = {w, h};
	m_defaultFbo.framebuffer = fbo;
	if(m_scene->renderTargets.count("_rt_default") != 0)
		m_rtm.ReleaseAndDeleteFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default"));
	m_scene->renderTargets["_rt_default"] = {w, h};
	for(const auto& el:m_scene->renderTargetBindMap) {
		if(!el.second.copy && el.second.name == "_rt_default") {
			uint32_t sw = w * el.second.scale;
			uint32_t sh = h * el.second.scale;
			if(sw == 0 || sh == 0) {
				m_scene->renderTargets[el.first] = {w, h};
			} else {
				m_scene->renderTargets[el.first] = {sw, sh};
			}
		}
	}
	float screenAspect = w/(float)h;
	float width,height;
	if(m_aspect > m_scene->activeCamera->Aspect())
		m_scene->activeCamera->SetWidth(m_scene->activeCamera->Height() * m_aspect);
	else 
		m_scene->activeCamera->SetHeight(m_scene->activeCamera->Width() / m_aspect);
	if(m_aspect > screenAspect) {
		height = m_scene->activeCamera->Height();	
		width = height * screenAspect;
		m_scene->activeCamera->SetWidth(width);
	} else {
		width = m_scene->activeCamera->Width();
		height = width / screenAspect;
		m_scene->activeCamera->SetHeight(height);
	}
	m_scene->UpdateLinkedCamera("global");
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
