#ifndef SOLANA_NETWORK_WEBSOCKET_HPP
#define SOLANA_NETWORK_WEBSOCKET_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/json.hpp>

#include <functional>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace Solana::Network
{
    class WebSocket : public std::enable_shared_from_this<WebSocket>
    {
    public:
        // Factory method to ensure proper shared_ptr creation
        static std::shared_ptr<WebSocket> create(
            net::io_context &ioc,
            ssl::context &ctx,
            const std::string &host,
            const std::string &port,
            const std::string &api_key);

        // Constructor is private - use create() instead
        ~WebSocket();

        // Start the WebSocket connection with retry logic
        void start(const std::vector<std::string> &subscription_messages,
                   std::function<void(beast::flat_buffer &&)> on_msg_callback,
                   int max_retries = 5);

        // Send a message through the WebSocket
        void doWrite(const std::string &message);

    private:
        // Private constructor - use factory method instead
        WebSocket(net::io_context &ioc, ssl::context &ctx,
                  const std::string &host, const std::string &port,
                  const std::string &api_key);

        // Connect and subscribe to the WebSocket
        void subscribe(net::yield_context yield,
                       const std::vector<std::string> subscription_messages,
                       std::function<void(beast::flat_buffer &&)> on_msg_callback);

        // Start reading messages
        void doRead();

        // Handle received messages
        void onRead(beast::error_code ec, std::size_t bytes_transferred);

        // Report a failure
        void fail(beast::error_code ec, char const *what);

        // Member variables
        net::io_context &ioc;
        ssl::context &ctx;
        std::string host;
        std::string port;
        std::string api_key;

        std::shared_ptr<websocket::stream<beast::ssl_stream<beast::tcp_stream>>> ws;
        beast::flat_buffer buffer;
        net::steady_timer timer_;
        bool cancelled;
        std::function<void(beast::flat_buffer &&)> onMessage_;
    };
}

#endif // SOLANA_NETWORK_WEBSOCKET_HPP