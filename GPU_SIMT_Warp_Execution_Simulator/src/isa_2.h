#pragma once
#include <cstdint>

constexpr static uint8_t warp_size = 32;

enum class Op : uint8_t {
    LD,       // dst = buf[tid + imm]
    ST,       // buf[tid + imm] = reg[a]
    VADD,     // dst = reg[a] + reg[b]
    CMP_LT,   // pred = (reg[a] < reg[b])
    SEL,      // dst = pred ? reg[a] : reg[b]
    BRA,      // if pred true (per lane), go to target pc (imm)
    JMP,      // unconditional jump to pc (imm)
    JOIN,     // reconvergence point for BRA
    HALT      // stop warp
};

struct Instr {
    Op op{Op::HALT};

    uint8_t dst{0};
    uint8_t a{0};
    uint8_t b{0};

    uint8_t buf{0};   // for LD/ST: 0,1,2
    int32_t imm{0};   // for LD/ST: addr = tid + imm
                      // for BRA/JMP: imm = absolute target PC (instruction index)
};
