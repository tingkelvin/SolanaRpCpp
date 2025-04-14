#include "Solana/Core/Transaction/TransactionBuilder.hpp"

using namespace Solana::Transaction;
using namespace Solana::Crypto;

namespace {
    struct CompAccount {
        bool operator()(const Account & lhs, const Account & rhs) const {
            if (lhs.isSigner != rhs.isSigner) return lhs.isSigner;
            if (lhs.isWritable != rhs.isWritable) return lhs.isWritable;
            return lhs.key > rhs.key;
        }
    };
}

Solana::Buffer TransactionBuilder::serializeMessage() {
    const auto message = compileMessage();
    Solana::Buffer buf{};
    message.serialize(buf);
    return buf;
}

TransactionBuilder & TransactionBuilder::sign(const Keypair & kp) {
    const auto sig = kp.sign(serializeMessage());
    sigs.push_back(sig);
    signers.push_back(kp.pubkey);
    return *this;
}

Message TransactionBuilder::compileMessage() {

    std::set<Account, CompAccount> accounts{};
    for (auto & ins : instructions) {
        for (auto & ac : ins.accounts) {
            if (accounts.contains(ac)) {
                auto & account = *accounts.find(ac);
                ac.isWritable = account.isWritable || ac.isWritable;
                ac.isSigner = account.isSigner || ac.isSigner;
                accounts.erase(account);
            }
            accounts.insert(ac);
        }
    }

    auto feePayerAc = Account{.key = feePayer, .isSigner = true, .isWritable = true};
    accounts.erase(feePayerAc);
    std::vector<Account> acVector{feePayerAc};
    for (auto & account : accounts) acVector.push_back(account);

    Header header{};
    AddressSection keys{};
    for (auto & ac : acVector) {
        if (ac.isSigner) {
            ++header.requiredSigs;
            header.readOnlyAddresses += !ac.isWritable;
        } else {
            ++header.readOnlyAddressNoSig;
        }
        keys.push_back(ac.key);
    }
    for (auto & ins : instructions) {
        keys.push_back(ins.programId);
    }
    CompactArray<CompiledInstruction> compiledIns{};
    compiledIns.reserve(instructions.size());

    const auto indexOf = [&](const Pubkey & key) -> u8 {
        for (u8 i = 0; i < keys.size(); ++i) {
            if (keys[i] == key) {
                return i;
            }
        }
        throw std::runtime_error("Invalid key: " + key.toStdString());
    };

    for (auto & ins : instructions) {
        CompactArray<u8> addresses(ins.accounts.size());
        std::transform(
        ins.accounts.begin(),
        ins.accounts.end(),
        addresses.begin(),
        [&](const auto & a){
            return indexOf(a.key);
        });
        compiledIns.push_back({
            .programIndex = indexOf(ins.programId),
            .addressIndices = addresses,
            .data = ins.data
        });
    }

    header.version += version;
    return Message(
        header,
        keys,
        recentBlockHash,
        compiledIns);
}