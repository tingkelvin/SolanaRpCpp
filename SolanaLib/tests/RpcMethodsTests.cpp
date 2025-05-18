#include <gtest/gtest.h>
#include "Solana/Rpc/Rpc.hpp"
#include "Solana/Rpc/Methods/GetAccountInfo.hpp"

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

    auto future = rpc.send(request);

    try
    {
        auto reply = future.get();

        // Basic field checks
        EXPECT_TRUE(reply.result.lamports > 0) << "Lamports should be greater than 0";
        EXPECT_GE(reply.result.space, 0) << "Space should be >= 0";

        // Owner must be valid base58
        auto ownerStr = reply.result.owner.toStdString();
        EXPECT_FALSE(ownerStr.empty()) << "Owner pubkey string should not be empty";

        // Check accountData type
        EXPECT_TRUE(reply.result.accountData.is_object() || reply.result.accountData.is_array() || reply.result.accountData.is_string())
            << "Account data should be a valid JSON structure";

        // Log output for debug
        std::cout << "Executable: " << (reply.result.executable ? "true" : "false") << "\n";
        std::cout << "Lamports: " << reply.result.lamports << "\n";
        std::cout << "Owner: " << ownerStr << "\n";
        std::cout << "Rent Epoch: " << reply.result.rentEpoch << "\n";
        std::cout << "Space: " << reply.result.space << "\n";
        std::cout << "Account Data (JSON): " << reply.result.accountData.dump(2) << "\n";
    }
    catch (const std::exception &ex)
    {
        FAIL() << "RPC call failed or threw an exception: " << ex.what();
    }
}
