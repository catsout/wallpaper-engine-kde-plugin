#include "FrameTimer.h"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <numeric>

using namespace wallpaper;
using namespace std::chrono;

FrameTimer::FrameTimer() {
	SetRequiredFps(15);
}

FrameTimer::~FrameTimer() {
	Stop();
}

void FrameTimer::SetCallback(const std::function<void()>& func) {
	m_callback = func;
}

void FrameTimer::FrameBegin() {
	m_clock = steady_clock::now();
}
void FrameTimer::FrameEnd() {
	auto now = steady_clock::now();
	insertFrameTime(duration_cast<microseconds>(now - m_clock));
	calculateFrimetime();
	m_not_end--;
	if(m_not_end.load() < 0) m_not_end.store(0);
}

void FrameTimer::insertFrameTime(std::chrono::microseconds t) {
	m_frametimes[m_frametimes_pos] = t;
	m_frametimes_pos = (m_frametimes_pos+1) % m_frametimes.size();
}


double FrameTimer::FrameTime() const {
	return duration_cast<duration<double>>(m_frametime.load()).count();
}

double FrameTimer::IdeaTime() const {
	auto frametime = m_frametime.load();
	auto ideatime = m_ideatime.load();
	auto time = frametime > ideatime ? frametime : ideatime;
	return duration_cast<duration<double>>(time).count();
}

void FrameTimer::calculateFrimetime() {
	m_frametime.store(
		std::accumulate(m_frametimes.begin(), m_frametimes.end(), duration_cast<microseconds>(0s)) 
		/
		m_frametimes.size());
}

void FrameTimer::SetRequiredFps(uint8_t value) {
	m_required_fps = value;
	microseconds ideatime = milliseconds(1000/m_required_fps);
	m_ideatime = ideatime;
	for(auto& t:m_frametimes) {
		t = ideatime;
	}
	calculateFrimetime();
}

void FrameTimer::Run() {
	if(m_running) return;
	m_running = true;
	m_thread = std::thread([this]() {
		m_clock = steady_clock::now();
		while(m_running) {
			if(m_callback && m_not_end.load() <= 3) {
				m_callback();
				m_not_end++;
			}
			microseconds wait_time = m_frametime.load();
			auto ideatime = m_ideatime.load();
			wait_time = wait_time > ideatime ? wait_time / 2 : ideatime;

			{
				std::unique_lock<std::mutex> lock(m_cvmutex);
				m_condition.wait_for(lock, wait_time, [this]() {
					return !m_running;
				});
			}
			if (!m_running) {
				return;
			}
		}
	});
}

void FrameTimer::Stop() {
	if (!m_running)
		return;
	assert(std::this_thread::get_id() != m_thread.get_id());
	{
		std::unique_lock<std::mutex> lock(m_cvmutex);
		m_running = false;
		m_condition.notify_one();
	}
	if(m_thread.joinable()) {
		m_thread.join();
	}
}