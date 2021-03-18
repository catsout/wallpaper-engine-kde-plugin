#include "FpsCounter.h"
#include <iostream>

using namespace wallpaper;
//using namespace std::chrono;

FpsCounter::FpsCounter():m_fps(0),m_frameCount(0),m_startTime(0) {};


void FpsCounter::RegisterFrame(uint32_t now) {
	m_frameCount++;
	const int32_t diff = now - m_startTime;		
	if(diff >= 2000 || diff < 0) {
		m_fps = m_frameCount*1000 / diff;
		m_frameCount = 0;
		m_startTime = now;
#if DEBUG_OPENGL
		std::cerr << m_fps << std::endl;
#endif
	}
}

