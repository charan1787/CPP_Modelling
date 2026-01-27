# GPU_Simulation — SIMT Fundamentals (C++)

A minimal C++ program designed to illustrate GPU-style SIMT (Single Instruction, Multiple Threads) execution using standard CPU constructs.

This project does not use CUDA, OpenCL, or GPU hardware. Instead, it models the conceptual execution model of a GPU warp : many lanes (threads) executing the same instruction on different data.

## Purpose

This project helps to build foundational intuition for GPU programming and modeling, including :

* What a warp is

* What a lane/thread is

* How one instruction operates across many lanes

* How data is conceptually private per lane

It serves as a stepping stone before moving to :

* Warp-level primitives

* GPU simulators or performance models

## Conceptual Model

* Warp size : 32 lanes

* Each lane : represents one GPU thread

* SIMT behavior :

  * One instruction

  * Executed across all lanes

  * Each lane operates on its own data

In real GPUs, this happens in hardware.

Here, it is modeled using a simple loop and an array.

## Code Overview 
```
constexpr int Lanes = 32;
std::array<int, Lanes> data;
```
* Lanes represents the warp width (32 threads)

* data[lane] represents lane-local registers / memory

```
for (int lane = 0; lane < Lanes; lane++) {
    data[lane] = lane * 2;
}
```

This loop models :

* One instruction (lane * 2)

* Applied uniformly across all lanes

* With different per-lane inputs

## Build & Run

Build : 
```
g++ -std=c++17 -O2 -Wall -Wextra hello_world.cpp -o simt_demo
```
Run : 
```
./simt_demo
```
## Example output 
```
Hello from lane 0, value 0
Hello from lane 1, value 2
Hello from lane 2, value 4
...
Hello from lane 31, value 62
```
*Each line corresponds to one lane executing the same instruction.*

## Note : 

This project intentionally starts small to demonstrate correct GPU mental models before introducing CUDA or hardware-specific APIs.

January 22 2026