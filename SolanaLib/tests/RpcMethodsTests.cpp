#include <gtest/gtest.h>
#include "Solana/Rpc/Rpc.hpp"
#include "Solana/Rpc/Methods/GetAccountInfo.hpp"
#include "Solana/Rpc/Methods/GetSignaturesForAddress.hpp"

class SolanaRpcTest : public ::testing::Test
{
protected:
    Solana::Rpc rpc{"https://api.devnet.solana.com"};
};

// Test fetching account info from a known devnet address
TEST_F(SolanaRpcTest, GetAccountInfoReturnsValidData)
{
    Solana::GetAccountInfo<> request("vines1vzrYbzLMRdu58ou5XTby4qAqVRLmqo36NKPTg");

    // Optional: set encoding explicitly
    // request.config.encoding = Solana::AccountEncoding(Solana::EncodingType::Base64);

    auto reply = rpc.send(request).get();

    // Basic field checks
    EXPECT_TRUE(reply.result.lamports > 0) << "Lamports should be greater than 0";
    EXPECT_GE(reply.result.space, 0) << "Space should be >= 0";

    // Owner must be valid base58
    auto ownerStr = reply.result.owner.toStdString();
    EXPECT_FALSE(ownerStr.empty()) << "Owner pubkey string should not be empty";

    // Check accountData type
    EXPECT_TRUE(reply.result.accountData.is_object() || reply.result.accountData.is_array() || reply.result.accountData.is_string())
        << "Account data should be a valid JSON structure";
}

// Test fetching account info from a known devnet address
TEST_F(SolanaRpcTest, GetSignaturesForAddressReturnsValidData)
{
    Solana::GetSignaturesForAddress baseRequest("Vote111111111111111111111111111111111111111");
    baseRequest.config.commitment = Solana::Commitment(Solana::CommitmentLevel::Confirmed);
    baseRequest.config.limit = 10;

    auto baseReply = rpc.send(baseRequest).get();

    // Basic field checks
    EXPECT_EQ(baseReply.result.signatures.size(), 10);
    for (const auto &sigInfo : baseReply.result.signatures)
    {
        EXPECT_FALSE(sigInfo.signature.empty()) << "Signature should not be empty";
        EXPECT_GT(sigInfo.slot, 0) << "Slot should be a positive integer";
    }

    for (size_t i = 1; i < baseReply.result.signatures.size(); ++i)
    {
        EXPECT_GE(baseReply.result.signatures[i - 1].slot, baseReply.result.signatures[i].slot)
            << "Signatures should be sorted by descending slot";
    }

    std::string oldestSig = baseReply.result.signatures.back().signature;
    std::string newestSig = baseReply.result.signatures.front().signature;

    Solana::GetSignaturesForAddress beforeRequest("Vote111111111111111111111111111111111111111");
    beforeRequest.config.before = oldestSig;
    beforeRequest.config.limit = 5;

    auto beforeReply = rpc.send(beforeRequest).get();

    // All signatures should be older (i.e., lower slot) than the `before` one
    for (const auto &sig : beforeReply.result.signatures)
    {
        EXPECT_LT(sig.slot, baseReply.result.signatures.back().slot)
            << "Signature should be older than the 'before' reference";
    }

    Solana::GetSignaturesForAddress untilRequest("Vote111111111111111111111111111111111111111");
    untilRequest.config.until = newestSig;
    untilRequest.config.limit = 5;

    auto untilReply = rpc.send(untilRequest).get();

    // All signatures should be newer (or equal) to the given one
    for (const auto &sig : untilReply.result.signatures)
    {
        EXPECT_LE(sig.slot, baseReply.result.signatures.front().slot)
            << "Signature should be equal or older than the 'until' reference";
    }
}
