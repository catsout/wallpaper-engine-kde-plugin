#pragma once
#include "TripleSwapchain.hpp"
#include <cstdint>

namespace wallpaper
{

struct ExHandle {
    int fd;
    std::uint32_t width;
    std::uint32_t height;
    std::size_t size;
    //format rgba8

    ExHandle() = default;
    ExHandle(int id):m_id(id) {};

    int id() const { return m_id; }
private:
    int m_id {0};
};

//class ExSwapchain : public TripleSwapchain<ExHandle> {};
using ExSwapchain = TripleSwapchain<ExHandle>;
}