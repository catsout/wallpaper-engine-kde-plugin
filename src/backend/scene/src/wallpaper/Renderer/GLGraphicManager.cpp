#include "GLGraphicManager.h"
#include "Log.h"
#include "SpriteAnimation.h"
#include "Algorism.h"
#include "SpecTexs.h"
#include "GLWrapper.h"

#include <functional>
#include <iostream>

using namespace wallpaper;


class GLGraphicManager::impl {
public:
	impl():glw(std::make_shared<gl::GLWrapper>()) {}
	std::shared_ptr<gl::GLWrapper> glw;
};

/*
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
*/



GLGraphicManager::GLGraphicManager():pImpl(std::make_unique<impl>()) {}

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
		auto texture = glw.CreateTexture(gl::ToGLType(TextureType::IMG_2D), img.width, img.height, mipmaps.size()-1, tex.sample);
		// mipmaps
		for(int i_mip=0;i_mip < mipmaps.size();i_mip++){
			auto& imgData = mipmaps.at(i_mip);
			glw.TextureImagePbo(texture, i_mip, imgData.width, imgData.height, img.format, imgData.data.get(), imgData.size);
		}
		texs.push_back(texture);
	}
	return texs;
}

void TraverseNode(const std::function<void(SceneNode*)>& func, SceneNode* node) {
	func(node);
	for(auto& child:node->GetChildren())
		TraverseNode(func, child.get());
}
/*
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
	for(const auto& el:program->uniformLocs) {
		if(el.name.empty()) continue;
		materialShader.valueSet.insert(el.name);
	}

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
	
	// set color mask
	bool alphaMask = !(node->Camera().empty() || node->Camera().compare(0, 6, "global") == 0);
	glw.SetColorMask(true, true, true, alphaMask);

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
}*/

HwShaderHandle InitShader(gl::GLWrapper* pglw, SceneMaterial* material) {
	auto& glw = *pglw;
	auto& materialShader = material->customShader;
	auto* shader = materialShader.shader.get();

	gl::GShader::Desc desc;
	desc.vs = shader->vertexCode;
	desc.fg = shader->fragmentCode;
	for(auto& el:shader->attrs) {
		desc.attrs.push_back({el.location, el.name});
	}
	for(const auto& def:material->defines)
		desc.texnames.push_back(def);
	auto handle = pglw->CreateShader(desc);
	materialShader.valueSet = pglw->GetUniforms(handle);

	glw.UseShader(handle, [=, &materialShader, &glw]() {
		for(auto& el:shader->uniforms)
			glw.UpdateUniform(handle, el.second);
		for(auto& el:materialShader.constValues) {
			glw.UpdateUniform(handle, el.second);
		}
	});
	return handle;
}

fg::FrameGraphResource AddCopyPass(fg::FrameGraph& fg, gl::GLWrapper& glw, fg::FrameGraphResource src) {
	struct PassData {
		fg::FrameGraphResource src;
		fg::FrameGraphMutableResource output;
	};
	auto& pass = fg.AddPass<PassData>("copy",
		[&](fg::FrameGraphBuilder& builder, PassData& data) {
			data.src = builder.Read(src);
			data.output = builder.CreateTexture(data.src);
			data.output = builder.Write(data.output);
		},
		[=,&glw](fg::FrameGraphResourceManager& rsm, const PassData& data) mutable {
			glw.CopyTexture(rsm.GetTexture(data.output)->handle, rsm.GetTexture(data.src)->handle);
		}
	);
	return pass->output;
}

