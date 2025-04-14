#pragma once
#include <type_traits>
#include <typeinfo>
#include <string>
#include <assert.h>
#include "Solana/Core/Encoding/Base58.hpp"
#include <iostream>
#include "Solana/Core/Types/Types.hpp"

namespace TypeUtils {

    template<class T>
    struct is_optional {
        static const bool value = false;
    };

    template<class T>
    struct is_optional<std::optional<T>> {
        static const bool value = true;
    };

    template<typename T>
    struct is_pubkey {
        static const bool value = false;
    };

    template<> struct is_pubkey<Solana::Pubkey> {
        static const bool value = true;
    };

    template <typename T>
    struct is_string
    {
        static const bool value = false;
    };

    template <int T>
    struct is_string<Solana::Bytes<T>>
    {
        static const bool value = true;
    };

}

// TODO: Fully implemented all of the expected data types

class LayoutEncoder {
public:
    template <typename ... T>
    constexpr LayoutEncoder & Encode(const T& ... types) {
        // Encode((types...));
        (Encode(types), ...);
        // printf("asd");
        return *this;
    }

    template<typename T>
    constexpr LayoutEncoder & Encode(const T value) {
        if constexpr (std::is_integral<T>::value) {
            const size_t typeSize = sizeof(T);
            uint8_t offset = 0;

            for (size_t i = 0; i < typeSize; i++)
            {
                buffer.push_back((value >> offset) & 0xff);
                offset += 8;
            }
        }
        if constexpr (std::is_floating_point<T>::value)
        {
            assert(!std::isnan(value));

            // From https://github.com/naphaso/cbor-cpp/blob/master/src/encoder.cpp
            const void *punny = &value;

            for (size_t i = 0; i < sizeof(T); i++)
            {
                buffer.push_back(*((uint8_t *)punny + i));
            }
        }
        if constexpr (std::is_placeholder<T>::value) {
            return *this;
        }

        return *this;
    }

    Solana::Buffer & getBuffer() { return buffer; }
private:
    Solana::Buffer buffer{};
};

class LayoutDecoder {
public:
    template <typename ... Types>
    void Decode(const Solana::Buffer & bufferBegin, Types& ... retrieveValues)
    {
        uint8_t *offset = (uint8_t *)bufferBegin.data();
        (DecodeInternal<Types>(&offset, retrieveValues), ...);
    }

private:

    void handlePubkey(uint8_t **offset, Solana::Pubkey & value) {
        std::copy_n(*offset, 32, value.begin());
        (*offset) += 32;
    }
    template <typename T>
    void DecodeInternal(uint8_t **offset, T& value)
    {
        if constexpr (TypeUtils::is_optional<T>::value) {
            using Inner = typename T::value_type;
            auto flag = (*(int*)*offset) == 1;
            (*offset) += 4;

            if constexpr (TypeUtils::is_pubkey<Inner>::value) {
                if (flag) {
                    Inner temp{};
                    handlePubkey(offset, temp);
                    value = temp;
                } else {
                    (*offset) += 32;
                }
            }
            else if constexpr (std::is_arithmetic<Inner>::value) {
                if (flag)
                    value = *((Inner *)*(offset));
                (*offset) += sizeof(Inner);
            }
        }
        else if constexpr (std::is_arithmetic<T>::value)
        {
            value = *((T *)*(offset));
            (*offset) += sizeof(T);
        }

        else if constexpr (TypeUtils::is_pubkey<T>::value) {
            handlePubkey(offset, value);
        }
        else if constexpr (TypeUtils::is_string<T>::value)
        {
            uint32_t strSize = *((uint32_t *)*(offset));
            (*offset) += 4;

            value = std::string((*offset), ((*offset) + strSize));

            (*offset) += strSize;
        }
        else if constexpr (std::is_enum<T>::value) {
            value = *((T *)*(offset));
            (*offset) += sizeof(T);
        }
        else if constexpr (std::is_placeholder<T>::value) {
            return;
        }
        else
        {
            assert(false && "The type of the array is not supported");
        }
    }
};