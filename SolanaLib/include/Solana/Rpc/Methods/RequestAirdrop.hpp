#pragma once
#include "Solana/Core/Types/Types.hpp"
#include "RpcMethod.hpp"
#include "Common.hpp"

namespace Solana {
    struct RequestAirdrop : public RpcMethod {

        // Reply structure

        using Reply = std::string;

        static Reply parseReply(const json & j) {
            return j["result"].get<std::string>();
        }

        // Config params

        struct Config {
            Commitment commitment;
        };


        // Command impl

        explicit RequestAirdrop(
            const std::string & address,
            u64 amount,
            const Config & config = {})
            : address(address)
            , amount(amount)
            , config(config)
        {}

        std::string methodName() const override { return "requestAirdrop"; }

        json toJson() const override {
            auto c = json::object();
            config.commitment.addToJson(c);
            return json::array({
                address,
                amount,
                c
            });
        }

        std::string address;
        u64 amount;
        Config config;
    };
}
