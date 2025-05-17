#pragma once
#include "RpcMethod.hpp"
#include <optional>
#include <string>
#include <array>
#include "Common.hpp"
#include "Solana/Core/Encoding/Base58.hpp"
#include "Solana/Core/Encoding/Base64.hpp"
#include "Solana/Core/Types/Types.hpp"

template<typename T>
struct is_json {
    static const bool value = false;
};

template<>
struct is_json<json> {
    static const bool value = false;
};

namespace Solana {

    // TODO: support base64+ztd encoded accounts
    template<typename AccountStruct = json>
    struct GetAccountInfo : RpcMethod {

        // Reply structure

        struct Reply {
            bool executable;
            int64_t lamports;
            Pubkey owner;
            int64_t rentEpoch;
            int64_t space;
            AccountStruct accountData;
        };

        static Reply parseReply(const json & j) {
            const auto d = j["result"]["value"];
            auto data = d["data"];

            auto account = AccountStruct();
            if constexpr (is_json<AccountStruct>::value) {
                account = data;
            } else {
                std::vector<unsigned char> dataBytes{};
                std::string accountData;
                std::string dataEncoding = "base58";
                if (data.is_array()) {
                    accountData = data[0].get<std::string>();
                    dataEncoding = data[1].get<std::string>();
                } else {
                    accountData = data.get<std::string>();
                }

                if (dataEncoding == "base58") {
                    const auto decoded = *Encoding::Base58::Decode(
                        accountData);
                    account.decode(Buffer(decoded));
                }
                else if (dataEncoding == "base64") {
                    std::string out;
                    auto error = Encoding::Base64::Decode(accountData, out);
                    if (!error.empty()) {
                        throw std::runtime_error("Failed to decode base64 string: " + error);
                    }
                    account.decode(Buffer(out));
                }
                else {
                    throw std::runtime_error("Unsupported encoding type: " + dataEncoding);
                }
            }

            return Reply {
                    .executable = d["executable"].get<bool>(),
                    .lamports = d["lamports"].get<int64_t>(),
                    .owner = Pubkey::fromString(d["owner"].get<std::string>()),
                    .rentEpoch = d["rentEpoch"].get<int64_t>(),
                    .space = d["space"].get<int64_t>(),
                    .accountData = account,
            };
        }

        // Config params
        struct Config {
            Commitment commitment;
            AccountEncoding encoding;
        };

        // Command impl

        explicit GetAccountInfo(const std::string & address, const Config & config = {})
            : key(address), config(config) {}

        std::string methodName() const override { return "getAccountInfo"; }

        json toJson() const override {
            auto ob = json::object();
            config.encoding.addToJson(ob);
            config.commitment.addToJson(ob);
            auto c = json::array({key, ob});

            return c;
        }

        std::string key;
        Config config = {};
    };
}