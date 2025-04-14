#pragma once
#include "Solana/Core/Transaction/Instruction.hpp"
#include "Solana/Core/Encoding/Layout.hpp"


namespace Solana::Programs::System {

    static const Pubkey ProgramId = Pubkey::fromString("11111111111111111111111111111111");

    struct Transfer : public Transaction::ConcreteInstruction {

        LAYOUT(Data,
               (u32, number),
               (u64, lamports))

        Transfer(
            const Pubkey & from,
            const Pubkey & to,
            u64 lamports);

        Transaction::Instruction toInstruction() const override;

        Transaction::Account toAccount;
        Transaction::Account fromAccount;
        Data data;
    };

    struct Allocate : public Transaction::ConcreteInstruction {
        LAYOUT(Data,
               (u32, number),
               (u64, allocatedSpace))

        Allocate(const Pubkey & key, u64 space);
        Transaction::Instruction toInstruction() const override;

        Transaction::Account target;
        Data data;
    };
};

