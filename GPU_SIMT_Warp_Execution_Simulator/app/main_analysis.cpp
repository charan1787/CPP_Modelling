#include "model.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

// ---------------- helpers ----------------
static uint32_t ceil_div(uint32_t a, uint32_t b) { return (a + b - 1) / b; }

static double utilization_from(const Metrics& m) {
    if (m.warp_cycles == 0) return 0.0;
    return (double)m.active_lane_cycles / (double)(m.warp_cycles * 32ull);
}

static void write_csv_header(std::ofstream& out) {
    out << "workload,N,n_warps,div_ratio,param,"
        << "warp_cycles,active_lane_cycles,utilization,"
        << "cycles_per_warp,cycles_per_thread,"
        << "mem_lane_ops,memops_per_cycle,"
        << "divergent_branches,reconverges\n";
}

static void write_csv_row(std::ofstream& out,
                          const std::string& workload,
                          uint32_t N,
                          double div_ratio,
                          double param,
                          const Metrics& m) {
    uint32_t n_warps = ceil_div(N, 32);
    double util = utilization_from(m);
    double cycles_per_warp = (n_warps > 0) ? (double)m.warp_cycles / (double)n_warps : 0.0;
    double cycles_per_thread = (N > 0) ? (double)m.warp_cycles / (double)N : 0.0;
    double memops_per_cycle = (m.warp_cycles > 0) ? (double)m.mem_lane_ops / (double)m.warp_cycles : 0.0;

    out << workload << ","
        << N << ","
        << n_warps << ","
        << std::fixed << std::setprecision(2) << div_ratio << ","
        << std::fixed << std::setprecision(0) << param << ","
        << m.warp_cycles << ","
        << m.active_lane_cycles << ","
        << std::fixed << std::setprecision(6) << util << ","
        << std::fixed << std::setprecision(6) << cycles_per_warp << ","
        << std::fixed << std::setprecision(8) << cycles_per_thread << ","
        << m.mem_lane_ops << ","
        << std::fixed << std::setprecision(6) << memops_per_cycle << ","
        << m.divergent_branches << ","
        << m.reconverges
        << "\n";
}

// ---------------- programs ----------------

// Single-level divergent min-like branch:
// if (r0 < r1) store r0 else store r1
static std::vector<Instr> make_branch_min_prog() {
    return {
        {Op::LD,     0,0,0, 0, 0},   // 0: r0 = buf0[tid]
        {Op::LD,     1,0,0, 1, 0},   // 1: r1 = buf1[tid]
        {Op::CMP_LT, 0,0,1, 0, 0},   // 2: pred = (r0 < r1)
        {Op::BRA,    0,0,0, 0, 7},   // 3: if pred -> pc=7

        {Op::ST,     0,1,0, 2, 0},   // 4: else: buf2 = r1
        {Op::JMP,    0,0,0, 0, 10},  // 5: skip then
        {Op::HALT,   0,0,0, 0, 0},   // 6 padding

        {Op::ST,     0,0,0, 2, 0},   // 7: then: buf2 = r0
        {Op::JOIN,   0,0,0, 0, 0},   // 8: reconverge
        {Op::HALT,   0,0,0, 0, 0},   // 9 padding
        {Op::HALT,   0,0,0, 0, 0}    // 10 halt
    };
}

// Nested divergence demo (forces stack depth 2)
static std::vector<Instr> make_nested_div_prog() {
    // Outer: pred1 = (r1 < r0)  -> taken for lanes >=16 (with threshold warp_base+15)
    // Inner: pred2 = (r2 < r0)  -> taken for lanes >=24 (with threshold warp_base+23)
    return {
        {Op::LD,     0,0,0, 0, 0},   // 0: r0 = buf0 (tid)
        {Op::LD,     1,0,0, 1, 0},   // 1: r1 = buf1 (outer threshold)
        {Op::LD,     2,0,0, 2, 0},   // 2: r2 = buf2 (inner threshold)

        {Op::CMP_LT, 0,1,0, 0, 0},   // 3: pred1 = (r1 < r0)
        {Op::BRA,    0,0,0, 0, 8},   // 4: outer taken -> pc=8

        {Op::ST,     0,1,0, 2, 0},   // 5: outer else writes r1
        {Op::JMP,    0,0,0, 0, 15},  // 6
        {Op::JMP,    0,0,0, 0, 15},  // 7 padding

        {Op::CMP_LT, 0,2,0, 0, 0},   // 8: pred2 = (r2 < r0)
        {Op::BRA,    0,0,0, 0, 12},  // 9: inner taken -> pc=12

        {Op::ST,     0,2,0, 2, 0},   // 10: inner else writes r2
        {Op::JMP,    0,0,0, 0, 14},  // 11

        {Op::ST,     0,0,0, 2, 0},   // 12: inner taken writes r0
        {Op::JMP,    0,0,0, 0, 14},  // 13

        {Op::JOIN,   0,0,0, 0, 0},   // 14: inner join
        {Op::JOIN,   0,0,0, 0, 0},   // 15: outer join
        {Op::HALT,   0,0,0, 0, 0}    // 16
    };
}

// Compute-heavy: 2 loads, many VADD, 1 store
static std::vector<Instr> make_compute_heavy_prog(int vadd_reps) {
    std::vector<Instr> p;
    p.push_back({Op::LD, 0,0,0, 0, 0});       // r0 = buf0[tid]
    p.push_back({Op::LD, 1,0,0, 1, 0});       // r1 = buf1[tid]
    for (int i = 0; i < vadd_reps; i++) {
        p.push_back({Op::VADD, 0,0,1, 0, 0}); // r0 += r1
    }
    p.push_back({Op::ST, 0,0,0, 2, 0});       // buf2[tid] = r0
    p.push_back({Op::HALT,0,0,0, 0, 0});
    return p;
}

