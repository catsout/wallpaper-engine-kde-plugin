#pragma once
#include "GLWrapper.h"
#include "GraphicManager.h"
#include "SceneRenderTarget.h"
#include <unordered_map>
#include <string>

namespace wallpaper
{

class GLRenderTargetManager {
public:
	GLRenderTargetManager(gl::GLWrapper* pGlw):m_pGlw(pGlw) {}
	gl::GLFramebuffer* GetFrameBuffer(const std::string& name, const SceneRenderTarget& rt) {
		if(rt.allowReuse) {
			uint64_t id = 0;	
			id += rt.width;
			id <<= 14;
			id += rt.height;
			id <<= 1;
			id += rt.withDepth;
			id <<= 8;
			id += rt.index;
			if(m_reuseTargets.count(id) != 0) {
				return m_reuseTargets.at(id);
			} else {
				auto* fb = m_pGlw->CreateFramebuffer(rt.width, rt.height, rt.sample);
				m_reuseTargets[id] = fb;
				return fb;
			}
		} else {
			if(!m_targets.count(name)) {
				auto* fb = m_pGlw->CreateFramebuffer(rt.width, rt.height, rt.sample);
				m_targets[name] = fb;
			}
			return m_targets.at(name);
		}
	}
private:
	gl::GLWrapper* m_pGlw {nullptr};
	std::unordered_map<uint64_t, gl::GLFramebuffer*> m_reuseTargets;
	std::unordered_map<std::string, gl::GLFramebuffer*> m_targets;
};

class GLGraphicManager : public GraphicManager {
public:
	GLGraphicManager():m_glw(),m_rtm(&m_glw) {}
	virtual ~GLGraphicManager() {}
	bool Initialize() override { return false; };
    bool Initialize(void *get_proc_addr(const char*)) override;
    void Destroy() override {}
	void Draw() override;
    void InitializeScene(Scene*) override;
	void SetDefaultFbo(uint fbo, uint32_t w, uint32_t h);
private:
	void LoadNode(SceneNode*);
	void RenderNode(SceneNode*);

	Scene* m_scene;
	gl::GLWrapper m_glw;
	gl::GLFramebuffer m_defaultFbo;
	std::shared_ptr<SceneNode> m_fboNode;
	GLRenderTargetManager m_rtm;
};
}