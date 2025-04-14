#pragma once
#include "Header.hpp"
#include "CompactArray.hpp"
#include "Component.hpp"
#include "Instruction.hpp"
#include "Solana/Core/Types/Types.hpp"

namespace Solana::Transaction {
    struct BlockHash : public Bytes<32> {
        fromStr(BlockHash, 32)
    };

    struct AddressTableLookup {
        Pubkey key;
        CompactArray<u8> writableIndices;
        CompactArray<u8> readonlyIndices;

        void serialize(Buffer & out) const {
            key.serialize(out);
            writableIndices.serialize(out);
            readonlyIndices.serialize(out);
        }
    };

    using AddressSection = CompactArray<Pubkey>;

    struct Message : public Component<Message> {

        Message(
            const Header & header,
            const AddressSection & addressSection,
            const BlockHash & blockHash,
            const CompactArray<CompiledInstruction> & instructions)
            : header(header)
            , addresses(addressSection)
            , recentBlockhash(blockHash)
            , instructions(instructions)
        {}

        bool operator==(const Message & other) const = default;

        Header header;
        AddressSection addresses;
        BlockHash recentBlockhash;
        CompactArray<CompiledInstruction> instructions;

        // This isn't fully implemented yet, just doing this
        // as upfront work for supporting versioned txns
        CompactArray<AddressTableLookup> lookupTable = {};

        void serializeImpl(Buffer & out) const {
            header.serialize(out);
            addresses.serialize(out);
            recentBlockhash.serialize(out);
            instructions.serialize(out);
            lookupTable.serialize(out);
        }
    };
}