// Memory-heavy: repeated LD/ST pairs with offsets
static std::vector<Instr> make_memory_heavy_prog(int ld_st_pairs) {
    std::vector<Instr> p;
    for (int i = 0; i < ld_st_pairs; i++) {
        p.push_back({Op::LD, 0,0,0, 0, i}); // r0 = buf0[tid+i]
        p.push_back({Op::ST, 0,0,0, 2, i}); // buf2[tid+i] = r0
    }
    p.push_back({Op::HALT,0,0,0, 0, 0});
    return p;
}

// ---------------- buffer initializers ----------------

// For branch divergence ratio r within each warp:
// pred = (r0 < r1), with r0=tid, r1=warp_base+k
// k ~ r*32 makes exactly k lanes taken per warp (except partial warp)
static void init_buffers_for_branch_ratio(Buffer& mem, uint32_t N, double div_ratio) {
    mem.buf0.resize(N);
    mem.buf1.resize(N);
    mem.buf2.assign(N, 0);

    int k = (int)std::round(div_ratio * 32.0);
    if (k < 0) k = 0;
    if (k > 32) k = 32;

    for (uint32_t tid = 0; tid < N; tid++) {
        mem.buf0[tid] = tid;                 // r0 = tid
        uint32_t warp_base = (tid / 32) * 32;
        mem.buf1[tid] = warp_base + (uint32_t)k; // r1 threshold
    }
}

// For nested divergence: choose thresholds so that
// pred1 true for lanes >=16  and pred2 true for lanes >=24 (inside pred1-taken)
static void init_buffers_for_nested(Buffer& mem, uint32_t N) {
    mem.buf0.resize(N);
    mem.buf1.resize(N);
    mem.buf2.resize(N);

    for (uint32_t tid = 0; tid < N; tid++) {
        uint32_t warp_base = (tid / 32) * 32;
        mem.buf0[tid] = tid;            // r0
        mem.buf1[tid] = warp_base + 15; // r1 threshold -> lanes >=16 satisfy (r1 < r0)
        mem.buf2[tid] = warp_base + 23; // r2 threshold -> lanes >=24 satisfy (r2 < r0)
    }
}

// For compute-heavy, just fill buf0, buf1
static void init_buffers_compute(Buffer& mem, uint32_t N) {
    mem.buf0.resize(N);
    mem.buf1.resize(N);
    mem.buf2.assign(N, 0);
    for (uint32_t i = 0; i < N; i++) {
        mem.buf0[i] = i;
        mem.buf1[i] = 1;
    }
}

// For memory-heavy, we need bigger arrays because of tid + imm
static void init_buffers_memory(Buffer& mem, uint32_t N, int ld_st_pairs) {
    uint32_t size = N + (uint32_t)ld_st_pairs + 4;
    mem.buf0.resize(size);
    mem.buf1.resize(size);
    mem.buf2.assign(size, 0);
    for (uint32_t i = 0; i < size; i++) {
        mem.buf0[i] = i;
        mem.buf1[i] = 0;
    }
}

// ---------------- main experiment runner ----------------
int main(int argc, char** argv) {
    bool trace = false;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--trace") trace = true;
    }

    std::ofstream csv("results.csv");
    write_csv_header(csv);

    GPU_Sim sim;
    Buffer mem;

    // Sweep thread counts: include partial warp + multiple warps
    std::vector<uint32_t> Ns = {48, 64, 96, 128, 256, 512};

    // ---------------- Divergence sweep workload ----------------
    auto branch_prog = make_branch_min_prog();
    std::vector<double> divs = {0.00, 0.10, 0.25, 0.50, 0.75, 0.90, 1.00};

    for (uint32_t N : Ns) {
        for (double r : divs) {
            init_buffers_for_branch_ratio(mem, N, r);
            Metrics m = sim.run(branch_prog, mem, N, trace);
            write_csv_row(csv, "branch_div", N, r, /*param*/0, m);
        }
    }

    // ---------------- Nested divergence workload ----------------
// ---------------- Nested divergence sweep (same Ns as others) ----------------
auto nested_prog = make_nested_div_prog();

for (uint32_t N : Ns) {
    init_buffers_for_nested(mem, N);
    Metrics m = sim.run(nested_prog, mem, N, trace);
    write_csv_row(csv, "nested_div", N, -1.0, /*param*/0, m);
}


    // ---------------- Compute-heavy sweeps ----------------
    std::vector<int> compute_reps = {10, 50, 200, 500};
    for (uint32_t N : Ns) {
        init_buffers_compute(mem, N);
        for (int reps : compute_reps) {
            auto prog = make_compute_heavy_prog(reps);
            Metrics m = sim.run(prog, mem, N, trace);
            write_csv_row(csv, "compute_heavy", N, -1.0, /*param*/reps, m);
        }
    }

    // ---------------- Memory-heavy sweeps ----------------
    std::vector<int> mem_pairs = {5, 20, 50, 100, 200};
    for (uint32_t N : Ns) {
        for (int pairs : mem_pairs) {
            init_buffers_memory(mem, N, pairs);
            auto prog = make_memory_heavy_prog(pairs);
            Metrics m = sim.run(prog, mem, N, trace);
            write_csv_row(csv, "memory_heavy", N, -1.0, /*param*/pairs, m);
        }
    }

    csv.close();
    std::cout << "Wrote results.csv\n";
    std::cout << "Run: python3 analysis/analyze.py\n";
    return 0;
}
