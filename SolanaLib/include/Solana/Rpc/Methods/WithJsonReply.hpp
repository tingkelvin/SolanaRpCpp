#pragma once

#include "RpcMethod.hpp"

namespace Solana {

    template<typename T>
    concept IsRpcMethod = std::is_base_of<RpcMethod, T>::value;

    template<IsRpcMethod T>
    struct WithJsonReply : public RpcMethod {

        using Reply = json;

        static Reply parseReply(const json & data) {
            return data["result"];
        }

        template <typename ...Args>
        explicit WithJsonReply(Args ...args) : inner(args...) {}

        std::string methodName() const override { return inner.methodName(); };
        json toJson() const override {
            return inner.toJson();
        }
    private:
        T inner;
    };
}