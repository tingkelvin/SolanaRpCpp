#pragma once

#include "RpcMethod.hpp"
#include <optional>
#include <string>
#include <vector>
#include "Common.hpp"
#include "Solana/Core/Types/Types.hpp"
#include "Solana/Logger.hpp"

namespace Solana
{
    struct SignatureInfo
    {
        int64_t blockTime;
        std::string confirmationStatus;
        std::optional<json> err;
        std::optional<std::string> memo;
        std::string signature;
        int64_t slot;
    };

    struct Config
    {
        Commitment commitment{};
        MinContextSlot minContextSlot{};
        RPCPARAM(int, limit);
        RPCPARAM(std::string, before);
        RPCPARAM(std::string, until);
        RPCPARAM(std::string, untilSignature);
    };

    struct GetSignaturesForAddress : RpcMethod<GetSignaturesForAddress>
    {
        struct Reply
        {
            std::vector<SignatureInfo> signatures;
        };

        static Reply parseReply(const json &j)
        {
            LOG_INFO("Parsing GetSignaturesForAddress reply: {}", j.dump());

            std::vector<SignatureInfo> results;
            for (const auto &entry : j["result"])
            {
                SignatureInfo info{
                    .blockTime = entry.value("blockTime", 0),
                    .confirmationStatus = entry.value("confirmationStatus", ""),
                    .err = entry.contains("err") ? std::optional<json>(entry["err"]) : std::nullopt,
                    .memo = entry.contains("memo") && !entry["memo"].is_null() ? std::optional<std::string>(entry["memo"].get<std::string>()) : std::nullopt,
                    .signature = entry["signature"].get<std::string>(),
                    .slot = entry["slot"].get<int64_t>()};
                results.push_back(std::move(info));
            }

            return Reply{.signatures = std::move(results)};
        }

        explicit GetSignaturesForAddress(const std::string &address, const Config &config = {})
            : key(address), config(config) {}

        std::string methodNameImpl() const
        {
            return "getSignaturesForAddress";
        }

        json toJsonImpl() const
        {
            auto ob = json::object();
            config.minContextSlot.addToJson(ob);
            config.commitment.addToJson(ob);
            config.limit.addToJson(ob);
            config.before.addToJson(ob);
            config.until.addToJson(ob);
            config.untilSignature.addToJson(ob);
            auto c = json::array({key, ob});
            return c;
        }

        bool hasParamsImpl() const
        {
            return true;
        }

        std::string key;
        Config config = {};
    };
}
