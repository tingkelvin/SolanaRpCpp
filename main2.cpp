//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket SSL client for Helius WebSocket API
//
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Start a periodic ping
void start_ping(
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> &ws,
    net::steady_timer &timer,
    beast::error_code &ec,
    net::yield_context yield)
{
    // Loop to send pings every 30 seconds
    for (;;)
    {
        // Set timer to expire after 30 seconds
        timer.expires_after(std::chrono::seconds(30));

        // Wait for the timer to expire
        timer.async_wait(yield[ec]);
        if (ec && ec != net::error::operation_aborted)
            return fail(ec, "timer");

        // Clear the error code from normal timer cancel
        if (ec == net::error::operation_aborted)
            ec = {};

        // Ping the server
        ws.async_ping({}, yield[ec]);
        if (ec)
            return fail(ec, "ping");

        std::cout << "Ping sent\n";
    }
}

// Sends a WebSocket message and handles responses
void do_session(
    std::string const &host,
    std::string const &port,
    std::string const &api_key,
    net::io_context &ioc,
    ssl::context &ctx,
    net::yield_context yield)
{
    beast::error_code ec;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, ctx);

    // Timer for pings
    net::steady_timer timer(ioc);

    // Look up the domain name
    auto const results = resolver.async_resolve(host, port, yield[ec]);
    if (ec)
        return fail(ec, "resolve");

    // Set a timeout on the operation
    beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    auto ep = beast::get_lowest_layer(ws).async_connect(results, yield[ec]);
    if (ec)
        return fail(ec, "connect");

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(
            ws.next_layer().native_handle(),
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
    beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));

    // Set a decorator to change the User-Agent of the handshake
    ws.set_option(websocket::stream_base::decorator(
        [](websocket::request_type &req)
        {
            req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-helius");
        }));

    // Perform the SSL handshake
    ws.next_layer().async_handshake(ssl::stream_base::client, yield[ec]);
    if (ec)
        return fail(ec, "ssl_handshake");

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws).expires_never();

    // Set suggested timeout settings for the websocket
    ws.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::client));

    // Perform the websocket handshake
    // Note that we're using the target with the api-key parameter
    std::string target = "/?api-key=" + api_key;
    ws.async_handshake(host_header, target, yield[ec]);
    if (ec)
        return fail(ec, "handshake");

    std::cout << "WebSocket is open\n";

    // Start the ping timer in a separate coroutine
    net::spawn(
        ioc,
        [&ws, &timer, &ioc](net::yield_context yield_ping)
        {
            beast::error_code ping_ec;
            start_ping(ws, timer, ping_ec, yield_ping);
            if (ping_ec)
                std::cerr << "Ping coroutine exited: " << ping_ec.message() << "\n";
        },
        [](std::exception_ptr ex)
        {
            if (ex)
            {
                try
                {
                    std::rethrow_exception(ex);
                }
                catch (std::exception &e)
                {
                    std::cerr << "Ping exception: " << e.what() << "\n";
                }
            }
        });

    // Send the subscription message
    std::string subscription_message = R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "programSubscribe",
        "params": [
          "6EF8rrecthR5Dkzon8Nwu78hRvfCKubJ14M5uBEwF6P",
          {
            "encoding": "jsonParsed"
          }
        ]
    })";

    // Send the subscription message
    std::cout << "Sending subscription request\n";
    ws.async_write(net::buffer(subscription_message), yield[ec]);
    if (ec)
        return fail(ec, "subscription_write");

    // Read and handle messages in a loop
    beast::flat_buffer buffer;
    for (;;)
    {
        // Read a message
        ws.async_read(buffer, yield[ec]);

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
    ws.async_close(websocket::close_code::normal, yield[ec]);
    if (ec)
        return fail(ec, "close");
}

void run_session_with_reconnect(
    std::string const &host,
    std::string const &port,
    std::string const &api_key,
    net::io_context &ioc,
    ssl::context &ctx,
    net::yield_context yield)
{
    while (true)
    {
        try
        {
            std::cout << "Connecting to Helius WebSocket service...\n";
            do_session(host, port, api_key, ioc, ctx, yield);
        }
        catch (std::exception &e)
        {
            std::cerr << "Exception in do_session: " << e.what() << std::endl;
        }

        std::cerr << "Connection dropped. Reconnecting in 3 seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: helius-websocket-client <api-key>\n";
        return EXIT_FAILURE;
    }

    std::string api_key = argv[1];
    std::string host = "mainnet.helius-rpc.com";
    std::string port = "443"; // WSS port

    // The SSL context is required, and holds certificates
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    ctx.load_verify_file("cert.pem"); // Load your root certificates

    // Retry mechanism
    int retry_count = 0;
    const int max_retries = 5;

    while (retry_count < max_retries)
    {
        try
        {
            // Launch the session operation with reconnect logic
            net::spawn(ioc, [&](net::yield_context yield)
                       { run_session_with_reconnect(
                             host,
                             port,
                             api_key,
                             ioc,
                             ctx,
                             yield); }, [](std::exception_ptr ex)
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
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}