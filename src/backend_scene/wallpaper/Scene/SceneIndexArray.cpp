#include "SceneIndexArray.h"
#include <cstring>

using namespace wallpaper;

SceneIndexArray::SceneIndexArray(std::size_t indexCount):m_capacity(indexCount*3),m_size(0) {
	m_pData = new uint32_t[m_capacity];
	std::memset(m_pData, 0, m_capacity*sizeof(uint32_t));
}
SceneIndexArray::SceneIndexArray(Span<uint32_t> data):m_size(data.size()),m_capacity(m_size) {
	auto dataSize = data.size();
	uint32_t* newdata = new uint32_t[dataSize];
	std::memcpy(newdata, &data[0], DataSizeOf());
	m_pData = newdata;
};


void SceneIndexArray::Assign(std::size_t index, const uint32_t* data, std::size_t count) {
	if(index+count > m_capacity) return;
	if(index + count > m_size) m_size = index + count; 
	std::memcpy(m_pData+index, data, count * sizeof(uint32_t));	
}