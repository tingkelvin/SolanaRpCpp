#pragma once

#include "Solana/Core/Types/Types.hpp"
#include "Component.hpp"

namespace Solana::Transaction {
    struct Header : public Component<Header> {
        bool operator==(const Header & other) const = default;

        Header() = default;

        Header(
            u8 version,
            u8 sigRequired,
            u8 readOnlyAddress,
            u8 readOnlyAddressNoSig)
            : requiredSigs(sigRequired)
            , readOnlyAddresses(readOnlyAddress)
            , readOnlyAddressNoSig(readOnlyAddressNoSig)
        {
            this->version += version;
        }

        u8 version = 1 << 7;
        u8 requiredSigs = 0;
        u8 readOnlyAddresses = 0;
        u8 readOnlyAddressNoSig = 0;

        void serializeImpl(Buffer & out) const {
            out.push_back(version);
            out.push_back(requiredSigs);
            out.push_back(readOnlyAddresses);
            out.push_back(readOnlyAddressNoSig);
        }
    };
}