#pragma once
#include "Solana/Core/Types/Types.hpp"
#include "RpcMethod.hpp"

namespace Solana
{

    struct GetBlockCommitment : public RpcMethod<GetBlockCommitment>
    {
        // Reply structure
        struct Reply
        {
            std::optional<std::vector<u64>> commitment;
            std::string totalStake;
        };

        static Reply parseReply(const json &j)
        {
            const auto &res = j["result"];
            return Reply{
                .commitment = j["commitment"].get<std::vector<u64>>(),
                .totalStake = j["totalStake"].get<std::string>()};
        }

        // Command impl

        /// This method is apparently actual broken https://github.com/solana-labs/solana/issues/32662
        explicit GetBlockCommitment(u64 slot) : slot(slot) {}

        std::string methodName() const { return "getBlockCommitment"; }

        json toJson() const
        {
            return json::array({slot});
        }

        u64 slot;
    };
}