void AddEndPass(fg::FrameGraph& fg, gl::GLWrapper& glw, fg::FrameGraphResource input,const std::array<bool, 2>& flips) {
	struct PassData {
		fg::FrameGraphResource input;
		std::shared_ptr<SceneMesh> mesh;
	};
	HwShaderHandle shader;
	float xflip = 1.0f;	
	float yflip = 1.0f;	
	fg.AddPass<PassData>("end",
		[&](fg::FrameGraphBuilder& builder, PassData& data) {
			data.input = builder.Read(input);
			std::string vs = R"(
#version 120
attribute vec3 a_position;
attribute vec2 a_texCoord;
uniform vec2 g_flips;
varying vec2 TexCoord;
void main()
{
	vec4 pos = vec4(a_position, 1.0f);
	pos.xy = pos.xy * g_flips;
	gl_Position = pos;
	TexCoord = a_texCoord;
}
)";
			std::string fg = R"(
#version 120
varying vec2 TexCoord;
uniform sampler2D g_Texture0;
void main() {
	gl_FragColor = texture2D(g_Texture0, TexCoord);
}
)";
			data.mesh = std::make_shared<SceneMesh>();
			SceneMesh::GenCardMesh(*data.mesh, {2, 2}, false);

			SceneMaterial material;
			material.textures.push_back("_rt_default");
			material.defines.push_back("g_Texture0");
			material.customShader.shader = std::make_shared<SceneShader>();
			material.customShader.shader->vertexCode = vs;
			material.customShader.shader->fragmentCode = fg;
			data.mesh->AddMaterial(std::move(material));
		},
		[=,&glw,&flips](fg::FrameGraphResourceManager& rsm, const PassData& data) mutable {
			gl::GPass gpass;
			gl::GBindings gbindings;
			{
				xflip = flips[0] ? -1.0f : 1.0f;
				yflip = flips[1] ? -1.0f : 1.0f;
			}
			if(shader.idx == 0) {
				shader = InitShader(&glw, data.mesh->Material());
			}
			gpass.shader = shader;
			gpass.blend = BlendMode::Disable;
			gpass.colorMask[3] = false;
			if(!glw.MeshLoaded(*data.mesh)) {
				glw.LoadMesh(*data.mesh);
				//LOG_INFO("e----" + std::to_string(data.mesh->ID()));
			}
			gbindings.texs[0] = rsm.GetTexture(data.input)->handle;

			glw.BeginPass(gpass);
			glw.ApplyBindings(gbindings);
			glw.UpdateUniform(shader, {.name = "g_flips", .value={xflip, yflip}});
			glw.ClearColor(0, 0, 0, 1.0f);
			glw.RenderMesh(*data.mesh);
			glw.EndPass(gpass);
		}
	);
};

