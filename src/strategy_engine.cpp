#include "strategy_engine.hpp"
#include <iostream>
#include <chrono>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
StrategyEngine::StrategyEngine(EventRing& ring): ring_(ring){}
void StrategyEngine::run(){
    MarketEvent ev;
    auto last_dump = std::chrono::steady_clock::now();
    while(running_){
        if(!ring_.pop(ev)){ std::this_thread::yield(); continue; }
        processed++;
        if(ev.type==MarketEvent::BOOK){
            double raw = top5_.raw_mid(ev.book);
            double filt = top5_.filtered_mid(ev.book);
            last_filtered_mid=filt;
            last_edge_bps=top5_.edge_bps(raw,filt);
            if(processed%1000==0){
                std::cout << "[STRAT] mid raw " << raw << " filt " << filt << " edge " << last_edge_bps << "bps $" << top5_.edge_usd_per_100btc(last_edge_bps,filt) << "/100BTC spread " << top5_.spread_bps(ev.book) << "bps liq " << top5_.is_liquid(ev.book) << std::endl;
            }
        }else{
            vol_.update(ev.trade);
            last_rv=vol_.rv;
            if(processed%1000==0){
                auto sig = vol_.signal();
                std::cout << "[STRAT] RV " << vol_.rv << "% GARCH " << vol_.garch_vol << "% edge " << vol_.edge_bps() << "bps sig " << sig << std::endl;
            }
        }
        auto now = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::seconds>(now-last_dump).count()>5){
            last_dump=now;
            json out; out["price"]=last_filtered_mid; out["edge_bps"]=last_edge_bps; out["rv"]=last_rv; out["garch"]=vol_.garch_vol;
            out["timestamp_human"]="2026-08-22T00:00:00Z"; out["processed"]=processed;
            std::ofstream f("data/live_verified.json"); f << out.dump(2); f.close();
        }
    }
}
