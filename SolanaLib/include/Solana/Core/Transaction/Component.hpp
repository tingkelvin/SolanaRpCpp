#pragma once
#include <vector>
#include "Solana/Core/Types/Types.hpp"

namespace Solana::Transaction {
    template<typename Derived>
    struct Component {
        bool operator==(const Component & other) const = default;

        void serialize(Buffer & out) const {
            static_cast<const Derived*>(this)->serializeImpl(out);
        }
    };
}
