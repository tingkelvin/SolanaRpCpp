// src/Something.cpp

#include "BorshCpp.hpp"
#include <iostream>

void example() {
    BorshEncoder encoder;
    encoder.Encode(uint32_t(123), "hello");

    auto bytes = encoder.GetBuffer();

    for (auto b : bytes)
        std::cout << (int)b << " ";

    std::cout << std::endl;
}
