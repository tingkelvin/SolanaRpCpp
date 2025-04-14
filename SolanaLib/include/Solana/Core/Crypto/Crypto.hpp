#pragma once
#include <array>
#include <string_view>
#include "Solana/Core/Types/Types.hpp"

namespace Solana::Crypto {
    class Keypair {
    public:
        Keypair(Pubkey pubKey, PrivateKey pk) : pubkey(pubKey), privateKey(pk){}

        Signature sign(const Buffer & data) const;
        bool verify(const Signature & sig, const Buffer & message);

        static Keypair generateKeyPair();
        static Keypair fromSecretKey(std::string_view sk);
//        static Keypair fromPrivateKey(std::string_view pk);

        Pubkey pubkey;
        PrivateKey privateKey;
    };
}