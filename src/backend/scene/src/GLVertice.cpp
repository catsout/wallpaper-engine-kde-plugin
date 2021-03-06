#include <numeric>
#include <sstream>
#include "GLVertice.h"
#include "common.h"
#include "wallpaper.h"
#include "pkg.h"

using namespace wallpaper::gl;

void Bind(int VAO) {
    glBindVertexArray(VAO);
	CHECK_GL_ERROR_IF_DEBUG();
}

VerticeArray VerticeArray::GenVerticeArray(GLWrapper* glWrapper, const std::vector<float>& pts, std::vector<int> format) {
	VerticeArray va = VerticeArray();
    glGenVertexArrays(1, &va.VAO_);
	Bind(va.VAO_);
	CHECK_GL_ERROR_IF_DEBUG();
	va.vertices = pts;
	va.format = format;
	va.glWrapper_ = glWrapper;
	va.buffer_ = glWrapper->CreateBuffer(GL_ARRAY_BUFFER, pts.size()*sizeof(float), GL_STATIC_DRAW);
	Bind(0);
	return va;
}

VerticeArray VerticeArray::GenDefault(GLWrapper* glWrapper) {
    std::vector<float> vertices = {
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
     1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f
    };
	return GenVerticeArray(glWrapper, vertices, {3, 2});
}

VerticeArray VerticeArray::GenSizedBox(GLWrapper* glWrapper, std::vector<float> size) {
	float left = -size[0]/2.0f;
	float right = size[0]/2.0f;
	float bottom = -size[1]/2.0f;
	float top = size[1]/2.0f;
	float z = 0.0f;
	std::vector<float> vertices = {
		 left, bottom, z,  0.0f, 0.0f,
		 right, bottom, z,  1.0f, 0.0f,
		 right,  top, z,  1.0f, 1.0f,
		 right,  top, z,  1.0f, 1.0f,
		 left,  top, z,  0.0f, 1.0f,
		 left, bottom, z,  0.0f, 0.0f
    };
	return GenVerticeArray(glWrapper, vertices, {3, 2});
}

VerticeArray VerticeArray::GenWPImage(GLWrapper* glWrapper, std::vector<float> origin, std::vector<float> size) {
	float left = origin[0] - size[0]/2.0;
	float right = origin[0] + size[0]/2.0;
	float bottom = origin[1] - size[1]/2.0;
	float top = origin[1] + size[1]/2.0;
	float z = origin[2];
	std::vector<float> vertices = {
		 left, bottom, z,  0.0f, 0.0f,
		 right, bottom, z,  1.0f, 0.0f,
		 right,  top, z,  1.0f, 1.0f,
		 right,  top, z,  1.0f, 1.0f,
		 left,  top, z,  0.0f, 1.0f,
		 left, bottom, z,  0.0f, 0.0f
    };
	return GenVerticeArray(glWrapper, vertices, {3, 2});
}

void VerticeArray::Update() {
	Bind(VAO_);
    glWrapper_->BufferSubData(buffer_, vertices.size()*sizeof(float), &vertices[0]);

    int vertice_size = std::accumulate(format.begin(), format.end(), 0);
	vertice_size_ = vertice_size;
	int i = 0, point = 0;
    for(auto& f: format) {
        glVertexAttribPointer(i, f, GL_FLOAT, GL_FALSE, vertice_size * sizeof(GLfloat), (GLvoid*)(point* sizeof(GLfloat)));
        glEnableVertexAttribArray(i);
		CHECK_GL_ERROR_IF_DEBUG();
        point += f;
        i++;
    }
    Bind(0);
}

void VerticeArray::Draw() const {
	Bind(VAO_);
	glDrawArrays(GL_TRIANGLES, 0, 6);//vertices.size()/vertice_size_);
	CHECK_GL_ERROR_IF_DEBUG();
    Bind(0);
}

void VerticeArray::Delete() {
	if(VAO_ != -1) {
		Bind(0);
		glDeleteVertexArrays(1, &VAO_);
		CHECK_GL_ERROR_IF_DEBUG();
		glWrapper_->DeleteBuffer(buffer_);
        VAO_ = -1;
	}
}
