#pragma once
#include "Solana/Core/Types/Types.hpp"
#include <vector>
#include "Message.hpp"
#include "CompactArray.hpp"

namespace Solana {
    namespace Transaction {
        using Signatures = Transaction::CompactArray<Signature>;
    }
    class Txn {
    public:
        Txn(const Transaction::Signatures & signatures,
            const Transaction::Message & message)
            : signatures(signatures), message(message){}
        Buffer serialize() const {
            auto b = Buffer();
            signatures.serialize(b);
            message.serialize(b);
            return b;
        }

    private:
        Transaction::Signatures signatures;
        Transaction::Message message;
    };
}


