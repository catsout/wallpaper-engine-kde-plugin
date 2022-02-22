#include "GLWrapper.h"
#include <iostream>
#include <functional>

#include "Scene/SceneMesh.h"
#include "Scene/SceneShader.h"

using namespace wallpaper::gl;


bool GLWrapper::Init(void *get_proc_address(const char *)) {
	if (!gladLoadGLLoader((GLADloadproc)get_proc_address))
	{
		LOG_ERROR("Failed to initialize GLAD");
		return false;
	}
	GFrameBuffer::Init(m_clearFb, {});
	GLuint mem;
	/*
	glCreateMemoryObjectsEXT(1, &mem);
	CHECK_GL_ERROR_IF_DEBUG();
	glImportMemoryFdEXT(mem, // mem object
						0, // image size
						GL_HANDLE_TYPE_OPAQUE_FD_EXT, // handle type
						0); // file descriptor
	CHECK_GL_ERROR_IF_DEBUG();
	*/
	return true;
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

/*
void GLWrapper::TextureImage(GLTexture* texture, int level, int width, int height, TextureFormat texformat, uint8_t *data, std::size_t imgsize) {
	GLTexture *tex = texture;
	BindTexture(texture);

	GLenum format, type;	
	GLint internalFormat;
	TextureFormat2GLFormat(texformat, internalFormat, format, type);
	switch(texformat) {
	case TextureFormat::R8:
	case TextureFormat::RG8:
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
	glFlush();
	glBindTexture(tex->target, 0);
}


void GLWrapper::TextureImagePbo(GLTexture *texture, int level, int width, int height, TextureFormat texformat, uint8_t *data, std::size_t imgsize) {
	GLTexture *tex = texture;
	GLuint pbo;
	std::size_t bufferSize = imgsize;
	if(texformat == TextureFormat::R8 || texformat == TextureFormat::RG8) bufferSize *= 2;
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
	void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	CHECK_GL_ERROR_IF_DEBUG();
	if(ptr == nullptr) {
		LOG_ERROR("Can't map pbo buffer");
		return;
	}
	memcpy(ptr, data, imgsize);
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	CHECK_GL_ERROR_IF_DEBUG();
	BindTexture(texture);

	GLenum format, type;
	GLint internalFormat;
	TextureFormat2GLFormat(texformat, internalFormat, format, type);
	switch(texformat) {
	case TextureFormat::R8:
	case TextureFormat::RG8:
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
		glTexImage2D(tex->target, level, internalFormat, width, height, 0, format, type, nullptr);
		break;
	case TextureFormat::BC1:
	case TextureFormat::BC2:
	case TextureFormat::BC3:
		glCompressedTexImage2D(tex->target, level, internalFormat, width, height, 0, imgsize, nullptr);
		break;
	}
	CHECK_GL_ERROR_IF_DEBUG();
	glBindTexture(tex->target, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glDeleteBuffers(1, &pbo);
	CHECK_GL_ERROR_IF_DEBUG();
}*/

void GLWrapper::SetBlend(BlendMode bm) {
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);	
	CHECK_GL_ERROR_IF_DEBUG();
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	if(bm != BlendMode::Disable)
		glEnable(GL_BLEND);
	switch (bm)
	{
	case BlendMode::Disable:
		glDisable(GL_BLEND);
		break;
	case BlendMode::Normal:
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
		break;
	case BlendMode::Translucent:
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendMode::Additive:
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_SRC_ALPHA, GL_ONE);
		break;
	}
}

void GLWrapper::UpdateUniform(HwShaderHandle h, const ShaderValue& sv) {
	auto* shader = m_shaderPool.Lookup(h);
	if(shader == nullptr) return;

	int32_t loc = GShader::GetUnifLoc(*shader, sv.name);
	if(loc == -1) {
		return;
	}
	std::size_t size = sv.value.size();
	const float* value = &sv.value[0];
	if(size == 16)
		SetUniformMat4(loc, static_cast<const float*>(value));
	else if(size == 9)
		SetUniformMat3(loc, static_cast<const float*>(value));
	else
		SetUniformF(loc, size, static_cast<const float*>(value));
}


void GLWrapper::LoadMesh(SceneMesh& mesh) {
	uint32_t vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint bufId;
	auto verCount = mesh.VertexCount();
	// vertices
	for(std::size_t i=0;i < verCount;i++) {
		auto& vArray = mesh.GetVertexArray(i);

		glGenBuffers(1, &bufId);
		glBindBuffer(GL_ARRAY_BUFFER, bufId);
		glBufferData(GL_ARRAY_BUFFER, vArray.CapacitySizeOf(), vArray.Data(), mesh.Dynamic()? GL_DYNAMIC_DRAW :GL_STATIC_DRAW);
		CHECK_GL_ERROR_IF_DEBUG();

		const auto& attrs = vArray.Attributes();
		intptr_t offset = 0;
		for(std::size_t j=0;j<attrs.size();j++) {
			glEnableVertexAttribArray(j);
			const auto& a = attrs.at(j);
			uint32_t size = SceneVertexArray::TypeCount(a.type);
			glVertexAttribPointer(j, size, GL_FLOAT, GL_FALSE, vArray.OneSizeOf(), (const void*)offset);
			offset += SceneVertexArray::RealAttributeSize(a) * sizeof(float);
		}
		CHECK_GL_ERROR_IF_DEBUG();
		vArray.SetID(m_bufidgen++);
		m_bufMap.insert({vArray.ID(), bufId});
	}

	auto indexCount = mesh.IndexCount();
	for(std::size_t i=0;i < indexCount;i++) {
		auto& iArray = mesh.GetIndexArray(i);
		glGenBuffers(1, &bufId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, iArray.CapacitySizeof(), iArray.Data(), mesh.Dynamic()? GL_DYNAMIC_DRAW :GL_STATIC_DRAW);
		CHECK_GL_ERROR_IF_DEBUG();
		iArray.SetID(m_bufidgen++);
		m_bufMap.insert({iArray.ID(), bufId});
		// current only use one index
		break;
	}
	mesh.SetID(m_vaoidgen++);
	m_vaoMap.insert({mesh.ID(), vao});	
	//LOG_INFO("load mesh:" + std::to_string(mesh.ID()) + ":" + std::to_string(vao));
    glBindVertexArray(0);
}

