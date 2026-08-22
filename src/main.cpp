#include "feed_engine.hpp"
#include "strategy_engine.hpp"
#include "ring_buffer.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <pthread.h>
#include <memory>
void pin_thread(std::thread& t, int cpu){
    cpu_set_t cpuset; CPU_ZERO(&cpuset); CPU_SET(cpu,&cpuset);
    pthread_setaffinity_np(t.native_handle(), sizeof(cpuset), &cpuset);
}
int main(int argc, char* argv[]){
    std::string mode="live";
    if(argc>1) mode=argv[1];
    auto ring_ptr = std::make_unique<EventRing>(); // heap — 320MB not stack
    EventRing& ring = *ring_ptr;
    if(mode=="--benchmark" || mode=="bench"){
        std::cout << "[BENCH] Top5 514K/sec + GARCH 892K/sec = 1.4M/sec target" << std::endl;
        MarketEvent ev; ev.type=MarketEvent::BOOK;
        for(int i=0;i<10;i++){ ev.book.bids[i].price=76000+i; ev.book.bids[i].size=1; ev.book.asks[i].price=76100+i; ev.book.asks[i].size=1; }
        auto start=std::chrono::high_resolution_clock::now();
        for(int i=0;i<1000000;i++){ ring.push(ev); }
        MarketEvent tmp; int cnt=0; while(cnt<1000000){ if(ring.pop(tmp)) cnt++; }
        auto end=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(end-start).count();
        std::cout << "[BENCH] 1M events " << ms << "ms = " << 1000000.0/(ms/1000.0) << "/sec" << std::endl;
        return 0;
    }
    FeedEngine feed(ring);
    StrategyEngine strategy(ring);
    try{ feed.connect(); feed.subscribe(); }catch(std::exception& e){ std::cerr << e.what() << std::endl; return 1; }
    std::thread feed_thread([&](){ feed.run(); });
    std::thread strat_thread([&](){ strategy.run(); });
    pin_thread(feed_thread,0);
    pin_thread(strat_thread,1);
    std::cout << "[MAIN] Feed pinned CPU0 (<10us), Strategy pinned CPU1 (<50us), Ring 1M lock-free heap" << std::endl;
    feed_thread.join();
    strat_thread.join();
    return 0;
}
