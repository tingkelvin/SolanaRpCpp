#pragma once

#include "RpcMethod.hpp"
#include <optional>
#include <string>
#include <array>
#include "Common.hpp"
#include "Solana/Core/Types/Types.hpp"

namespace Solana
{
    namespace Private
    {
        template <EncodingType>
        struct TxnDataType;
        template <>
        struct TxnDataType<EncodingType::Json>
        {
            using type = json;
        };
        template <>
        struct TxnDataType<EncodingType::JsonParsed>
        {
            using type = json;
        };
        template <>
        struct TxnDataType<EncodingType::Base58>
        {
            using type = std::string;
        };
        template <>
        struct TxnDataType<EncodingType::Base64>
        {
            using type = std::string;
        };
    }

    // TODO: Figure how tf to parse txn data
    // TODO: build out proper parsing for all this shit
    template <EncodingType Encoding = EncodingType::Json,
              typename ParseStruct = void>
    struct GetTransaction : RpcMethod<GetTransaction<Encoding, ParseStruct>>
    {

        // Reply structure

        struct SwapTx
        {
            std::string outputMint;
            std::string inputMint;
            std::string amm;
            uint64_t inputAmount;
            uint64_t outputAmount;
        };

        struct Reply
        {
            std::optional<SwapTx> swapTx;
            std::optional<json> tx;
            std::string signature;
            u64 slot;
            i64 blockTime;
        };

        static Reply parseReply(const json &j)
        {
            typename Private::TxnDataType<Encoding>::type txn{};
            const auto d = j["result"];
            const auto txnData = d["transaction"];
            const auto txnMeta = d["meta"];

            if (txnMeta.is_null())
            {
                LOG_ERROR("Transaction meta is null");
                return {};
            }

            const auto &ixns = txnMeta["innerInstructions"];
            if (ixns.is_null())
            {
                LOG_ERROR("Transaction inner instructions are null");
                return {};
            }

            LOG_INFO("Inner instruction sets count: {}", ixns.size());

            for (const auto &ixnSet : ixns)
            {
                for (const auto &ixn : ixnSet["instructions"])
                {
                    // Early return if not Jupiter V6 program or missing expected accounts
                    if (identify_program(ixn["programId"]) != KnownProgram::JupiterV6 ||
                        !ixn["accounts"].is_array() ||
                        identify_account(ixn["accounts"][0]) != KnownAccount::JupiterAggregatorEventAuthority)
                    {
                        continue;
                    }

                    LOG_INFO("Found Jupiter instruction with target account");
                    const std::string &encoded = ixn["data"].get_ref<const std::string &>();
                    std::optional<std::vector<uint8_t>> buf = Encoding::Base58::DecodeToBytes(encoded);

                    if (!buf || buf->size() != 128)
                    {
                        continue;
                    }

                    constexpr size_t OFFSET = 16;
                    // Work directly with the buffer
                    const uint8_t *data = buf->data();

                    std::string amm = Encoding::Base58::Encode(data + OFFSET, data + OFFSET + 32);
                    std::string inputMint = Encoding::Base58::Encode(data + OFFSET + 32, data + OFFSET + 64);
                    std::string outputMint = Encoding::Base58::Encode(data + OFFSET + 72, data + OFFSET + 104);

                    uint64_t inputAmount = 0;
                    uint64_t outputAmount = 0;
                    std::memcpy(&inputAmount, data + OFFSET + 64, 8);
                    std::memcpy(&outputAmount, data + OFFSET + 104, 8);

                    LOG_INFO("Decoded Jupiter V6:");
                    LOG_INFO(" amm: {}", amm);
                    LOG_INFO(" inputMint: {}", inputMint);
                    LOG_INFO(" inputAmount: {}", std::to_string(inputAmount));
                    LOG_INFO(" outputMint: {}", outputMint);
                    LOG_INFO(" outputAmount: {}", std::to_string(outputAmount));
                    return Reply{
                        .swapTx = SwapTx{
                            .amm = std::move(amm),
                            .inputMint = std::move(inputMint),
                            .outputMint = std::move(outputMint),
                            .inputAmount = inputAmount,
                            .outputAmount = outputAmount,
                        },
                        .tx = d,
                        .signature = d["transaction"]["signatures"][0].get<std::string>(),
                        .slot = d["slot"].get<u64>(),
                        .blockTime = d["blockTime"].get<i64>()};
                }
            }
            return Reply{
                .swapTx = std::nullopt,
                .tx = d,
                .signature = d["transaction"]["signatures"][0].get<std::string>(),
                .slot = d["slot"].get<u64>(),
                .blockTime = d["blockTime"].get<i64>()};
        }

        // Config params

        struct Config
        {
            Commitment commitment;
            RPCPARAM(int, maxSupportedTransactionVersion);
            TransactionEncoding encoding;
        };

        // Command impl

        explicit GetTransaction(
            const std::string &signature,
            const Config &config = {})
            : signature(signature), config(config)
        {
        }

        std::string methodNameImpl() const { return "getTransaction"; }

        json toJsonImpl() const
        {
            auto ob = json::object();
            config.commitment.addToJson(ob);
            config.maxSupportedTransactionVersion.addToJson(ob);
            config.encoding.addToJson(ob);

            return json::array({signature,
                                ob});
        }

        bool hasParamsImpl() const
        {
            return true;
        }

        std::string signature;
        Config config;
    };

    // SwapTx to_json
    template <EncodingType Encoding, typename ParseStruct>
    inline void to_json(json &j, const typename GetTransaction<Encoding, ParseStruct>::SwapTx &s)
    {
        j = json{
            {"amm", s.amm},
            {"inputMint", s.inputMint},
            {"outputMint", s.outputMint},
            {"inputAmount", s.inputAmount},
            {"outputAmount", s.outputAmount}};
    }

    // Reply to_json
    template <EncodingType Encoding, typename ParseStruct>
    inline void to_json(json &j, const typename GetTransaction<Encoding, ParseStruct>::Reply &r)
    {
        j = json{
            {"signature", r.signature},
            {"slot", r.slot},
            {"blockTime", r.blockTime},
            {"tx", r.tx.value_or(json(nullptr))},
            {"swapTx", r.swapTx.has_value() ? json(*r.swapTx) : json(nullptr)}};
    }

}
