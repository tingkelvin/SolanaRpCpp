
#include "Solana/Core/Encoding/Base58.hpp"
#include <cassert>
#include <vector>

using namespace Solana::Encoding;

/** All alphanumeric characters except for "0", "I", "O", and "l" */
static const char *pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const int8_t mapBase58[256] = {
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    -1,
    17,
    18,
    19,
    20,
    21,
    -1,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    33,
    34,
    35,
    36,
    37,
    38,
    39,
    40,
    41,
    42,
    43,
    -1,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
};

std::string Base58::Encode(const unsigned char *pbegin, const unsigned char *pend)
{
    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (pbegin != pend && *pbegin == 0)
    {
        pbegin++;
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    int size = (pend - pbegin) * 138 / 100 + 1; // log(256) / log(58), rounded up.
    std::vector<unsigned char> b58(size);
    // Process the bytes.
    while (pbegin != pend)
    {
        int carry = *pbegin;
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (auto it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend()); it++, i++)
        {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }

        assert(carry == 0);
        length = i;
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    auto it = b58.begin() + (size - length);
    while (it != b58.end() && *it == 0)
        it++;
    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];
    return str;
}
// std::string Base58::Encode(std::string_view input) {
//
//     // Skip & count leading zeroes.
//     int zeroes = 0;
//     int length = 0;
//     while (input.size() > 0 && input[0] == 0) {
//         input = input.substr(1);
//         zeroes++;
//     }
//     // Allocate enough space in big-endian base58 representation.
//     int size = input.size() * 138 / 100 + 1; // log(256) / log(58), rounded up.
//     std::vector<unsigned char> b58(size);
//     // Process the bytes.
//     while (input.size() > 0) {
//         unsigned char carry = input[0];
//         int i = 0;
//         // Apply "b58 = b58 * 256 + ch".
//         for (auto it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend()); it++, i++) {
//             carry += 256 * (*it);
//             *it = carry % 58;
//             carry /= 58;
//         }
//
//         assert(carry == 0);
//         length = i;
//         input = input.substr(1);
//     }
//     // Skip leading zeroes in base58 result.
//     auto it = b58.begin() + (size - length);
//     while (it != b58.end() && *it == 0)
//         it++;
//     // Translate the result into a string.
//     std::string str;
//     str.reserve(zeroes + (b58.end() - it));
//     str.assign(zeroes, '1');
//     while (it != b58.end())
//         str += pszBase58[*(it++)];
//     return str;
//
// }
std::optional<std::string> Base58::Decode(std::string_view input)
{
    auto psz = input.data();
    // Skip leading spaces.
    while (*psz && std::isspace(*psz))
        psz++;
    // Skip and count leading '1's.
    int zeroes = 0;
    int length = 0;
    while (*psz == '1')
    {
        zeroes++;
        psz++;
    }
    // Allocate enough space in big-endian base256 representation.
    int size = input.size() * 733 / 1000 + 1; // log(58) / log(256), rounded up.
    std::vector<unsigned char> b256(size);
    // Process the characters.
    static_assert(std::size(mapBase58) == 256, "mapBase58.size() should be 256"); // guarantee not out of range
    while (*psz && !std::isspace(*psz))
    {
        // Decode base58 character
        int carry = mapBase58[(uint8_t)*psz];
        if (carry == -1) // Invalid b58 character
            return {};
        int i = 0;
        for (auto it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i)
        {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        assert(carry == 0);
        length = i;
        psz++;
    }
    // Skip trailing spaces.
    while (std::isspace(*psz))
        psz++;
    if (*psz != 0)
        return {};
    // Skip leading zeroes in b256.
    auto it = b256.begin() + (size - length);
    // Copy result into output vector.
    std::string out;
    out.reserve(zeroes + (b256.end() - it));
    out.assign(zeroes, 0x00);
    while (it != b256.end())
        out.push_back(*(it++));
    return out;
}

std::optional<std::vector<uint8_t>> Base58::DecodeToBytes(std::string_view input)
{
    auto psz = input.data();
    // Skip leading spaces
    while (*psz && std::isspace(*psz))
        psz++;
    // Skip and count leading '1's
    int zeroes = 0;
    int length = 0;
    while (*psz == '1')
    {
        zeroes++;
        psz++;
    }

    // Allocate enough space in base256 representation
    int size = input.size() * 733 / 1000 + 1; // log(58)/log(256)
    std::vector<uint8_t> b256(size);

    // Process input characters
    static_assert(sizeof(mapBase58) == 256, "mapBase58 size mismatch");
    while (*psz && !std::isspace(*psz))
    {
        int carry = mapBase58[(uint8_t)*psz];
        if (carry == -1) // invalid char
            return std::nullopt;

        int i = 0;
        for (auto it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i)
        {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }

        assert(carry == 0);
        length = i;
        psz++;
    }

    // Skip trailing spaces
    while (std::isspace(*psz))
        psz++;
    if (*psz != 0)
        return std::nullopt;

    // Skip leading zeroes
    auto it = b256.begin() + (size - length);
    std::vector<uint8_t> result;
    result.reserve(zeroes + std::distance(it, b256.end()));
    result.insert(result.end(), zeroes, 0x00);
    result.insert(result.end(), it, b256.end());
    return result;
}
