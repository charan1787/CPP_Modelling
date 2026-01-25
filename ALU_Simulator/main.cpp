#include"alu.h"
#include<iostream>

// <> system headers
// "" for your own header files

int main(){

    ALU alu;

    alu.trace_enables = true;

    alu.A = 10;
    alu.B = 20;
    alu.step(Opcode :: ADD);
    std::cout<<std::endl;

    alu.A = 20;
    alu.B = 50;
    alu.step(Opcode::SUB);
    std::cout<<std::endl;

    alu.A = 0xF0F0F0F0;
    alu.B = 0x0F0F0F0F;
    alu.step(Opcode::AND_);
    std::cout<<std::endl;

    alu.A = 0x80000001;
    alu.step(Opcode::SHL, 1);
    std::cout<<std::endl;

    alu.A = 0x00000010;
    alu.step(Opcode::SHR, 35); 
    std::cout<<std::endl;

    alu.A = 50000;
    alu.B = 50000;
    alu.step(Opcode::MUL);
    std::cout<<std::endl;

    std::cout << "\nALU simulation finished.\n";

    return 0;

}