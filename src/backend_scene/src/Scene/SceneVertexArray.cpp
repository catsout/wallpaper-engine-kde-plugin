#include "SceneVertexArray.h"
#include <cstring>
#include <iostream>
#include <utility>
#include <algorithm>
#include <cassert>

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
                                   const usize                              count)
    : m_attributes(attrs) {
    for (const auto& el : m_attributes) {
        usize size = SceneVertexArray::RealAttributeSize(el);
        m_oneSize += size;
    }
    m_capacity = m_oneSize * count;
    m_pData    = new float[m_capacity];
    std::fill(m_pData, m_pData + m_capacity, 0.0f);
}

SceneVertexArray::~SceneVertexArray() {
    if (m_pData != nullptr) delete[] m_pData;
}
SceneVertexArray::SceneVertexArray(SceneVertexArray&& o) noexcept
    : m_attributes(o.m_attributes),
      m_pData(std::exchange(o.m_pData, nullptr)),
      m_oneSize(o.m_oneSize),
      m_size(o.m_size),
      m_capacity(o.m_capacity),
      m_id(o.m_id) {}

SceneVertexArray& SceneVertexArray::operator=(SceneVertexArray&& o) noexcept {
    m_attributes = o.m_attributes;
    m_pData      = std::exchange(o.m_pData, nullptr);
    m_oneSize    = o.m_oneSize;
    m_size       = o.m_size;
    m_capacity   = o.m_capacity;
    m_id         = o.m_id;
    return *this;
}

bool SceneVertexArray::AddVertex(const float* data) {
    if (m_size + m_oneSize >= m_capacity) return false;
    usize  pos   = 0;
    usize  mpos  = 0;
    float* mData = m_pData + m_size;
    for (const auto& el : m_attributes) {
        auto typeSize = SceneVertexArray::TypeCount(el.type);
        std::copy(data + pos, data + pos + typeSize, m_pData + mpos);
        pos += typeSize;
        mpos += SceneVertexArray::RealAttributeSize(el);
    }
    m_size += m_oneSize;
    return true;
}

bool SceneVertexArray::SetVertex(std::string_view name, std::span<const float> data) noexcept {
    u32 offset = 0;
    for (const auto& el : m_attributes) {
        if (el.name == name) {
            usize typeSize = SceneVertexArray::TypeCount(el.type);
            usize count    = data.size() / typeSize;
            if (! TrySetSize(count * m_oneSize)) return false;

            for (usize i = 0; i < data.size(); i += typeSize) {
                auto  start = data.begin() + (isize)i;
                usize num   = i / typeSize;
                std::copy(start, start + (isize)typeSize, m_pData + offset + num * m_oneSize);
            }
            return true;
        } else
            offset += RealAttributeSize(el);
    }
    return false;
}

bool SceneVertexArray::SetVertexs(usize index, std::span<const float> data) noexcept {
    usize start = index * m_oneSize;
    if (TrySetSize(start + data.size())) {
        std::copy(data.begin(), data.end(), m_pData + start);
        return true;
    }
    return false;
}

bool SceneVertexArray::TrySetSize(usize new_size) noexcept {
    assert(new_size <= m_capacity);
    if (new_size > m_capacity) {
        return false;
    }
    if (new_size > m_size) m_size = new_size;
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
