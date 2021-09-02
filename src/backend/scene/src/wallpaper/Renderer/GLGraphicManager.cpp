#include "GLGraphicManager.h"
#include "Log.h"
#include "SpriteAnimation.h"
#include "Algorism.h"
#include "SpecTexs.h"
#include "GLWrapper.h"

#include <functional>

using namespace wallpaper;


class GLGraphicManager::impl {
public:
	impl():glw(std::make_shared<gl::GLWrapper>()) {}
	std::shared_ptr<gl::GLWrapper> glw;
};


GLGraphicManager::GLGraphicManager():pImpl(std::make_unique<impl>()),m_fg(std::make_unique<fg::FrameGraph>()) {}


std::string OutImageType(const Image& img) {
	if(img.header.type == ImageType::UNKNOWN)
		return ToString(img.header.format);
	else 
		return ToString(img.header.type);
}

/*
std::vector<gl::GLTexture*> LoadImage(gl::GLWrapper* pglw, const SceneTexture& tex, const Image& img) {
	auto& glw = *pglw;
	LOG_INFO(std::string("Load tex ") + OutImageType(img) + " " + tex.url);
	std::vector<gl::GLTexture*> texs;
	for(int i_img=0;i_img < img.count;i_img++) {
		auto& mipmaps = img.imageDatas.at(i_img);
		if(mipmaps.empty()) {
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
*/

void UpdateSceneRenderTargetSize(Scene& scene, std::array<uint16_t, 2> screenSize) {
	for(auto& el:scene.renderTargets) {
		auto& rt = el.second;
		if(rt.bind.enable && rt.bind.screen) {
			rt.width = screenSize[0] * rt.bind.scale;
			rt.height = screenSize[1] * rt.bind.scale;
		}
	}
}

void TraverseNode(const std::function<void(SceneNode*)>& func, SceneNode* node) {
	func(node);
	for(auto& child:node->GetChildren())
		TraverseNode(func, child.get());
}

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
	for(const auto& def:material->defines) {
		desc.texnames.push_back(def);
	}
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

fg::FrameGraphMutableResource AddCopyPass(fg::FrameGraph& fg, gl::GLWrapper& glw, fg::FrameGraphResource src, std::string copyName = {""}) {
	struct PassData {
		fg::FrameGraphResource src;
		fg::FrameGraphMutableResource output;
	};
	auto& pass = fg.AddPass<PassData>("copy",
		[&](fg::FrameGraphBuilder& builder, PassData& data) {
			data.src = builder.Read(src);
			data.output = builder.CreateTexture(data.src, copyName);
			data.output = builder.Write(data.output);
		},
		[=,&glw](fg::FrameGraphResourceManager& rsm, const PassData& data) mutable {
			glw.CopyTexture(rsm.GetTexture(data.output)->handle, rsm.GetTexture(data.src)->handle);
		}
	);
	return pass->output;
}

fg::TextureResource::Desc GenFgTextureRsc(Scene& scene, const std::string& name, 
		const std::array<uint16_t,2>& screenSize, 
		bool temperary,
		SceneRenderTarget* pRt = nullptr) {
	SceneRenderTarget rt {.width = 1920, .height = 1080};
	if(scene.renderTargets.count(name) > 0) {
		rt = scene.renderTargets[name];
	}
	fg::TextureResource::Desc desc {
		.width = rt.width,
		.height = rt.height,
		.sample = rt.sample,
		.temperary = temperary,
		.name = name
	};
	if(rt.bind.enable) {// && rt.bind.screen) {
		auto scale = rt.bind.scale;
		desc.UpdateDescOp = [&, scale](fg::TextureResource::Desc& d) {
			d.width = screenSize[0] * scale;
			d.height = screenSize[1] * scale;
		};
	}
	if(pRt) *pRt = rt;
	return desc;
}

void GLGraphicManager::AddCopyCmdPasses(const std::string& dst, const std::string src) {
	bool createSrc = m_fgrscMap.count(src) == 0;
	bool createDst = m_fgrscMap.count(dst) == 0;
	struct PassData {
		fg::FrameGraphMutableResource src;
		fg::FrameGraphMutableResource output;
	};
	auto& pass = m_fg->AddPass<PassData>("copy command",
		[&](fg::FrameGraphBuilder& builder, PassData& data) {
			if(createSrc) {
				data.src = builder.CreateTexture(GenFgTextureRsc(*m_scene, src, m_screenSize, false));
				m_fgrscMap[src] = data.src;
			} else data.src = m_fgrscMap[src];
			if(createDst) {
				data.output = builder.CreateTexture(GenFgTextureRsc(*m_scene, dst, m_screenSize, true));
				m_fgrscMap[dst] = data.output;
			} else data.output = m_fgrscMap[dst];
			data.src = builder.Read(data.src);
			data.output = builder.Write(data.output);
		},
		[this](fg::FrameGraphResourceManager& rsm, const PassData& data) mutable {
			pImpl->glw->CopyTexture(rsm.GetTexture(data.output)->handle, rsm.GetTexture(data.src)->handle);
		}
	);
}


