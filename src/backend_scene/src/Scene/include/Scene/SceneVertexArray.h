#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include <string_view>
#include <span>
#include "Core/MapSet.hpp"

#include "Core/Literals.hpp"
#include "Core/NoCopyMove.hpp"
#include "Type.hpp"

namespace wallpaper
{

class SceneVertexArray : NoCopy {
public:
    struct SceneVertexAttribute {
        std::string name;
        VertexType  type;
        bool        padding { true };
    };
    struct SceneVertexAttributeOffset {
        SceneVertexAttribute attr;
        usize                offset;
    };

    SceneVertexArray(const std::vector<SceneVertexAttribute>& attrs, const std::size_t count);
    ~SceneVertexArray();

    SceneVertexArray(SceneVertexArray&&) noexcept;
    SceneVertexArray& operator=(SceneVertexArray&&) noexcept;

    bool AddVertex(const float*);
    bool SetVertex(std::string_view name, std::span<const float> data) noexcept;
    bool SetVertexs(std::size_t index, std::span<const float> data) noexcept;

    bool GetOption(std::string_view) const;
    void SetOption(std::string_view, bool);

    const float* Data() const { return m_pData; }
    usize        DataSize() const { return m_size; }
    usize        DataSizeOf() const { return m_size * sizeof(float); }
    usize        VertexCount() const { return m_size / m_oneSize; }
    usize        CapacitySize() const { return m_capacity; }
    usize        CapacitySizeOf() const { return m_capacity * sizeof(float); }
    usize        OneSize() const { return m_oneSize; }
    usize        OneSizeOf() const { return m_oneSize * sizeof(float); }

    const auto&                                  Attributes() const { return m_attributes; }
    Map<std::string, SceneVertexAttributeOffset> GetAttrOffsetMap() const;

    uint32_t ID() const { return m_id; }
    void     SetID(uint32_t id) { m_id = id; }

    static uint8_t TypeCount(VertexType);
    static uint8_t RealAttributeSize(const SceneVertexAttribute&);

private:
    bool TrySetSize(usize) noexcept;

    std::vector<SceneVertexAttribute> m_attributes;

    Map<std::string, bool> m_options;

    float* m_pData { nullptr };
    usize  m_oneSize { 0 };
    usize  m_size { 0 };
    usize  m_capacity { 0 };

    uint32_t m_id;
};
} // namespace wallpaper
