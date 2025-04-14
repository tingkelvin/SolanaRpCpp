
#pragma once
#include "Solana/Core/Transaction/Instruction.hpp"
#include "Transaction.hpp"
#include <memory>
#include <vector>
#include <set>
#include "Solana/Core/Crypto/Crypto.hpp"

namespace Solana::Transaction {

    class TransactionBuilder {
    public:
        TransactionBuilder(const BlockHash & recentHash, Pubkey feepayer)
            : recentBlockHash(recentHash), feePayer(feepayer){}

        TransactionBuilder & add(const ConcreteInstruction & instruction) {
            return add(instruction.toInstruction());
        }

        TransactionBuilder & add(const Instruction & instruction) {
            instructions.push_back(instruction);
            return *this;
        }

        TransactionBuilder & setVersion(u8 v) {
            version = v;
            return *this;
        }

        Txn build() {
            return Txn(
                sigs,
                compileMessage()
            );
        }

        Buffer serializeMessage();
        TransactionBuilder & sign(const Solana::Crypto::Keypair & kp);
        Message compileMessage();

    private:
        BlockHash recentBlockHash;
        Signatures sigs;
        std::vector<Pubkey> signers;
        std::vector<Instruction> instructions;
        Pubkey feePayer;
        u8 version = 0;
    };
}
