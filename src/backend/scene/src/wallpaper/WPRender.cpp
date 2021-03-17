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
	std::string vsCode = "#version 150\n"
		"uniform mat4 fboTrans;\n"
		"attribute vec3 a_position;\n"
		"attribute vec2 a_texCoord;\n"
		"varying vec2 TexCoord;\n"
		"void main()\n"
		"{gl_Position = fboTrans*vec4(a_position, 1.0f);TexCoord = a_texCoord;}";
	std::string fgCode = "#version 150\n"
		"varying vec2 TexCoord;\n"
		"uniform sampler2D g_Texture0;\n"
		"void main() {gl_FragColor = texture2D(g_Texture0, TexCoord);}";
	shaderMgr.CreateShader("displayFbo", vsCode, fgCode);
	shaderMgr.CreateLinkedShader("displayFbo");
	fbo_ = std::unique_ptr<gl::GLFramebuffer>(glWrapper.CreateFramebuffer(width, height));
}

void WPRender::UseGlobalFbo() {
	UseGlobalFbo({});
}

void WPRender::UseGlobalFbo(const gl::Shadervalues& shadervalues) {
	glWrapper.BindFramebufferViewport(fbo_.get());
	shaderMgr.BindShader("displayFbo");
	shaderMgr.UpdateUniforms("displayFbo", shadervalues);
}
		
void WPRender::GenCameraParallaxVec(float x, float y) {
	float w = (0.5f - x) * 2.0f * m_cameraParallax.amount * m_cameraParallax.mouseinfluence;
	float h = (0.5f - y) * 2.0f * m_cameraParallax.amount * m_cameraParallax.mouseinfluence;
	m_cameraParallaxVec = std::vector<float>({w, h});
}
