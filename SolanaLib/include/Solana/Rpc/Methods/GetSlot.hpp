#pragma once
#include "RpcMethod.hpp"

namespace Solana
{
    struct GetSlot : public RpcMethod<GetSlot>
    {
        REPLY
        using Reply = u64;

        static Reply parseReply(const json &data)
        {
            return data["result"].get<u64>();
        }

        CONFIG

        struct Config
        {
            Commitment commitment;
            MinContextSlot minContextSlot;
        };

        COMMAND

        explicit GetSlot(const Config &config = {}) : config(config) {}

        std::string methodName() const { return "getSlot"; }

        json toJson() const
        {
            auto c = json::object();

            config.commitment.addToJson(c);
            config.minContextSlot.addToJson(c);
            return json::array({c});
        }

        Config config;
    };
}