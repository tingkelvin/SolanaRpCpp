#include "Solana/Network/Websocket.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
// #include <cstdlib>
// #include <functional>
#include <iostream>
// #include <memory>
// #include <string>
// #include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace Solana::Network
{

    WebSocket::WebSocket(net::io_context &ioc, ssl::context &ctx, const std::string &host, const std::string &port, const std::string &api_key)
        : ioc(ioc), ctx(ctx), host(host), port(port), api_key(api_key),
          ws(std::make_unique<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(ioc, ctx)),
          timer_(ioc)
    {
        std::cout << "WebSocket initialized with host: " << host << ", port: " << port << "\n";
    }

    WebSocket::~WebSocket()
    {
        if (ws && ws->is_open())
        {
            beast::error_code ec;
            ws->close(websocket::close_code::normal, ec);
        }
    }

    void WebSocket::fail(beast::error_code ec, char const *what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    void WebSocket::subscribe(net::yield_context yield, const std::string subscription_message)
    {
        std::cout << "Connecting to WebSocket...\n";
        beast::error_code ec;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);

        // Timer for pings
        net::steady_timer timer(ioc);

        // Look up the domain name
        auto const results = resolver.async_resolve(host, port, yield[ec]);
        if (ec)
            return fail(ec, "resolve");

        // Set a timeout on the operation
        beast::get_lowest_layer(*ws).expires_after(std::chrono::seconds(30));
        std::cout << "Connecting to " << host << ":" << port << "\n";
        // Make the connection on the IP address we get from a lookup
        auto ep = beast::get_lowest_layer(*ws).async_connect(results, yield[ec]);
        if (ec)
            return fail(ec, "connect");

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(
                ws->next_layer().native_handle(),
                host.c_str()))
        {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                                   net::error::get_ssl_category());
            return fail(ec, "connect");
        }

        // Update the host string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        std::string host_header = host + ':' + std::to_string(ep.port());

        // Set a timeout on the operation
        beast::get_lowest_layer(*ws).expires_after(std::chrono::seconds(30));
        std::cout << "Connecting to " << host_header << "\n";
        // Set a decorator to change the User-Agent of the handshake
        ws->set_option(websocket::stream_base::decorator(
            [](websocket::request_type &req)
            {
                req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-client-helius");
            }));

        // Perform the SSL handshake
        ws->next_layer().async_handshake(ssl::stream_base::client, yield[ec]);
        if (ec)
            return fail(ec, "ssl_handshake");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(*ws).expires_never();

        // Set suggested timeout settings for the websocket
        ws->set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Perform the websocket handshake
        // Note that we're using the target with the api-key parameter
        std::string target = "/?api-key=" + api_key;
        ws->async_handshake(host_header, target, yield[ec]);
        if (ec)
            return fail(ec, "handshake");

        std::cout << "WebSocket is open\n";

        // Send the subscription message
        std::cout << "Sending subscription request\n";
        ws->async_write(net::buffer(subscription_message), yield[ec]);
        if (ec)
            return fail(ec, "subscription_write");

        // Read and handle messages in a loop
        beast::flat_buffer buffer;
        for (;;)
        {
            // Read a message
            ws->async_read(buffer, yield[ec]);

            // Handle error or closed connection
            if (ec)
            {
                if (ec == websocket::error::closed)
                    std::cout << "WebSocket is closed\n";
                else
                    fail(ec, "read");
                break;
            }

            // Display the message
            std::cout << "Received: " << beast::make_printable(buffer.data()) << "\n";

            // Clear the buffer for the next message
            buffer.consume(buffer.size());
        }

        // Gracefully close the WebSocket connection
        ws->async_close(websocket::close_code::normal, yield[ec]);
        if (ec)
            return fail(ec, "close");
    }

    void WebSocket::start(const std::string &subscription_message, int max_retries)
    {
        int retry_count = 0;
        while (retry_count < max_retries)
        {
            try
            {
                // Launch the session operation with reconnect logic
                net::spawn(ioc, [&](net::yield_context yield)
                           { this->subscribe(yield, subscription_message); }, [](std::exception_ptr ex)
                           {
                    if(ex)
                    {
                        try
                        {
                            std::rethrow_exception(ex);
                        }
                        catch(std::exception& e)
                        {
                            std::cerr << "Unhandled exception: " << e.what() << "\n";
                        }
                    } });

                // Run the I/O service. The call will return when
                // the socket is closed.
                ioc.run();
                break; // Exit loop on success
            }
            catch (std::exception &e)
            {
                std::cerr << "Connection failed: " << e.what() << "\n";
                ++retry_count;

                if (retry_count < max_retries)
                {
                    std::cerr << "Retrying in 3 seconds... (Attempt " << retry_count << " of " << max_retries << ")\n";
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                    ioc.restart();
                }
                else
                {
                    std::cerr << "Max retries reached. Exiting.\n";
                }
            }
        }
    }
}