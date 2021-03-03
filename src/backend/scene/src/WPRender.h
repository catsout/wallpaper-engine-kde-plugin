#pragma once
#include "common.h"
#include "GLWrapper.h"
#include "WPShaderManager.h"
#include "WPTextureCache.h"

namespace wallpaper {

class wallpaperGL;
class WPRender {

public:
	WPRender():glWrapper(),shaderMgr(&glWrapper),texCache(&glWrapper),clearcolor_({0.7f,0.7f,0.7f}),timeDiffFrame(0) {};
	~WPRender() {};
	bool Init(void *get_proc_address(const char*));
	void Clear();
	void Clear(float alpha);
	void SetClearcolor(const std::vector<float>& value) {clearcolor_ = value;};
	void CreateGlobalFbo(int width, int height);
	void UseGlobalFbo();
	void UseGlobalFbo(const gl::Shadervalues& shadervalues);
	gl::GLFramebuffer* GlobalFbo() {return fbo_.get();};

	gl::GLWrapper glWrapper;
	gl::WPShaderManager shaderMgr;	
	gl::WPTextureCache texCache;

	int timeDiffFrame;

//	gl::GLFramebuffer* GlobalFbo;
private:
	std::unique_ptr<gl::GLFramebuffer> fbo_;
	std::vector<float> clearcolor_;
};

class Renderable
{
public:
    virtual ~Renderable() {};
    virtual void Load(WPRender&) = 0;
    virtual void Render(WPRender&) = 0;
};

}
