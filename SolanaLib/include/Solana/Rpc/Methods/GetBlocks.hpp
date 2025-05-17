#pragma once
#include "RpcMethod.hpp"

namespace Solana {
    struct GetBlocks : public RpcMethod {
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

        explicit GetBlocks(u64 start, std::optional<u64> end = {}, const Config & config = {})
            : start(start), end(end), config() {}

        std::string methodName() const override { return "getBlocks"; }

        json toJson() const override {
            auto c = json::object();
            config.commitment.addToJson(c);
            auto arr = json::array();
            arr.push_back(start);
            if (end) arr.push_back(*end);
            if (config.commitment.has_value()) arr.push_back(c);
            return arr;
        }

        // Command impl

        u64 start;
        std::optional<u64> end;
        Config config;
    };
}