void GLGraphicManager::AddPreParePass() {
	struct PassData {
		fg::FrameGraphMutableResource output;
	};
	m_fg->AddPass<PassData>("prepare",
		[&](fg::FrameGraphBuilder& builder, PassData& data) {
			std::string def {SpecTex_Default};

			const auto& desc = GenFgTextureRsc(*m_scene, def, m_screenSize, true);
			data.output = builder.CreateTexture(desc);
			data.output = builder.Write(data.output);
			m_fgrscMap[def] = data.output;
		},
		[this](fg::FrameGraphResourceManager& rsm, const PassData& data) mutable {
			//glw.ClearTexture(, );
			const auto& cc = m_scene->clearColor;
			auto* tex = rsm.GetTexture(data.output);
			assert(tex != nullptr);
			pImpl->glw->ClearTexture(tex->handle, {cc[0], cc[1], cc[2], 1.0f});
		}
	);

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

void GLGraphicManager::ToFrameGraphPass(SceneNode* node, std::string output, uint32_t imgId) {
	auto glw = pImpl->glw.get();
	struct PassData {
		std::vector<fg::FrameGraphResource> inputs;
		fg::FrameGraphMutableResource output;
		std::shared_ptr<fg::RenderPassData> renderpassData;
		std::array<bool, 4> colorMask;
		std::function<void(gl::ViewPort&)> dynViewportOp;
	};
	auto loadImage = [this, glw](const std::string& url) {
		return m_scene->imageParser->Parse(url);
		//tex.desc.width = img->width;
		//tex.desc.height = img->height;
	};

	auto loadEffect = [this, glw, node](SceneImageEffectLayer* effs) {
		for(int32_t i=0;i<effs->EffectCount();i++) {
			auto& eff = effs->GetEffect(i);
			auto cmdItor = eff->commands.begin();
			auto cmdEnd = eff->commands.end();
			int nodePos = 0;
			for(auto& n:eff->nodes) {
				if(cmdItor != cmdEnd && nodePos == cmdItor->afterpos) {
					AddCopyCmdPasses(cmdItor->dst, cmdItor->src);
					cmdItor++;
				}
				auto& name = n.output;
				ToFrameGraphPass(n.sceneNode.get(), name, node->ID());
				nodePos++;
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

	std::string passName = material->name;

	if(imgId == 0) {
		imgId = node->ID();
	}

	m_fg->AddPass<PassData>(passName, 
	[&,loadImage, glw, imgId](fg::FrameGraphBuilder& builder, PassData& data) {

		// input
		data.inputs.resize(material->textures.size());
		int32_t i=-1;
		for(const auto& url:material->textures) {
			i++;
			if(url.empty()) {}
			else if(IsSpecTex(url)) {
				if(m_fgrscMap.count(url) > 0) {
					if(url == output) {
						data.inputs[i] = AddCopyPass(*m_fg, *glw, m_fgrscMap[url]);
						if(url != SpecTex_Default)
							LOG_INFO("copy bind: " + url);
					} else {
						data.inputs[i] = m_fgrscMap[url];
					}
				} else if(m_scene->renderTargets.count(url) > 0) {
					m_fgrscMap[url] = builder.CreateTexture(GenFgTextureRsc(*m_scene, url, m_screenSize, true));
					data.inputs[i] = m_fgrscMap[url];
				} else if(IsSpecLinkTex(url)) {
					auto id = ParseLinkTex(url);
					if(m_fgrscIdMap.count(id)) {
						data.inputs[i] = AddCopyPass(*m_fg, *glw, m_fgrscIdMap.at(id), url);
					} else {
						LOG_ERROR(url + " link tex not found, at pass " + passName);
					}
				} else {
					LOG_ERROR(url + " not found, at pass " + passName);
				}
			} else {
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
		
		// output
		fg::RenderPassData rPassData;
		{
			SceneRenderTarget rt {.width = 1920, .height = 1080};
			if(m_scene->renderTargets.count(output) > 0) {
				rt = m_scene->renderTargets[output];
			}
			rPassData.viewPort.width = rt.width;
			rPassData.viewPort.height = rt.height;
			if(rt.bind.enable) {
				auto scale = rt.bind.scale;
				data.dynViewportOp = [&,scale](gl::ViewPort& v) {
					v.width = m_screenSize[0] * scale;
					v.height = m_screenSize[1] * scale;
				};
			}
		}

		if(m_fgrscMap.count(output) > 0) {
			data.output = m_fg->AddMovePass(m_fgrscMap.at(output));
		} else {
			const auto& desc = GenFgTextureRsc(*m_scene, output, m_screenSize, true);
			data.output = builder.CreateTexture(desc);
		}
		data.output = builder.Write(data.output);
		m_fgrscMap[output] = data.output;

		data.colorMask = {
			true,true,true,
			!(node->Camera().empty() || node->Camera().compare(0, 6, "global") == 0)
		};

		rPassData.attachments = {data.output};
		data.renderpassData = builder.UseRenderPass(rPassData);

		{
			if(output == SpecTex_Default) {
				m_fgrscIdMap[imgId] = data.output;
			}
		}
	}, 
	[this, material, mshaderPtr, mesh, node, output, glw](fg::FrameGraphResourceManager& rsm, const PassData& data) {
		gl::GPass gpass;
		gl::GBindings gbindings;

		gpass.target = data.renderpassData->target;
		{
			if(data.dynViewportOp) {
				data.dynViewportOp(gpass.viewport);
			} else {
				auto& v = data.renderpassData->viewPort;
				gpass.viewport = {v.x, v.y, v.width, v.height};
			}
		}
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
					if(stex.isSprite) {
						imageId = stex.spriteAnim.GetCurFrame().imageId;
						glw->UpdateTextureSlot(tex->handle, imageId);
					}
				}
				gbindings.texs[i] = tex->handle;
				gbindings.texNum = i+1;
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
	gdesc.sample = desc.sample;
	return pImpl->glw->CreateTexture(gdesc);
}

HwTexHandle GLGraphicManager::CreateTexture(const Image& img) {
	gl::GTexture::Desc desc; 
	const auto& header = img.header;
	desc.w = header.width;
	desc.h = header.height;
	desc.numMips = img.slots[0].size();
	desc.numMips = desc.numMips>0?desc.numMips-1:0;
	desc.numSlots = header.count;
	desc.target = gl::ToGLType(TextureType::IMG_2D);
	desc.format = header.format;
	desc.sample = header.sample;
	return pImpl->glw->CreateTexture(desc, &img);
}


void GLGraphicManager::ClearTexture(HwTexHandle thandle, std::array<float, 4> clearcolors) {
	pImpl->glw->ClearTexture(thandle, clearcolors);
}

HwRenderTargetHandle GLGraphicManager::CreateRenderTarget(RenderTargetDesc desc) {
	gl::GFrameBuffer::Desc gdesc;
	gdesc.width = desc.width;
	gdesc.height = desc.height;
	gdesc.attachs = desc.attachs;
	return pImpl->glw->CreateRenderTarget(gdesc);
}

void GLGraphicManager::UpdateRenderTarget(HwRenderTargetHandle h, RenderTargetDesc desc) {
	gl::GFrameBuffer::Desc gdesc;
	gdesc.width = desc.width;
	gdesc.height = desc.height;
	gdesc.attachs = desc.attachs;
	return pImpl->glw->UpdateRenderTarget(h, gdesc);
}

void GLGraphicManager::DestroyTexture(HwTexHandle h) {
	pImpl->glw->DestroyTexture(h);
}

void GLGraphicManager::DestroyRenderTarget(HwRenderTargetHandle h) {
	pImpl->glw->DestroyRenderTarget(h);
}

void GLGraphicManager::InitializeScene(Scene* scene) {
	using namespace std::placeholders;
	m_scene = scene;

	AddPreParePass();
	TraverseNode(std::bind(&GLGraphicManager::ToFrameGraphPass, this, _1, std::string(SpecTex_Default), 0), scene->sceneGraph.get());
	AddEndPass(*m_fg, *(pImpl->glw), m_fgrscMap.at(std::string(SpecTex_Default)), m_xyflip);
	m_fg->Compile();
}

void GLGraphicManager::Draw() {
	m_scene->paritileSys.Emitt();

	UpdateSceneRenderTargetSize(*m_scene, m_screenSize);
	m_scene->shaderValueUpdater->FrameBegin();

	const auto& cc = m_scene->clearColor;

	auto glw = pImpl->glw.get();


	glw->ClearColor(cc[0], cc[1], cc[2], 1.0f);

	m_fg->Execute(*this);

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

void GLGraphicManager::SetDefaultFbo(uint fbo, uint16_t w, uint16_t h, FillMode fillMode) {
	m_screenSize = {w, h};

	pImpl->glw->SetDefaultFrameBuffer(fbo, w, h);

	//UpdateDefaultRenderTargetBind(*m_scene);
	UpdateCameraForFbo(*m_scene, w, h, fillMode);
}

void GLGraphicManager::ChangeFillMode(FillMode fillMode) {
	if(m_scene == nullptr) return;
		UpdateCameraForFbo(*m_scene, m_screenSize[0], m_screenSize[1], fillMode);
}

void GLGraphicManager::Destroy() {
	m_scene = nullptr;
	m_fg = std::make_unique<fg::FrameGraph>();
	m_shaderMap.clear();
	m_fgrscMap.clear();
	pImpl->glw->ClearAll();
}

void GLGraphicManager::OutGriphviz(std::string_view path) {
	m_fg->ToGraphviz(path);
}

GLGraphicManager::~GLGraphicManager() {
	Destroy();
}
