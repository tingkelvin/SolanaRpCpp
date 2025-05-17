#pragma once

#include "RpcMethod.hpp"
#include <optional>
#include <string>
#include <array>
#include "Common.hpp"
#include "Solana/Core/Types/Types.hpp"

namespace Solana {
    namespace Private {
        template<EncodingType> struct TxnDataType;
        template<> struct TxnDataType<EncodingType::Json> {
            using type = json;
        };
        template<> struct TxnDataType<EncodingType::JsonParsed> {
            using type = json;
        };
        template<> struct TxnDataType<EncodingType::Base58> {
            using type = std::string;
        };
        template<> struct TxnDataType<EncodingType::Base64> {
            using type = std::string;
        };
    }

    // TODO: Figure how tf to parse txn data
    // TODO: build out proper parsing for all this shit
    template<EncodingType Encoding = EncodingType::Json,
        typename ParseStruct = void>
    struct GetTransaction : RpcMethod {

        // Reply structure

        struct Reply {
            u64 slot;
            typename Private::TxnDataType<Encoding>::type transaction;
            std::optional<i64> blockTime;
            json meta;
        };

        static Reply parseReply(const json & j) {
            const auto d = j["result"];

            const auto txnData = d["transaction"];
            typename Private::TxnDataType<Encoding>::type txn{};

            if (txnData.is_array()) {
                txn = txnData[0].get<std::string>();
            }
            else if (txnData.is_object()) {
                txn = txnData;
            }
            else {
                throw std::runtime_error("invalid txn data type");
            }

            return Reply {
                .meta = d["meta"],
                .slot = d["slot"].get<u64>(),
                .transaction = txn,
                .blockTime = d["blockTime"].is_null()
                    ? std::nullopt
                    : std::make_optional(d["blockTime"].get<i64>())
            };
        }

        // Config params

        struct Config {
            Commitment commitment;
        };

        // Command impl

        explicit GetTransaction(
            const std::string & signature,
            const Config & config = {})
            : signature(signature)
            , config(config)
        {}

        std::string methodName() const override { return "getTransaction"; }

        json toJson() const override {
            auto ob = json::object();
            config.commitment.addToJson(ob);
            TransactionEncoding(Encoding).addToJson(ob);
            return json::array({
                signature,
                ob
            });
        }

        std::string signature;
        Config config;
    };
}
