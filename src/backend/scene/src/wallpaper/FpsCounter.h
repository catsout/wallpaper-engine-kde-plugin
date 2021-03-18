#pragma once
#include <cstdint>

namespace wallpaper {

class FpsCounter {
public:
	FpsCounter();
	uint32_t Fps() const { return m_fps; };
	uint32_t FrameCount() const { return m_frameCount; };
	void RegisterFrame(uint32_t now);
private:
	uint32_t m_fps;
	uint32_t m_frameCount;
	uint32_t m_startTime;
};

}
