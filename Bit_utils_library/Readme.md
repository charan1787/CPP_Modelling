# Bit Utils — 32-bit Bit Manipulation Library (C++)

A lightweight C++ module that provides common 32-bit bit-manipulation primitives used in low-level systems work, simulation/emulation, binary encoding/decoding, and performance-critical code paths.




Bit primitives like these show up in:

    instruction decoding (CPU/GPU ISA work)
    packed numeric formats and bitfields (SIMD-friendly layouts)
    hashing / checksums / crypto-style transforms (rotations)
    quant tooling that parses binary market feeds or compressed data formats

This repo demonstrates correctness-first handling of common pitfalls such as:

    avoiding invalid shifts (e.g., 1u << 32)
    using fixed-width integer types for stable semantics


This project includes :

    A small utility library (bit_utils.h/.cpp)
    A simple executable demo/test (main.cpp) showing correct usage and expected outputs

Features : 

    Bit-field extraction:                       get_bits(x, hi, lo)
    Bit-field modification:                     set_bits(x, hi, lo, value)
    Bit rotations (32-bit):                     rotl32, rotr32 (handles n >= 32 via modulo)
    Sign extension from n-bit encoded values:   sign_extend(x, nbits)

*Uses fixed-width integer types (uint32_t, int32_t) for portability and correctness* 


Project Structure : 

Typical layout:

    .
    ├── bit_utils.h
    ├── bit_utils.cpp
    └── main.cpp


Expected Inputs / Constraints : 

- For `get_bits` and `set_bits`:
  - `0 <= lo <= hi < 32`
- For `sign_extend`:
  - `1 <= nbits <= 32`

*These preconditions are enforced with `assert` in debug builds.*

Build and Run : 

Prerequisites :

    C++17 compatible compiler
    macOS: clang++
    Linux: g++

Build Command : 
    clang++ -std=c++17 bit_utils.cpp main.cpp -o bit_utils_demo

Run Command : 
    ./bit_utils_demo

January 5th 2026

    



