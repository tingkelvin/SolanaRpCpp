#pragma once

#include "RpcMethod.hpp"

namespace Solana
{
    template <typename T>
    struct WithJsonReply : public RpcMethod<WithJsonReply<T>>
    {
        using Reply = json;

        static Reply parseReply(const json &data)
        {
            return data["result"];
        }

        template <typename... Args>
        explicit WithJsonReply(Args &&...args) : inner(std::forward<Args>(args)...) {}

        json toJsonImpl() const { return inner.toJson(); }
        std::string methodNameImpl() const { return inner.methodName(); }

    private:
        T inner;
    };

}