#pragma once
#include "RpcMethod.hpp"
#include <string>
#include "Common.hpp"
#include <unordered_map>
#include <array>

namespace Solana {
    struct GetBlockProduction : RpcMethod {

        // Reply structure

        struct Reply {
            std::unordered_map<std::string
                ,std::array<int, 2>> byIdentity;
        };

        // Config params

        struct Config {
            Commitment commitment;
            std::optional<std::string> identity;
            std::optional<
                std::pair<int64_t, int64_t>> range;
        };

        // Command impl

        explicit GetBlockProduction(const Config & config = {})
            : config(config) {}

        static Reply parseReply(const json & data) {
            auto out = Reply{};
            for (auto & [k, v] : data["result"]["value"]["byIdentity"].items()) {
                out.byIdentity[k] = std::array<int, 2>{
                    v[0].get<int>(),
                    v[1].get<int>()
                };
            }
            return out;
        }

        json toJson() const override {
            auto ob = json::object();
            config.commitment.addToJson(ob);
            if (config.identity.has_value()) {
                ob["identity"] = config.identity.value();
            }
            if (config.range.has_value()) {
                auto range = json::object();
                range["firstSlot"] = config.range->first;
                range["lastSlot"] = config.range->second;
                ob["range"] = range;
            }

            return json::array({ob});
        }

        std::string methodName() const override { return "getBlockProduction"; }

        Config config;
    };
}