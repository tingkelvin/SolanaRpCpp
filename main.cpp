#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include "Solana/Network/Websocket.hpp"

#include <thread>
#include <chrono>
#include <iostream>

int main()
{
    // Initialize Boost.Asio IO context and SSL context
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    // Create WebSocket object
    Solana::Network::WebSocket ws(ioc, ctx, "mainnet.helius-rpc.com", "443", "");
    // Send the subscription message
    const std::string subscription_message = R"({
  "jsonrpc": "2.0",
  "id": 1,
  "method": "logsSubscribe",
  "params": [
    {
      "mentions": ["TSLvdd1pWpHVjahSpsvCXUbgwsL3JAcvokwaKt1eokM"]
    },
    {
      "commitment": "finalized"
    }
  ]
})";
    // Retry mechanism
    ws.start(subscription_message, [](beast::flat_buffer &&buf)
             {
                 // Convert buffer to string
                 std::string message = beast::buffers_to_string(buf.data());
                 std::cout << "Received: " << message << "\n";

                 //  // Try parsing it as a JSON value
                 //  json::value parsed = json::parse(message);

                 //  // Write to file directly (without wrapping)
                 //  file << json::serialize(parsed) << "\n";
                 //  if (file.fail())
                 //  {
                 //      fail(beast::error_code(errno, beast::system_category()), "file write");
                 //      break;
                 //  }
                 //  file.flush();
             });
    return 0;
}
