#include "SceneVertexArray.h"
#include <cstring>
#include <iostream>

using namespace wallpaper;

SceneVertexArray::SceneVertexArray(const char* attr, const std::vector<float>& data, const VertexType type):m_attribute(attr), m_count(data.size()), m_type(type) {
	auto dataSize = data.size();
	float* newdata = new float[dataSize];
	memcpy(newdata, &data[0], DataSize());
	m_pData = newdata;
};
