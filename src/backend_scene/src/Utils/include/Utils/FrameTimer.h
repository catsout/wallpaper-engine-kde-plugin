#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include <cstdint>
#include <chrono>

#include "Core/NoCopyMove.hpp"

namespace wallpaper 
{
class FrameTimer : NoCopy,NoMove {
	constexpr static size_t FRAMETIME_SIZE {5};
public:
	FrameTimer();
	~FrameTimer();

	void Run();
	void Stop();
	void SetCallback(const std::function<void()>&);

	uint8_t RequiredFps() const {return m_required_fps;};
	bool IsRunning() const { return m_running; }
	double FrameTime() const;
	double IdeaTime() const;

	void SetRequiredFps(uint8_t);

	// only used with one render
	void FrameBegin();
	void FrameEnd();
private:
	using micros = std::chrono::microseconds;
	void calculateFrimetime();
	void insertFrameTime(micros t);

	uint8_t m_required_fps;
	std::atomic<bool> m_running {false};

	std::function<void()> m_callback;
	std::mutex m_cvmutex;
	std::condition_variable m_condition;	
	std::thread m_thread;

	std::array<micros, FRAMETIME_SIZE> m_frametimes;
	size_t m_frametimes_pos {0};

	std::atomic<micros> m_frametime;
	std::atomic<micros> m_ideatime;

	std::atomic<int> m_not_end {0};

	// out of time thread	
 	std::chrono::time_point<std::chrono::steady_clock> m_clock;
};

}
