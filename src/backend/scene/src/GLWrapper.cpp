#include "GLWrapper.h"
#include <iostream>
using namespace wallpaper::gl;

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

std::string TextureFormat::to_string(TextureFormat::Format format) {
#define Fmt(str) case str: return #str;
	switch(format) {
        Fmt(RGBA8)
		Fmt(BC1)
		Fmt(BC2)
		Fmt(BC3)
		Fmt(A8)
		Fmt(RGB8)
		default:
			LOG_ERROR("Not valied tex format: " + std::to_string((int)format));
			return "";
	}
}

GLFramebuffer::~GLFramebuffer() {
	if (framebuffer)
		glDeleteFramebuffers(1,&framebuffer);
	if (color_texture.texture)
		glDeleteTextures(1,&color_texture.texture);
}

bool GLWrapper::Init(void *get_proc_address(const char *name)) {
    if (!gladLoadGLLoader((GLADloadproc)get_proc_address))
    {
        LOG_ERROR("Failed to initialize GLAD");
    }
	return true;
}


GLTexture* GLWrapper::CreateTexture(GLenum target, int width, int height, int numMips) {
	GLTexture* tex = new GLTexture(target, width, height, numMips);
	glGenTextures(1, &tex->texture);
//	glBindTexture(tex->target, tex->texture);
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

GLFramebuffer* GLWrapper::CreateFramebuffer(int width, int height) {
	GLFramebuffer* fbo = new GLFramebuffer(width, height);
	glGenFramebuffers(1, &fbo->framebuffer);
	GLTexture* tex = CreateTexture(GL_TEXTURE_2D , width, height, 0);
	TextureImage(tex, 0, width, height, TextureFormat::RGBA8, NULL, true, 0);
	fbo->color_texture.texture = tex->texture;
	delete tex;
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color_texture.texture, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	default:
		LOG_ERROR("framebuffer not complite");
		break;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERROR_IF_DEBUG();
	return fbo;
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

void GLWrapper::DeleteFramebuffer(GLFramebuffer *framebuffer) {
	delete framebuffer;
	CHECK_GL_ERROR_IF_DEBUG();
}


void GLWrapper::BufferSubData(GLBuffer* buffer, size_t size, const float* data) {
    glBufferSubData(buffer->target, 0, size, data);
	CHECK_GL_ERROR_IF_DEBUG();
}

void TextureFormat2GLFormat(TextureFormat::Format texformat, GLint& internalFormat, GLenum& format, GLenum& type) {
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

void GLWrapper::TextureImage(GLTexture* texture, int level, int width, int height, TextureFormat::Format texformat, uint8_t *data, bool linearFilter, size_t imgsize) {
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
	
	tex->wrapS = GL_CLAMP_TO_EDGE;
	tex->wrapT = GL_CLAMP_TO_EDGE;
	tex->magFilter = linearFilter ? GL_LINEAR : GL_NEAREST;
	tex->minFilter = linearFilter ? GL_LINEAR : GL_NEAREST;
	glTexParameteri(tex->target, GL_TEXTURE_WRAP_S, tex->wrapS);
	glTexParameteri(tex->target, GL_TEXTURE_WRAP_T, tex->wrapT);
	glTexParameteri(tex->target, GL_TEXTURE_MAG_FILTER, tex->magFilter);
	glTexParameteri(tex->target, GL_TEXTURE_MIN_FILTER, tex->minFilter);
	CHECK_GL_ERROR_IF_DEBUG();
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

GLWrapper::GLWrapper() {
	uniformCount_[GL_FLOAT] = 1;
	uniformCount_[GL_FLOAT_VEC2] = 2;
	uniformCount_[GL_FLOAT_VEC3] = 3;
	uniformCount_[GL_FLOAT_VEC4] = 4;
	uniformCount_[GL_INT] = 1;
	uniformCount_[GL_SAMPLER_2D] = 1;
}
