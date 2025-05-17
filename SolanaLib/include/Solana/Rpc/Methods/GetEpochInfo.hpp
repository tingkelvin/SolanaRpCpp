#pragma once
#include "RpcMethod.hpp"

namespace Solana {
    struct GetEpochInfo : public RpcMethod {
        REPLY
        struct Reply {
            u64 absoluteSlot;
            u64 blockHeight;
            u64 epoch;
            u64 slotIndex;
            u64 slotsInEpoch;
            std::optional<u64> transactionCount;
        };

        static Reply parseReply(const json & data) {
            const auto res = data["result"];
            return Reply {
                .absoluteSlot = res["absoluteSlot"].get<u64>(),
                .blockHeight = res["blockHeight"].get<u64>(),
                .epoch = res["epoch"].get<u64>(),
                .slotIndex = res["slotIndex"].get<u64>(),
                .slotsInEpoch = res["slotsInEpoch"].get<u64>(),
                .transactionCount = res["transactionCount"].is_null()
                        ? std::nullopt
                        : std::optional<u64>(res["transactionCount"].get<u64>())
            };
        }

        CONFIG

        struct Config {
            Commitment commitment;
            MinContextSlot minContextSlot;
        };

        COMMAND

        explicit GetEpochInfo(const Config & config = {}): config(config){}

        std::string methodName() const override { return "getEpochInfo"; }

        json toJson() const override {
            auto c = json::object();
            config.commitment.addToJson(c);
            config.minContextSlot.addToJson(c);
            return json::array({c});
        }

        Config config;
    };
}