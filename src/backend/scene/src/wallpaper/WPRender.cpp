#include "WPRender.h"
using namespace wallpaper;

bool WPRender::Init(void *get_proc_address(const char*)) {
	return glWrapper.Init(get_proc_address);
}

void WPRender::Clear() {
	Clear(1.0f);
}

void WPRender::Clear(float alpha) {
	glClearColor(clearcolor_[0], clearcolor_[1], clearcolor_[2], alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECK_GL_ERROR_IF_DEBUG();
}

void WPRender::CreateGlobalFbo(int width, int height) {
	fbo_ = std::unique_ptr<gl::GLFramebuffer>(glWrapper.CreateFramebuffer(width, height));
}

void WPRender::GenMouseParallaxVec(float x, float y) {
	// *2.0f to -1,1
	float w = (x - 0.5f) * 2.0f * m_cameraParallax.mouseinfluence * m_origin[0];
	float h = (y - 0.5f) * 2.0f * m_cameraParallax.mouseinfluence * m_origin[1];
	m_mouseParallaxVec = std::vector<float>({w, h});
}
