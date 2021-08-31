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
	void ClearTexture(HwTexHandle thandle, std::array<float, 4> clearcolors) override;
	HwRenderTargetHandle CreateRenderTarget(RenderTargetDesc) override;
	void DestroyTexture(HwTexHandle) override;
	void DestroyRenderTarget(HwRenderTargetHandle) override;

	void SetDefaultFbo(uint fbo, uint16_t w, uint16_t h, FillMode FillMode = FillMode::ASPECTCROP);
	void ChangeFillMode(FillMode);
	virtual void SetFlip(bool xflip, bool yflip) override { m_xyflip = {xflip, yflip}; };
private:
	class impl;
    std::unique_ptr<impl> pImpl;

	void AddCopyCmdPasses(const std::string& dst, const std::string src);
	void AddPreParePass();
	void ToFrameGraphPass(SceneNode*, std::string output="");
	std::unique_ptr<fg::FrameGraph> m_fg;
	Scene* m_scene;
	std::array<bool, 2> m_xyflip {false, false};
	std::array<uint16_t, 2> m_screenSize {1920, 1080};

	std::unordered_map<std::string, fg::FrameGraphMutableResource> m_fgrscMap;

	std::unordered_map<void*, HwShaderHandle> m_shaderMap;
};
}
