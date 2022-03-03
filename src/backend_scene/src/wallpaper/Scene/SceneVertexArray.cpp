#include "SceneVertexArray.h"
#include <cstring>
#include <iostream>

using namespace wallpaper;

uint8_t SceneVertexArray::TypeCount(VertexType t) {
	uint8_t type = (uint8_t)t;
	uint8_t first = (uint8_t)VertexType::FLOAT1;
	return (type - first + 1);
}

uint8_t SceneVertexArray::RealAttributeSize(const SceneVertexArray::SceneVertexAttribute& attr) {
	return attr.padding ? 4 : TypeCount(attr.type);
}

SceneVertexArray::SceneVertexArray(const std::vector<SceneVertexAttribute>& attrs, const std::size_t count):
m_attributes(attrs) {
	for(const auto& el:m_attributes) {
		std::size_t size = SceneVertexArray::RealAttributeSize(el);
		m_oneSize += size;
	}
	m_capacity = m_oneSize * count;
	m_pData = new float[m_capacity];
	std::memset(m_pData, 0, m_capacity * sizeof(float));
}

SceneVertexArray::~SceneVertexArray() {
	if(m_pData != nullptr)
		delete[] m_pData;
}

bool SceneVertexArray::AddVertex(const float* data) {
	if(m_size + m_oneSize >= m_capacity) return false;
	std::size_t pos = 0;
	std::size_t mpos = 0;
	float* mData = m_pData + m_size;
	for(const auto& el:m_attributes) {
		auto typeSize = SceneVertexArray::TypeCount(el.type);
		std::memcpy(mData + mpos, data+pos, typeSize * sizeof(float));
		pos += typeSize;
		mpos += SceneVertexArray::RealAttributeSize(el);
	}
	m_size += m_oneSize;
	return true;
}

bool SceneVertexArray::SetVertex(const std::string& name, const std::vector<float>& data) {
	uint32_t offset = 0;
	for(const auto& el:m_attributes) {
		if(el.name == name) {
			uint32_t typeSize = SceneVertexArray::TypeCount(el.type);
			uint32_t count = data.size() / typeSize; 
			if(m_capacity < count*m_oneSize) return false;
			if(m_size < count*m_oneSize) m_size = count * m_oneSize;

			for(uint32_t i=0;i<data.size();i+=typeSize) {
				uint32_t num = i / typeSize;
				std::memcpy(m_pData + offset + num*m_oneSize, &data[i], typeSize * sizeof(float));
			}
			return true;
		} else offset += RealAttributeSize(el);
	}
	return false;
}


bool SceneVertexArray::SetVertexs(std::size_t index, std::size_t count, const float* data) {
	auto size = (index + count)*m_oneSize;
	if(size > m_capacity) return false;
	if(size > m_size) m_size = size;
	std::memcpy(m_pData + index*m_oneSize, data, count*m_oneSize*sizeof(float));
	return true;
}

std::vector<SceneVertexArray::SceneVertexAttributeOffset> SceneVertexArray::GetAttrOffset() const {
	std::vector<SceneVertexAttributeOffset> attr_offet;
	uint offset {0};
	for(const auto& attr:m_attributes) {
		attr_offet.push_back(SceneVertexAttributeOffset{
			.attr = attr,
			.offset = offset
		});
		offset += SceneVertexArray::RealAttributeSize(attr);
	}
	return attr_offet;
}