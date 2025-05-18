#pragma once
#include <string>
#include "Solana/Network/HttpClient.hpp"
#include "Solana/Network/WebSocket.hpp"
#include "Solana/Rpc/Methods/GetBalance.hpp"
#include "Solana/Rpc/Methods/GetBlockHeight.hpp"
#include "Solana/Rpc/Methods/GetBlock.hpp"
#include "Solana/Rpc/Methods/GetBlockTime.hpp"
#include "Solana/Rpc/Methods/GetBlocksWithLimit.hpp"
#include "Solana/Rpc/Methods/GetBlocks.hpp"
#include "Solana/Rpc/Methods/GetBlockProduction.hpp"
#include "Solana/Rpc/Methods/GetBlockCommitment.hpp"
#include "Solana/Rpc/Methods/GetAccountInfo.hpp"
#include "Solana/Rpc/Methods/GetEpochInfo.hpp"
#include "Solana/Rpc/Methods/GetTransaction.hpp"
#include "Solana/Rpc/Methods/GetLatestBlockhash.hpp"
#include "Solana/Rpc/Methods/GetSlot.hpp"
#include "Solana/Rpc/Methods/SimulateTransaction.hpp"
#include "Solana/Rpc/Methods/SendTransaction.hpp"
#include "Solana/Rpc/Methods/RequestAirdrop.hpp"
#include "Solana/Rpc/Methods/WithJsonReply.hpp"
#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <memory>

namespace Solana
{
    class Rpc
    {
    public:
        static Rpc DefaultMainnet();

        explicit Rpc(const std::string &endpoint)
            : client(endpoint), wsThread(&Rpc::runWs, this)
        {
        }

        ~Rpc();

        Rpc(const Rpc &other) = delete;

        template <typename T>
        std::future<RpcReply<T>> send(const T &req)
        {
            auto j = json();

            j["jsonrpc"] = "2.0";
            j["id"] = "1";
            j["method"] = req.methodName();
            if (req.hasParams())
                j["params"] = req.toJson();

#if !NDEBUG
#include <iostream>
            std::cout << "SENDING: " << j.dump() << "\n";
#endif
            std::cout << "SENDING: " << j.dump() << "\n";
            auto res = client.post<RpcReply<T>>(j);
            return res;
        }

        // std::future<int> onSlot(MessageHandler &&handler);
        // std::future<bool> removeSubscription(int subId);

    private:
        void runWs();
        // std::future<int> createSubscription(
        //     const json &message,
        //     MessageHandler &&handler);

    private:
        Network::HttpClient client;
        std::shared_ptr<Network::WebSocket> ws;
        std::thread wsThread;
        std::mutex wsMutex;
        std::condition_variable cv;
        std::atomic_bool ready = false;
    };
}
