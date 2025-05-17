#pragma once
#include "nlohmann/json.hpp"
#include <iostream>
#include "Solana/Rpc/Methods/Common.hpp"
#include "Solana/Core/Types/Types.hpp"

using json = nlohmann::json;

#define REPLY public:
#define CONFIG public:
#define COMMAND public:

namespace Solana {
    struct RpcMethod {
        virtual json toJson() const = 0;
        virtual std::string methodName() const = 0;
        virtual bool hasParams() const { return true; }
    };

    template<typename T>
    struct RpcReply {
        std::string jsonrpc;
        std::string id;
        typename T::Reply result;

        static RpcReply<T> parse(std::string_view data) {
            const auto j = json::parse(data);
            if (j.contains("error"))
                throw std::runtime_error(
                    "request error: " + j["error"].dump());
            return RpcReply {
                .jsonrpc = j["jsonrpc"],
                .id = j["id"].get<std::string>(),
                .result = T::parseReply(j)
            };
        }
    };
}