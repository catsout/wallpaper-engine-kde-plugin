#include "GLWrapper.h"
#include <iostream>
//#include "WPTextureCache.h"

#include <functional>
#include <glad/glad.h> 	

using namespace wallpaper::gl;

typedef wallpaper::TextureFormat TextureFormat;
template <typename Getiv, typename GetLog>
static std::string GetInfoLog(GLuint name, Getiv getiv, GetLog getLog) {
	GLint bufLength = 0;
	getiv(name, GL_INFO_LOG_LENGTH, &bufLength);
	if (bufLength <= 0)
		bufLength = 2048;

	std::string infoLog;
	infoLog.resize(bufLength);
	GLsizei len = 0;
	getLog(name, (GLsizei)infoLog.size(), &len, &infoLog[0]);
	if (len <= 0)
		return "(unknown reason)";

	infoLog.resize(len);
	return infoLog;
}

GLBuffer::~GLBuffer() {
	if (buffer) {
		glDeleteBuffers(1, &buffer);
	}
}

GLProgram::~GLProgram() {
	if(program) {
		glDeleteProgram(program);
	}
}

GLShader::~GLShader() {
	if(shader) {
		glDeleteShader(shader);
	}
}

GLFramebuffer::GLFramebuffer():width(0),height(0),color_texture(GL_TEXTURE_2D, width, height, 1) {}
GLFramebuffer::GLFramebuffer(uint32_t _width, uint32_t _height)
		: width(_width), height(_height),color_texture(GL_TEXTURE_2D, _width, _height, 1) {}
/*
GLFramebuffer::~GLFramebuffer() {
	if (framebuffer)
		glDeleteFramebuffers(1,&framebuffer);
	if (color_texture.texture)
		glDeleteTextures(1,&color_texture.texture);
}
*/

bool GLWrapper::Init(void *get_proc_address(const char *name)) {
    if (!gladLoadGLLoader((GLADloadproc)get_proc_address))
    {
        LOG_ERROR("Failed to initialize GLAD");
    }
	return true;
}


