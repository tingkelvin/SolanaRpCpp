#include <gtest/gtest.h>
#include "Solana/Core/Encoding/Base58.hpp"
#include "Solana/Core/Encoding/Layout.hpp"
#include <string_view>
#include "Solana/Core/Types/Types.hpp"
#include "Solana/Core/Transaction/TransactionBuilder.hpp"
#include "Solana/System/System.hpp"
#include "Solana/Core/Crypto/Crypto.hpp"

#define CLASS Encoding_tests
using namespace Solana::Encoding;
using namespace Solana;
using namespace Solana::Crypto;
using namespace Solana::Transaction;

namespace {
    const std::string pubKeyString = "G3QzaxUKTxsrb2yY3gsW59tjvJCktrux54XedNY6BAJo";
}

LAYOUT(TestAccount, (u32, t1), (u16, t2))

TEST(CLASS, Pukey_B58) {
    const auto decoded = Pubkey::fromString(pubKeyString);
    EXPECT_EQ(pubKeyString, decoded.toStdString());
}

TEST(CLASS, TestSerializePubkey) {
    auto key = Pubkey{2, 32, 45, 56, 34, 64, 64, 34, 34, 35, 34, 65, 57, 34, 45, 34, 45, 64, 34, 43, 34, 34, 34, 43, 43, 43, 34, 56, 56, 56, 45, 98};
    Buffer buf{};
    key.serialize(buf);
    auto expected = Buffer{2, 32, 45, 56, 34, 64, 64, 34, 34, 35, 34, 65, 57, 34, 45, 34, 45, 64, 34, 43, 34, 34, 34, 43, 43, 43, 34, 56, 56, 56, 45, 98};
    EXPECT_EQ(expected, buf);
}

// Test Buffer construction
TEST(BufferTest, ConstructFromString) {
    std::string testStr = "Hello, world!";
    Buffer buf(testStr);
    
    EXPECT_EQ(buf.size(), testStr.size());
    EXPECT_EQ(std::string(buf.begin(), buf.end()), testStr);
}

// Test adding integers
TEST(BufferTest, AddInteger) {
    Buffer buf;
    uint32_t value = 0x12345678;
    buf.add(value);
    
    EXPECT_EQ(buf.size(), sizeof(uint32_t));
    // Test depends on endianness
    EXPECT_EQ(buf[0], 0x78);  // Little endian representation
    EXPECT_EQ(buf[1], 0x56);
    EXPECT_EQ(buf[2], 0x34);
    EXPECT_EQ(buf[3], 0x12);
}

// Test serialization
TEST(BufferTest, Serialization) {
    Buffer original;
    original.add(uint32_t(42));
    original.add("test");
    
    Buffer serialized;
    original.serialize(serialized);
    
    EXPECT_EQ(original.size(), serialized.size());
    EXPECT_TRUE(std::equal(original.begin(), original.end(), serialized.begin()));
}

TEST(BufferTest, AddInteger2) {
    Buffer buf;

    // Test with different types of integers
    uint32_t num32 = 0x12345678;
    uint64_t num64 = 0x123456789ABCDEF0;

    buf.add(num32);
    buf.add(num64);

    EXPECT_EQ(buf.size(), sizeof(uint32_t)+sizeof(uint64_t));
    // Test depends on endianness
    EXPECT_EQ(buf[0], 0x78);  // Little endian representation
    EXPECT_EQ(buf[1], 0x56);
    EXPECT_EQ(buf[2], 0x34);
    EXPECT_EQ(buf[3], 0x12);
        // Test depends on endianness
    EXPECT_EQ(buf[4], 0xF0);  // Little endian representation

}

// Test Base58 encoding
TEST(BufferTest, Base58Encoding) {
    Buffer buf;
    buf.add(uint8_t(0x00));  // Add a known byte sequence
    buf.add(uint8_t(0x01));
    buf.add(uint8_t(0x02));
    
    std::string encoded = buf.toString();
    // Expected result based on your Base58 implementation
    EXPECT_EQ(encoded, "15T"); // This would depend on your actual Base58 encoding
}


TEST(CLASS, TestSerializeCompactArrayPubkey) {
    auto arr = CompactArray<Pubkey>{
            Pubkey{2, 32, 45, 56, 34, 64, 64, 34, 34, 35, 34, 65, 57, 34, 45, 34, 45, 64, 34, 43, 34, 34, 34, 43, 43, 43, 34, 56, 56, 56, 45, 98},
            Pubkey{2, 32, 45, 56, 34, 64, 64, 34, 34, 35, 34, 65, 57, 34, 45, 34, 45, 64, 34, 43, 34, 34, 74, 43, 43, 43, 34, 56, 56, 56, 45, 98}
    };

    Buffer buf{};
    arr.serialize(buf);

    auto expected = Buffer{2,
                           2, 32, 45, 56, 34, 64, 64, 34, 34, 35, 34, 65, 57, 34, 45, 34, 45, 64, 34, 43, 34, 34, 34, 43, 43, 43, 34, 56, 56, 56, 45, 98,
                           2, 32, 45, 56, 34, 64, 64, 34, 34, 35, 34, 65, 57, 34, 45, 34, 45, 64, 34, 43, 34, 34, 74, 43, 43, 43, 34, 56, 56, 56, 45, 98
                           };
    EXPECT_EQ(expected, buf);
}

TEST(CLASS, AccountSerialization) {
    auto account = TestAccount {
        .t1 = 120,
        .t2 = 45
    };
    auto serialized = account.encode();

    account.t1 = 200;
    account.decode(serialized);
    auto orig = TestAccount {
        .t1 = 120,
        .t2 = 45
    };
    EXPECT_TRUE(account == orig);
}

