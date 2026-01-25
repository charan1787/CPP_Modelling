#include <gtest/gtest.h>
#include <random>
#include "alu.h"

TEST(ALU_Random, AddAgainstGolden) {
    uint32_t seed = 12345;  // seed value
    std::mt19937 rng(seed); // Mersenne Twister PRNG // rng - random num gen obj
    std::uniform_int_distribution<uint32_t> dist; // defines a uniform dict

    for (int i = 0; i < 100000; ++i) {
        uint32_t A = dist(rng);
        uint32_t B = dist(rng);

        ALU alu;
        alu.A = A;
        alu.B = B;
        alu.step(Opcode::ADD);

        uint64_t temp = (uint64_t)A + (uint64_t)B;
        uint32_t golden = (uint32_t)temp;
        bool golden_C = (temp >> 32) & 1;

        if (alu.result != golden || alu.flag.C != golden_C) {
            FAIL() << "Seed=" << seed
                   << " A=" << A
                   << " B=" << B;
        }
    }
}
