#pragma once

#include "RpcMethod.hpp"
#include <optional>
#include <string>
#include <array>
#include "Common.hpp"
#include "Solana/Core/Encoding/Base58.hpp"
#include "Solana/Core/Encoding/Base64.hpp"
#include "Solana/Core/Types/Types.hpp"
#include "Solana/Logger.hpp"

template <typename T>
struct is_json
{
    static const bool value = false;
};

template <>
struct is_json<json>
{
    static const bool value = true;
};

namespace Solana
{
    // TODO: support base64+ztd encoded accounts
    template <typename AccountStruct = json>
    struct GetAccountInfo : RpcMethod<GetAccountInfo<>>
    {
        struct Reply
        {
            bool executable;
            int64_t lamports;
            Pubkey owner;
            int64_t rentEpoch;
            int64_t space;
            AccountStruct accountData;
        };

        static Reply parseReply(const json &j)
        {
            LOG_INFO("Parsing GetAccountInfo reply: {}", j.dump());

            const auto d = j["result"]["value"];
            auto data = d["data"];

            LOG_INFO("Account value fields: executable={}, lamports={}, owner={}, rentEpoch={}, space={}",
                     d["executable"].get<bool>(),
                     d["lamports"].get<int64_t>(),
                     d["owner"].get<std::string>(),
                     d["rentEpoch"].get<int64_t>(),
                     d["space"].get<int64_t>());

            AccountStruct account{};

            if constexpr (is_json<AccountStruct>::value)
            {
                LOG_INFO("AccountStruct is raw JSON; assigning directly.");
                account = data;
            }
            else
            {
                std::vector<unsigned char> dataBytes{};
                std::string accountData;
                std::string dataEncoding = "None";

                if (data.is_array())
                {
                    accountData = data[0].get<std::string>();
                    dataEncoding = data[1].get<std::string>();
                    if (dataEncoding == "base58")
                    {
                        auto decodedOpt = Encoding::Base58::Decode(accountData);
                        if (!decodedOpt)
                        {
                            throw std::runtime_error("Base58 decoding failed.");
                        }
                        const auto &decoded = *decodedOpt;
                        // account.decode(Buffer(decoded));
                        LOG_INFO("Base58 decoded data size: {}", decoded.size());
                    }
                    else if (dataEncoding == "base64")
                    {
                        std::string out;
                        auto error = Encoding::Base64::Decode(accountData, out);
                        if (!error.empty())
                        {
                            throw std::runtime_error("Failed to decode base64 string: " + error);
                        }
                        account.decode(Buffer(out));
                        LOG_INFO("Base64 decoded data size: {}", out.size());
                    }
                    else
                    {
                        LOG_ERROR("Unsupported encoding type: {}", dataEncoding);
                    }
                }
            }
            LOG_INFO("Account data size: {}", account.size());
            return Reply{
                .executable = d["executable"].get<bool>(),
                .lamports = d["lamports"].get<int64_t>(),
                .owner = Pubkey::fromString(d["owner"].get<std::string>()),
                .rentEpoch = d["rentEpoch"].get<int64_t>(),
                .space = d["space"].get<int64_t>(),
                .accountData = account,
            };
        }

        struct Config
        {
            Commitment commitment;
            AccountEncoding encoding;
        };

        explicit GetAccountInfo(const std::string &address, const Config &config = {})
            : key(address), config(config) {}

        std::string methodNameImpl() const { return "getAccountInfo"; }

        json toJsonImpl() const
        {
            auto ob = json::object();
            config.encoding.addToJson(ob);
            config.commitment.addToJson(ob);
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