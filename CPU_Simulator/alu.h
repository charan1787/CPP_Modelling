#pragma once
#include<cstdint>
#include<string>

// enum of flag
enum class Opcode{
    ADD,SUB,MUL,AND_,OR_,XOR_,SHR,SHL
};


// struct for flag
// As ALU gives 2 set of outputs Flags and result. It is good practice to seperate the outputs in the simulation.
// Flags here represent the state and does not have any behaviour. So to present in a class we need to have objects/state and behaviours as well. 
struct Flag{
    bool N = 0; // Negative Value
    bool Z = 0; // Zero Value
    bool C = 0; // Unsigned overflow (carry)
    bool V = 0; // Signed overflow
};

// why not class? becuase flags dont have any behaviour it is just a bundle of data variables. struct makes more sense here. and also not to worry about private and public.


// class for ALU
class ALU{

public :

    // have to initialise these input variables aas zero as that is their initial state. (good practice for hardware modelling)
    uint32_t A = 0;
    uint32_t B = 0;
    Opcode op;

    uint32_t result = 0;
    Flag flag{}; // Flag is a type which holds 4 bool values. flag is a variable of type Flag. 
    // {} helps to initialise everything to zero. if we dont use it we have to manually initialise its objects to zero like flags.N = 0
    // if no {}, object is created but data variables will have garbage values.

    bool trace_enables = true; // to print the state at current time.
    // should be always enabled to help us log the results. and this happens in the step function at last.
    void step(Opcode op, uint32_t shift_amount = 0);

private :

// Golden rule : everything you see outside the ALU put them in public as you need to interact with it. Put the rest of helper functions in the private section.

    void update_ZN(); // sets Z and N values, this happens after the execution of the program so kept in private.
    void trace(Opcode op, uint32_t shift_amount) const; // use const as we should not be able to change the variables inside. This will be just read only output.
    static const char* op_name(Opcode op);
    // Here this helper function does not use any class variables and just a helper. So we use static so we can easily access it with class name by not initialising an object.
    // const used to put it as read only function.
    // char * is telling it is pointing to a string like const char* s = 'ADD'

};

// step should be the only function that should change the outputs (result and flags), thats how Hardware operates. 



