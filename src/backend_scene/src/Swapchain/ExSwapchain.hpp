#pragma once
#include "TripleSwapchain.hpp"
#include <cstdint>

namespace wallpaper
{

enum class TexTiling
{
    OPTIMAL,
    LINEAR
};

struct ExHandle {
    int         fd;
    int32_t     width;
    int32_t     height;
    std::size_t size;
    // format rgba8

    ExHandle() = default;
    ExHandle(int id): m_id(id) {};

    int32_t id() const { return m_id; }

private:
    int32_t m_id { 0 };
};

// class ExSwapchain : public TripleSwapchain<ExHandle> {};
using ExSwapchain = TripleSwapchain<ExHandle>;
} // namespace wallpaper
