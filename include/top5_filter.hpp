#pragma once
#include "book.hpp"
#include "asm_vwap.hpp"
struct alignas(64) Top5Filter {
    double filtered_mid(const OrderBook& b) const {
        double bid_p[5], bid_s[5], ask_p[5], ask_s[5];
        for(int i=0;i<5;i++){ bid_p[i]=b.bids[i].price; bid_s[i]=b.bids[i].size; }
        for(int i=0;i<5;i++){ ask_p[i]=b.asks[i].price; ask_s[i]=b.asks[i].size; }
        return asm_filtered_mid(bid_p, bid_s, ask_p, ask_s);
    }
    double raw_mid(const OrderBook& b) const {
        if(b.bids[0].price==0||b.asks[0].price==0) return 0;
        return (b.bids[0].price+b.asks[0].price)*0.5;
    }
    double spread_bps(const OrderBook& b) const {
        double rm=raw_mid(b); if(rm==0) return 9999;
        return (b.asks[0].price-b.bids[0].price)/rm*10000.0;
    }
    double edge_bps(double raw, double filt) const {
        if(raw==0) return 0; return (filt-raw)/raw*10000.0;
    }
    double edge_usd_per_100btc(double v, double p) const {
        return v/10000.0*p*100.0;
    }
    bool is_liquid(const OrderBook& b) const {
        double n = b.bids[0].price*b.bids[0].size + b.asks[0].price*b.asks[0].size;
        return spread_bps(b) < 5.0 && n > 20000;
    }
    uint64_t tick() const { return rdtsc_tick(); }
};
