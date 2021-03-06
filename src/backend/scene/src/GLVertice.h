#pragma once
#include <vector>
#include <list>
#include <string>
#include "common.h"
#include "GLWrapper.h"

namespace wallpaper
{
namespace gl
{

struct Vertice {
	float x;
	float y;
	float z;
	
	float u;
	float v;
};

class VerticeArray
{
public:
	VerticeArray():glWrapper_(nullptr),buffer_(nullptr),VAO_(-1) {};
	~VerticeArray() {};
    std::vector<float> vertices;
    std::vector<int> format;
    void Update();
    void Delete();
	void Draw() const;

	static VerticeArray GenVerticeArray(GLWrapper* glWrapper, const std::vector<float>& pts, std::vector<int> format);
	static VerticeArray GenDefault(GLWrapper* glWrapper);
	static VerticeArray GenWPImage(GLWrapper* glWrapper, std::vector<float> origin, std::vector<float> size);
	static VerticeArray GenSizedBox(GLWrapper* glWrapper, std::vector<float> size);
private:
	GLWrapper* glWrapper_;
    unsigned int VAO_;
	GLBuffer* buffer_;
	int vertice_size_;
};
}
}
