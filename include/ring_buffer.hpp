#pragma once
#include <atomic>
#include <cstddef>
#include <array>
#include "book.hpp"
template<typename T, size_t Capacity>
class RingBuffer {
    static_assert((Capacity & (Capacity-1)) == 0, "Capacity must be power of two");
public:
    RingBuffer(): head_(0), tail_(0) {}
    bool push(const T& item) {
        size_t h=head_.load(std::memory_order_relaxed);
        size_t nh=(h+1)&(Capacity-1);
        if(nh==tail_.load(std::memory_order_acquire)) return false;
        buffer_[h]=item;
        head_.store(nh,std::memory_order_release);
        return true;
    }
    bool pop(T& item) {
        size_t t=tail_.load(std::memory_order_relaxed);
        if(t==head_.load(std::memory_order_acquire)) return false;
        item=buffer_[t];
        tail_.store((t+1)&(Capacity-1),std::memory_order_release);
        return true;
    }
private:
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
    alignas(64) std::array<T, Capacity> buffer_;
};
using EventRing = RingBuffer<MarketEvent, 1<<20>;
