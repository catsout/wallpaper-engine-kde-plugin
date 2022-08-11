#include "SceneIndexArray.h"
#include <cstring>

using namespace wallpaper;

SceneIndexArray::SceneIndexArray(std::size_t indexCount)
    : m_size(0), m_capacity(indexCount * 3), m_render_size(0) {
    m_pData = new uint32_t[m_capacity];
    std::memset(m_pData, 0, m_capacity * sizeof(uint32_t));
}
SceneIndexArray::SceneIndexArray(std::span<const uint32_t> data)
    : m_size(data.size()), m_capacity(m_size), m_render_size(0) {
    auto      dataSize = data.size();
    uint32_t* newdata  = new uint32_t[dataSize];
    std::memcpy(newdata, &data[0], DataSizeOf());
    m_pData = newdata;
};

bool SceneIndexArray::IncreaseCheckSet(size_t nsize) {
    if (nsize > CapacitySizeof()) return false;
    if (nsize > DataSizeOf()) {
        m_size = nsize / Unit_Byte_Size + (nsize % Unit_Byte_Size == 0 ? 0 : 1);
    }
    return true;
}
