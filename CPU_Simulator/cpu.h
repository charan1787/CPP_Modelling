#pragma once
#include<cstdint>
#include "alu.h"
#include<array> // for registers
#include<vector> // for memory

// CPU Instruction Set (ISA)
// This is used for opcodes of the operations for the CPU coming form the RAM, there will be many more like JUMP and compare
enum class IOp : uint8_t{
    ADD,SUB,AND_,OR_,XOR_,
    SHL,SHR,
    ADDI,
    HALT
};

// Instruction format
struct Instr{
    IOp op{IOp :: HALT};
    uint32_t rd = 0;
    uint32_t rs1 = 0;
    uint32_t rs2 = 0;
    uint32_t imm = 0;
};

// CPU class
class CPU{
public :
    CPU(); // constructor

    void load_program(const std::vector<Instr> & program);

    // If we don't use & 
    // Copies every instruction
    // Allocates new memory
    // Slow for large programs
    // Completely unnecessary
    // const is used so that anyone will not be able top change this=-8876`   program function.

    void run(uint32_t max_tricks = 500);

    bool trace = true;
    std::array<uint32_t, 8> R{}; 

private : 
    uint32_t PC = 0;
    uint32_t MAR = 0;

    Instr IR{};
    Instr MDR{};

    std::vector<Instr> mem;
    
    ALU alu;

    enum class Phase { FETCH1, FETCH2, FETCH3, DECODE, EXECUTE, HALTED };
    Phase phase = Phase::FETCH1;

    void tick(); // function operations per clock cycle
    void dump_regs() const; // for registors output

    static const char* phase_name(Phase p);
    static const char* iop_name(IOp op);
    static Opcode to_alu(IOp op);
};

