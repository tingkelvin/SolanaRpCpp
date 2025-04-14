#include "Solana/Core/Crypto/Crypto.hpp"
#include <openssl/evp.h>
#include "Solana/Core/Encoding/Base58.hpp"
#include <iostream>


using namespace Solana::Crypto;

Keypair Keypair::generateKeyPair() {
    EVP_PKEY * pKey = nullptr;
    auto * ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_keygen(ctx, &pKey);

    size_t pubKeylen;
    EVP_PKEY_get_raw_public_key(pKey, nullptr, &pubKeylen);

    Pubkey pubKey;
    EVP_PKEY_get_raw_public_key(pKey, pubKey.data(), &pubKeylen);

    size_t privKeyLen;
    EVP_PKEY_get_raw_private_key(pKey, nullptr, &privKeyLen);
    PrivateKey privKey;
    EVP_PKEY_get_raw_private_key(pKey, privKey.data(), &privKeyLen);

    EVP_PKEY_CTX_free(ctx);

    return {pubKey, privKey};
}

Keypair Keypair::fromSecretKey(std::string_view sk) {
    auto ret = Solana::Encoding::Base58::Decode(sk);
    PrivateKey privKey;
    std::copy_n(ret->begin(), 32, privKey.begin());
    Pubkey pubKey;
    std::copy_n(ret->cbegin() + 32, 32, pubKey.begin());


    return {pubKey, privKey};
}

Solana::Signature Keypair::sign(const Solana::Buffer & data) const {
    size_t sig_len;
    Solana::Signature sig{};
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    auto * ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
    EVP_PKEY * pKey = EVP_PKEY_new_raw_private_key(
        EVP_PKEY_ED25519,
        nullptr,
        privateKey.data(),
        reinterpret_cast<size_t>(privateKey.size()));

    EVP_DigestSignInit(md_ctx, nullptr, nullptr, nullptr, pKey);
    EVP_DigestSign(md_ctx, nullptr, &sig_len, data.data(), data.size());
    assert(sig_len == 64);
    EVP_DigestSign(md_ctx, sig.data(), &sig_len, data.data(), data.size());
    EVP_MD_CTX_free(md_ctx);

    return sig;
}

bool Keypair::verify(const Signature & sig, const Buffer & message) {
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    EVP_PKEY * pKey = EVP_PKEY_new_raw_public_key(
            EVP_PKEY_ED25519,
            nullptr,
            pubkey.data(),
            reinterpret_cast<size_t>(pubkey.size()));

    EVP_DigestVerifyInit(md_ctx, nullptr, nullptr, nullptr, pKey);

    auto ret = EVP_DigestVerify(
            md_ctx,
            sig.data(),
            sig.size(),
            message.data(),
            message.size());

    EVP_MD_CTX_free(md_ctx);

    if (ret < 0) throw std::runtime_error("Error during signature verification");
    return ret == 1;
}

