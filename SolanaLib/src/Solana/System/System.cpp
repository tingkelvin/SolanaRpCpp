//
// Created by Brian Grenier on 2023-12-21.
//

#include "Solana/System/System.hpp"

using namespace Solana::Programs::System;

Transfer::Transfer(
        const Pubkey & from,
        const Pubkey & to,
        u64 lamports)
        : data(Data{.number=2, .lamports = lamports})
        , toAccount(Transaction::Account {
                .key = to,
                .isSigner = false,
                .isWritable = true,
        })
        , fromAccount(Transaction::Account {
                .key = from,
                .isSigner = true,
                .isWritable = true,
        }){}

Solana::Transaction::Instruction Transfer::toInstruction() const {

    return Solana::Transaction::Instruction {
        .programId = ProgramId,
        .accounts = Transaction::CompactArray<Transaction::Account>{
            fromAccount,
            toAccount
        },
        .data = data.encode()
    };
}

Allocate::Allocate(const Solana::Pubkey &key, Solana::u64 space)
    : target(Transaction::Account{
        .key = key,
        .isSigner = true,
        .isWritable = true
    }),
    data(Data{.number = 8, .allocatedSpace = space})
{}

Solana::Transaction::Instruction Allocate::toInstruction() const {
    return Solana::Transaction::Instruction {
        .programId = ProgramId,
        .accounts = Transaction::CompactArray<Transaction::Account>{
            target
        },
        .data = data.encode()
    };
}

