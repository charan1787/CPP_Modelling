#include <gtest/gtest.h> // Includes googletest framework
// provides TEST, EXPECT_EQ, ASSERT_*, FAIL()
#include "alu.h"

TEST(ALU_Add, Basic) { 
    // Test - Googletest Macro
    // ALU_Add - test suite name (group)
    // Basic - test name

    ALU alu; // builds ALU object and calls its constructor
    alu.A = 10;
    alu.B = 20;
    alu.step(Opcode::ADD);

    EXPECT_EQ(alu.result, 30u); // EXPECT_EQ(actual, expected)
    EXPECT_FALSE(alu.flag.Z); // assets flag value
    EXPECT_FALSE(alu.flag.N); // assets flag value
}

TEST(ALU_Add, Overflow) {
    ALU alu;
    alu.A = 0xFFFFFFFF;
    alu.B = 1;
    alu.step(Opcode::ADD);

    EXPECT_EQ(alu.result, 0u);
    EXPECT_TRUE(alu.flag.C);
    EXPECT_TRUE(alu.flag.Z);
}
