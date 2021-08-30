#pragma once
#include "Interface/IGraphicManager.h"
#include "Scene/SceneRenderTarget.h"
#include "Fg/FrameGraph.h"
#include <unordered_map>
#include <list>
#include <string>
#include <memory>

namespace wallpaper
{
/*
class GLRenderTargetManager {
public:
	GLRenderTargetManager(const std::shared_ptr<gl::GLWrapper> pGlw):m_pGlw(pGlw) {}
	~GLRenderTargetManager() {
		Clear();
	}
	void Clear();
	uint64_t GetID(const SceneRenderTarget& rt) const;
	gl::GLFramebuffer* GetFrameBuffer(const std::string& name, const SceneRenderTarget& rt);
	void ReleaseFrameBuffer(const std::string& name, const SceneRenderTarget& rt);
	void ReleaseAndDeleteFrameBuffer(const std::string& name, const SceneRenderTarget& rt);
	auto UnusedCount() const { return m_unuse.size(); }
private:
	std::shared_ptr<gl::GLWrapper> m_pGlw {nullptr};
	std::unordered_map<std::string, gl::GLFramebuffer*> m_inuse;
	std::list<std::pair<uint64_t, gl::GLFramebuffer*>> m_unuse;
};
*/

class GLGraphicManager : public IGraphicManager {
public:
	GLGraphicManager();
	virtual ~GLGraphicManager();
	bool Initialize() override { return false; };
    bool Initialize(void *get_proc_addr(const char*)) override;
    void Destroy() override;
	void Draw() override;
    void InitializeScene(Scene*) override;


	HwTexHandle CreateTexture(TextureDesc) override;
	HwTexHandle CreateTexture(const Image&) override; 
	HwRenderTargetHandle CreateRenderTarget(RenderTargetDesc) override;
	void DestroyTexture(HwTexHandle) override;
	void DestroyRenderTarget(HwRenderTargetHandle) override;

	void SetDefaultFbo(uint fbo, uint32_t w, uint32_t h, FillMode FillMode = FillMode::ASPECTCROP);
	void ChangeFillMode(FillMode);
	virtual void SetFlip(bool xflip, bool yflip) override { m_xyflip = {xflip, yflip}; };
private:
	class impl;
    std::unique_ptr<impl> pImpl;

	void ToFrameGraphPass(SceneNode*, std::string output="");

	fg::FrameGraph m_fg;

	Scene* m_scene;
	std::array<bool, 2> m_xyflip {false, false};
	//gl::GLFramebuffer m_defaultFbo;
	std::shared_ptr<SceneNode> m_fboNode;
	std::unordered_map<std::string, fg::FrameGraphMutableResource> m_fgrscMap;

	std::unordered_map<void*, HwShaderHandle> m_shaderMap;
	//std::vector<std::vector<gl::GLTexture*>> m_textureMap;
};
}
