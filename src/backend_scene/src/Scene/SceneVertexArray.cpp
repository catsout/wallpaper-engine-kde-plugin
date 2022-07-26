#include "SceneVertexArray.h"
#include <cstring>
#include <iostream>

using namespace wallpaper;

uint8_t SceneVertexArray::TypeCount(VertexType t) {
    switch (t) {
    case VertexType::FLOAT1:
    case VertexType::UINT1: return 1;
    case VertexType::FLOAT2:
    case VertexType::UINT2: return 2;
    case VertexType::FLOAT3:
    case VertexType::UINT3: return 3;
    case VertexType::FLOAT4:
    case VertexType::UINT4: return 4;
    }
    return 1;
}

uint8_t SceneVertexArray::RealAttributeSize(const SceneVertexArray::SceneVertexAttribute& attr) {
    return attr.padding ? 4 : TypeCount(attr.type);
}

SceneVertexArray::SceneVertexArray(const std::vector<SceneVertexAttribute>& attrs,
                                   const std::size_t                        count)
    : m_attributes(attrs) {
    for (const auto& el : m_attributes) {
        std::size_t size = SceneVertexArray::RealAttributeSize(el);
        m_oneSize += size;
    }
    m_capacity = m_oneSize * count;
    m_pData    = new float[m_capacity];
    std::memset(m_pData, 0, m_capacity * sizeof(float));
}

SceneVertexArray::~SceneVertexArray() {
    if (m_pData != nullptr) delete[] m_pData;
}

bool SceneVertexArray::AddVertex(const float* data) {
    if (m_size + m_oneSize >= m_capacity) return false;
    std::size_t pos   = 0;
    std::size_t mpos  = 0;
    float*      mData = m_pData + m_size;
    for (const auto& el : m_attributes) {
        auto typeSize = SceneVertexArray::TypeCount(el.type);
        std::memcpy(mData + mpos, data + pos, typeSize * sizeof(float));
        pos += typeSize;
        mpos += SceneVertexArray::RealAttributeSize(el);
    }
    m_size += m_oneSize;
    return true;
}

bool SceneVertexArray::SetVertex(std::string_view name, Span<const float> data) {
    uint32_t offset = 0;
    for (const auto& el : m_attributes) {
        if (el.name == name) {
            usize typeSize = SceneVertexArray::TypeCount(el.type);
            usize count    = data.size() / typeSize;
            if (m_capacity < count * m_oneSize) return false;
            if (m_size < count * m_oneSize) m_size = count * m_oneSize;

            for (usize i = 0; i < data.size(); i += typeSize) {
                usize num = i / typeSize;
                std::memcpy(m_pData + offset + num * m_oneSize, &data[i], typeSize * sizeof(float));
            }
            return true;
        } else
            offset += RealAttributeSize(el);
    }
    return false;
}

bool SceneVertexArray::SetVertexs(std::size_t index, std::size_t count, const float* data) {
    auto size = (index + count) * m_oneSize;
    if (size > m_capacity) return false;
    if (size > m_size) m_size = size;
    std::memcpy(m_pData + index * m_oneSize, data, count * m_oneSize * sizeof(float));
    return true;
}

Map<std::string, SceneVertexArray::SceneVertexAttributeOffset>
SceneVertexArray::GetAttrOffsetMap() const {
    Map<std::string, SceneVertexArray::SceneVertexAttributeOffset> result;
    usize                                                          offset { 0 };
    for (const auto& attr : m_attributes) {
        result[attr.name] = (SceneVertexAttributeOffset { .attr = attr, .offset = offset });
        offset += SceneVertexArray::RealAttributeSize(attr) * sizeof(float);
    }
    return result;
}

bool SceneVertexArray::GetOption(std::string_view name) const {
    return exists(m_options, name) && m_options.at(std::string(name));
}
void SceneVertexArray::SetOption(std::string_view name, bool value) {
    m_options[std::string(name)] = value;
}
