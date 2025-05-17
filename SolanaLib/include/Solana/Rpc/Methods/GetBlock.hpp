#pragma once
#include "RpcMethod.hpp"
#include "Common.hpp"

namespace Solana {
    struct GetBlock : RpcMethod {

        // Reply structure

        struct Reply {
            json blockData;
        };

        static Reply parseReply(const json & data) {
            return Reply {
                .blockData = data["result"]
            };
        }

        // Config params

        struct Config {
            Commitment commitment;
            AccountEncoding encoding;
        };

        // Command impl

        explicit GetBlock(int64_t slot, const Config & config = {}) :
            slot(slot),
            config(config)
        {}

        json toJson() const override {
            auto ob = json::object();
            config.commitment.addToJson(ob);
            config.encoding.addToJson(ob);
            return json::array({
                slot,
                ob
            });
        }

        std::string methodName() const override { return "getBlock"; }

        int64_t slot;
        Config config;
    };
}