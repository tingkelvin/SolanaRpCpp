#pragma once

#define BOOST_ASIO_DISABLE_CONCEPTS

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include "nlohmann/json.hpp"
#include <thread>
#include <atomic>
#include "Solana/Core/Network/HttpClient.hpp"
#include <unordered_map>
#include "Solana/Core/Util/Util.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using json = nlohmann::json;

namespace Solana {
    using MessageHandler = std::function<void(const json &)>;
}

namespace Solana::Network {

    class Websocket : public std::enable_shared_from_this<Websocket> {
    public:
        Websocket(net::io_context & ioc, ssl::context & ctx);
        ~Websocket() = default;

        void shutdown() {
            unsubscribeAll();
            cancelled = true;
            try {
                ws.close(websocket::close_code::normal);
            } catch (...) {
                /*  sometimes "stream truncated" gets thrown here
                    but vinnie says you can ignore it
                    https://github.com/boostorg/beast/issues/824#issuecomment-338412225

                    frankly this is the end of the websockets lifetime anyway
                    so who really cares anyway.
                */
            }

        }

        void run(const Url & url);

        std::future<int> subscribe(
            const json & message,
            MessageHandler && callback);
        
        std::future<bool> unsubscribe(json & message);

    private:
        void doRead();
        void doWrite(const json & message);

        void onRead(beast::error_code ec, std::size_t bytes_transferred);

        void unsubscribeAll();

        websocket::stream<
                beast::ssl_stream<beast::tcp_stream>> ws;
        tcp::resolver resolver;
        beast::flat_buffer buffer;

        struct Subscription {
            std::string method;
            MessageHandler handler;
        };

        struct PendingSubscription {
            std::promise<int> prom;
            Subscription sub;
        };

        std::unordered_map<int, PendingSubscription> pendingHandlers;
        std::unordered_map<int, Subscription> activeHandlers;
        std::unordered_map<int, std::promise<bool>> pendingUnsubs;

        std::atomic_bool cancelled = false;
    };
}

