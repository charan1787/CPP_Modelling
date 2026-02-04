#include "model.h"
#include <iostream>
#include <vector>

static void print_metrics(const Metrics& m) {
    double util = 0.0;
    if (m.warp_cycles > 0) {
        util = (double)m.active_lane_cycles / (double)(m.warp_cycles * 32ull);
    }

    std::cout << "\n--- Metrics ---\n";
    std::cout << "warp_cycles         : " << m.warp_cycles << "\n";
    std::cout << "active_lane_cycles  : " << m.active_lane_cycles << "\n";
    std::cout << "utilization         : " << util << "\n";
    std::cout << "mem_lane_ops        : " << m.mem_lane_ops << "\n";
    std::cout << "divergent_branches  : " << m.divergent_branches << "\n";
    std::cout << "reconverges         : " << m.reconverges << "\n";
}

int main() {
    uint32_t N = 48;

    Buffer mem;
    mem.buf0.resize(N);
    mem.buf1.resize(N);
    mem.buf2.resize(N, 0);

    for (uint32_t i = 0; i < N; i++) {
    mem.buf0[i] = i;

    // Force divergence only in warp 0 (tid 0..31):
    // tid 0..15 -> buf1=0   => pred false (i < 0 false)
    // tid 16..31-> buf1=1000=> pred true
    // tid 32..47-> buf1=1000=> pred true (uniform)
    if (i < 32) mem.buf1[i] = (i < 16) ? 0 : 1000;
    else        mem.buf1[i] = 1000;
    }

    std::vector<Instr> prog = {
       {Op::LD,     0,0,0, 0, 0},   // 0
       {Op::LD,     1,0,0, 1, 0},   // 1
       {Op::CMP_LT, 0,0,1, 0, 0},   // 2
       {Op::BRA,    0,0,0, 0, 7},   // 3

       {Op::ST,     0,1,0, 2, 0},   // 4
       {Op::JMP,    0,0,0, 0, 10},  // 5

       {Op::HALT,   0,0,0, 0, 0},   // 6 padding

      {Op::ST,     0,0,0, 2, 0},   // 7
       {Op::JOIN,   0,0,0, 0, 0},   // 8

       {Op::HALT,   0,0,0, 0, 0},   // 9 padding
       {Op::HALT,   0,0,0, 0, 0}    // 10
    };


    GPU_Sim sim;

    bool trace = true; // set true for pc/mask/stack print
    Metrics m = sim.run(prog, mem, N, trace);

    std::cout << "i  buf0  buf1  buf2(min)\n";
    for (u_int32_t i = 0; i < N; i++) {
        std::cout << i << "   " << mem.buf0[i]
                  << "    " << mem.buf1[i]
                  << "     " << mem.buf2[i] << "\n";
    }

    print_metrics(m);
    return 0;
}
