#pragma once
#include "book.hpp"
#include <boost/circular_buffer.hpp>
#include <cmath>
struct alignas(64) VolEngine {
    boost::circular_buffer<double> prices{5000};
    boost::circular_buffer<double> rets{5000};
    double var=0; double lambda=0.94; double rv=0; double garch_vol=0;
    void update(const Trade& t){
        if(prices.size()>0){
            double prev=prices.back();
            if(prev>0){ double r = std::log(t.price/prev); rets.push_back(r); var = lambda*var + (1-lambda)*r*r; }
        }
        prices.push_back(t.price);
        if(rets.size()>10){ rv = std::sqrt(252.0*24*3600*2) * std::sqrt(var) * 100.0; garch_vol = rv*0.92; }
    }
    enum Signal { FLAT, LONG_VOL, SHORT_VOL };
    Signal signal() const {
        if(rv==0) return FLAT;
        if(rv > garch_vol + 5.0) return LONG_VOL;
        if(rv + 5.0 < garch_vol) return SHORT_VOL;
        return FLAT;
    }
    double edge_bps() const { return (rv - garch_vol)*100.0; }
};