GLTexture* GLWrapper::CreateTexture(GLenum target, int32_t width, int32_t height, int32_t numMips, SceneTextureSample sample) {
	GLTexture* tex = new GLTexture(target, width, height, numMips);
	glGenTextures(1, &tex->texture);
	glBindTexture(tex->target, tex->texture);
	glTexParameteri(tex->target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(tex->target, GL_TEXTURE_MAX_LEVEL, numMips);
	CHECK_GL_ERROR_IF_DEBUG();

	tex->wrapS = ToGLType(sample.wrapS);
	tex->wrapT = ToGLType(sample.wrapT);
	tex->magFilter = ToGLType(sample.magFilter);
	tex->minFilter = ToGLType(sample.minFilter);
	glTexParameteri(tex->target, GL_TEXTURE_WRAP_S, tex->wrapS);
	glTexParameteri(tex->target, GL_TEXTURE_WRAP_T, tex->wrapT);
	glTexParameteri(tex->target, GL_TEXTURE_MAG_FILTER, tex->magFilter);
	glTexParameteri(tex->target, GL_TEXTURE_MIN_FILTER, tex->minFilter);

	glBindTexture(tex->target, 0);
	CHECK_GL_ERROR_IF_DEBUG();
	return tex;
}

GLBuffer* GLWrapper::CreateBuffer(GLenum target, size_t size, GLuint usage) {
	GLBuffer* buffer = new GLBuffer(target, size);
	buffer->size = (int)size;
	buffer->usage = usage;
	glGenBuffers(1, &buffer->buffer);
	glBindBuffer(buffer->target, buffer->buffer);
	glBufferData(buffer->target, buffer->size, nullptr, buffer->usage);
	return buffer;
}


GLFramebuffer* GLWrapper::CreateFramebuffer(int32_t width, int32_t height, SceneTextureSample sample) {
	GLFramebuffer* fbo = new GLFramebuffer(width, height);
	glGenFramebuffers(1, &fbo->framebuffer);
	GLTexture* tex = CreateTexture(GL_TEXTURE_2D , width, height, 0, sample);
	TextureImage(tex, 0, width, height, TextureFormat::RGBA8, NULL, 0);
	fbo->color_texture.texture = tex->texture;
	delete tex;
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color_texture.texture, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		LOG_ERROR("GL_FRAMEBUFFER_UNSUPPORTED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		LOG_ERROR("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		break;
	default:
		LOG_ERROR("framebuffer not complite " + std::to_string(status));
		break;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERROR_IF_DEBUG();
	m_curFbo = nullptr;
	return fbo;
}

GLProgram* GLWrapper::CreateProgram(std::vector<GLShader *> shaders, 
						 std::vector<ShaderAttribute> attribLocs) {
	GLProgram* program = new GLProgram();
	program->program = glCreateProgram();
	for(auto& shader:shaders) {
		glAttachShader(program->program, shader->shader);
		CHECK_GL_ERROR_IF_DEBUG();
	}
	for(auto& attrib:attribLocs) {
		glBindAttribLocation(program->program, attrib.location, attrib.name.c_str());
		CHECK_GL_ERROR_IF_DEBUG();
	}

	glLinkProgram(program->program);
	glUseProgram(program->program);
	int success;
    glGetProgramiv(program->program, GL_LINK_STATUS, &success);
    if(!success) {
        std::string infoLog = GetInfoLog(program->program, glGetProgramiv, glGetProgramInfoLog);
        LOG_ERROR("LINKING_FAILED\n" + infoLog);
    }
	for(auto& shader:shaders) {
		DeleteShader(shader);
	}
	CHECK_GL_ERROR_IF_DEBUG();
    return program;
}

GLProgram* GLWrapper::CreateProgram(std::vector<GLShader *> shaders,
									std::vector<GLProgram::AttribLoc> attribLocs) {
	GLProgram* program = new GLProgram();
	program->program = glCreateProgram();
	for(auto& shader:shaders) {
		glAttachShader(program->program, shader->shader);
		CHECK_GL_ERROR_IF_DEBUG();
	}
	for(auto& attrib:attribLocs) {
		glBindAttribLocation(program->program, attrib.location, attrib.name);
		CHECK_GL_ERROR_IF_DEBUG();
	}

	glLinkProgram(program->program);
	glUseProgram(program->program);
	int success;
    glGetProgramiv(program->program, GL_LINK_STATUS, &success);
    if(!success) {
        std::string infoLog = GetInfoLog(program->program, glGetProgramiv, glGetProgramInfoLog);
        LOG_ERROR("LINKING_FAILED\n" + infoLog);
    }
	CHECK_GL_ERROR_IF_DEBUG();
    return program;
}

GLShader* GLWrapper::CreateShader(GLuint stage, const std::string& source) {
	GLShader* shader = new GLShader();
	shader->shader = glCreateShader(stage);
	const char* source_char = source.c_str();	
	glShaderSource(shader->shader, 1, &source_char, nullptr);
    glCompileShader(shader->shader);
	CHECK_GL_ERROR_IF_DEBUG();
	GLint success = 0;
	glGetShaderiv(shader->shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        std::string infolog = GetInfoLog(shader->shader, glGetShaderiv, glGetShaderInfoLog);
        LOG_ERROR("COMPILATION_FAILED\n" + infolog);
    }
	CHECK_GL_ERROR_IF_DEBUG();
	return shader;
}


GLTexture* GLWrapper::CopyTexture(GLFramebuffer* fbo) {
	GLFramebuffer* backfbo = m_curFbo;
	BindFramebuffer(fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	int width = fbo->width,height = fbo->height;
	GLTexture* tex = CreateTexture(GL_TEXTURE_2D , width, height, 0);
	TextureImage(tex, 0, width, height, TextureFormat::RGBA8, NULL, 0);
	BindTexture(tex);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
//	BindFramebuffer(backfbo);
	CHECK_GL_ERROR_IF_DEBUG();
	return tex;
}

void GLWrapper::ClearColor(float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::Viewport(int x,int y,int w,int h) {
	glViewport(x, y, w, h);
	CHECK_GL_ERROR_IF_DEBUG();
}
void GLWrapper::ActiveTexture(int index) {
	glActiveTexture(GL_TEXTURE0 + index);
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::BindTexture(GLTexture *tex) {
	glBindTexture(tex->target, tex->texture);
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::BindProgram(GLProgram *program) {
	glUseProgram(program->program);
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::BindFramebuffer(GLFramebuffer* fbo) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->framebuffer);
	CHECK_GL_ERROR_IF_DEBUG();
	m_curFbo = fbo;
}

void GLWrapper::BindFramebufferViewport(GLFramebuffer* fbo) {
	BindFramebuffer(fbo);
	Viewport(0,0, fbo->width, fbo->height);
}

void GLWrapper::BindFramebufferTex(GLFramebuffer* fbo) {
	BindTexture(&fbo->color_texture);
}

void GLWrapper::BindDefaultFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::DeleteBuffer(GLBuffer* buffer) {
	delete buffer;
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::DeleteTexture(GLTexture* texture) {
	glDeleteTextures(1, &texture->texture);
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::DeleteShader(GLShader* shader) {
	delete shader;
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::DeleteProgram(GLProgram* program) {
	delete program;
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::DeleteFramebuffer(GLFramebuffer *glfbo) {
	if(glfbo == nullptr)
		return;
	if (glfbo->framebuffer)
		glDeleteFramebuffers(1, &glfbo->framebuffer);
	if (glfbo->color_texture.texture)
		glDeleteTextures(1, &glfbo->color_texture.texture);
	CHECK_GL_ERROR_IF_DEBUG();
}


void GLWrapper::BufferSubData(GLBuffer* buffer, size_t size, const float* data) {
    glBufferSubData(buffer->target, 0, size, data);
	CHECK_GL_ERROR_IF_DEBUG();
}

void TextureFormat2GLFormat(TextureFormat texformat, GLint& internalFormat, GLenum& format, GLenum& type) {
	type = GL_UNSIGNED_BYTE;	
	format = GL_RGBA;
	switch(texformat) {
	case TextureFormat::RGB8:
		internalFormat = GL_RGB8;	
		format = GL_RGB;
		break;
	case TextureFormat::RGBA8:
		internalFormat = GL_RGBA8;
		break;
	case TextureFormat::BC1:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case TextureFormat::BC2:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case TextureFormat::BC3:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	}
}

void GLWrapper::TextureImage(GLTexture* texture, int level, int width, int height, TextureFormat texformat, uint8_t *data, size_t imgsize) {
	GLTexture *tex = texture;
	BindTexture(texture);

	GLenum format, type;	
	GLint internalFormat;
	TextureFormat2GLFormat(texformat, internalFormat, format, type);
	switch(texformat) {
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
		glTexImage2D(tex->target, level, internalFormat, width, height, 0, format, type, data);
		break;
	case TextureFormat::BC1:
	case TextureFormat::BC2:
	case TextureFormat::BC3:
		glCompressedTexImage2D(tex->target, level, internalFormat, width, height, 0, imgsize, data);
		break;
	}
	CHECK_GL_ERROR_IF_DEBUG();
}

void GLWrapper::QueryProUniforms(GLProgram* program) {
	int n_uniform = 0;
	glUseProgram(program->program);
	glGetProgramiv(program->program, GL_ACTIVE_UNIFORMS, &n_uniform);
	CHECK_GL_ERROR_IF_DEBUG();
	auto& uniforms = program->uniformLocs;
	uniforms.resize(n_uniform);
	for(int i=0; i<n_uniform ;i++) {
		char* name = new char[256];
		glGetActiveUniform(program->program, i, 256, nullptr, &uniforms[i].count, &uniforms[i].type, name);
		uniforms[i].name = std::string(name);
		delete [] name;
		uniforms[i].location = glGetUniformLocation(program->program, uniforms[i].name.c_str());
		CHECK_GL_ERROR_IF_DEBUG();
	}
}
void GLWrapper::SetBlend(BlendMode bm) {
	if(bm != BlendMode::Disable)
		glEnable(GL_BLEND);
	switch (bm)
	{
	case BlendMode::Disable:
		glDisable(GL_BLEND);
		break;
	case BlendMode::Normal:
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		break;
	case BlendMode::Additive:
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	}
}

int GLWrapper::GetUniforms(GLProgram* program, std::vector<GLUniform>& uniforms) {
	int n_uniform = 0;
	glUseProgram(program->program);
	glGetProgramiv(program->program, GL_ACTIVE_UNIFORMS, &n_uniform);
	CHECK_GL_ERROR_IF_DEBUG();
	uniforms.resize(n_uniform);
	for(int i=0; i<n_uniform ;i++) {
		uniforms[i] = GLUniform();
		glGetActiveUniform(program->program, i, 256, nullptr, &uniforms[i].count, &uniforms[i].type, uniforms[i].name);
		uniforms[i].location = glGetUniformLocation(program->program, uniforms[i].name);
		CHECK_GL_ERROR_IF_DEBUG();
	}
	return n_uniform;
}

void GLWrapper::SetUniformF(GLint loc, int count, const float *udata) {
	if (loc >= 0) {
		switch (count) {
		case 1:
			glUniform1f(loc, udata[0]);
			break;
		case 2:
			glUniform2fv(loc, 1, udata);
			break;
		case 3:
			glUniform3fv(loc, 1, udata);
			break;
		case 4:
			glUniform4fv(loc, 1, udata);
			break;
		}
		CHECK_GL_ERROR_IF_DEBUG();
	}
}

void GLWrapper::SetUniformI(GLint loc, int count, const int *udata) {
	if (loc >= 0) {
		switch (count) {
		case 1:
			glUniform1iv(loc, 1, (GLint *)udata);
			break;
		case 2:
			glUniform2iv(loc, 1, (GLint *)udata);
			break;
		case 3:
			glUniform3iv(loc, 1, (GLint *)udata);
			break;
		case 4:
			glUniform4iv(loc, 1, (GLint *)udata);
			break;
		}
		CHECK_GL_ERROR_IF_DEBUG();
	}
}

void GLWrapper::SetUniformMat4(GLint loc, const float *udata) {
	if (loc >= 0) {
		glUniformMatrix4fv(loc, 1, false, udata);
		CHECK_GL_ERROR_IF_DEBUG();
	}
}


void GLWrapper::SetUniform(GLProgram* program, GLUniform* uniform,const void* value) {
	GLint loc = uniform->location;
	if(loc == -1){
		loc = glGetUniformLocation(program->program, uniform->name);
		uniform->location = loc;
	}

	int count = uniformCount_[uniform->type];

    switch(uniform->type)
    {   
	case GL_FLOAT:
	case GL_FLOAT_VEC2:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC4:
        SetUniformF(loc, count, static_cast<const float*>(value));
        break;
    case GL_INT:
	case GL_SAMPLER_2D:
        SetUniformI(loc, count, static_cast<const int*>(value));
		break;
	case GL_FLOAT_MAT4:
		SetUniformMat4(loc, static_cast<const float*>(value));
        break;
    }   
}

void GLWrapper::LoadMesh(SceneMesh& mesh) {
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

	GLuint bufId;
	auto verCount = mesh.VertexCount();
	// vertices
	for(size_t i=0;i < verCount;i++) {
		const auto& vArray = mesh.GetVertexArray(i);

		glGenBuffers(1, &bufId);
		glBindBuffer(GL_ARRAY_BUFFER, bufId);
		glBufferData(GL_ARRAY_BUFFER, vArray.DataSize(), vArray.Data(), GL_STATIC_DRAW);
		CHECK_GL_ERROR_IF_DEBUG();

		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, vArray.TypeCount(), GL_FLOAT, GL_FALSE, 0, 0);
		CHECK_GL_ERROR_IF_DEBUG();
		m_meshBuf.push_back(bufId);
	}

	auto indexCount = mesh.IndexCount();
	for(size_t i=0;i < indexCount;i++) {
		const auto& iArray = mesh.GetIndexArray(i);
		glGenBuffers(1, &bufId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufId);
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iArray.DataSize(), iArray.Data(), GL_STATIC_DRAW);
		CHECK_GL_ERROR_IF_DEBUG();
		m_meshBuf.push_back(bufId);
		// current only use one index
		break;
	}
    glBindVertexArray(0);
	m_meshVao.push_back(mesh.vao);
}

void GLWrapper::RenderMesh(const SceneMesh& mesh) {
	glBindVertexArray(mesh.vao);
	auto count = mesh.GetIndexArray(0).DataCount();
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
	CHECK_GL_ERROR_IF_DEBUG();
	glBindVertexArray(0);
}

void GLWrapper::CleanMeshBuf() {
	glBindVertexArray(0);
	for(auto& v:m_meshVao) {
		if(v != 0)
			glDeleteVertexArrays(1, &v);
	}
	for(auto& v:m_meshBuf) {
		if(v != 0)
			glDeleteBuffers(1, &v);
	}
	CHECK_GL_ERROR_IF_DEBUG();
	m_meshVao.clear();
	m_meshBuf.clear();
}


GLFramebuffer* GLWrapper::GetNowFramebuffer() {
	return m_curFbo;
}

GLWrapper::GLWrapper() {
	uniformCount_[GL_FLOAT] = 1;
	uniformCount_[GL_FLOAT_VEC2] = 2;
	uniformCount_[GL_FLOAT_VEC3] = 3;
	uniformCount_[GL_FLOAT_VEC4] = 4;
	uniformCount_[GL_INT] = 1;
	uniformCount_[GL_SAMPLER_2D] = 1;
}

namespace wp = wallpaper;

int32_t GLProgram::GetUniformLocation(GLProgram* pro, const std::string name) {
	for(const auto& u:pro->uniformLocs) {
		if(name == u.name)
			return u.location;
	}
	return -1;
}

void GLWrapper::UpdateUniform(GLProgram* pro, const wp::ShaderValue& sv) {
	int32_t loc = GLProgram::GetUniformLocation(pro, sv.name);
	if(loc == -1) return;
	size_t size = sv.value.size();
	const float* value = &sv.value[0];
	if(size == 16)
		SetUniformMat4(loc, static_cast<const float*>(value));
	else
		SetUniformF(loc, size, static_cast<const float*>(value));
}

void GLWrapper::SetTexSlot(GLProgram* pro, const std::string& name, int32_t slot) {
	int32_t loc = GLProgram::GetUniformLocation(pro, name);
	if(loc == -1) return;
	SetUniformI(loc, 1, &slot);
}


GLuint GLWrapper::ToGLType(ShaderType st) {
	switch (st)
	{
	case ShaderType::VERTEX:
		return GL_VERTEX_SHADER;
	case ShaderType::FRAGMENT:
		return GL_FRAGMENT_SHADER;
	}
	LOG_ERROR("wrong shadertype");
	return 0;
}

GLenum GLWrapper::ToGLType(TextureType tt) {
	switch (tt)
	{
	case TextureType::IMG_2D:
		return GL_TEXTURE_2D;
	default:
		return GL_TEXTURE_2D;
	}
	
}

GLenum GLWrapper::ToGLType(TextureWrap wrap) {
	switch (wrap)
	{
	case TextureWrap::REPEAT:
		return GL_REPEAT;
	case TextureWrap::CLAMP_TO_EDGE:
		return GL_CLAMP_TO_EDGE;
	default:
		return GL_REPEAT;
	}
}
GLenum GLWrapper::ToGLType(TextureFilter filter) {
	switch (filter)
	{
	case TextureFilter::LINEAR:
		return GL_LINEAR; 
	case TextureFilter::NEAREST:
		return GL_NEAREST;
	default:
		return GL_NEAREST;
	}
}
