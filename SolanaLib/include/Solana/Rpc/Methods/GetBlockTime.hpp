#pragma once

#include "RpcMethod.hpp"

namespace Solana {
    struct GetBlockTime : public RpcMethod {
        using Reply = i64;

        static Reply parseReply(const json & data) {
            return data["result"].get<i64>();
        }

        // Command impl
        explicit GetBlockTime(u64 slot): slot(slot){}

        std::string methodName() const override { return "getBlockTime"; }

        json toJson() const override {
            return json::array({slot});
        }

        u64 slot;
    };
}