#pragma once
#include "Solana/Core/Types/Types.hpp"
#include "RpcMethod.hpp"
#include "Solana/Core/Transaction/Transaction.hpp"

namespace Solana {
    struct SendTransaction : public RpcMethod {

        // Reply structure
        using Reply = json;

        static Reply parseReply(const json & j) {
            return j["result"];
        }

        // Config params
        struct Config {
            SimpleEncoding encoding;
            RPCPARAM(bool, skipPreflight);
            RPCPARAM(u32, maxRetries);
            MinContextSlot minContextSlot;
        };

        // Command impl

        explicit SendTransaction(
            const Txn & txn, const Config & config = {})
            : txn(txn.serialize().toString())
            , config(config)
        {}

        explicit SendTransaction(const std::string & txn, const Config & config = {})
            : txn(txn)
            , config(config)
        {}

        std::string methodName() const override { return "sendTransaction"; }

        json toJson() const override {
            return json::array({
               txn
           });
        }

        std::string txn;
        Config config;
    };
}
