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
    void cmd();
    uint genExTexture(wallpaper::ExHandle&);
    
    Span<std::uint8_t> uuid() const;
private:
	class impl;
    std::unique_ptr<impl> pImpl;

    bool inited {false};
};