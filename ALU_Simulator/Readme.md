

                    ------32-bit Bit-Accurate ALU Simulator ------



A bit-accurate C++ model of a simple 32-bit Arithmetic Logic Unit (ALU) with explicitly defined flag semantics.
This project focuses on hardware-style correctness, determinism, and clarity rather than emulating any specific commercial ISA.

Overview :

    This simulator models a single-cycle 32-bit ALU with arithmetic, logical, and shift operations.
    It produces two outputs per operation:

    result — 32-bit datapath output
    flags — condition/status flags (Z, N, C, V)

The model is bit-accurate with respect to its own specification, making it suitable for :

    Hardware modeling practice
    CPU datapath learning
    Verification-oriented thinking
    Foundations for CPU / GPU / ML kernel modeling

Datapath : 

    32-bit unsigned datapath (uint32_t)
    Wrap-around arithmetic (mod 2^32)

Operations : 

    Arithmetic : ADD, SUB, MUL
    Logical : AND, OR, XOR
    Shifts : SHL, SHR (logical shifts)

Shift semantics : 

    Shift amount masked to 5 bits (0..31)
    Logical shifts only
    Multi-bit shifts allowed
    Carry = last bit shifted out
    Shift by 0 leaves carry = 0 (explicitly defined)

Flag semantics : 

     Z = (result == 0)
     N = MSB(result)
     C:

      ADD: carry-out of bit 31
      SUB: no borrow (A >= B)
      SHL: last MSB shifted out
      SHR: last LSB shifted out
      MUL: upper 32 bits ≠ 0

     V:

    ADD: signed overflow
    SUB: signed overflow
    MUL: signed result outside int32 range
    Logical ops & shifts: cleared

Flag update policy :

    Flags are fully overwritten every instruction

BIT Accurate to ARM/x86/RISC-V -> No, as certain operations vary from system to system.

    ALU_Simulator/
      ├── alu.h
      ├── alu.cpp
      ├── main.cpp
      └── README.md

Build & Run : 

Prerequisites : 
          
    C++17 compatible compiler
     macOS: clang++
     Linux: g++
    
Build Command : 

    clang++ -std=c++17 main.cpp alu.cpp -o alu_sim
    
Run Command :  

    ./alu_sim

Output :

When tracing is enabled, each ALU operation prints:

    Input operands (A, B)
    Opcode
    Shift amount (if applicable)
    Result
    Flags (Z, N, C, V)


January 10, 2026
