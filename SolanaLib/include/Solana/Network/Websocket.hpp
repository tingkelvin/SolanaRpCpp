#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace Solana::Network
{
    class WebSocket
    {
    public:
        WebSocket(net::io_context &ioc,
                  ssl::context &ctx,
                  const std::string &host,
                  const std::string &port,
                  const std::string &api_key);

        ~WebSocket();

        // Starts the reconnecting session loop
        void start(const std::string &subscription_message, int max_retries = 10);

    private:
        void subscribe(net::yield_context yield, const std::string subscription_message);
        void fail(boost::beast::error_code ec, const char *what);

    private:
        net::io_context &ioc;
        ssl::context &ctx;
        std::string host;
        std::string port;
        std::string api_key;

        std::unique_ptr<boost::beast::websocket::stream<
            boost::beast::ssl_stream<boost::beast::tcp_stream>>>
            ws;

        boost::asio::steady_timer timer_;
    };
}
