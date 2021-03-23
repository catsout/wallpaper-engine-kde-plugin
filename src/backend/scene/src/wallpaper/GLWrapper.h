#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include "common.h"

#include "SceneMesh.h"

namespace wallpaper
{
namespace gl
{

class TextureFormat {
public:
	enum Format {
		BC1,  // DXT1
		BC2,  // DXT3
		BC3,  // DXT5
		A8,
		RGB8,
		RGBA8
	};
	static std::string to_string(TextureFormat::Format format);
};

struct GLTexture{
	GLTexture(GLint target, int width, int height, int numMips):target(target),w(width),h(height),numMips(numMips) {};
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
	GLBuffer(GLenum target, size_t size) : target(target), size((int)size) {}
	~GLBuffer() {
		if (buffer) {
			glDeleteBuffers(1, &buffer);
		}
	}
	GLuint buffer = 0;
	GLint usage;
	GLenum target;
	int size;
};


class GLFramebuffer {
public:
	GLFramebuffer():width(0),height(0),color_texture(GL_TEXTURE_2D, width, height, 1) {};
	GLFramebuffer(int _width, int _height)
		: width(_width), height(_height),color_texture(GL_TEXTURE_2D, _width, _height, 1) {}
	~GLFramebuffer();

	GLuint framebuffer = 0;
	GLTexture color_texture;

	int width;
	int height;
};

struct GLUniform {
	char name[256];
	GLenum type;
	int location = -1;
	int count;
};

class GLProgram {
public:
    ~GLProgram() {
		if(program) {
			glDeleteProgram(program);
		}
	}
    struct AttribLoc {
		int location;
		const char* name;
	};

    GLuint program = 0;

	std::vector<AttribLoc> attribLocs_;
};

class GLShader
{
public:
    ~GLShader() {
		if(shader) {
			glDeleteShader(shader);
		}
	}
    GLuint shader = 0;
	std::string source;
};

class GLWrapper{
public:
	GLWrapper();
	bool Init(void *get_proc_address(const char*));
	GLTexture* CreateTexture(GLenum target, int width, int height, int numMips);
	GLBuffer* CreateBuffer(GLuint target, size_t size, GLuint usage);
	GLFramebuffer *CreateFramebuffer(int width, int height);
	GLProgram* CreateProgram(std::vector<GLShader *> shaders, 
							 std::vector<GLProgram::AttribLoc> attribLocs);

	GLShader* CreateShader(GLuint stage, const std::string& source);
	GLTexture* CopyTexture(GLFramebuffer* fbo);
	void Viewport(int ,int ,int ,int);
	void ActiveTexture(int);
	void BindTexture(GLTexture *tex);
	void BindProgram(GLProgram *program);
	void BindFramebuffer(GLFramebuffer* fbo);
	void BindFramebufferViewport(GLFramebuffer* fbo);
	void BindDefaultFramebuffer();
	void BindFramebufferTex(GLFramebuffer* fbo);
	void DeleteBuffer(GLBuffer* buffer);
	void DeleteTexture(GLTexture* texture);
	void DeleteShader(GLShader* shader);
	void DeleteProgram(GLProgram* program);
	void DeleteFramebuffer(GLFramebuffer *framebuffer);

    void BufferSubData(GLBuffer* buffer, size_t size, const float* data);
	void TextureImage(GLTexture *texture, int level, int width, int height, TextureFormat::Format texformat, uint8_t *data, bool linearFilter=false, bool clampEdge=false,size_t imgsize=0);

	int GetUniforms(GLProgram* program, std::vector<GLUniform>& uniforms);
	void SetUniformF(GLint loc, int count, const float *udata);
	void SetUniformI(GLint loc, int count, const int *udata);
	void SetUniformMat4(GLint loc, const float *udata);
	void SetUniform(GLProgram* program, GLUniform* uniform, const void* value);

	// scene
	void LoadMesh(SceneMesh&);
	void RenderMesh(const SceneMesh&);
	void CleanMeshBuf();

	GLFramebuffer* GetNowFramebuffer();
private:
	std::unordered_map<int, int> uniformCount_;	
	GLFramebuffer* m_curFbo;

	std::vector<uint32_t> m_meshBuf;
};
}
}
