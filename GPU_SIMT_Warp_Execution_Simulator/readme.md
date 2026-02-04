# GPU SIMT Warp & Divergence Simulator

This project implements a warp-level SIMT (Single Instruction, Multiple Threads) GPU execution simulator in C++.

It models how modern GPUs execute threads in warps, handle branch divergence, and incur performance penalties.

The focus of this project is architectural understanding and correctness.

## Key Features

* Warp-based SIMT execution model (32 lanes per warp)

* Per-lane registers and predicate flags

* Active mask–based execution control

* Full branch divergence handling using a SIMT reconvergence stack

* Support for:

* * Uniform and divergent branches
* * Nested divergence
* * Memory-heavy workloads
* * Compute-heavy workloads

* Detailed performance metrics:

* * Warp cycles (units of time taken for the program)
* * Active lane cycles
* * Warp utilization % 
* * Memory lane operations counter
* * Divergent branches and reconvergence counters
* * CSV-based output and Python visualization of graphs


## GPU Concepts Modeled

This simulator explicitly demonstrates :

* Warp-level program counters

* Predicated execution replacing control flow

* Divergence-induced serialization

* Reconvergence using a structured JOIN mechanism

Performance impact of :

* Branch divergence

* Memory intensity

* Partial warps

* Thread count scaling


## Instruction Set Architecture (ISA)

| Opcode | Description             |
| ------ | ----------------------- |
| LD     | Load from global buffer |
| ST     | Store to global buffer  |
| VADD   | Vector addition         |
| CMP_LT | Per-lane comparison     |
| SEL    | Predicate-based select  |
| BRA    | Predicated branch       |
| JMP    | Unconditional jump      |
| JOIN   | Reconvergence point     |
| HALT   | Stop warp execution     |

## Project Structure 
```css
GPU_SIMT_Warp_Execution_Simulator/
├── src/
│   ├── isa.h        
│   ├── model.h        
│   └── model.cpp      
│
├── app/
│   ├── main_analysis.cpp 
│   └── main.cpp       driver
│
├── analysis/
│   └── plot_results.py     
│
├── results.csv        # Auto-generated metrics
├── simt_all_in_one.png
├── compute_scaling.png
├── memory_scaling.png
└── README.md
```

## Simulator Architecture

The simulator is built using 4 point approach.

1 - Instruction Architecture

* What instructions do the program hold

* How is the instruction in a program formatted

2 - State of the machine

* Each warp maintains:

* * Per-lane registers
* * Predicate flags
* * Active mask
* * Program counter
* * SIMT reconvergence stack

* Buffers

* Result metric

3 - Warp Execution Step function

Each warp executes one instruction per cycle. 

4 - Scheduler Function for the program

All warps are triggered their step functions until thy are halted.


## Experimental Results

### Branch Divergence

* Uniform branches execute efficiently

* Divergent branches incurs penalty

* N = 48 is partial warp, rest all full warps.

* As we are doing exp on same program for the divergence the graph is flat at top, once divergence happens we have to run them one after the other. Time(TRUE) + Time(FALSE) is same for all divergence ratios. so the flat line.

### Utilization vs Performance

* Low utilization often correlates with higher cost

* Divergence reduces parallelism and increases total cycles

* Branch divergence (blue): mid utilization, low–medium cost

* Nested divergence (purple): lowest utilization, higher cost

* Compute-heavy (green): high utilization, high cost (lots of work)

* Memory-heavy (red): high utilization, high cost (memory dominates)

* Utilization alone does not predict performance — computational operations matter too.

### Memory vs Compute Intensity

* Memory - heavy workloads increase execution cost

* Compare the first 4 Red dots and Green dots, With same N values there is comparative difference in memory operations. 

## Workloads 

* Uniform branch

* Single-level divergence

* Nested divergence

* Compute-heavy loops

* Memory-heavy loops

* Multiple thread counts (N = 48, 64, 128, 256, 512)

## Build and Run 


### Compile
```C++
g++ -std=c++17 -O2 -Wall src/model.cpp app/main.cpp -I src -o gpu_sim
```


### Run
```C++
./gpu_sim
```

### Analysis
```C++
python3 analysis/analysis.py
```

4th, February 2026