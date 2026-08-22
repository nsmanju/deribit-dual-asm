#pragma once
#include <array>
#include <cstdint>
struct Level { double price=0; double size=0; };
struct alignas(64) OrderBook {
    std::array<Level,10> bids{};
    std::array<Level,10> asks{};
    uint64_t exchange_ts_ms=0;
    uint64_t local_ts_ns=0;
};
struct alignas(64) Trade {
    double price=0; double size=0;
    uint64_t exchange_ts_ms=0;
    uint64_t local_ts_ns=0;
    bool is_liquid=false;
};
struct MarketEvent {
    enum Type { BOOK, TRADE } type;
    OrderBook book;
    Trade trade;
};
