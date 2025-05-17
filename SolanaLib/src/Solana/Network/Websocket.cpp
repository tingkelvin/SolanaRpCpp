#include "Solana/Network/WebSocket.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/json.hpp>

#include <iostream>
#include <fstream>
#include <memory>

namespace Solana::Network
{
    // Key fix: create a proper factory method to ensure shared_ptr initialization
    std::shared_ptr<WebSocket> WebSocket::create(net::io_context &ioc, ssl::context &ctx,
                                                 const std::string &host, const std::string &port,
                                                 const std::string &api_key)
    {
        return std::shared_ptr<WebSocket>(new WebSocket(ioc, ctx, host, port, api_key));
    }

    WebSocket::WebSocket(net::io_context &ioc, ssl::context &ctx, const std::string &host,
                         const std::string &port, const std::string &api_key)
        : ioc(ioc), ctx(ctx), host(host), port(port), api_key(api_key),
          ws(std::make_shared<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(ioc, ctx)),
          timer_(ioc),
          buffer(),
          cancelled(false)
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

    void WebSocket::subscribe(net::yield_context yield, const std::vector<std::string> subscription_messages,
                              std::function<void(beast::flat_buffer &&)> on_msg_callback)
    {
        std::cout << "Connecting to WebSocket...\n";
        beast::error_code ec;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);

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

        // Send the subscription message if provided
        if (!subscription_messages.empty())
        {
            for (auto message : subscription_messages)
            {
                std::cout << "Sending subscription request: " << message << "\n";
                ws->async_write(net::buffer(message), yield[ec]);
                if (ec)
                    return fail(ec, "subscription_write");
            }
        }

        // Start the reading loop
        doRead();
    }

    void WebSocket::doRead()
    {
        // Make sure we're not destroyed during this operation
        auto self = shared_from_this();

        ws->async_read(
            buffer,
            [self](beast::error_code ec, std::size_t bytes_transferred)
            {
                self->onRead(ec, bytes_transferred);
            });
    }

    void WebSocket::onRead(beast::error_code ec, std::size_t bytes_transferred)
    {
        if (ec)
        {
            if (ec == websocket::error::closed)
                std::cout << "WebSocket connection closed normally\n";
            else
                fail(ec, "read");
            return;
        }

        if (!ws->is_open())
            return;

        // Use boost::json namespace for parsing
        using json = boost::json::value;

        json message;
        try
        {
            auto data = static_cast<const char *>(buffer.data().data());
            size_t size = buffer.size();

            message = boost::json::parse(std::string(data, size));
        }
        catch (const std::exception &e)
        {
            std::cout << "Failed to parse message: " << e.what() << "\n";
        }

        if (message.is_object())
        {
            std::cout << "Received: " << message << "\n";

            // Handle message based on its content
            // Add your message handling logic here
        }

        // Clear the buffer for the next message
        buffer.consume(buffer.size());

        // If we're not cancelled, continue reading
        if (!cancelled)
        {
            doRead();
        }
    }

    void WebSocket::doWrite(const std::string &message)
    {
        // Make sure we're not destroyed during this operation
        auto self = shared_from_this();

        ws->async_write(
            net::buffer(message),
            [self](beast::error_code ec, std::size_t bytes_transferred)
            {
                if (ec)
                    self->fail(ec, "write");
            });
    }

    void WebSocket::start(const std::vector<std::string> &subscription_messages,
                          std::function<void(beast::flat_buffer &&)> on_msg_callback,
                          int max_retries)
    {
        int retry_count = 0;
        while (retry_count < max_retries)
        {
            try
            {
                // Launch the session operation with reconnect logic
                net::spawn(ioc, [self = shared_from_this(), subscription_messages, on_msg_callback](net::yield_context yield)
                           { self->subscribe(yield, subscription_messages, on_msg_callback); }, [](std::exception_ptr ex)
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