# pragma once
# include <cstdint>

constexpr static uint8_t warp_size = 32;
// static uint8_t warp_size = 32;
// In order to have this as constant in the linked files as well we use constexpr

enum class Op : uint8_t{
    LD,ST,VADD,CMP_LT, SEL,HALT
};

struct Instr{
    Op op{Op::HALT}; // halt is the defualt command if not given any other command
    
    uint8_t dst{0};
    uint8_t a{0};
    uint8_t b{0};
    
    uint8_t buf{0}; // 0,1,2
    int32_t imm{0}; // add = tid + imm

};

