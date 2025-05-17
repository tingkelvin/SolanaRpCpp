#pragma once
#include <optional>
#include <string>
#include "RpcMethod.hpp"

namespace Solana {

    template<size_t N>
    struct StringLiteral {
        constexpr StringLiteral(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }

        char value[N];
    };

    template<typename T, StringLiteral name>
    struct ConfigParam : std::optional<T> {
        ConfigParam(const T & val) : std::optional<T>(val){}
        ConfigParam() : std::optional<T>(){};

        void addToJson(json & data) const {
            if (this->has_value()) {
                data[name.value] = this->value();
            }
        }
    };

#define RPCPARAM(type, name) ConfigParam<type, #name> name

    enum CommitmentLevel {
        Confirmed,
        Finalized
    };

    struct Commitment : ConfigParam<std::string, "commitment"> {

        template<typename T>
        Commitment(const T & val) : ConfigParam<std::string, "commitment">(val){}
        Commitment(CommitmentLevel level)
            : ConfigParam<std::string, "commitment">([level]() {
               switch (level) {
                   case Confirmed:
                       return "confirmed";
                   case Finalized:
                       return "finalized";
               }
            }())
        {}
        Commitment() = default;
    };

    struct MinContextSlot : ConfigParam<int64_t, "minContextSlot"> {

        template<typename T>
        MinContextSlot(const T & val) : ConfigParam<int64_t, "minContextSlot">(val){}
        MinContextSlot() = default;
    };

    enum EncodingType {
        JsonParsed,
        Base58,
        Base64,
        Json
    };

    struct SimpleEncoding : ConfigParam<std::string, "encoding"> {
        template<typename T>
        SimpleEncoding(const T & val) : ConfigParam<std::string, "encoding">(val) {}
        SimpleEncoding(EncodingType type)
        : ConfigParam<std::string, "encoding">(str(type))
        {}

        SimpleEncoding() = default;

        static std::string str(EncodingType t) {
            switch (t) {
                case Base58:
                    return "base58";
                case Base64:
                    return "base64";
                default:
                    throw std::runtime_error("Unsupported type");
            }
        }
    };

    struct TransactionEncoding : ConfigParam<std::string, "encoding"> {
        template<typename T>
        TransactionEncoding(const T & val) : ConfigParam<std::string, "encoding">(val) {}
        TransactionEncoding(EncodingType type)
            : ConfigParam<std::string, "encoding">(str(type))
        {}

        static std::string str(EncodingType t) {
            switch (t) {
                case JsonParsed:
                    return "jsonParsed";
                case Base58:
                    return "base58";
                case Base64:
                    return "base64";
                case Json:
                    return "json";
                default:
                    throw std::runtime_error("Unsupported type");
            }
        }
        TransactionEncoding() = default;
    };

    struct AccountEncoding : ConfigParam<std::string, "encoding"> {
        template<typename T>
        AccountEncoding(const T & val) : ConfigParam<std::string, "encoding">(val) {}
        AccountEncoding(EncodingType type)
            : ConfigParam<std::string, "encoding">(str(type))
        {}

        static std::string str(EncodingType t) {
            switch (t) {
                case JsonParsed:
                    return "jsonParsed";
                case Base58:
                    return "base58";
                case Base64:
                    return "base64";
                default:
                    throw std::runtime_error("Unsupported type");
            }
        }

        AccountEncoding() = default;
    };
}

