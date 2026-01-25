#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

static constexpr int WARP_SIZE = 32;

// 1) Tiny instruction set
enum class Op {
    ADD1,      // x[lane] += 1  
    HALT
};

// 2) Instruction: operation 
struct Instr {
    Op op;
    int reps = 1;
};

// 3) Warp state
struct WarpState {
    std::array<int, WARP_SIZE> x{};       //value of the warp thread
    std::array<bool, WARP_SIZE> pred{};   // For branch split
    std::array<bool, WARP_SIZE> active{}; 
 } ;

// 4) Simulation counters
struct SimResult {
    uint64_t cycles = 0;              
    uint64_t active_lane_cycles = 0;  
};

// Count active lanes (how many threads are working this cycle)
static int count_active(const array<bool, WARP_SIZE>& mask) {
    int c = 0;
    for (bool b : mask) if (b) c++;
    return c;
}

// Execute one "ADD1" instruction for 'reps' cycles on currently active lanes
static void exec_add1(WarpState& w, SimResult& r, int reps) {
    for (int k = 0; k < reps; k++) {
        int active_lanes = count_active(w.active);

        // Toy model: one instruction = one cycle
        r.cycles += 1;
        r.active_lane_cycles += (uint64_t)active_lanes;

        // Only active lanes do the work
        for (int lane = 0; lane < WARP_SIZE; lane++) {
            if (w.active[lane]) {
                w.x[lane] += 1;
            }
        }
    }
}

// Execute branch:
// if (pred[lane]) run trueBlock else run falseBlock
static void exec_branch(WarpState& w,
                        const vector<Instr>& trueBlock,
                        const vector<Instr>& falseBlock,
                        SimResult& r) {
    // Save mask at branch entry (for reconvergence)
    array<bool, WARP_SIZE> baseMask = w.active;

    // Build masks for each path
    array<bool, WARP_SIZE> trueMask{};
    array<bool, WARP_SIZE> falseMask{};

    for (int lane = 0; lane < WARP_SIZE; lane++) {
        bool isActive = baseMask[lane];
        trueMask[lane]  = isActive && w.pred[lane];
        falseMask[lane] = isActive && !w.pred[lane];
    }

    // Run TRUE path with TRUE mask (others idle)
    if (count_active(trueMask) > 0) {
        w.active = trueMask;
        for (const auto& ins : trueBlock) {
            if (ins.op == Op::ADD1) exec_add1(w, r, ins.reps);
        }
    }

    // Run FALSE path with FALSE mask (others idle)
    if (count_active(falseMask) > 0) {
        w.active = falseMask;
        for (const auto& ins : falseBlock) {
            if (ins.op == Op::ADD1) exec_add1(w, r, ins.reps);
        }
    }

    // Reconverge: restore original mask
    w.active = baseMask;
}

// Run: pre-work - branch - post-work
static SimResult run_program(WarpState w,
                             int preWork,
                             const vector<Instr>& trueBlock,
                             const vector<Instr>& falseBlock,
                             int postWork) {
    SimResult r;

    // Start: all lanes active
    for (int lane = 0; lane < WARP_SIZE; lane++) w.active[lane] = true;

    // PRE work
    exec_add1(w, r, preWork);

    // BRANCH work
    exec_branch(w, trueBlock, falseBlock, r);

    // POST work
    exec_add1(w, r, postWork);

    return r;
}

static void print_summary(const string& title, const SimResult& r) {
    cout << "=== " << title << " ===\n";
    cout << "cycles             : " << r.cycles << "\n";
    cout << "active_lane_cycles : " << r.active_lane_cycles << "\n";

    double util = 0.0;
    if (r.cycles > 0) util = (double)r.active_lane_cycles / (double)(r.cycles * WARP_SIZE);

    cout << "utilization        : " << util << " (1.0 = perfect)\n\n";
}

int main() {
   
    int preWork = 5;
    int postWork = 5;

    vector<Instr> trueBlock  = { {Op::ADD1, 10} }; // true work = 10
    vector<Instr> falseBlock = { {Op::ADD1, 10} }; // false work = 10

    // Case 1: NO divergence (all lanes go true)
    WarpState w_no_div{};
    for (int lane = 0; lane < WARP_SIZE; lane++) w_no_div.pred[lane] = true;

    SimResult r1 = run_program(w_no_div, preWork, trueBlock, falseBlock, postWork);
    print_summary("No divergence (all lanes TRUE)", r1);

    // Case 2: Divergence (half true, half false)
    WarpState w_div{};
    for (int lane = 0; lane < WARP_SIZE; lane++) w_div.pred[lane] = (lane < 16); // FIRST 16 are True, next 16 are False;

    SimResult r2 = run_program(w_div, preWork, trueBlock, falseBlock, postWork);
    print_summary("Divergence (16 TRUE, 16 FALSE)", r2);

    return 0;
};
