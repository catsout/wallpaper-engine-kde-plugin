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
			   m_cameraParallaxVec({0.0f, 0.0f}),
			   timeDiffFrame(0) {};
	~WPRender() {};
	bool Init(void *get_proc_address(const char*));
	void Clear();
	void Clear(float alpha);
	void SetClearcolor(const std::vector<float>& value) {clearcolor_ = value;};
	void CreateGlobalFbo(int width, int height);
	void UseGlobalFbo();
	void UseGlobalFbo(const gl::Shadervalues& shadervalues);
	gl::GLFramebuffer* GlobalFbo() {return fbo_.get();};
	const CameraParallax& GetCameraParallax() const {return m_cameraParallax;};
	void SetCameraParallax(const CameraParallax& value) {m_cameraParallax = value;};
	const std::vector<float>& GetCameraParallaxVec() const {return m_cameraParallaxVec;};
	// x, y is at [0,1]
	void GenCameraParallaxVec(float x, float y);

	gl::GLWrapper glWrapper;
	gl::WPShaderManager shaderMgr;	
	gl::WPTextureCache texCache;

	int timeDiffFrame;

//	gl::GLFramebuffer* GlobalFbo;
private:
	std::unique_ptr<gl::GLFramebuffer> fbo_;
	std::vector<float> clearcolor_;
	CameraParallax m_cameraParallax;
	std::vector<float> m_cameraParallaxVec;
};

class Renderable
{
public:
    virtual ~Renderable() {};
    virtual void Load(WPRender&) = 0;
    virtual void Render(WPRender&) = 0;
};

}