void GLGraphicManager::ToFrameGraphPass(SceneNode* node, std::string output) {
	auto glw = pImpl->glw.get();
	struct PassData {
		std::vector<fg::FrameGraphResource> inputs;
		fg::FrameGraphMutableResource output;
		std::shared_ptr<fg::RenderPassData> renderpassData;
		std::array<bool, 4> colorMask;
	};
	auto loadImage = [this, glw](const std::string& url) {
		return m_scene->imageParser->Parse(url);
		//tex.desc.width = img->width;
		//tex.desc.height = img->height;
	};

	auto loadEffect = [this](SceneImageEffectLayer* effs) {
		for(int32_t i=0;i<effs->EffectCount();i++) {
			auto& eff = effs->GetEffect(i);
			for(auto& n:eff->nodes) {
				auto& name = n.output;
				ToFrameGraphPass(n.sceneNode.get(), name);
			}
		}
	};

	if(node->Mesh() == nullptr) return;
	auto* mesh = node->Mesh();
	if(mesh->Material() == nullptr) return;
	auto* material = mesh->Material();
	auto* mshaderPtr = material->customShader.shader.get();

	SceneImageEffectLayer* imgeff = nullptr;
	if(!node->Camera().empty()) {
		auto& cam = m_scene->cameras.at(node->Camera());
		if(cam->HasImgEffect()) {
			imgeff = cam->GetImgEffect().get();
			output = imgeff->FirstTarget();
		}
	}

	m_fg.AddPass<PassData>("test", 
	[&,loadImage, glw](fg::FrameGraphBuilder& builder, PassData& data) {
		LOG_INFO("-----------" + output)
		data.inputs.resize(material->textures.size());
		int32_t i=-1;
		for(const auto& url:material->textures) {
			i++;
			if(url.empty()) {}
			else if(IsSpecTex(url)) {
				if(m_fgrscMap.count(url) > 0) {
					if(url == output) {
						data.inputs[i] = AddCopyPass(m_fg, *glw, m_fgrscMap[url]);
						LOG_INFO("++++bind: " + url);
					} else {
						data.inputs[i] = m_fgrscMap[url];
						LOG_INFO("bind: " + url);
					}
				} else {
					LOG_ERROR(url + " not found");
				}
			}
			else {
				fg::TextureResource::Desc desc;
				desc.path = url;
				desc.name = url;
				desc.getImgOp = [=]() {
					return loadImage(url);
				};
				data.inputs[i] = builder.CreateTexture(desc);
			}
			data.inputs[i] = builder.Read(data.inputs[i]);
		}
		if(m_fgrscMap.count(output) > 0) {
			data.output = m_fg.AddMovePass(m_fgrscMap.at(output));
		} else {
			data.output = builder.CreateTexture({
				.width = 1920,
				.height = 1080,
				.name = output
			});
		}
		data.output = builder.Write(data.output);
		m_fgrscMap[output] = data.output;
		data.colorMask = {
			true,true,true,
			!(node->Camera().empty() || node->Camera().compare(0, 6, "global") == 0)
		};
		data.renderpassData = builder.UseRenderPass({
			.attachments = {data.output},
			.viewport = {0, 0, 1920, 1080},
		});
	}, 
	[this, material, mshaderPtr, mesh, node, output, glw](fg::FrameGraphResourceManager& rsm, const PassData& data) {
		gl::GPass gpass;
		gl::GBindings gbindings;

		gpass.target = data.renderpassData->target;
		gpass.viewport = {0,0, 1920, 1080};
		gpass.colorMask = data.colorMask;
		gpass.blend = material->blenmode;

		if(m_shaderMap.count(mshaderPtr) == 0) {
			m_shaderMap[mshaderPtr] = InitShader(glw, material);
		}
		gpass.shader = m_shaderMap[mshaderPtr];

		if(!glw->MeshLoaded(*mesh)) {
			glw->LoadMesh(*mesh);
		}

		m_scene->shaderValueUpdater->UpdateShaderValues(node, mshaderPtr);

		for(uint16_t i=0;i<data.inputs.size();i++) {
			//std::cout << rsm.GetTexture(el).desc.path << std::endl; 
			auto tex = rsm.GetTexture(data.inputs[i]);
			if(tex != nullptr) {
				uint16_t imageId = 0;
				if(!(IsSpecTex(tex->desc.name) || tex->desc.name.empty())) {
					const auto& stex = m_scene->textures.at(tex->desc.name);
					if(stex->isSprite) {
						imageId = stex->spriteAnim.GetCurFrame().imageId;
						glw->UpdateTextureSlot(tex->handle, imageId);
					}
				}
				gbindings.texs[i] = tex->handle;
			}
		}
		{
			glw->BeginPass(gpass);
			for(auto& el:material->customShader.updateValueList)
				glw->UpdateUniform(gpass.shader, el);

			glw->ApplyBindings(gbindings);
			material->customShader.updateValueList.clear();
			glw->RenderMesh(*mesh);

			glw->EndPass(gpass);
		}

	});

	// load effect
	if(imgeff != nullptr) loadEffect(imgeff);
}


HwTexHandle GLGraphicManager::CreateTexture(TextureDesc desc) {
	gl::GTexture::Desc gdesc; 
	gdesc.w = desc.width;
	gdesc.h = desc.height;
	gdesc.numMips = desc.numMips;
	gdesc.target = gl::ToGLType(desc.type);
	gdesc.format = desc.format;
	return pImpl->glw->CreateTexture(gdesc);
}

