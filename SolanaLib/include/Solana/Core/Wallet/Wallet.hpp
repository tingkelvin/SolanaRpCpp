#pragma once
#include "Solana/Core/Crypto/Crypto.hpp"
#include <string_view>

namespace Solana::Wallet {
    class Wallet {
    public:
        explicit Wallet(Crypto::Keypair kp) : kp(kp){}
        static Wallet fromSecretKey(std::string_view key) {
            return Wallet(Crypto::Keypair::fromSecretKey(key));
        }
    private:
        Crypto::Keypair kp;
    };
}
