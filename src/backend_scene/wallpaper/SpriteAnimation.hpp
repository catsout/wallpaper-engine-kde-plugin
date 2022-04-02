#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <cstdint>
#include <array>

namespace wallpaper
{
struct SpriteFrame {
    uint32_t imageId { 0 };
    float    frametime { 0 };
    float    x { 0 };
    float    y { 0 };
    float    width { 1 };
    float    height { 1 };
    float    rate { 1 }; // real h / w

    std::array<float, 2> xAxis { 1, 0 };
    std::array<float, 2> yAxis { 0, 1 };
};

class SpriteAnimation {
public:
    const auto& GetAnimateFrame(double newtime) {
        if ((m_remainTime -= newtime) < 0.0f) {
            SwitchToNext();
            const auto& frame = m_frames.at(m_curFrame);
            m_remainTime      = frame.frametime;
        }
        const auto& frame = m_frames.at(m_curFrame);
        return frame;
    }
    const auto& GetCurFrame() const { return m_frames.at(m_curFrame); }
    void        AppendFrame(const SpriteFrame& frame) { m_frames.push_back(frame); }

    uint32_t numFrames() const { return m_frames.size(); }

private:
    void SwitchToNext() {
        if (m_curFrame >= m_frames.size() - 1)
            m_curFrame = 0;
        else
            m_curFrame++;
    }
    int32_t m_curFrame { 0 };
    double  m_remainTime { 0 };

    std::vector<SpriteFrame> m_frames;
};
} // namespace wallpaper
