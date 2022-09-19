#pragma once

#include "Core/Literals.hpp"
#include "Core/NoCopyMove.hpp"

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

#include <functional>
#include <chrono>

namespace wallpaper
{

class ThreadTimer : NoCopy, NoMove {
public:
    ThreadTimer(std::function<void()> callback);
    ~ThreadTimer();

    void Start();
    void Stop();

    bool Running() const;

    void SetInterval(std::chrono::microseconds);

private:
    std::function<void()> m_callback;

    std::mutex m_op_mutex;

    std::thread             m_timer_thread;
    std::mutex              m_cond_mutex;
    std::condition_variable m_condition;

    // init
    std::atomic<std::chrono::microseconds> m_interval;
    std::atomic<bool>                      m_running;
};

} // namespace wallpaper
