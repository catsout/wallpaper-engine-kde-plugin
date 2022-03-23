#pragma once
#include <memory>
#include <array>
#include <cstdint>

#include "span.hpp"
#include "ExSwapchain.hpp"

class GlExtra {
public:
    GlExtra();
    ~GlExtra();
    bool init(void *get_proc_address(const char *));
    uint genExTexture(wallpaper::ExHandle&);
    void deleteTexture(uint);
    
    Span<std::uint8_t> uuid() const;
    wallpaper::TexTiling tiling() const;
private:
	class impl;
    std::unique_ptr<impl> pImpl;

    uint semaphore {0};

    wallpaper::TexTiling m_tiling {wallpaper::TexTiling::OPTIMAL};
    bool inited {false};
};