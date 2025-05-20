#include <iostream>
#include <future>
#include <fstream>
#include <thread> // for std::this_thread::sleep_for
#include <chrono> // for std::chrono::milliseconds
#include <nlohmann/json.hpp>

// Assume these are defined in your project
#include "Solana/Network/WebSocket.hpp"
#include "Solana/Rpc/Rpc.hpp"
#include "Solana/Rpc/Methods/GetSignaturesForAddress.hpp"
#include "Solana/Rpc/Methods/GetTransaction.hpp"
int main()
{
    // Initialize Boost.Asio IO context and SSL context
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};

    // Load default certificates
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(ssl::verify_peer);

    Solana::Rpc rpcSig("https://mainnet.helius-rpc.com/?api-key=8bdcaeaa-3e64-4d76-ba46-a4fb8802eafb");
    // Define initial request object
    auto sig_request = std::make_shared<Solana::GetSignaturesForAddress>(
        "2Rf9qzW9rhCnJmEbErrHDDZfeEXtemYdLkyJ1TE12pa7");
    sig_request->config.commitment = Solana::Commitment(Solana::CommitmentLevel::Confirmed);
    sig_request->config.limit = 1;

    Solana::Rpc rpcTx("https://mainnet.helius-rpc.com/?api-key=7b0e15f4-3d3b-4e17-be8d-3ada2a0e9e3d");

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
    })"};

    // Create WebSocket object using the factory method
    auto ws = Solana::Network::WebSocket::create(
        ioc, ctx,
        "mainnet.helius-rpc.com", "443",
        "7b0e15f4-3d3b-4e17-be8d-3ada2a0e9e3d");

    // Open file for logging messages
    std::ofstream file("messages.json", std::ios::app); // Open in append mode

    // Define the callback function for handling received messages
    auto message_handler = [&file, sig_request, &rpcSig, &rpcTx](beast::flat_buffer &&buf)
    {
        std::string message = beast::buffers_to_string(buf.data());
        try
        {
            auto parsed = json::parse(message);
            std::cout << "Parsed JSON:\n";
            std::cout << parsed << std::endl;

            std::cout << "----------------------------------------\n";
            auto sig_reply = rpcSig.send(*sig_request).get();
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            for (const auto &sigInfo : sig_reply.result.signatures)
            {
                std::cout << "Signature: " << sigInfo.signature << "\n";

                const int max_retries = 3;
                int attempt = 0;
                bool success = false;

                while (attempt < max_retries && !success)
                {
                    try
                    {
                        auto tx_request = Solana::GetTransaction(sigInfo.signature);
                        tx_request.config.maxSupportedTransactionVersion = 0;
                        tx_request.config.encoding = Solana::TransactionEncoding(Solana::EncodingType::JsonParsed);

                        auto tx_reply = rpcTx.send(tx_request).get();
                        json j = tx_reply.result.tx;

                        file << j.dump(4) << std::endl;
                        file.flush();

                        success = true;
                    }
                    catch (const std::exception &e)
                    {
                        ++attempt;
                        std::cerr << "Error on attempt " << attempt << " for signature " << sigInfo.signature
                                  << ": " << e.what() << std::endl;

                        if (attempt < max_retries)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // small backoff before retry
                        }
                        else
                        {
                            std::cerr << "Giving up on signature " << sigInfo.signature << "\n";
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // slight delay between signature requests
            }
        }
        catch (const std::exception &e)
        {
            auto retry_sig_reply = rpcSig.send(*sig_request).get();
            std::cerr << "Failed to parse/write message: " << e.what() << "\n";
        }
    };
    // Start the WebSocket connection
    ws->start(subscription_messages, message_handler);
    ioc.run();
    return 0;
}