#pragma once
#include "Component.hpp"
#include "CompactArray.hpp"
#include <string>
#include "Solana/Logger.hpp"

namespace Solana::Transaction
{

    struct Account
    {
        Pubkey key;
        bool isSigner;
        bool isWritable;

        bool operator==(const Account &other) const = default;

        void serialize(Solana::Buffer &out) const
        {
            key.serialize(out);
            out.push_back(isSigner);
            out.push_back(isWritable);
        }
    };

    struct Instruction
    {
        Pubkey programId;
        CompactArray<Account> accounts;
        Buffer data;
    };

    struct ConcreteInstruction
    {
        virtual Instruction toInstruction() const = 0;
    };

    struct JupiterV6Instruction
    {
        std::array<uint8_t, 8> instruction_id; // e.g., method discriminator
        std::array<uint8_t, 32> source;
        std::array<uint8_t, 32> destination;
        std::array<uint8_t, 32> authority;
        uint64_t amount_in;
        uint64_t min_amount_out;
    };

    static std::optional<JupiterV6Instruction> decodeJupiterV6Instruction(std::string data)
    {
        LOG_INFO("Decoded Jupiter Base58 data: {}", data.size());

        std::ostringstream oss;
        for (auto byte : data)
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
        std::string hexString = oss.str();
        LOG_INFO("Decoded Jupiter Base58 data: {}", hexString);

        JupiterV6Instruction instr;
        std::memcpy(instr.instruction_id.data(), data.data(), 8);
        std::memcpy(instr.source.data(), data.data() + 8, 32);
        std::memcpy(instr.destination.data(), data.data() + 40, 32);
        std::memcpy(instr.authority.data(), data.data() + 72, 32);
        std::memcpy(&instr.amount_in, data.data() + 104, 8);
        std::memcpy(&instr.min_amount_out, data.data() + 112, 8);

        return instr;
    }

    struct CompiledInstruction
    {
        u8 programIndex;
        CompactArray<u8> addressIndices;
        Buffer data;
        void serialize(Buffer &out) const
        {
            out.add(programIndex);
            addressIndices.serialize(out);
            CompactArray<u8> dataCA(data.begin(), data.end());
            dataCA.serialize(out);
        }
    };
}

namespace std
{
    template <>
    struct hash<Solana::Transaction::Account>
    {
        size_t operator()(const Solana::Transaction::Account &ac) const
        {
            return std::hash<std::string>()(
                ac.key.toStdString() + std::to_string(ac.isSigner) + std::to_string(ac.isWritable));
        }
    };
}