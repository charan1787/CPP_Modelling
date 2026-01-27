# CPU_Simulator (C++)

A step-driven CPU simulator implemented in modern C++. The simulator models classic CPU datapath/control ideas: an explicit architectural state (PC, registers, IR/MDR/MAR), an ALU module with flags, and a micro-sequenced control loop implemented as a Fetch → Decode → Execute finite state machine.

This project is built with a hardware-modeling mindset: state is stored in registers, and computation happens in discrete “ticks” (phases), making behavior deterministic and easy to trace.

## Features

### CPU

    8 × 32-bit registers (R0..R7)

    Instruction memory as std::vector<Instr>

    Multi-phase control FSM:

    FETCH1 → FETCH2 → FETCH3 → DECODE → EXECUTE → ...

Trace mode prints :

    current phase, PC, decoded instruction, ALU flags, register dump

Runtime safety checks :

    throws std::runtime_error("PC out of bounds") if the program counter fetches outside memory

    throws std::runtime_error("CPU timeout") if run() exceeds a max tick limit

### ALU

    Operations : ADD, SUB, MUL, AND, OR, XOR, SHL, SHR

Outputs:

    result

    flags : Z (zero), N (negative), C (carry/borrow), V (signed overflow)

    Optional ALU-local tracing (trace_enables) (disabled by default inside CPU)

## Project Structure

    .
    ├── alu.h
    ├── alu.cpp
    ├── cpu.h
    ├── cpu.cpp
    └── main.cpp

## Instruction Set (ISA) :

### Opcode enum (IOp) : 

Supported instructions:


| Instruction         | Meaning                      |
| ------------------- | ---------------------------- |
| `ADDI rd, rs1, imm` | `R[rd] = R[rs1] + imm`       |
| `ADD rd, rs1, rs2`  | `R[rd] = R[rs1] + R[rs2]`    |
| `SUB rd, rs1, rs2`  | `R[rd] = R[rs1] - R[rs2]`    |
| `AND rd, rs1, rs2`  | bitwise AND                  |
| `OR  rd, rs1, rs2`  | bitwise OR                   |
| `XOR rd, rs1, rs2`  | bitwise XOR                  |
| `SHL rd, rs1, imm`  | logical left shift by `imm`  |
| `SHR rd, rs1, imm`  | logical right shift by `imm` |
| `HALT`              | stop execution               |

### Instruction format (Instr) : 

   struct Instr{
        IOp op;
        uint32_t rd;
        uint32_t rs1;
        uint32_t rs2;
        uint32_t imm;
    };

## Execution Model : 

Each CPU::tick() advances the simulator by one micro-phase :

1. FETCH1 : MAR = PC

2. FETCH2 : MDR = mem[MAR] (bounds checked)

3. FETCH3 : IR = MDR; PC++

4. DECODE : print decoded instruction (trace)

5. EXECUTE : drive ALU inputs, call alu.step(...), write back to registers

This split is useful for :

1. debugging

2. future extensions (pipeline, hazards, memory latency, etc.)

## Build & Run :

Build (g++) :

    g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
     main.cpp cpu.cpp alu.cpp \
     -o cpu_sim

Run :

    ./cpu_sim

## Example Program : 

main.cpp loads and runs the following program:

* R1 = 10

* R2 = 20

* R3 = R1 + R2

* R4 = R3 << 1

* HALT

      std::vector<Instr> program = {
          {IOp::ADDI, 1, 0, 0, 10},
          {IOp::ADDI, 2, 0, 0, 20},
          {IOp::ADD,  3, 1, 2, 0},
          {IOp::SHL,  4, 3, 0, 1},
          {IOp::HALT, 0, 0, 0, 0}
      };


### Expected final state :

R3 = 30

R4 = 60

## Tracing / Debugging

Enable full CPU trace :

    CPU cpu;
    cpu.trace = true;

Trace output includes :

* phase name + PC

* decoded instruction fields

* ALU result + flags (Z/N/C/V)

* register dump (R0..R7)


## Design Notes : 

* Flags are a struct because they represent state only (no behavior).

* The ALU is a separate module to mimic actual behaviour and make unit testing easier.

January 15 2026


