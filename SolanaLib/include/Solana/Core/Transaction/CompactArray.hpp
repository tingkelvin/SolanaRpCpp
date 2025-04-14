#pragma once
#include "Solana/Core/Types/Types.hpp"
#include <array>
#include "Component.hpp"
#include <type_traits>

namespace Solana::Transaction {

    template<typename T = unsigned char>
    class CompactArray :
        public std::vector<T>,
        public Component<CompactArray<T>>
    {
        using std::vector<T>::vector;
        public:
            void serializeImpl(Buffer & out) const {
                auto len = this->size();
                while (true) {
                   u8 l = len & 0x7F;
                   len >>= 7;
                   if (len == 0) {
                       out.push_back(l);
                       break;
                   } else {
                       l |= 0x80;
                       out.push_back(l);
                   }
                }

                if constexpr (std::is_same_v<T, u8>) {
                    for (auto el : *this) {
                        out.push_back(el);
                    }
                }
                else if constexpr (std::is_arithmetic_v<T>) {
                    for (auto el : *this) {
                        out.add(el);
                    }
                }
                else {
                    for (auto & el : *this) {
                        el.serialize(out);
                    }
                }
            }
    private:
        static constexpr u8 MASK = 0x7F;
        static constexpr u16 MID_SEVEN = 0x1F80;
    };
}
