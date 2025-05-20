#pragma once
#include "nlohmann/json.hpp"
#include <iostream>
#include "Solana/Rpc/Methods/Common.hpp"
#include "Solana/Core/Types/Types.hpp"

using json = nlohmann::json;

#define REPLY public:
#define CONFIG public:
#define COMMAND public:

namespace Solana
{
    template <typename Derived>
    struct RpcMethod
    {
        json toJson() const
        {
            return static_cast<const Derived *>(this)->toJsonImpl();
        }

        std::string methodName() const
        {
            return static_cast<const Derived *>(this)->methodNameImpl();
        }

        bool hasParams() const
        {
            return static_cast<const Derived *>(this)->hasParamsImpl();
        }
    };

    template <typename T>
    struct RpcReply
    {
        std::string jsonrpc;
        std::string id;
        typename T::Reply result;

        static RpcReply<T> parse(std::string_view data)
        {
            const auto j = json::parse(data);
            if (j.contains("error"))
                throw std::runtime_error(
                    "request error: " + j["error"].dump());
            return RpcReply{
                .jsonrpc = j["jsonrpc"],
                .id = j["id"].get<std::string>(),
                .result = T::parseReply(j)};
        }
    };
}