#pragma once
#include <cstdint>
#include <chrono>

namespace wallpaper {

class FpsCounter {
public:
	FpsCounter();
	uint32_t Fps() const { return m_fps; };
	uint32_t FrameCount() const { return m_frameCount; };
	void RegisterFrame();
private:
	uint32_t m_fps;
	uint32_t m_frameCount;
	std::chrono::time_point<std::chrono::steady_clock> m_startTime;
};

}
