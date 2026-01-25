#include "alu.h"
#include<iostream>
#include<iomanip> // this gives tool to modify the ouput format

// update_ZN Function
// This function is private function of ALU class. It has to use its variables. So classic OOP format "Class_Name::Function" 
// :: is scope resolution, telling this func belongs to this class
void ALU::update_ZN(){
    flag.Z = (result == 0);
    flag.N = ((result >> 31) & 1u) == 1u;
};

// trace function
void ALU::trace(Opcode op, uint32_t shift_amount) const{
    // store current O/P format
    std::ios old_state(nullptr);
    old_state.copyfmt(std::cout);  

    std::cout << std::hex << std::setfill('0'); 
    //filling the space with zeros if any and every cout will be hexadecimal
    // setfill wont change until it is modified unlike the setw which needs to be specified everywhere.


    std::cout << " A = 0x" << std::setw(8) << A; // adding 8 width space and in this the zeros will be filles.
    std::cout << " B = 0x" << std::setw(8) << B;
    std::cout << " Op-Name " << op_name(op);

    if(op == Opcode::SHL || op == Opcode::SHR){
        std::cout<<" sh = " << std::dec << shift_amount; 
        std::cout << std::hex;

        // cout is changed to deciaml for shift_amount and changed back to hex
    }

    std::cout<< " Result = 0x" << std::setw(8) << result;
    std::cout<< " [Z = " << flag.Z <<" N = " << flag.N <<" C = " << flag.C<<" Z = " << flag.Z << "] " ;
    
    // Restore the old cout format
    std::cout.copyfmt(old_state); 
    //std::cout << endl;
};

// op_name function
const char* ALU::op_name(Opcode op){
    switch(op){
        case Opcode::ADD  : return "ADD"; // Opcode here represents the enum class
        case Opcode::AND_ : return "AND";
        case Opcode::SUB  :  return "SUB";
        case Opcode::MUL  :  return "MUL";
        case Opcode::OR_  :  return "OR";
        case Opcode::XOR_ : return "XOR";
        case Opcode::SHL  :  return "SHL";
        case Opcode::SHR  :  return "SHR";
        default:           return "???";
    }
};


// step function
void ALU::step(Opcode op, uint32_t shift_amount){

    // reset flags to zero
    // Flag flag{} initializes flags once when the ALU object is created.
    // flag = Flag{} resets flags every time a step function is executed.
    flag = Flag{};

    switch(op){
        // 8 cases
        case Opcode::ADD: {
            // 64 bit is used to identify the carry bit
            uint64_t temp = (uint64_t)A + (uint64_t)B;
            result = uint32_t(temp);
            // we do addition as it is unsigned int.

            // checking unsigned overflow
            flag.C = (temp >> 32 & 1u) == 1u;

            //checking for signed overflow
            uint32_t signA = (A >> 31) & 1u;
            uint32_t signB = (B >> 31) & 1u;
            uint32_t signresult = (result >> 31) & 1u;

            flag.Z = (signA == signB) && (signA != signresult);

            break;
        }

        case Opcode::SUB: {

            uint64_t temp = (uint64_t)A - (uint64_t)B;
            result = uint32_t(temp);
            // we do addition as it is unsigned int.            
            
            // checking unsigned overflow
            flag.C = (A >= B);
            // Subtraction does as A + (¬B + 1). So A > B means there will be a carry.
            // subtraction happens in 2's compliment

            //checking for signed overflow
            uint32_t signA = (A >> 31) & 1u;
            uint32_t signB = (B >> 31) & 1u;
            uint32_t signresult = (result >> 31) & 1u;

            flag.Z = (signA == signB) && (signA != signresult);

            break;

        }

        case Opcode::MUL:{
            uint64_t temp = (uint64_t)A * (uint64_t)B;
            result = (uint32_t)temp;

            flag.C = ((temp >> 32) != 0);

            int64_t flow = (int64_t)(int32_t)A * (int64_t)(int32_t)B;
            // we are typecasting from 32bit to 64bit
            flag.V = ((flow > INT32_MAX) || (flow < INT32_MIN));
            break; 
        } 
        
        case Opcode::XOR_: result = A ^ B;
        case Opcode::OR_ : result = A | B;
        case Opcode::AND_ : result = A & B;

        case Opcode::SHL : {
            shift_amount &= 31u; 
            // important if the number is more than 32 (31 is 01111)
            // last 5 bits of the number is only taken (32 is 5 bits)
            // this is equivalent to num % 32
            result = A << shift_amount;
            flag.C = (shift_amount == 0) ? 0 :(A >> (32 - shift_amount) & 1u) != 0 ;
            break;
        }

        case Opcode::SHR : {
            shift_amount &= 31u;
            result = A >> shift_amount;
            flag.C = (shift_amount == 0) ? 0 :(A >> (shift_amount - 1) & 1u) != 0 ;
            break;
        }

        default:
            result = 0;
            break;
    }

    update_ZN();

    if(trace_enables){
        trace(op,shift_amount);
    }

};

