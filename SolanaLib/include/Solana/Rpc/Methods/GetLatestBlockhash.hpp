#pragma once
#include "Solana/Core/Types/Types.hpp"
#include "RpcMethod.hpp"

namespace Solana
{
    struct GetLatestBlockhash : public RpcMethod<GetLatestBlockhash>
    {

        // Reply structure

        struct Reply
        {
            std::string blockHash;
            u64 lastValidBlockHeight;
        };

        static Reply parseReply(const json &j)
        {
            auto v = j["result"]["value"];

            return Reply{
                .blockHash = v["blockhash"].get<std::string>(),
                .lastValidBlockHeight = v["lastValidBlockHeight"].get<u64>()};
        }

        // Command impl

        explicit GetLatestBlockhash() = default;

        json toJson() const { return {}; }

        std::string methodName() const { return "getLatestBlockhash"; }

        bool hasParams() const { return false; }
    };
}
