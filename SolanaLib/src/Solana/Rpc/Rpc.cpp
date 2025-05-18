#include "Solana/Rpc/Rpc.hpp"
#include <chrono>

using namespace Solana;

namespace
{
    net::io_context ioc = {};
    std::atomic_int messageCounter = 0;
}

Rpc Rpc::DefaultMainnet()
{
    return Rpc("https://api.mainnet-beta.solana.com");
}

Rpc::~Rpc()
{
    // ws->shutdown();
    wsThread.join();
}

void Rpc::runWs()
{
    // net::io_context ioc;
    // ssl::context ctx{ssl::context::tlsv12_client};

    // ws = std::make_shared<Network::WebSocket>(ioc, ctx, "mainnet.helius-rpc.com", "443", "7b0e15f4-3d3b-4e17-be8d-3ada2a0e9e3d");
    // auto url = client.getUrl();
    // url.service = "443";
    // ws->run(url);

    // if (ioc.stopped())
    //     ioc.restart();
    // ready = true;
    // cv.notify_all();

    // ioc.run();
}

// std::future<bool> Rpc::removeSubscription(int subId)
// {
//     auto message = json::object();
//     message["jsonrpc"] = "2.0";
//     message["id"] = ++messageCounter;
//     message["params"] = json::array({subId});

//     return ws->unsubscribe(message);
// }

// std::future<int> Rpc::createSubscription(
//     const json &message,
//     MessageHandler &&handler)
// {

//     std::unique_lock lock(wsMutex);

//     cv.wait(lock, [this]()
//             { return ready.load(); });

//     return ws->subscribe(message, std::move(handler));
// }

// std::future<int> Rpc::onSlot(MessageHandler &&handler)
// {
//     auto message = json::object();
//     message["jsonrpc"] = "2.0";
//     message["id"] = ++messageCounter;
//     message["method"] = "slotSubscribe";

//     return createSubscription(message, std::move(handler));
// }