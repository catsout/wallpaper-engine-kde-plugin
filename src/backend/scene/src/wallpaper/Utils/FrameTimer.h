#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include <cstdint>
#include <chrono>

namespace wallpaper 
{
class FrameTimer {
public:
	FrameTimer();
	~FrameTimer();
	FrameTimer(const FrameTimer&) = delete;
	FrameTimer(FrameTimer&&) = delete;
	FrameTimer& operator=(const FrameTimer&) = delete;
	FrameTimer& operator=(FrameTimer&&) = delete;

	void Run();
	void Stop();
	void SetCallback(const std::function<void()>&);

	uint8_t Fps() const {return m_fps;};
	void SetFps(uint8_t);
	bool IsRunning() const { return m_running; }
	bool NoFrame() const;
	// only used with one render
	void RenderFrame();
private:
	std::atomic<uint32_t> m_frames;
	uint8_t m_fps;
	std::atomic<bool> m_running;
	std::function<void()> m_callback;
	std::mutex m_cvmutex;
	std::condition_variable m_condition;	
	std::thread m_thread;
	
 	std::chrono::time_point<std::chrono::steady_clock> m_clock;
};

}
