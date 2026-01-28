# Simple Warp-Level GPU Simulator (C++)

This project is a small GPU-style simulator written in C++.
It models how GPUs execute programs using warps, lanes, and SIMT (Single Instruction, Multiple Threads) execution.

The goal of this project is understanding and correctness, not performance or real GPU timing.

## Overview

* Groups threads into warps of 32 lanes

* Executes instructions warp by warp

* Each lane has:

  * its own registers

  * its own predicate flag

  * an active/inactive state

* Supports basic vector operations, predication, and memory access

* Ensures inactive lanes never write to memory

* Includes unit tests to verify correctness

This is similar to how real GPUs work, but simplified for learning and modeling.

## Key Concepts : 

* Thread ID (tid) : Each lane represents one thread with a global ID.

* Warp : A group of 32 threads executed together.

* Lane : One thread inside a warp.

* Active Mask : Lanes where tid < n_threads are active. Others are disabled.

* Predicate : A lane boolean used for conditional execution.

## Project Structure 
```css
.
├── app/
│   └── main.cpp
├── src/
│   ├── isa.h
│   ├── model.h
│   └── model.cpp
└── tests/
    ├── test_conformance.cpp
    └── test_random.cpp
```

## Instruction Set (ISA)

*Defined in isa.h*

| Instruction | Description                                |
| ----------- | ------------------------------------------ |
| `LD`        | Load from buffer into a register           |
| `ST`        | Store register value into buffer           |
| `VADD`      | Add two registers (per lane)               |
| `CMP_LT`    | Compare two registers (`<`), set predicate |
| `SEL`       | Select between two values using predicate  |
| `HALT`      | Stop execution                             |

## Instruction Format 
```c++
struct Instr {
    Op op;        // instruction opcode
    uint8_t dst;  // destination register
    uint8_t a;    // source register A
    uint8_t b;    // source register B
    uint8_t buf;  // buffer selector (0,1,2)
    int32_t imm;  // immediate value
};
```

## Memory Model 

* Three global buffers :

    * buf0

    * buf1

    * buf2

* Selected using Instr.buf

* Address calculation :
```c++
address = tid + imm
```

* Safety rules :

    * Negative addresses are ignored

    * Out-of-bounds accesses are ignored

    * Inactive lanes do nothing

## Warp State

### Each warp contains :

-> 32 lanes

### Per lane :

-> 16 registers

-> 1 predicate flag

-> 1 active flag

```c++
struct Warp_state {
    regs[lane][register]
    pred[lane]
    active[lane]
};
```

## Execution Model 

1. Threads are grouped into warps of 32

2. Instructions are executed in order

3. For each instruction:

    * All warps execute the instruction

    * Each lane checks if it is active

    * Only active lanes can read/write registers and memory

* Execution stops at HALT

## Example Program in main.cpp

The demo program computes :
```c++
buf2[i] = min(buf0[i], buf1[i])
```

### Steps:

* Load values from buf0 and buf1

* Compare values (CMP_LT)

* Select the smaller value (SEL)

* Store result in buf2

*This demonstrates predicated execution, which is common in real GPUs.*



## Build & Run 

Build : 

```c++
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
    app/main.cpp src/model.cpp \
    -I src \
    -o gpu_sim

```

Run : 
```c++
./gpu_sim
```

*we will see output showing values from buf0, buf1, and the computed minimum in buf2.*

## Testing

This project includes GoogleTest-based tests.

1. Conformance Test (test_conformance.cpp)

* To check Partial warps do not write outside n_threads

* To check Inactive lanes do not affect memory

2. Randomized Test (test_random.cpp)

* Runs many random tests

* Compares VADD results with a CPU reference implementation

* Validates correctness for different sizes and values

### Test Build : 
```c++
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -pthread \
    src/model.cpp \
    tests/test_conformance.cpp tests/test_random.cpp \
    -I src \
    -lgtest -lgtest_main \
    -o gpu_sim_tests

```

```c++
./gpu_sim_tests
```

## Conclusion : 

This project helps us understand :

* How GPU warps and lanes work

* Why masking is important

* How predication replaces branches

* How partial warps are handled safely

* How to verify simulator correctness with tests

January 26th 2026