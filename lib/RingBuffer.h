#pragma once

#include "Export.h"
#include <array>
#include <atomic>
#include <mutex>

namespace Visca {

template <typename T, size_t Capacity> class VISCA_EXPORT RingBuffer {
public:
    RingBuffer() = default;
    ~RingBuffer() = default;

    bool push(const T& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t next = (m_head + 1) % Capacity;
        if (next == m_tail)
            return false;

        m_buffer[m_head] = item;
        m_head = next;
        return true;
    }

    bool push(T&& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t next = (m_head + 1) % Capacity;
        if (next == m_tail)
            return false;

        m_buffer[m_head] = std::move(item);
        m_head = next;
        return true;
    }

    bool pop(T& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_head == m_tail)
            return false;

        item = std::move(m_buffer[m_tail]);
        m_tail = (m_tail + 1) % Capacity;
        return true;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_head >= m_tail)
            return m_head - m_tail;
        return Capacity - (m_tail - m_head);
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_head == m_tail;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_head = m_tail = 0;
    }

private:
    std::array<T, Capacity> m_buffer;
    size_t m_head { 0 };
    size_t m_tail { 0 };
    mutable std::mutex m_mutex;
};

}
