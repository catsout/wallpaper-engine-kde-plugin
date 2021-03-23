#include "SceneIndexArray.h"
#include <cstring>

using namespace wallpaper;

SceneIndexArray::SceneIndexArray(const std::vector<uint32_t>& data):m_count(data.size()) {
	auto dataSize = data.size();
	uint32_t* newdata = new uint32_t[dataSize];
	memcpy(newdata, &data[0], DataSize());
	m_pData = newdata;
};

