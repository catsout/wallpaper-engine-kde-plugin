#include "FrameTimer.h"
#include <ctime>
#include <iostream>

using namespace wallpaper;

FrameTimer::FrameTimer():m_running(false),
						m_callback([](){}),
						m_fps(15),
						m_condition(),
						m_cvmutex(),
						m_thread(),
						m_clock() {}

FrameTimer::~FrameTimer() {
	Stop();
}

void FrameTimer::SetCallback(const std::function<void()>& func) {
	m_callback = func;
}

void FrameTimer::SetFps(uint8_t value) {
	m_fps = value;
}

void FrameTimer::Run() {
	using namespace std::chrono;
	if(m_running) return;
	m_running = true;
	m_thread = std::thread([this]() {
		m_clock = steady_clock::now();
		while(m_running) {   
			++m_frames;
			m_callback();
			auto idealTime = milliseconds(1000/m_fps);
			auto callTime = steady_clock::now() - m_clock;
			{   
				std::unique_lock<std::mutex> lock(m_cvmutex);
				m_condition.wait_for(lock, idealTime - callTime, [this]() {
					return !m_running;
				}); 
			}   
			if (!m_running) {
				return;
			}   
			m_clock = steady_clock::now();
		}
	});
}

void FrameTimer::Stop() {
	m_running = false;
	m_condition.notify_one();
	if(m_thread.joinable()) {
		m_thread.join();
	}
}

bool FrameTimer::NoFrame() const {
	return m_frames == 0;
}

void FrameTimer::RenderFrame() {
	--m_frames;
}
