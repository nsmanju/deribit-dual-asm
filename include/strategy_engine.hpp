#pragma once
#include "book.hpp"
#include "ring_buffer.hpp"
#include "top5_filter.hpp"
#include "vol_engine.hpp"
#include <atomic>
#include <cstdint>
class StrategyEngine {
public:
    explicit StrategyEngine(EventRing& ring);
    void run();
    void stop(){ running_=false; }
    uint64_t processed=0;
    double last_filtered_mid=0;
    double last_edge_bps=0;
    double last_rv=0;
private:
    EventRing& ring_;
    std::atomic<bool> running_{true};
    Top5Filter top5_;
    VolEngine vol_;
};
