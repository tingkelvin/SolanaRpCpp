#pragma once
#include "Solana/Core/Types/Types.hpp"
#include "RpcMethod.hpp"
#include "Solana/Core/Transaction/Transaction.hpp"


namespace Solana {
    struct SimulateTransaction : public RpcMethod {

        // Reply structure
        using Reply = json;

        static Reply parseReply(const json & j) {

            return j["result"];
        }

        struct SimReturnAccounts {
            std::vector<std::string> addresses;
            AccountEncoding encodingType = AccountEncoding(EncodingType::Base64);
        };

        // Config params

        struct Config {
            Commitment commitment;
            RPCPARAM(bool, sigVerify);
            RPCPARAM(bool, replaceRecentBlockhash);
            MinContextSlot minContextSlot;
            TransactionEncoding encodingType;
            std::optional<SimReturnAccounts> accounts;
        };

        // Command impl

        explicit SimulateTransaction(const Txn & txn, const Config & config = {}) {
            std::string encodedTxn{};
            if (config.encodingType) {
                if (*config.encodingType == TransactionEncoding::str(EncodingType::Base58)) {
                    encodedTxn = txn.serialize().toString();
                } else if (*config.encodingType == TransactionEncoding::str(EncodingType::Base64)) {
                    encodedTxn = txn.serialize().toBase64();
                } else {
                    throw std::runtime_error("Invalid txn encoding: " + *config.encodingType);
                }
            } else {
                encodedTxn = txn.serialize().toString();
            }
            this->txn = encodedTxn;
            this->config = config;
        }

        explicit SimulateTransaction(const std::string & txn, const Config & config = {})
        : txn(txn), config(config) {}

        std::string methodName() const override { return "simulateTransaction"; }

        json toJson() const override {
            auto c = json::object();
            config.commitment.addToJson(c);
            config.sigVerify.addToJson(c);
            config.replaceRecentBlockhash.addToJson(c);
            config.minContextSlot.addToJson(c);
            config.encodingType.addToJson(c);
            if (config.accounts) {
                auto accounts = json::object();
                auto addresses = json::array();
                for (auto & ac : config.accounts->addresses) {
                    addresses.emplace_back(ac);
                }
                accounts["addresses"] = addresses;
                accounts["encoding"] = *config.accounts.value().encodingType;
                c["accounts"] = accounts;
            }
            return json::array({
                txn,
                c
            });
        }

        std::string txn;
        Config config;
    };
}