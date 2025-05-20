#pragma once

#include "RpcMethod.hpp"
#include <string>
#include <iostream>
#include <optional>
#include "Common.hpp"

namespace Solana
{
    struct GetBalance : RpcMethod<GetBalance>
    {
        // Reply structure

        struct Reply
        {
            int64_t value;
        };

        // Config params

        struct Config
        {
            Commitment commitment;
            MinContextSlot minContextSlot;
        };

        // Command impl

        explicit GetBalance(const std::string &address, const Config &config = {})
            : address(address), config(config) {}

        static Reply parseReply(const json &data)
        {
            return Reply{.value = data["result"]["value"].get<int64_t>()};
        }

        json toJson() const
        {
            json c = {};

            config.commitment.addToJson(c);
            config.minContextSlot.addToJson(c);
            return json::array({json(address),
                                c});
        }

        std::string methodName() const { return "getBalance"; };

    private:
        std::string address;
        Config config;
    };
}