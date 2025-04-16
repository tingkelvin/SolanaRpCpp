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
    Solana::Network::WebSocket ws(ioc, ctx, "api.mainnet-beta.solana.com", "443", "");
    // Send the subscription message
    const std::string subscription_message = R"({
  "jsonrpc": "2.0",
  "id": 1,
  "method": "programSubscribe",
  "params": [
    "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA", 
    {
      "encoding": "jsonParsed"
    }
  ]
})";
    // Retry mechanism
    int retry_count = 0;
    const int max_retries = 5;
    ws.start(subscription_message, max_retries);
    return 0;
}
