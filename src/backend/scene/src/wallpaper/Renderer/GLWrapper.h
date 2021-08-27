#pragma once
#include <glad/glad.h> 	

#include <string>
#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <array>

#include "Interface/IGraphicManager.h"
#include "Scene/SceneMesh.h"
#include "Scene/SceneShader.h"
#include "Scene/SceneTexture.h"
#include "Scene/Scene.h"
#include "Type.h"
#include "Image.h"
#include "Handle.h"
#include "Log.h"

#if defined(DEBUG_OPENGL)
#define CHECK_GL_ERROR_IF_DEBUG() checkGlError(__FILE__, __FUNCTION__, __LINE__);
#else
#define CHECK_GL_ERROR_IF_DEBUG()
#endif

namespace wallpaper
{
namespace gl
{

inline void checkGlError(const char* file, const char* func, int line)
{
	int err = glGetError();
	if(err != 0)
		std::cerr << "GL_ERROR: " << err << "  " << func << "  at: " << file << "  line: " << line << std::endl;
}


GLuint ToGLType(ShaderType);
GLenum ToGLType(TextureType);
GLenum ToGLType(TextureWrap);
GLenum ToGLType(TextureFilter);
GLenum ToGLType(MeshPrimitive);


template <typename Getiv, typename GetLog>
std::string GetInfoLog(GLuint name, Getiv getiv, GetLog getLog) {
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

inline void SetUniformF(GLint loc, int count, const float *udata) {
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

inline void SetUniformI(GLint loc, int count, const int *udata) {
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

inline void SetUniformMat4(GLint loc, const float *udata) {
	if (loc >= 0) {
		glUniformMatrix4fv(loc, 1, false, udata);
		CHECK_GL_ERROR_IF_DEBUG();
	}
}

/*
typedef uint32_t GLenum;
typedef unsigned char GLboolean;
typedef uint32_t GLbitfield;
typedef void GLvoid;
typedef int32_t GLint;
typedef uint32_t GLuint;
typedef int32_t GLsizei;
typedef float GLfloat;
typedef double GLdouble;
typedef double GLclampd;
typedef char GLchar;
typedef char GLcharARB;
*/

constexpr uint16_t MaxTexBinding {16};

struct ViewPort {
	uint16_t x {0};
	uint16_t y {0};
	uint16_t width {1};
	uint16_t height {1};
};

struct GContext {
	GLuint defaultFb {0};
	ViewPort viewPort;
};

struct GBindings {
	std::array<HwTexHandle, MaxTexBinding> texs;
};

struct GPass {
	HwRenderTargetHandle target;
	ViewPort viewport;
	HwShaderHandle shader;
	BlendMode blend {BlendMode::Normal};
	std::array<bool, 4> colorMask {true, true, true, true};
};

struct GTexture {
	struct Desc {
		uint16_t w {2};
		uint16_t h {2};
    	uint8_t numMips {0};
		uint16_t numSlots {1};
		uint16_t activeSlot {0};
		GLenum target {0xFFFF};
		TextureFormat format {TextureFormat::RGBA8};
		TextureSample sample;
	};
	std::array<GLuint, 12> gltexs;
	Desc desc;
	static void Init(GTexture& t, const Desc& d) {
		t.desc = d;
		for(uint16_t i=0;i<t.desc.numSlots;i++) {
			auto& gltex = t.gltexs[i];
			glGenTextures(1, &gltex);
			glBindTexture(t.desc.target, gltex);
			glTexParameteri(t.desc.target, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(t.desc.target, GL_TEXTURE_MAX_LEVEL, t.desc.numMips);
			CHECK_GL_ERROR_IF_DEBUG();
			glBindTexture(t.desc.target, 0);
		}
	}
	static void Destroy(GTexture& t) {
		for(uint16_t i=0;i<t.desc.numSlots;i++) {
			glDeleteTextures(1, &t.gltexs[i]);
			t.gltexs[i] = 0;
		}
		CHECK_GL_ERROR_IF_DEBUG();
	}
};

struct GFrameBuffer {
	struct Desc {
		uint16_t width {1};
		uint16_t height {1};
		std::array<HwTexHandle, MaxAttachmentNum> attachs;
	};
	GLuint glfb {0};
	uint16_t width {1};
	uint16_t height {1};
	static void Init(GFrameBuffer& f, const Desc& d) { 
		f.width = d.width;
		f.height = d.width;
		glGenFramebuffers(1, &f.glfb);
	}
	static void Destroy(GFrameBuffer& f) {
		glDeleteFramebuffers(1, &f.glfb);
		CHECK_GL_ERROR_IF_DEBUG();
	}
};


struct GShader {
	struct Desc {
		std::string vs;
		std::string fg;
		struct AttribLoc {
			uint32_t location;
			std::string name;
		};
		std::vector<AttribLoc> attrs;
		std::vector<std::string> texnames;
	};
	struct UniformLoc {
		std::string name;
		int32_t location = -1;
		uint32_t type;
		int32_t count;
	};
	GLuint glpro;
	std::vector<UniformLoc> uniformLocs;

	static int GetUnifLoc(const GShader& s, std::string_view name) {
		for(const auto& u:s.uniformLocs) {
			if(name == u.name)
				return u.location;
		}
		return -1;
	}
	static GLuint Compile(GLenum stage, const std::string& source) {
		GLuint shader = glCreateShader(stage);
		const char* source_char = source.c_str();	
		glShaderSource(shader, 1, &source_char, nullptr);
		glCompileShader(shader);
		GLint success = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			std::string infolog = GetInfoLog(shader, glGetShaderiv, glGetShaderInfoLog);
			LOG_ERROR("COMPILATION_FAILED\n" + infolog);
			LOG_INFO(source);
		}
		CHECK_GL_ERROR_IF_DEBUG();
		return shader;
	}
	static void QueryProUniforms(GShader& s) {
		int n_uniform = 0;
		glGetProgramiv(s.glpro, GL_ACTIVE_UNIFORMS, &n_uniform);
		CHECK_GL_ERROR_IF_DEBUG();
		auto& uniforms = s.uniformLocs;
		uniforms.resize(n_uniform);
		for(int i=0; i<n_uniform ;i++) {
			char* name = new char[256];
			glGetActiveUniform(s.glpro, i, 256, nullptr, &uniforms[i].count, &uniforms[i].type, name);
			uniforms[i].name = std::string(name);
			delete [] name;
			uniforms[i].location = glGetUniformLocation(s.glpro, uniforms[i].name.c_str());
		}
		CHECK_GL_ERROR_IF_DEBUG();
	}
	static void Init(GShader& s, const Desc& desc) {
		s.glpro = glCreateProgram();

		GLuint glvs,glfg;
		glvs = Compile(GL_VERTEX_SHADER, desc.vs);
		glfg = Compile(GL_FRAGMENT_SHADER, desc.fg);
		glAttachShader(s.glpro, glvs);
		glAttachShader(s.glpro, glfg);
		CHECK_GL_ERROR_IF_DEBUG();
		for(auto& attr:desc.attrs) {
			glBindAttribLocation(s.glpro, attr.location, attr.name.c_str());
		}
		CHECK_GL_ERROR_IF_DEBUG();

		glLinkProgram(s.glpro);
		glUseProgram(s.glpro);
		int success;
		glGetProgramiv(s.glpro, GL_LINK_STATUS, &success);
		if(!success) {
			std::string infoLog = GetInfoLog(s.glpro, glGetProgramiv, glGetProgramInfoLog);
			LOG_ERROR("LINKING_FAILED\n" + infoLog);
		}

		QueryProUniforms(s);
		{
			for(int i=0;i<desc.texnames.size();i++) {
				int32_t loc = GetUnifLoc(s, desc.texnames[i]);
				if(loc == -1) continue;
				int slot = i;
				SetUniformI(loc, 1, &slot);
			}
		}

		glUseProgram(0);
		glDeleteShader(glvs);
		glDeleteShader(glfg);
		CHECK_GL_ERROR_IF_DEBUG();
	}
	static void Destroy(GShader& s) {
		glDeleteProgram(s.glpro);
		s.glpro = 0;
	}
};

struct GLTexture{
	GLTexture(GLint target, uint16_t width, uint16_t height, uint16_t numMips);
	GLuint texture = 0;
	uint16_t w;
	uint16_t h;
    uint8_t numMips = 0;	

	GLint target = 0xFFFF;
	GLenum wrapS = 0xFFFF;
	GLenum wrapT = 0xFFFF;
	GLenum magFilter = 0xFFFF;
	GLenum minFilter = 0xFFFF;
};

class GLBuffer {
public:
	GLBuffer(GLenum target, std::size_t size) : target(target), size((int)size) {}
	~GLBuffer();
	GLuint buffer = 0;
	GLint usage;
	GLenum target;
	int size;
};


class GLFramebuffer {
public:
	GLFramebuffer();
	GLFramebuffer(uint32_t w, uint32_t h);

	GLuint framebuffer = 0;
	GLTexture color_texture;

	uint32_t width;
	uint32_t height;
};

struct GLUniform {
	char name[256];
	GLenum type;
	int location = -1;
	int count;
};

class GLProgram {
public:
    ~GLProgram();
    struct AttribLoc {
		int32_t location;
		const char* name;
	};

	struct UniformLoc {
		std::string name;
		int32_t location = -1;
		uint32_t type;
		int32_t count;
	};

    GLuint program = 0;

	std::vector<AttribLoc> attribLocs_;
	std::vector<UniformLoc> uniformLocs;
	static int32_t GetUniformLocation(GLProgram*, const std::string name);
};

class GLShader
{
public:
    ~GLShader();
    GLuint shader = 0;
	std::string source;
};
inline void TextureFormat2GLFormat(TextureFormat texformat, GLint& internalFormat, GLenum& format, GLenum& type) {
	type = GL_UNSIGNED_BYTE;	
	format = GL_RGBA;
	switch(texformat) {
	case TextureFormat::R8:
		internalFormat = GL_R8;
		format = GL_RED;
		break;
	case TextureFormat::RG8:
		internalFormat = GL_RG8;
		format = GL_RG;
		break;
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

class GLWrapper{
public:
	GLWrapper();
	~GLWrapper() = default;

	bool Init(void *get_proc_address(const char*));

	HwTexHandle CreateTexture(const GTexture::Desc& desc, const Image* image = nullptr) {
		HwTexHandle texh = m_texPool.Alloc(desc);
		auto* tex = m_texPool.Lookup(texh);
		if(image != nullptr) {
			tex->desc.numSlots = image->imageDatas.size();
			for(uint16_t i=0;i<image->imageDatas.size();i++) {
				auto target = tex->desc.target;
				glBindTexture(target, tex->gltexs[i]);
				auto& img = image->imageDatas[i];
				for(uint16_t imip=0;imip<img.size();imip++) {
					auto& mip = img[imip];
					GLenum format, type;	
					GLint internalFormat;
					auto texformat = tex->desc.format;
					TextureFormat2GLFormat(texformat, internalFormat, format, type);
					switch(texformat) {
					case TextureFormat::R8:
					case TextureFormat::RG8:
					case TextureFormat::RGB8:
					case TextureFormat::RGBA8:
						glTexImage2D(tex->desc.target, imip, internalFormat, mip.width, mip.height, 0, format, type, mip.data.get());
						break;
					case TextureFormat::BC1:
					case TextureFormat::BC2:
					case TextureFormat::BC3:
						glCompressedTexImage2D(tex->desc.target, imip, internalFormat, mip.width, mip.height, 0, mip.size, mip.data.get());
						break;
					}
					CHECK_GL_ERROR_IF_DEBUG();
				}
				glTexParameteri(target, GL_TEXTURE_WRAP_S, ToGLType(tex->desc.sample.wrapS));
				glTexParameteri(target, GL_TEXTURE_WRAP_T, ToGLType(tex->desc.sample.wrapT));
				glTexParameteri(target, GL_TEXTURE_MAG_FILTER, ToGLType(tex->desc.sample.magFilter));
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, ToGLType(tex->desc.sample.minFilter));
				glBindTexture(target, 0);
			}
		} else {
			tex->desc.numSlots = 1;
			glBindTexture(tex->desc.target, tex->gltexs[0]);
			GLenum format, type;	
			GLint internalFormat;
			auto texformat = tex->desc.format;
			TextureFormat2GLFormat(texformat, internalFormat, format, type);
			auto target = tex->desc.target;
			glTexImage2D(target, 0, internalFormat, tex->desc.w, tex->desc.h, 0, format, type, 0);
    		tex->desc.sample = {TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE,
                                TextureFilter::LINEAR, TextureFilter::LINEAR};
			glTexParameteri(target, GL_TEXTURE_WRAP_S, ToGLType(tex->desc.sample.wrapS));
			glTexParameteri(target, GL_TEXTURE_WRAP_T, ToGLType(tex->desc.sample.wrapT));
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, ToGLType(tex->desc.sample.magFilter));
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, ToGLType(tex->desc.sample.minFilter));
			glBindTexture(target, 0);
		}
		return texh;
	}
	void DestroyTexture(HwTexHandle h) {
		m_texPool.Free(h);
	}
	void UpdateTextureSlot(HwTexHandle h, uint16_t index) {
		auto* img = m_texPool.Lookup(h);
		if(img != nullptr) {
			img->desc.activeSlot = index;
		}
	}
	void CopyTexture(HwTexHandle dst, HwTexHandle src) {
		auto* srcTex = m_texPool.Lookup(src);
		if(srcTex == nullptr) return;
		auto* dstTex = m_texPool.Lookup(dst);
		if(dstTex == nullptr) return;
		GLuint fbo {0};
		auto target = srcTex->desc.target;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			target, srcTex->gltexs[srcTex->desc.activeSlot], 0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindTexture(target, dstTex->gltexs[dstTex->desc.activeSlot]);
		GLenum format, type;	
		GLint internalFormat;
		auto texformat = srcTex->desc.format;
		TextureFormat2GLFormat(texformat, internalFormat, format, type);
		glCopyTexSubImage2D(target, 0, 0, 0, 0, 0, srcTex->desc.w, srcTex->desc.h);
		glBindTexture(target, 0);
		glDeleteFramebuffers(1, &fbo);
		CHECK_GL_ERROR_IF_DEBUG();
	}
	void ApplyBindings(const GBindings& binds) {
		for(uint16_t i=0;i<binds.texs.size();i++) {
			auto* tex = m_texPool.Lookup(binds.texs[i]);
			if(tex != nullptr) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(tex->desc.target, tex->gltexs[tex->desc.activeSlot]);
			}
		}
		CHECK_GL_ERROR_IF_DEBUG();
	}

	HwShaderHandle CreateShader(const GShader::Desc& desc) {
		return m_shaderPool.Alloc(desc);
	}
	void DestroyShader(HwShaderHandle h) {
		m_shaderPool.Free(h);
	}

	void BeginPass(GPass& pass) {
		auto* shader = m_shaderPool.Lookup(pass.shader);
		if(shader != nullptr) glUseProgram(shader->glpro);
		CHECK_GL_ERROR_IF_DEBUG();
		{
			auto* fb = m_fbPool.Lookup(pass.target);
			if(fb != nullptr) {
				glBindFramebuffer(GL_FRAMEBUFFER, fb->glfb);
				auto& v = pass.viewport;
				glViewport(v.x, v.y, v.width, v.height);
			} else {
				glBindFramebuffer(GL_FRAMEBUFFER, m_context.defaultFb);
				auto& v = m_context.viewPort;
				glViewport(v.x, v.y, v.width, v.height);
			}
		}

		SetBlend(pass.blend);
		{
			auto& c = pass.colorMask;
			glColorMask(c[0], c[1], c[2], c[3]);
		}
		glDisable(GL_DEPTH_TEST);
		
	}
	void EndPass(GPass& pass) {
		glUseProgram(0);
	}

	HwRenderTargetHandle CreateRenderTarget(const GFrameBuffer::Desc& desc) {
		auto rtHandle = m_fbPool.Alloc(desc);
		auto* fb = m_fbPool.Lookup(rtHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, fb->glfb);

		for(uint8_t i=0;i<desc.attachs.size();i++) {
			auto* tex = m_texPool.Lookup(desc.attachs[i]);
			if(i == 0 && tex == nullptr)
				assert(false);
			if(tex == nullptr) break;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
				tex->desc.target, tex->gltexs[tex->desc.activeSlot], 0);
		}

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
		glColorMask(true, true, true, true);
		glClearColor(0,0,0,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CHECK_GL_ERROR_IF_DEBUG();
		return rtHandle;
	}
	void DestroyRenderTarget(HwRenderTargetHandle h) {
		m_fbPool.Free(h);
	}

	GLTexture* CreateTexture(GLenum target, uint32_t width, uint32_t height, uint32_t numMips, TextureSample sample={});
	GLBuffer* CreateBuffer(GLuint target, std::size_t size, GLuint usage);
	GLFramebuffer *CreateFramebuffer(uint32_t width, uint32_t height, TextureSample sample={});

	void CopyTexture(GLFramebuffer* src, GLTexture* dst);
	GLTexture* CopyTexture(GLFramebuffer* fbo);
	void ClearColor(float r, float g, float b, float a);
	void Viewport(int32_t ,int32_t ,int32_t ,int32_t);

	void BindTexture(GLTexture *tex);

	void BindFramebuffer(GLFramebuffer* fbo);
	void BindFramebufferViewport(GLFramebuffer* fbo);
	void BindDefaultFramebuffer();
	void BindFramebufferTex(GLFramebuffer* fbo);
	void DeleteBuffer(GLBuffer* buffer);
	void DeleteFramebuffer(GLFramebuffer *framebuffer);

    void BufferSubData(GLBuffer* buffer, std::size_t size, const float* data);
	void TextureImage(GLTexture *texture, int level, int width, int height, TextureFormat texformat, uint8_t *data, std::size_t imgsize=0);
	void TextureImagePbo(GLTexture *texture, int level, int width, int height, TextureFormat texformat, uint8_t *data, std::size_t imgsize);

	void SetBlend(BlendMode);
	std::unordered_set<std::string> GetUniforms(HwShaderHandle h) {
		auto* shader = m_shaderPool.Lookup(h);
		std::unordered_set<std::string> result;
		for(auto& el:shader->uniformLocs) {
			if(!el.name.empty())
				result.insert(el.name);
		}
		return result;
	}

	void UpdateUniform(HwShaderHandle h, const ShaderValue& sv) {
		auto* shader = m_shaderPool.Lookup(h);
		if(shader == nullptr) return;

		int32_t loc = GShader::GetUnifLoc(*shader, sv.name);
		if(loc == -1) return;
		std::size_t size = sv.value.size();
		const float* value = &sv.value[0];
		if(size == 16)
			SetUniformMat4(loc, static_cast<const float*>(value));
		else
			SetUniformF(loc, size, static_cast<const float*>(value));
	}

	void UseShader(HwShaderHandle h, const std::function<void()>& func) {
		auto* shader = m_shaderPool.Lookup(h);
		if(shader == nullptr)  
			glUseProgram(0);
		else 
			glUseProgram(shader->glpro);
		func();
		glUseProgram(0);
	}

	void SetUniform(HwShaderHandle h, GLUniform* uniform, const void* value) {
		auto* shader = m_shaderPool.Lookup(h);
		if(shader == nullptr) return;
		GLint loc = uniform->location;
		if(loc == -1){
			loc = glGetUniformLocation(shader->glpro, uniform->name);
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

	void SetDefaultFrameBuffer(uint hw, uint16_t width, uint16_t height) {
		m_context.defaultFb = hw;
		m_context.viewPort.width = width;
		m_context.viewPort.height = height;
	}

	// scene
	void LoadMesh(SceneMesh&);
	void RenderMesh(const SceneMesh&);
	bool MeshLoaded(const SceneMesh& m) const { return m_vaoMap.count(m.ID()) > 0; }
	void CleanMeshBuf();

	GLFramebuffer* GetNowFramebuffer();

private:
	GContext m_context;	
	std::unordered_map<int, int> uniformCount_;	
	GLFramebuffer* m_curFbo;

	std::unordered_map<uint32_t, uint32_t> m_bufMap;
	std::unordered_map<uint32_t, uint32_t> m_vaoMap;
	uint32_t m_bufidgen {0};
	uint32_t m_vaoidgen {0};

	HandlePool<GTexture, HwTexHandle> m_texPool { HandleMaxNum };
	HandlePool<GShader, HwShaderHandle> m_shaderPool { HandleMaxNum };
	HandlePool<GFrameBuffer, HwRenderTargetHandle> m_fbPool { HandleMaxNum };
};
}
}
