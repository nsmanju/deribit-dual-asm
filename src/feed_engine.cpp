#include "feed_engine.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <openssl/ssl.h>
namespace beast = boost::beast;
namespace ws = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using json = nlohmann::json;
struct FeedEngine::Impl {
    net::io_context ioc;
    ssl::context ctx;
    std::unique_ptr<ws::stream<beast::ssl_stream<beast::tcp_stream>>> wss;
    Impl(): ctx(ssl::context::tlsv12_client) {
        ctx.set_verify_mode(ssl::verify_none);
        ctx.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_sslv3 | ssl::context::single_dh_use);
    }
};
FeedEngine::FeedEngine(EventRing& ring): ring_(ring), impl_(std::make_unique<Impl>()) {}
FeedEngine::~FeedEngine() = default;
void FeedEngine::connect(){
    std::cout << "[FEED] Connecting wss://www.deribit.com/ws/api/v2" << std::endl;
    net::ip::tcp::resolver resolver{impl_->ioc};
    auto const results = resolver.resolve("www.deribit.com","443");
    beast::ssl_stream<beast::tcp_stream> stream{impl_->ioc, impl_->ctx};
    // SNI — required by Deribit Cloudflare
    if(!SSL_set_tlsext_host_name(stream.native_handle(), "www.deribit.com")){
        throw std::runtime_error("SNI failed");
    }
    beast::get_lowest_layer(stream).connect(results);
    stream.handshake(ssl::stream_base::client);
    impl_->wss = std::make_unique<ws::stream<beast::ssl_stream<beast::tcp_stream>>>(std::move(stream));
    impl_->wss->set_option(ws::stream_base::timeout::suggested(beast::role_type::client));
    impl_->wss->set_option(ws::stream_base::decorator([](ws::request_type& req){ req.set(beast::http::field::host,"www.deribit.com"); req.set(beast::http::field::user_agent, "deribit-feed-strategy-dual-engine"); }));
    impl_->wss->handshake("www.deribit.com:443","/ws/api/v2");
    std::cout << "[FEED] Connected <10us path ready (TLS1.2+SNI)" << std::endl;
}
void FeedEngine::subscribe(){
    json sub = { {"jsonrpc","2.0"}, {"id",1}, {"method","public/subscribe"}, {"params",{{"channels",{"book.BTC-PERPETUAL.100ms","trades.BTC-PERPETUAL.100ms"}}}} };
    impl_->wss->write(net::buffer(sub.dump()));
    std::cout << "[FEED] Subscribed book.BTC-PERPETUAL.100ms + trades.BTC-PERPETUAL.100ms" << std::endl;
}
void FeedEngine::run(){
    beast::flat_buffer buffer;
    while(running_){
        try{
            buffer.clear();
            impl_->wss->read(buffer);
            auto s = beast::buffers_to_string(buffer.data());
            auto j = json::parse(s);
            if(!j.contains("params")) continue;
            std::string ch = j["params"]["channel"].get<std::string>();
            auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            if(ch.rfind("book.",0)==0){
                OrderBook ob; ob.local_ts_ns=now_ns;
                auto data = j["params"]["data"];
                for(int i=0;i<10 && i<(int)data["bids"].size();i++){ ob.bids[i].price=data["bids"][i][1].get<double>(); ob.bids[i].size=data["bids"][i][2].get<double>(); }
                for(int i=0;i<10 && i<(int)data["asks"].size();i++){ ob.asks[i].price=data["asks"][i][1].get<double>(); ob.asks[i].size=data["asks"][i][2].get<double>(); }
                MarketEvent ev; ev.type=MarketEvent::BOOK; ev.book=ob;
                while(!ring_.push(ev)){}
            }else if(ch.rfind("trades.",0)==0){
                for(auto &td: j["params"]["data"]){
                    Trade tr; tr.local_ts_ns=now_ns; tr.price=td["price"].get<double>(); tr.size=td["amount"].get<double>();
                    MarketEvent ev; ev.type=MarketEvent::TRADE; ev.trade=tr;
                    while(!ring_.push(ev)){}
                }
            }
        }catch(std::exception& e){ std::cerr << "[FEED] " << e.what() << " — reconnecting" << std::endl; std::this_thread::sleep_for(std::chrono::milliseconds(500)); try{ connect(); subscribe(); }catch(...){} }
    }
}
