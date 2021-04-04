#pragma once
//#include <glad/glad.h>
#include <string>
#include <cstdint>
#include <unordered_map>
#include "common.h"

#include "SceneMesh.h"
#include "SceneShader.h"
#include "Scene.h"
#include "GraphicManager.h"
#include "Type.h"
#include "Image.h"
#include "SceneTexture.h"

namespace wallpaper
{
namespace gl
{

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


struct GLTexture{
	GLTexture(GLint target, uint16_t width, uint16_t height, uint16_t numMips)
		:target(target),
		 w(width),
		 h(height),
		 numMips(numMips) {};
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
	~GLBuffer();
	GLuint buffer = 0;
	GLint usage;
	GLenum target;
	int size;
};


class GLFramebuffer {
public:
	GLFramebuffer();
	GLFramebuffer(int32_t w, int32_t h);

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

class GLWrapper{
public:
	GLWrapper();
	bool Init(void *get_proc_address(const char*));
	GLTexture* CreateTexture(GLenum target, int32_t width, int32_t height, int32_t numMips, SceneTextureSample sample={});
	GLBuffer* CreateBuffer(GLuint target, size_t size, GLuint usage);
	GLFramebuffer *CreateFramebuffer(int32_t width, int32_t height, SceneTextureSample sample={});
	GLProgram* CreateProgram(std::vector<GLShader *> shaders, 
							 std::vector<GLProgram::AttribLoc> attribLocs);
	GLProgram* CreateProgram(std::vector<GLShader *> shaders, 
							 std::vector<ShaderAttribute> attribLocs);

	GLShader* CreateShader(GLuint stage, const std::string& source);
	GLTexture* CopyTexture(GLFramebuffer* fbo);
	void ClearColor(float r, float g, float b, float a);
	void Viewport(int32_t ,int32_t ,int32_t ,int32_t);
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
	void TextureImage(GLTexture *texture, int level, int width, int height, TextureFormat texformat, uint8_t *data, size_t imgsize=0);

	void SetBlend(BlendMode);

	int GetUniforms(GLProgram* program, std::vector<GLUniform>& uniforms);
	void SetUniformF(GLint loc, int count, const float *udata);
	void SetUniformI(GLint loc, int count, const int *udata);
	void SetUniformMat4(GLint loc, const float *udata);
	void SetUniform(GLProgram* program, GLUniform* uniform, const void* value);

	void UpdateUniform(GLProgram* pro, const wallpaper::ShaderValue&);
	void QueryProUniforms(GLProgram* program);
	void SetTexSlot(GLProgram* pro, const std::string& name, int32_t slot);

	// scene
	void LoadMesh(SceneMesh&);
	void RenderMesh(const SceneMesh&);
	void CleanMeshBuf();

	GLuint ToGLType(ShaderType);
	GLenum ToGLType(TextureType);
	GLenum ToGLType(TextureWrap);
	GLenum ToGLType(TextureFilter);

	GLFramebuffer* GetNowFramebuffer();

	std::unordered_map<void*, GLProgram*> programMap;
	std::unordered_map<std::string, std::vector<GLTexture*>> textureMap;

private:
	std::unordered_map<int, int> uniformCount_;	
	GLFramebuffer* m_curFbo;

	std::vector<uint32_t> m_meshBuf;
	std::vector<uint32_t> m_meshVao;

};
}
}
