#include "FpsCounter.h"
#include <iostream>
#include <chrono>

using namespace wallpaper;
using namespace std::chrono;

FpsCounter::FpsCounter(): m_fps(0), m_frameCount(0), m_startTime(steady_clock::now()) {};

constexpr seconds timeout { 2s };

void FpsCounter::RegisterFrame() {
    auto now  = steady_clock::now();
    auto diff = now - m_startTime;

    m_frameCount++;
    if (diff > timeout) {
        m_fps        = (u32)(m_frameCount / duration<double>(diff).count());
        m_frameCount = 0;
        m_startTime  = now;
        std::cerr << m_fps << std::endl;
    }
}
