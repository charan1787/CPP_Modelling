#include <iostream>
#include <iomanip>
#include "bit_utils.h"

int main() {
    // 1) get_bits test
    uint32_t x = 0xDEADBEEFu; // 11011110101011011011111011101111
    uint32_t part = get_bits(x, 15, 8); // take bits 15..8
    std::cout << "get_bits(0xDEADBEEF, 15, 8) = 0x"
              << std::hex << part << std::dec << "\n";

    // 2) set_bits test
    uint32_t y = 0xFFFF0000u;
    y = set_bits(y, 7, 0, 0xAB);
    std::cout << "set_bits(0xFFFF0000, 7, 0, 0xAB) = 0x"
              << std::hex << y << std::dec << "\n";

    // 3) rotate tests
    uint32_t r = 0x12345678u;
    std::cout << "rotl32(0x12345678, 8) = 0x"
              << std::hex << rotl32(r, 8) << std::dec << "\n";

    std::cout << "rotr32(0x12345678, 8) = 0x"
              << std::hex << rotr32(r, 8) << std::dec << "\n";

    // 4) sign extend tests
    uint32_t a = 0x7Fu;  // 8-bit: 127
    uint32_t b = 0x80u;  // 8-bit: -128
    uint32_t c = 0xF0u;  // 8-bit: -16

    std::cout << "sign_extend(0x7F, 8) = " << sign_extend(a, 8) << "\n";
    std::cout << "sign_extend(0x80, 8) = " << sign_extend(b, 8) << "\n";
    std::cout << "sign_extend(0xF0, 8) = " << sign_extend(c, 8) << "\n";

    return 0;
}
