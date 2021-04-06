#include "GLGraphicManager.h"
#include "common.h"
#include "SpriteAnimation.h"

#include <functional>
#include <iostream>

using namespace wallpaper;

static Scene* curScene;
static gl::GLWrapper* pGlw;

std::string OutImageType(const Image& img) {
	if(img.type == ImageType::UNKNOWN)
		return ToString(img.format);	
	else 
		return ToString(img.type);
}

void LoadImage(const SceneTexture& tex, const Image& img) {
    auto& glw = *pGlw;
	if(glw.textureMap.count(tex.url) > 0) return;
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
			glw.TextureImage(texture, i_mip, imgData.width, imgData.height, img.format, imgData.data.get(), imgData.size);
		}
		texs.push_back(texture);
	}
	glw.textureMap[tex.url] = texs;
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
		auto img = curScene->imageParser->Parse(url);;
		LoadImage(*curScene->textures[url].get(), *img.get());
	}
	auto& materialShader = material->customShader;
	auto* shader = materialShader.shader.get();
	
	auto sv = glw.CreateShader(glw.ToGLType(ShaderType::VERTEX), shader->vertexCode);
	auto fg = glw.CreateShader(glw.ToGLType(ShaderType::FRAGMENT), shader->fragmentCode);
	glw.programMap[shader] = glw.CreateProgram({sv, fg}, shader->attrs);
	auto* program = glw.programMap[shader];
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
	if(!node->Camera().empty()) {
		auto& cam = m_scene->cameras.at(node->Camera());
		if(cam->HasImgEffect()) {
			const auto& name = cam->GetImgEffect()->FirstTarget();
			m_glw.BindFramebufferViewport(m_rtm.GetFrameBuffer(name, m_scene->renderTargets.at(name)));
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
			const auto& rt = m_scene->renderTargets.at(name);
			glw.BindFramebufferTex(m_rtm.GetFrameBuffer(name, rt));
		}
		else if(glw.textureMap.count(name) != 0) {
			// deal sprite
			if(curScene->textures.count(name) != 0) {
				const auto& stex = curScene->textures.at(name);
				if(stex->isSprite)
					imageId = stex->spriteAnim.GetCurFrame().imageId;
			}
			auto& texs = glw.textureMap.at(name);
			if(texs.size() > 0)
				glw.BindTexture(texs[imageId]);
		}
    }
	auto program = glw.programMap[shader];
	glw.BindProgram(program);
	for(auto& el:materialShader.updateValueList)
		glw.UpdateUniform(program, el);
	materialShader.updateValueList.clear();
	glw.SetBlend(material->blenmode);
	glw.RenderMesh(*mesh);

	if(!node->Camera().empty()) {
		auto& cam = m_scene->cameras.at(node->Camera());
		if(cam->HasImgEffect()) {
			auto& effs = cam->GetImgEffect();
			for(int32_t i=0;i<effs->EffectCount();i++) {
				auto& eff = effs->GetEffect(i);
				for(auto& n:eff->nodes) {
					auto& name = n.output;
					m_glw.BindFramebufferViewport(m_rtm.GetFrameBuffer(name, m_scene->renderTargets.at(name)));
					if(name != "_rt_default") {
						//m_glw.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					}
					RenderNode(n.sceneNode.get());
					m_glw.BindFramebufferViewport(m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default")));
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
	pGlw = &m_glw;
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
		std::string fgCode = "#version 150\n"
			"varying vec2 TexCoord;\n"
			"uniform sampler2D g_Texture0;\n"
			"void main() {gl_FragColor = texture2D(g_Texture0, TexCoord);}";
		m_fboNode = std::make_shared<SceneNode>();
		auto mesh = SceneMesh();
		SceneMesh::GenCardMesh(mesh, {2, 2}, false);
		SceneMaterial material;
		material.textures.push_back("_rt_default");
		material.defines.push_back("g_Texture0");
		material.customShader.shader = std::make_shared<SceneShader>();
		material.customShader.shader->vertexCode = vsCode;
		material.customShader.shader->fragmentCode = fgCode;
		mesh.AddMaterial(std::move(material));
		m_fboNode->AddMesh(std::move(mesh));
		LoadNode(m_fboNode.get());
	}
}

void GLGraphicManager::Draw() {
	m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default"));
	curScene->shaderValueUpdater->FrameBegin();
	const auto& cc = m_scene->clearColor;
	m_glw.BindFramebufferViewport(m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default")));
	m_glw.ClearColor(cc[0], cc[1], cc[2], 1.0f);
	TraverseNode(this, &GLGraphicManager::RenderNode, m_scene->sceneGraph.get());
	curScene->shaderValueUpdater->FrameEnd();
	m_glw.BindFramebufferViewport(&m_defaultFbo);
	m_glw.SetBlend(BlendMode::Disable);
	m_glw.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	if(m_fboNode)
		RenderNode(m_fboNode.get());
}

bool GLGraphicManager::Initialize(void *get_proc_addr(const char*)) {
	bool ok = m_glw.Init(get_proc_addr);
	return ok;
}

void GLGraphicManager::SetDefaultFbo(uint fbo, uint32_t w, uint32_t h) {
	m_defaultFbo = {w, h};
	m_scene->renderTargets["_rt_default"] = {w, h};
	m_defaultFbo.framebuffer = fbo;
}