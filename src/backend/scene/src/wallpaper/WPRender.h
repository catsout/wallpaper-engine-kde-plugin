#pragma once
#include "common.h"
#include "GLWrapper.h"
#include "WPShaderManager.h"
#include "WPTextureCache.h"

namespace wallpaper {

struct CameraParallax {
	bool enable = false;
	float amount;
	float delay;
	float mouseinfluence;
};

class wallpaperGL;
class WPRender {

public:
	WPRender():glWrapper(),
			   shaderMgr(&glWrapper),
			   texCache(&glWrapper),
			   clearcolor_({0.7f,0.7f,0.7f}),
			   m_mouseParallaxVec({0.0f, 0.0f}),
			   m_origin({960, 540}),
			   frametime(0) {};
	~WPRender() {};
	bool Init(void *get_proc_address(const char*));
	void Clear();
	void Clear(float alpha);
	void SetClearcolor(const std::vector<float>& value) {clearcolor_ = value;};
	void CreateGlobalFbo(int width, int height);
	gl::GLFramebuffer* GlobalFbo() {return fbo_.get();};
	const CameraParallax& GetCameraParallax() const {return m_cameraParallax;};
	void SetCameraParallax(const CameraParallax& value) {m_cameraParallax = value;};

	const std::vector<float>& GetMouseParallaxVec() const {return m_mouseParallaxVec;};
	// x, y is at [0,1]
	void GenMouseParallaxVec(float x, float y);

	const auto& Origin() const { return m_origin; }
	void SetOrigin(uint32_t x, uint32_t y) { m_origin = std::vector<uint32_t>{x, y}; }

	gl::GLWrapper glWrapper;
	gl::WPShaderManager shaderMgr;	
	gl::WPTextureCache texCache;

	uint32_t frametime;

//	gl::GLFramebuffer* GlobalFbo;
private:
	std::unique_ptr<gl::GLFramebuffer> fbo_;
	std::vector<float> clearcolor_;
	CameraParallax m_cameraParallax;
	std::vector<float> m_mouseParallaxVec;
	std::vector<uint32_t> m_origin;
};

class Renderable
{
public:
    virtual ~Renderable() {};
    virtual void Load(WPRender&) = 0;
    virtual void Render(WPRender&) = 0;
};

}
