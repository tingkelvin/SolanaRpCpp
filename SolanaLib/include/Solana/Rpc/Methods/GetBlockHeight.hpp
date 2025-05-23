#pragma once

#include "RpcMethod.hpp"
#include <string>

namespace Solana
{
    struct GetBlockHeight : RpcMethod<GetBlockHeight>
    {

        // Reply structure

        struct Reply
        {
            int64_t height;
        };

        // Config params

        struct Config
        {
            Commitment commitment;
            MinContextSlot minContextSlot;
        };

        // Command impl

        GetBlockHeight(const Config &config = {}) : config(config) {}

        static Reply parseReply(const json &data)
        {
            return Reply{.height = data["result"].get<int64_t>()};
        }

        json toJson() const
        {
            auto c = json();
            config.commitment.addToJson(c);
            config.minContextSlot.addToJson(c);
            return c;
        }
        bool hasParams() const { return false; }
        std::string methodName() const { return "getBlockHeight"; };

        Config config;
    };
}