# GPU_Simulation — Warp Divergence Utilization Demo (C++)

A tiny C++ demo that builds intuition for warp divergence on GPUs by estimating how many cycles a warp needs with and without divergence, and computing a simple lane utilization metric.

This is a conceptual performance model (not CUDA, not a real simulator). It’s meant to help you reason about why if/else divergence can reduce throughput in SIMT architectures.

## What this demonstrates

On a SIMT GPU, a warp (typically 32 threads) executes a single instruction stream.

If threads in a warp take different control paths (e.g., half go into if, half into else), the warp often must execute both paths serially while masking inactive lanes.

### This demo compares :

* No divergence : all lanes take the same path

* With divergence : lanes split between a true-path and false-path

### It prints :

* total cycles (rough estimate)

 * util - how much of the warp’s capacity is used

## Model assumptions

* Warp size : 32 lanes

* Divergent split : 16 lanes true, 16 lanes false

* Work sections :

  * pre work (all lanes active)

  * true_case work (only true lanes active)

  * false_case work (only false lanes active)

  * post work (all lanes active)

### Cycle model :

No divergence cycles :
```
pre + true_case + post
```

Divergence cycles :
```
pre + true_case + false_case + post
(true and false paths executed serially)
```

## Utilization model (divergence)

Utilization is computed as :
```
util = [(pre+post)⋅warp+true_case⋅true_lanes+false_case⋅false_lanes​] / 
[warp⋅cycles_div]
```
active lane-cycles in numerator

total capacity (warp in total × total cycles) in denominator

## Build and Run  

Build : 
```
g++ -std=c++17 -O2 -Wall -Wextra Basic_warp_exec_simulation.cpp -o divergence_demo
```

Run : 
```
./divergence_demo
```

## Example output : 
```
No divergence cycles : 20  util : 1
Divergence cycles :    30  util : 0.666667
```

## Interpretation:

* Divergence increases total cycles because both branches execute.

* Utilization drops because some lanes are inactive during each masked branch.

January 23 2026

<!--

Models :
 Basic divergence cost intuition (serializing control paths)
 Lane masking and reduced utilization

Does not model :

 Instruction-level details (real ISA scheduling)
 Memory effects (coalescing, cache)
 Multiple warps / occupancy
 Latency hiding / warp scheduling
 Predication optimizations or compiler transformations
>