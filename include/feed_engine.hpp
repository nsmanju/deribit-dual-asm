#pragma once
#include "book.hpp"
#include "ring_buffer.hpp"
#include <thread>
#include <atomic>
#include <memory>
class FeedEngine {
public:
    explicit FeedEngine(EventRing& ring);
    ~FeedEngine();
    void connect();
    void subscribe();
    void run();
    void stop(){ running_=false; }
private:
    EventRing& ring_;
    std::atomic<bool> running_{true};
    struct Impl; std::unique_ptr<Impl> impl_;
};
