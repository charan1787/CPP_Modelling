#include "cpu.h"
#include <iostream>

int main() {
    CPU cpu;
    cpu.trace = true;

    // Program:
    // R1 = 10
    // R2 = 20
    // R3 = R1 + R2
    // R4 = R3 << 1
    // HALT
    std::vector<Instr> program = {
        {IOp::ADDI, 1, 0, 0, 10},
        {IOp::ADDI, 2, 0, 0, 20},
        {IOp::ADD,  3, 1, 2, 0},
        {IOp::SHL,  4, 3, 0, 1},
        {IOp::HALT, 0, 0, 0, 0}
    };

    cpu.load_program(program);
    cpu.run();

    std::cout << "\nFINAL RESULT\n";
    std::cout << "R3 = " << cpu.R[3] << " (expected 30)\n";
    std::cout << "R4 = " << cpu.R[4] << " (expected 60)\n";
}
