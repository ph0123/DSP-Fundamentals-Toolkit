#pragma once
#include <atomic>
#include <cstddef>
#include <vector>

namespace dsp {

// Single-producer single-consumer lock-free ring buffer.
// Real-time audio rule #1: never lock, never allocate in the audio callback.
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity) : buf_(capacity + 1) {}

    bool push(const float* data, size_t n) {
        const size_t w = write_.load(std::memory_order_relaxed);
        const size_t r = read_.load(std::memory_order_acquire);
        if (free_space(w, r) < n) return false;
        for (size_t i = 0; i < n; ++i)
            buf_[(w + i) % buf_.size()] = data[i];
        write_.store((w + n) % buf_.size(), std::memory_order_release);
        return true;
    }

    bool pop(float* out, size_t n) {
        const size_t w = write_.load(std::memory_order_acquire);
        const size_t r = read_.load(std::memory_order_relaxed);
        if (available(w, r) < n) return false;
        for (size_t i = 0; i < n; ++i)
            out[i] = buf_[(r + i) % buf_.size()];
        read_.store((r + n) % buf_.size(), std::memory_order_release);
        return true;
    }

    size_t readable() const {
        return available(write_.load(std::memory_order_acquire),
                         read_.load(std::memory_order_acquire));
    }

private:
    size_t available(size_t w, size_t r) const {
        return (w + buf_.size() - r) % buf_.size();
    }
    size_t free_space(size_t w, size_t r) const {
        return buf_.size() - 1 - available(w, r);
    }
    std::vector<float> buf_;
    std::atomic<size_t> write_{0}, read_{0};
};

}  // namespace dsp