TEST(CLASS, CompactArraySerializationTest) {
    auto arr = CompactArray<u8> {
        1, 255, 67
    };
    Buffer buf; arr.serialize(buf);
    auto expected = Buffer{3, 1, 255, 67};
    EXPECT_EQ(expected, buf);
}

TEST(CLASS, HeaderSerializationTest) {
    auto header = Header(0, 3, 2, 1);
    Buffer buf;
    header.serialize(buf);
    auto expected = Buffer{(1 << 7), 3, 2, 1 };
    EXPECT_EQ(expected, buf);
}

TEST(CLASS, InstructionSerializationTest) {
    LAYOUT(TestLayout, (u32, t1), (u64, t2))
    auto ins = CompiledInstruction {
        .programIndex = 3,
        .addressIndices = CompactArray<u8>{ 12, 34 },
        .data = TestLayout{.t1 = 10000, .t2 = 100}.encode()
    };
    Buffer buf;
    ins.serialize(buf);
    auto expected = Buffer{
        3, // programIndex
        2, 12, 34, // CompactArray<u8>{12, 34} → length=2, values=12, 34
        12, // length of `data` (encoded layout)
        16, 39, 0, 0, // u32 t1 = 10000
        100, 0, 0, 0, 0, 0, 0, 0, 
    };
    EXPECT_EQ(expected, buf);
}

TEST(CLASS, CompactArrayCompiledInstructionSerializationTest) {
    auto arr = CompactArray<CompiledInstruction> {
        CompiledInstruction {
            .programIndex = 3,
            .addressIndices = {1, 2},
            .data = Buffer{2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0}
        }
    };

    Buffer buf{};
    arr.serialize(buf);

    auto expected = Buffer{
        1, 3,
        2, 1, 2,
        12, 2, 0, 0, 0,
        100, 0, 0, 0, 0, 0, 0, 0,
    };

    EXPECT_EQ(expected, buf);

    auto arr2 = CompactArray<CompiledInstruction> {
        CompiledInstruction {
            .programIndex = 3,
            .addressIndices = {1, 2},
            .data = Buffer{2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0}
        },
        CompiledInstruction {
            .programIndex = 3,
            .addressIndices = {1, 2},
            .data = Buffer{2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0}
        }
    };

    Buffer buf2{};
    arr2.serialize(buf2);

    auto expected2 = Buffer{
        2, // CompactArray size (2)
        3,
        2, 1, 2,
        12, 2, 0, 0, 0,
        100, 0, 0, 0, 0, 0, 0, 0,
        3,
        2, 1, 2,
        12, 2, 0, 0, 0,
        100, 0, 0, 0, 0, 0, 0, 0,
    };

    EXPECT_EQ(expected2, buf2);
}

TEST(CLASS, TransferSerializationTest) {
    auto kp = Keypair::fromSecretKey("3ffS3Y7v2iVFjpxe83WK6RxzYwCpfbVwvvEyuG52pyrvf6umUiVXUXLWKsHwRUKUtyhP99LfV4ciNYuWx2gRhhKd");
    auto transfer = Programs::System::Transfer(
        kp.pubkey,
        Pubkey::fromString("5dEU1ec2Dw6C8v1jhtnRN6ZYnnVE54Yn3hJDh4U4fyZJ"),
        100
    );

    auto buf = transfer.data.encode();
    auto expected = Buffer{2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(buf, expected);
}

TEST(CLASS, TxnMessageSerializationTest) {

    auto signer = Pubkey::fromString("6fY6rYZyJcNJsBkQkkAS64nS4LRWcLdkKAs1eYWqJpEb");

    auto builder = TransactionBuilder(
            BlockHash::fromString("4C76AqhSHrWND8tuqvZ37p62ssdtK4NGsfCm5kMUhNJt"), signer)
            .add(Programs::System::Transfer(
                    signer,
                    Pubkey::fromString("5dEU1ec2Dw6C8v1jhtnRN6ZYnnVE54Yn3hJDh4U4fyZJ"),
                    100
            ));

    auto message = builder.compileMessage();

    auto header = Header(0, 1, 0, 1);
    const auto expected = Message(
        header,
        AddressSection{
            Pubkey::fromString("6fY6rYZyJcNJsBkQkkAS64nS4LRWcLdkKAs1eYWqJpEb"),
            Pubkey::fromString("5dEU1ec2Dw6C8v1jhtnRN6ZYnnVE54Yn3hJDh4U4fyZJ"),
            Pubkey::fromString("11111111111111111111111111111111")
        },
        BlockHash::fromString("4C76AqhSHrWND8tuqvZ37p62ssdtK4NGsfCm5kMUhNJt"),
        CompactArray<CompiledInstruction> {
            CompiledInstruction {
                .programIndex = 2,
                .addressIndices = {0, 1},
                .data = Buffer{2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0}
            }
        }
    );

    EXPECT_EQ(expected, message);

    const std::string expectedTxn = "6GhS2ZJZxJoJbYDstaxrESoUWEMqED9cYc8gvsFwomUHGvQCYxQY1bSHNmXHJYNGvtptbbWSEFe3y2XukfuVZxJRLbCa4zq5zrDHK2fr5sP9pgh2bKCqNz5zFvWzty9f3BhgeGwc35SK9XZE58xpTdhoE8aaKJD1o28hfab5yPiwcNJ7ppXykyKGbshoq32E2SBMYYbqiGAcNMHy";

    const auto decoded = *Encoding::Base58::Decode(expectedTxn);

    const auto actual = builder.serializeMessage();

    EXPECT_EQ(Buffer(decoded), actual);

    EXPECT_EQ(
        decoded.size(),
        actual.size());

    EXPECT_EQ(expectedTxn, actual.toString());
}