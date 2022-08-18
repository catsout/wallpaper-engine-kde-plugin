#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <span>
#include <limits>

#include "Core/NoCopyMove.hpp"
#include "Core/Literals.hpp"

namespace wallpaper
{
class SceneIndexArray : NoCopy {
    constexpr static size_t Unit_Byte_Size { sizeof(uint32_t) };

public:
    SceneIndexArray(usize indexCount);
    SceneIndexArray(std::span<const uint32_t> data);

    SceneIndexArray(SceneIndexArray&&) noexcept;
    ~SceneIndexArray();

    void Assign(usize index, std::span<const uint32_t> data) { AssignSpan(index, data); }
    void AssignHalf(usize index, std::span<const uint16_t> data) { AssignSpan(index, data); }

    // Get
    const uint32_t* Data() const { return m_pData; }
    usize           DataCount() const { return m_size; }
    usize           DataSizeOf() const { return m_size * Unit_Byte_Size; }

    usize RenderDataCount() const noexcept {
        return m_render_size > m_size ? m_size : m_render_size;
    }
    void SetRenderDataCount(usize val) noexcept { m_render_size = val; }

    usize CapacityCount() const { return m_capacity; }
    usize CapacitySizeof() const { return m_capacity * Unit_Byte_Size; }

    uint32_t ID() const { return m_id; }
    void     SetID(uint32_t id) { m_id = id; }

private:
    bool IncreaseCheckSet(size_t size);

    template<typename T>
    void AssignSpan(usize index, std::span<const T> data) {
        using in_value_type = T;
        if (! IncreaseCheckSet((index + data.size()) * sizeof(in_value_type))) return;
        std::copy(data.begin(), data.end(), ((in_value_type*)m_pData) + index);
    }

    uint32_t* m_pData;
    usize     m_size;
    usize     m_capacity;

    usize m_render_size { std::numeric_limits<usize>::max() };

    uint32_t m_id;
};
} // namespace wallpaper
