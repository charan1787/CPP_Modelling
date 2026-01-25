#include "cpu.h"
#include <iostream> 
#include <stdexcept> 

const char* CPU::iop_name(IOp op) {
    switch (op) {
        case IOp::ADD:  return "ADD";
        case IOp::SUB:  return "SUB";
        case IOp::AND_: return "AND";
        case IOp::OR_:  return "OR";
        case IOp::XOR_: return "XOR";
        case IOp::SHL:  return "SHL";
        case IOp::SHR:  return "SHR";
        case IOp::ADDI: return "ADDI";
        case IOp::HALT: return "HALT";
        default:        return "???";
    }
}

const char* CPU::phase_name(Phase p){
    switch(p){
        case Phase::FETCH1: return "FETCH1";
        case Phase::FETCH2: return "FETCH2";
        case Phase::FETCH3: return "FETCH3";
        case Phase::DECODE: return "DECODE";
        case Phase::EXECUTE: return "EXECUTE";
        case Phase::HALTED: return "HALTED";
        default:        return "???";
    }
}

Opcode CPU::to_alu(IOp op) {
    switch (op) {
        case IOp::ADD:  return Opcode::ADD;
        case IOp::SUB:  return Opcode::SUB;
        case IOp::AND_: return Opcode::AND_;
        case IOp::OR_:  return Opcode::OR_;
        case IOp::XOR_: return Opcode::XOR_;
        case IOp::SHL:  return Opcode::SHL;
        case IOp::SHR:  return Opcode::SHR;
        default:        return Opcode::ADD;
    }
}

// constructor
CPU::CPU() {
    alu.trace_enables = false;
    R.fill(0);
}

void CPU::load_program(const std::vector<Instr>& program) {
    mem = program;
    R.fill(0);
    PC = 0;
    MAR = 0;
    IR = Instr{};
    MDR = Instr{};
    phase = Phase::FETCH1;
}

void CPU::dump_regs() const {
    std::cout<<std::endl;
    std::cout << "REGS: ";
    for (int i = 0; i < 8; ++i) {
        std::cout << "R" << i << "=" << R[i] << (i == 7 ? "" : " ");
    }
    
    std::cout << "\n";
    std::cout<<std::endl;
}

void CPU::tick() {
    if (phase == Phase::HALTED) return;

    if (trace) {
        std::cout << "\n--- TICK " << phase_name(phase)
                  << " PC = " << PC << " ---\n";
    }

    switch (phase) {
        case Phase::FETCH1:
            MAR = PC;
            phase = Phase::FETCH2;
            break;

        case Phase::FETCH2:
            if (MAR >= mem.size())
                throw std::runtime_error("PC out of bounds");
            MDR = mem[MAR];
            phase = Phase::FETCH3;
            break;

        case Phase::FETCH3:
            IR = MDR;
            PC++; // only after the fetching is done PC is increased
            phase = Phase::DECODE;
            break;

        case Phase::DECODE:
            if (trace) {
                std::cout<<std::endl;
                std::cout << "DECODE: " << iop_name(IR.op)
                          << " rd=R" << int(IR.rd)
                          << " rs1=R" << int(IR.rs1)
                          << " rs2=R" << int(IR.rs2)
                          << " imm=" << IR.imm << "\n";
            }
            phase = Phase::EXECUTE;
            break;

        case Phase::EXECUTE:
            switch (IR.op) {
                case IOp::HALT:
                    phase = Phase::HALTED;
                    return;

                case IOp::ADDI:
                    alu.A = R[IR.rs1];
                    alu.B = (uint32_t)IR.imm;
                    alu.step(Opcode::ADD);
                    R[IR.rd] = alu.result;
                    break;

                case IOp::ADD:
                case IOp::SUB:
                case IOp::AND_:
                case IOp::OR_:
                case IOp::XOR_:
                    alu.A = R[IR.rs1];
                    alu.B = R[IR.rs2];
                    alu.step(to_alu(IR.op));
                    R[IR.rd] = alu.result;
                    break;

                case IOp::SHL:
                case IOp::SHR:
                    alu.A = R[IR.rs1];
                    alu.step(to_alu(IR.op), (uint32_t)IR.imm);
                    R[IR.rd] = alu.result;
                    break;
            }

            if (trace) {
                std::cout<<std::endl;
                std::cout << "ALU result=" << alu.result
                          << " [Z=" << alu.flag.Z
                          << " N=" << alu.flag.N
                          << " C=" << alu.flag.C
                          << " V=" << alu.flag.V << "]\n";
                dump_regs();
            }

            phase = Phase::FETCH1;
            break;

        case Phase::HALTED:
            break;
    }
}



void CPU::run(uint32_t max_ticks) {
    for (uint32_t i = 0; i < max_ticks; ++i) {
        if (phase == Phase::HALTED) return;
        tick();
        std::cout << "------------------------------------------------------";
    }
    throw std::runtime_error("CPU timeout");
}




