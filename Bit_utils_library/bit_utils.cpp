#include "bit_utils.h"
#include <cassert>

uint32_t get_bits(uint32_t x, unsigned hi, unsigned lo){

    assert(lo <= hi);
    assert(hi < 32);
    // lo is unsigned type it starts with 0 so we dont need to add assert(lo >= 0)

    unsigned width = hi - lo + 1;

    uint32_t mask;

    if(width == 32){
        mask = 0xFFFFFFFFu;
    }
    else{
        mask = (1u << width) - 1u; // as 1u << 32 is not defiend we used above if clause
    }

    return (x >> lo) & mask;
};

uint32_t set_bits(uint32_t x, unsigned hi, unsigned lo, uint32_t value){

    assert(lo <= hi);
    assert(hi < 32);

    unsigned width = hi - lo + 1;

    uint32_t mask;

    if(width == 32){
        mask = 0xFFFFFFFFu;
    }
    else{
        mask = ((1u << width) - 1u) << lo;
    }

    x = x & ~mask; // clear target bits before inserting
    
    uint32_t shift = (value << lo) & mask; 

    return x | shift;

};

uint32_t rotr32(uint32_t x,unsigned n){
    assert(n < 32 || "rotation count is normalized via modulo");
    
    n = n % 32;
    if(n == 0) return x;
    
    return (x >> n) | (x << (32 - n));
};

uint32_t rotl32(uint32_t x,unsigned n){
    assert(n < 32 || "rotation count is normalized via modulo");
    
    n = n % 32;
    if(n == 0) return x;    
    
    return (x << n) | (x >> (32 - n));
};

// convert unsigned to signed
int32_t sign_extend(uint32_t x, unsigned nbits){

    assert(nbits >= 1);
    assert(nbits <= 32);

    uint32_t signbit = (1u << (nbits - 1));

    return (x ^ signbit) - signbit;

};