HwTexHandle GLGraphicManager::CreateTexture(const Image& img) {
	gl::GTexture::Desc desc; 
	desc.w = img.width;
	desc.h = img.height;
	desc.numMips = img.imageDatas[0].size();
	desc.numMips = desc.numMips>0?desc.numMips-1:0;
	desc.numSlots = img.count;
	desc.target = gl::ToGLType(TextureType::IMG_2D);
	desc.format = img.format;
	desc.sample = img.sample;
	return pImpl->glw->CreateTexture(desc, &img);
}


HwRenderTargetHandle GLGraphicManager::CreateRenderTarget(RenderTargetDesc desc) {
	gl::GFrameBuffer::Desc gdesc;
	gdesc.width = desc.width;
	gdesc.height = desc.height;
	gdesc.attachs = desc.attachs;
	return pImpl->glw->CreateRenderTarget(gdesc);
}

void GLGraphicManager::DestroyTexture(HwTexHandle h) {
	pImpl->glw->DestroyTexture(h);
}

void GLGraphicManager::InitializeScene(Scene* scene) {
	using namespace std::placeholders;
	m_scene = scene;

	/*
	if(m_defaultFbo.width != 0) {
		//UpdateDefaultRenderTargetBind(*m_scene, m_rtm, m_defaultFbo.width, m_defaultFbo.height);
	}
	*/
	TraverseNode(std::bind(&GLGraphicManager::ToFrameGraphPass, this, _1, std::string(Tex_Default)), scene->sceneGraph.get());
	LOG_INFO("--------------------end");
	AddEndPass(m_fg, *(pImpl->glw), m_fgrscMap.at(std::string(Tex_Default)), m_xyflip);
	m_fg.Compile();
	m_fg.ToGraphviz();
}

void GLGraphicManager::Draw() {
	if(m_scene == nullptr) return;
	//m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default"));

	m_scene->paritileSys.Emitt();
	m_scene->shaderValueUpdater->FrameBegin();

	const auto& cc = m_scene->clearColor;
	/*
	m_glw->BindFramebufferViewport(m_rtm.GetFrameBuffer("_rt_default", m_scene->renderTargets.at("_rt_default")));
	m_glw->SetDepthTest(false);
	m_glw->SetColorMask(true, true, true, true);
	m_glw->ClearColor(cc[0], cc[1], cc[2], 1.0f);
	TraverseNode(this, &GLGraphicManager::RenderNode, m_scene->sceneGraph.get());
	*/
	auto glw = pImpl->glw.get();


	//glw->BindFramebufferViewport(&m_defaultFbo);
	/*
	glw->SetDepthTest(false);
	*/
	glw->ClearColor(cc[0], cc[1], cc[2], 1.0f);
	//if(m_fboNode) RenderNode(m_fboNode.get());
	m_fg.Execute(*this);

	m_scene->shaderValueUpdater->FrameEnd();
}

bool GLGraphicManager::Initialize(void *get_proc_addr(const char*)) {
	bool ok = pImpl->glw->Init(get_proc_addr);
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

	//m_defaultFbo = {w, h};
	//m_defaultFbo.framebuffer = fbo;
	pImpl->glw->SetDefaultFrameBuffer(fbo, w, h);

	//UpdateDefaultRenderTargetBind(*m_scene, m_rtm, w, h);
	UpdateCameraForFbo(*m_scene, w, h, fillMode);
}

void GLGraphicManager::ChangeFillMode(FillMode fillMode) {
	if(m_scene == nullptr) return;
	//UpdateCameraForFbo(*m_scene, m_defaultFbo.width, m_defaultFbo.height, fillMode);
}

void GLGraphicManager::Destroy() {
	auto glw = pImpl->glw.get();
	m_scene = nullptr;
	for(const auto& el:m_shaderMap) {
		glw->DestroyShader(el.second);
	}
	m_shaderMap.clear();
	/*
	for(const auto& el:m_textureMap) {
		for(const auto& t:el) {
			glw->DeleteTexture(t);
		}
	}
	m_textureMap.clear();
	*/
	glw->CleanMeshBuf();
	//m_rtm.Clear();
	m_fboNode = nullptr;
}

GLGraphicManager::~GLGraphicManager() {
	Destroy();
}
