#pragma once
#include "Component.hpp"
#include "CompactArray.hpp"
#include <string>

namespace Solana::Transaction {

    struct Account {
        Pubkey key;
        bool isSigner;
        bool isWritable;

        bool operator==(const Account & other) const = default;

        void serialize(Solana::Buffer & out) const {
            key.serialize(out);
            out.push_back(isSigner);
            out.push_back(isWritable);
        }
    };

    struct Instruction {
        Pubkey programId;
        CompactArray<Account> accounts;
        Buffer data;
    };

    struct ConcreteInstruction {
        virtual Instruction toInstruction() const = 0;
    };

    struct CompiledInstruction {
        u8 programIndex;
        CompactArray<u8> addressIndices;
        Buffer data;
        void serialize(Buffer & out) const {
            out.add(programIndex);
            addressIndices.serialize(out);
            CompactArray<u8> dataCA(data.begin(), data.end());
            dataCA.serialize(out);
        }
    };
}

namespace std {
    template<>
    struct hash<Solana::Transaction::Account> {
        size_t operator()(const Solana::Transaction::Account & ac) const {
            return std::hash<std::string>()(
                    ac.key.toStdString()
                    + std::to_string(ac.isSigner)
                    + std::to_string(ac.isWritable));
        }
    };
}