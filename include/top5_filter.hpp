#pragma once
#include "book.hpp"
struct alignas(64) Top5Filter {
    double filtered_mid(const OrderBook& b) const {
        double bid_px_sz=0, bid_n=0, ask_px_sz=0, ask_n=0;
        for(int i=0;i<5;i++){ bid_px_sz+=b.bids[i].price*b.bids[i].size; bid_n+=b.bids[i].size; }
        for(int i=0;i<5;i++){ ask_px_sz+=b.asks[i].price*b.asks[i].size; ask_n+=b.asks[i].size; }
        if(bid_n==0||ask_n==0) return 0;
        double w_bid=bid_px_sz/bid_n; double w_ask=ask_px_sz/ask_n;
        return (w_bid+w_ask)*0.5;
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
    double edge_usd_per_100btc(double eb, double p) const { return eb/10000.0*p*100.0; }
    bool is_liquid(const OrderBook& b) const {
        double notional = b.bids[0].price*b.bids[0].size + b.asks[0].price*b.asks[0].size;
        return spread_bps(b) < 5.0 && notional > 20000;
    }
};
