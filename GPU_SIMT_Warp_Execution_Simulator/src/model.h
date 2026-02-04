
# include <array>
# include <cstdint>
# include <vector>
# include <stdexcept>
# include "isa_2.h"

// ---------------- Memory ----------------
struct Buffer {
    std::vector<uint32_t> buf0, buf1, buf2;
    std::vector<uint32_t>& get(uint8_t id);
};

// ---------------- Metrics ----------------
struct Metrics {
    uint64_t warp_cycles = 0;
    uint64_t active_lane_cycles = 0;
    uint64_t mem_lane_ops = 0;
    uint64_t divergent_branches = 0;
    uint64_t reconverges = 0;
};

// ---------------- SIMT Stack Frame ----------------
struct StackFrame {
    uint32_t deferred_mask = 0;
    uint32_t deferred_pc = 0;
    uint32_t join_pc = 0;
};

// ---------------- Warp State ----------------
struct Warp_state {
    std::array<std::array<uint32_t, 16>, warp_size> regs{};
    std::array<bool, warp_size> pred{};

    uint32_t base_mask = 0;    // lanes with tid < n_threads
    uint32_t active_mask = 0;  // dynamic lanes executing current path

    uint32_t pc = 0;
    bool halted = false;

    std::vector<StackFrame> stack;
};

class GPU_Sim {
public:
    Metrics run(const std::vector<Instr>& program, Buffer& mem, uint32_t n_threads, bool trace=false);

private:
    // For structured programs : each BRA reconverges at the next JOIN after it.
    std::vector<int32_t> compute_bra_join_map(const std::vector<Instr>& program) const;

    void init_warp(Warp_state& w, uint32_t warp_base_tid, uint32_t n_threads);

    // Execute one instruction for one warp (one warp-cycle).
    void step_warp(Warp_state& w,
                   const std::vector<Instr>& program,
                   const std::vector<int32_t>& bra_to_join,
                   Buffer& mem,
                   uint32_t warp_base_tid,
                   Metrics& m,
                   bool trace);

    static const char* op_name(Op op);
};