void GLWrapper::RenderMesh(const SceneMesh& mesh) {
	if(m_vaoMap.count(mesh.ID()) == 0) {
		assert(false);
		return;
	}

	if(mesh.Dynamic() && mesh.Dirty()) {
		const auto& varray = mesh.GetVertexArray(0);
		const auto& iarray = mesh.GetIndexArray(0);
		if(varray.CapacitySizeOf() > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, m_bufMap.at(varray.ID()));
			void *old_data = glMapBufferRange(GL_ARRAY_BUFFER, 0, varray.CapacitySizeOf(),
			GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
			CHECK_GL_ERROR_IF_DEBUG();
			std::memcpy(old_data, varray.Data(), varray.DataSizeOf());
			glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, varray.DataSizeOf());
			glUnmapBuffer(GL_ARRAY_BUFFER);
			CHECK_GL_ERROR_IF_DEBUG();
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufMap.at(iarray.ID()));
			old_data = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, iarray.CapacitySizeof(),
			GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
			CHECK_GL_ERROR_IF_DEBUG();
			std::memcpy(old_data, iarray.Data(), iarray.DataSizeOf());
			glFlushMappedBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, iarray.DataSizeOf());
			glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		} else return;
	}

	glBindVertexArray(m_vaoMap.at(mesh.ID()));
	//LOG_INFO(std::to_string(mesh.ID()) + ":" + std::to_string(m_vaoMap.at(mesh.ID())));
	if(mesh.Primitive() == MeshPrimitive::POINT) {
		auto count = mesh.GetVertexArray(0).VertexCount();
		glPointSize(mesh.PointSize());
		glDrawArrays(GL_POINTS, 0, count);
	} else  {
		auto count = mesh.GetIndexArray(0).DataCount();
		glDrawElements(ToGLType(mesh.Primitive()), count, GL_UNSIGNED_INT, nullptr);
	}
	CHECK_GL_ERROR_IF_DEBUG();
	glBindVertexArray(0);
}

void GLWrapper::CleanMeshBuf() {
	glBindVertexArray(0);
	for(auto& v:m_vaoMap) {
		if(v.second != 0)
			glDeleteVertexArrays(1, &v.second);
	}
	for(auto& v:m_bufMap) {
		if(v.second != 0)
			glDeleteBuffers(1, &v.second);
	}
	CHECK_GL_ERROR_IF_DEBUG();
	m_vaoMap.clear();
	m_vaoidgen = 0;
	m_bufMap.clear();
	m_bufidgen = 0;
}

bool GLWrapper::MeshLoaded(const SceneMesh& m) const { return m_vaoMap.count(m.ID()) > 0; }


GLWrapper::GLWrapper() {
	uniformCount_[GL_FLOAT] = 1;
	uniformCount_[GL_FLOAT_VEC2] = 2;
	uniformCount_[GL_FLOAT_VEC3] = 3;
	uniformCount_[GL_FLOAT_VEC4] = 4;
	uniformCount_[GL_INT] = 1;
	uniformCount_[GL_SAMPLER_2D] = 1;
}

namespace wp = wallpaper;


GLuint wallpaper::gl::ToGLType(ShaderType st) {
	switch (st)
	{
	case ShaderType::VERTEX:
		return GL_VERTEX_SHADER;
	case ShaderType::GEOMETRY:
		return GL_GEOMETRY_SHADER;
	case ShaderType::FRAGMENT:
		return GL_FRAGMENT_SHADER;
	}
	LOG_ERROR("wrong shadertype");
	return 0;
}

GLenum wallpaper::gl::ToGLType(TextureType tt) {
	switch (tt)
	{
	case TextureType::IMG_2D:
		return GL_TEXTURE_2D;
	default:
		return GL_TEXTURE_2D;
	}
	
}

GLenum wallpaper::gl::ToGLType(TextureWrap wrap) {
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
GLenum wallpaper::gl::ToGLType(TextureFilter filter) {
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
GLenum wallpaper::gl::ToGLType(MeshPrimitive p) {
	switch (p)
	{
	case MeshPrimitive::POINT:
		return GL_POINTS;
	case MeshPrimitive::TRIANGLE:
		return GL_TRIANGLES;
	default:
		return GL_TRIANGLES;
	}
}
