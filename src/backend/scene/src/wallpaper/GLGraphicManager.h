#pragma once
#include "GLWrapper.h"
#include "GraphicManager.h"
#include "SceneRenderTarget.h"
#include <unordered_map>
#include <list>
#include <string>
#include <memory>

namespace wallpaper
{

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

class GLGraphicManager : public GraphicManager {
public:
	GLGraphicManager():m_glw(new gl::GLWrapper()),m_rtm(m_glw) {}
	virtual ~GLGraphicManager();
	bool Initialize() override { return false; };
    bool Initialize(void *get_proc_addr(const char*)) override;
    void Destroy() override;
	void Draw() override;
    void InitializeScene(Scene*) override;
	void SetDefaultFbo(uint fbo, uint32_t w, uint32_t h, FillMode FillMode = FillMode::ASPECTCROP);
	void ChangeFillMode(FillMode);
private:
	void LoadNode(SceneNode*);
	void RenderNode(SceneNode*);

	Scene* m_scene;
	std::shared_ptr<gl::GLWrapper> m_glw;
	gl::GLFramebuffer m_defaultFbo;
	std::shared_ptr<SceneNode> m_fboNode;
	GLRenderTargetManager m_rtm;

	std::unordered_map<void*, gl::GLProgram*> m_programMap;
	std::unordered_map<std::string, std::vector<gl::GLTexture*>> m_textureMap;
};
}
