#pragma once
#include <cstdint>
#include <chrono>
#include <functional>

#include "Core/Literals.hpp"

namespace wallpaper
{

class FpsCounter {
public:
    FpsCounter();
    u32  Fps() const { return m_fps; };
    u32  FrameCount() const { return m_frameCount; };
    void RegisterFrame();

private:
    u32                                                m_fps;
    u32                                                m_frameCount;
    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
};

} // namespace wallpaper
