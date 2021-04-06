#pragma once
#include "GLWrapper.h"
#include "GraphicManager.h"
#include "SceneRenderTarget.h"
#include <unordered_map>
#include <list>
#include <string>

namespace wallpaper
{

class GLRenderTargetManager {
public:
	GLRenderTargetManager(gl::GLWrapper* pGlw):m_pGlw(pGlw) {}
	uint64_t GetID(const SceneRenderTarget& rt) const {
		uint64_t id = 0;	
		id += rt.width;
		id <<= 14;
		id += rt.height;
		id <<= 1;
		id += rt.withDepth;
		return id;
	}
	gl::GLFramebuffer* GetFrameBuffer(const std::string& name, const SceneRenderTarget& rt) {
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
	void ReleaseFrameBuffer(const std::string& name, const SceneRenderTarget& rt) {
		auto id = GetID(rt);
		std::string keystr = name + std::to_string(id);
		if(m_inuse.count(keystr) != 0) {
			m_unuse.push_back({id, m_inuse.at(keystr)});
			m_inuse.erase(keystr);
		}
	}
private:
	gl::GLWrapper* m_pGlw {nullptr};
	std::unordered_map<std::string, gl::GLFramebuffer*> m_inuse;
	std::list<std::pair<uint64_t, gl::GLFramebuffer*>> m_unuse;
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