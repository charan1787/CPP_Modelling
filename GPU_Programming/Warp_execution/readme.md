# Warp Simulation (C++) — SIMT Divergence + Reconvergence Model

A small C++ program that simulates warp-level SIMT execution with per-lane predicate masks, divergent branches, and reconvergence. It provides a simple metric called lane utilization to measure the throughput impact of divergence.

The goal is to build correct intuition for how warps execute control flow and why divergence reduces effective throughput.

## What it models : 

* Warp size : 32 lanes (WARP_SIZE = 32)

* Per-lane predicate : pred[lane] controls branch direction

* Active mask : active[lane] controls which lanes execute an instruction in a given cycle

* Divergence behavior : True/false paths execute serially with different masks

* Reconvergence : After both paths are executed, the warp restores the original active mask

* Performance counters:

    * cycles : total simulated cycles

    * active_lane_cycles : sum over cycles of active lanes (capacity actually used)

    * utilization = active_lane_cycles / (cycles * WARP_SIZE)

## Instruction Set (minimal) : 

```c++
enum class Op {
    ADD1,  // x[lane] += 1 on active lanes
    HALT
};
```

Each Instr can repeat an operation for multiple cycles :
```c++
struct Instr {
    Op op;
    int reps = 1;
};
```

In this model :

* 1 instruction repeat = 1 cycle

* Only active lanes contribute work and change state

## Architecture Overview

### Warp State : 
```c++
struct WarpState {
    std::array<int,  WARP_SIZE> x{};       // per-lane data
    std::array<bool, WARP_SIZE> pred{};    // branch predicate per lane
    std::array<bool, WARP_SIZE> active{};  // active mask per cycle
};
```

### Execution model : 

The program structure is :

1. Pre-work : all lanes active (ADD1 for preWork cycles)

2. Branch : split into true/false masks based on pred[lane]

    * Execute true block with true mask (others idle)

    * Execute false block with false mask (others idle)

3. Post-work: reconverged mask, all lanes active again

## Example Scenario 

The demo runs two cases with identical work sizes :

  * preWork = 5

  * trueBlock = ADD1 × 10

  * falseBlock = ADD1 × 10

  * postWork = 5

Case 1 : No divergence

*All 32 lanes take the TRUE path (pred[lane] = true).*

Case 2: Divergence

*16 lanes take TRUE and 16 lanes take FALSE (pred[lane] = (lane < 16)).*

The output shows :

  * total cycles

  * active lane cycles

  * utilization

## Build & Run

Build : 
```
g++ -std=c++17 -O2 -Wall -Wextra -pedantic warp_execution_simulator.cpp -o warp_sim
```

Run : 

```
./warp_sim
```

## Output Interpretation : 


* cycles : total time assuming 1 instruction = 1 cycle

* active_lane_cycles : “useful throughput” summed over active lanes

* utilization :

  * 1.0 means perfect SIMD/SIMT efficiency

  * lower values indicate masked-off lanes due to divergence

This mirrors real GPU intuition :

* With divergence, both paths are executed, but only part of the warp is active in each path.

* Even if total work is the same, effective throughput drops.

## Design Notes

* Uses explicit active masks (active[lane]) to represent SIMD/SIMT masking.

* Branching is modeled by :

  * saving the mask at the branch entry (base mask)

  * building per-path masks (trueMask / falseMask)

  * executing each block under its mask

  * restoring base mask (reconvergence)

* The simulator counts lane-cycles, which is a useful metric for understanding utilization.

January 23rd 2026