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
	std::string vsCode = "#version 330 core\n"
		"uniform mat4 g_ModelViewProjectionMatrix;\n"
		"in vec3 a_position;\n"
		"in vec2 a_texCoord;\n"
		"out vec2 TexCoord;\n"
		"void main()\n"
		"{gl_Position = g_ModelViewProjectionMatrix*vec4(a_position, 1.0f);TexCoord = a_texCoord;}";
	std::string fgCode = "#version 330 core\n"
		"in vec2 TexCoord;\n"
		"out vec4 color;\n"
		"uniform sampler2D g_Texture0;\n"
		"void main() {color = texture2D(g_Texture0, TexCoord);}";
	shaderMgr.CreateShader("displayFbo", vsCode, fgCode);
	shaderMgr.CreateLinkedShader("displayFbo");
	fbo_ = std::unique_ptr<gl::GLFramebuffer>(glWrapper.CreateFramebuffer(width, height));
}

void WPRender::UseGlobalFbo() {
	UseGlobalFbo({});
}

void WPRender::UseGlobalFbo(const gl::Shadervalues& shadervalues) {
	glWrapper.BindFramebuffer(fbo_.get());
	glWrapper.Viewport(0, 0, fbo_->width, fbo_->height);
	shaderMgr.BindShader("displayFbo");
	shaderMgr.UpdateUniforms("displayFbo", shadervalues);
}
