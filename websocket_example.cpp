#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/json.hpp>
#include "Solana/Network/WebSocket.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace json = boost::json;

int main()
{
  // Initialize Boost.Asio IO context and SSL context
  net::io_context ioc;
  ssl::context ctx{ssl::context::tlsv12_client};

  // Load default certificates
  ctx.set_default_verify_paths();
  ctx.set_verify_mode(ssl::verify_peer);

  // Create WebSocket object using the factory method
  auto ws = Solana::Network::WebSocket::create(
      ioc, ctx,
      "mainnet.helius-rpc.com", "443",
      "7b0e15f4-3d3b-4e17-be8d-3ada2a0e9e3d");

  // Define the subscription message
  const std::vector<std::string> subscription_messages = {R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "accountSubscribe",
        "params": [
            "2Rf9qzW9rhCnJmEbErrHDDZfeEXtemYdLkyJ1TE12pa7",
            {
                "encoding": "jsonParsed",
                "commitment": "confirmed"
            }
        ]
    })",
                                                          R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "accountSubscribe",
        "params": [
            "2Rf9qzW9rhCnJmEbErrHDDZfeEXtemYdLkyJ1TE12pa7",
            {
                "encoding": "jsonParsed",
                "commitment": "confirmed"
            }
        ]
    })"};

  // Open file for logging messages
  std::ofstream file("messages.json", std::ios::app); // Open in append mode

  // Define the callback function for handling received messages
  auto message_handler = [&file](beast::flat_buffer &&buf)
  {
    std::string message = beast::buffers_to_string(buf.data());
    try
    {
      auto parsed = json::parse(message);
      std::cout << "Parsed JSON:\n";
      std::cout << parsed << std::endl;
      file << message << "\n";
      file.flush();
      std::cout << "----------------------------------------\n";
    }
    catch (const std::exception &e)
    {
      std::cerr << "Failed to parse/write message: " << e.what() << "\n";
    }
  };

  // Start the WebSocket connection with the subscription message
  ws->start(subscription_messages, message_handler);

  // The ioc.run() call is inside start() so we don't need to call it here

  // To keep the program running (if needed)
  std::cout << "Press Enter to exit..." << std::endl;
  std::cin.get();

  return 0;
}