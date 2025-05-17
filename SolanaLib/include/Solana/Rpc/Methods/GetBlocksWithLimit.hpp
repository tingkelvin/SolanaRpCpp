#pragma once
#include "RpcMethod.hpp"

namespace Solana {
    struct GetBlocksWithLimit : public RpcMethod {

        // Reply structure
        using Reply = std::vector<u64>;

        static Reply parseReply(const json & data) {
            return data["result"].get<std::vector<u64>>();
        }

        // Config params

        struct Config {
            Commitment commitment;
        };

        // Command impl

        explicit GetBlocksWithLimit(u64 start, std::optional<u64> limit = {}, const Config & config = {})
            : start(start), limit(limit), config(config)
        {}

        std::string methodName() const override { return "getBlocksWithLimit"; }

        json toJson() const override {
            auto c = json::object();
            config.commitment.addToJson(c);
            auto arr = json::array();
            arr.push_back(start);
            if (limit) arr.push_back(*limit);
            if (config.commitment.has_value()) arr.push_back(c);
            return arr;
        }

        u64 start;
        std::optional<u64> limit;
        Config config;
    };
}