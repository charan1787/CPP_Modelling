#pragma once // this tells the preprocessor to ignore this header if it is already included
#include<cstdint> 

// unit32_t has fixed 32 bit 
// unsigned as bits depended on the platform
// both represents the same, use undigned when you are just counting or dont care about the bits

uint32_t get_bits(uint32_t x, unsigned hi, unsigned lo); // t means type

uint32_t set_bits(uint32_t x, unsigned hi, unsigned lo, uint32_t value);

uint32_t rotr32(uint32_t x,unsigned n);

uint32_t rotl32(uint32_t x,unsigned n);

int32_t sign_extend(uint32_t x, unsigned nbits);


