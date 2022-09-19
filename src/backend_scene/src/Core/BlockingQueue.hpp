#pragma once

#include "NoCopyMove.hpp"
#include "Literals.hpp"

#include <queue>
#include <mutex>
#include <condition_variable>

namespace wallpaper
{

template<typename T>
class BlockingQueue : NoCopy, NoMove {
public:
    using value_type      = T;
    using size_type       = std::size_t;
    using reference       = T&;
    using const_reference = const T&;

    using lock_type = std::unique_lock<std::mutex>;

    constexpr static usize DEF_CAPACITY { 20 };

    BlockingQueue(const usize cap = DEF_CAPACITY): m_capacity(cap) {}
    ~BlockingQueue() = default;

    void full() const {
        lock_type lock(m_op_mtx);
        return full_();
    }

    void empty() const {
        lock_type lock(m_op_mtx);
        return empty_();
    }

    void size() const {
        lock_type lock(m_op_mtx);
        return size_();
    }

    void push(const value_type& item) {
        lock_type lock(m_op_mtx);
        while (full_()) {
            m_cond_not_full.wait(lock);
        }
        m_queue.push(item);
        m_cond_not_empty.notify_all();
    }
    // void push(value_type&& item) { push(std::move(item)); }

    T pop() {
        lock_type lock(m_op_mtx);
        while (empty_()) {
            m_cond_not_empty.wait(lock);
        }
        T front_item { m_queue.pop() };
        m_cond_not_full.notify_all();
        return front_item;
    }

    T front() {
        lock_type lock(m_op_mtx);
        return m_queue.front();
    }
    T back() {
        lock_type lock(m_op_mtx);
        return m_queue.back();
    }

private:
    void full_() const { return m_capacity == m_queue.size(); }
    void empty_() const { return m_queue.empty(); }
    void size_() const { return m_queue.size(); }

private:
    mutable std::mutex      m_op_mtx;
    std::condition_variable m_cond_not_full;
    std::condition_variable m_cond_not_empty;
    std::queue<T>           m_queue;
    usize                   m_capacity;
};

} // namespace wallpaper
