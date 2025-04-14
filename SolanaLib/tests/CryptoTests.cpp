#include <gtest/gtest.h>
#include "Solana/Core/Crypto/Crypto.hpp"
#include "Solana/Core/Encoding/Base58.hpp"

#define CLASS CryptoTests


namespace  {
    const std::string TransferTxn = "87PYsNDxaKYiA1gma7e34RnUZ5aXvZuKdzHjYCuPWhjHGHMoKA2haEuqedg9eyfTXc3FCeVVyFt3HS4hdS5YAM8AJaREBRfT1ZDsv4KzW8qMfK16tPVdqaLSnDae8Fz7xEQSTnjeLnXQhgMPw3X1yjaB9nTuMutDJDEGq2WzrLdR72yvUChC7obh1Mh9pKHfvZvrwZsEPm6o";
}

namespace Solana::Crypto {

    TEST(CLASS, MessageSign) {
        const auto message =
                Encoding::Base58::Decode(TransferTxn);

        auto kp = Keypair::fromSecretKey("3ffS3Y7v2iVFjpxe83WK6RxzYwCpfbVwvvEyuG52pyrvf6umUiVXUXLWKsHwRUKUtyhP99LfV4ciNYuWx2gRhhKd");


        const auto buf = Buffer(*message);

        EXPECT_EQ(Encoding::Base58::Encode(buf.data(), buf.data() + buf.size()), TransferTxn);

        auto sig = kp.sign(buf);

        EXPECT_TRUE(kp.verify(sig, buf));

        EXPECT_TRUE(kp.verify(
                Signature::fromString("5qWRotP6jvVVr5gC7Bm8zry9nSstXnpvBBuQUAEjFjfxzK26Q533Mg6c1YYTyvrkTpria3z4yvro23y8scu6jLnJ"),
                buf));
    }
}