#include "SceneIndexArray.h"
#include <cstring>
#include <utility>

using namespace wallpaper;

SceneIndexArray::SceneIndexArray(std::size_t indexCount): m_size(0), m_capacity(indexCount * 3) {
    m_pData = new uint32_t[m_capacity];
    std::memset(m_pData, 0, m_capacity * sizeof(uint32_t));
}
SceneIndexArray::SceneIndexArray(std::span<const uint32_t> data)
    : m_size(data.size()), m_capacity(m_size) {
    auto      dataSize = data.size();
    uint32_t* newdata  = new uint32_t[dataSize];
    std::memcpy(newdata, &data[0], DataSizeOf());
    m_pData = newdata;
};
SceneIndexArray::SceneIndexArray(SceneIndexArray&& o) noexcept
    : m_pData(std::exchange(o.m_pData, nullptr)),
      m_size(o.m_size),
      m_capacity(o.m_capacity),
      m_render_size(o.m_render_size),
      m_id(o.m_id) {}

SceneIndexArray::~SceneIndexArray() {
    if (m_pData != nullptr) delete[] m_pData;
}

bool SceneIndexArray::IncreaseCheckSet(size_t nsize) {
    if (nsize > CapacitySizeof()) return false;
    if (nsize > DataSizeOf()) {
        m_size = nsize / Unit_Byte_Size + (nsize % Unit_Byte_Size == 0 ? 0 : 1);
    }
    return true;
}
