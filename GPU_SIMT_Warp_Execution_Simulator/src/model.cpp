#include "model.h"
#include <iostream>
#include <stdexcept>


// local helper (keeps header simpler)
static inline uint32_t popcount32(uint32_t x) {
#if defined(_MSC_VER)
    return (uint32_t)__popcnt(x);
#else
    return (uint32_t)__builtin_popcount(x);
#endif
}

// ---------------- Buffer ----------------
std::vector<uint32_t>& Buffer::get(uint8_t id) {
    switch (id) {
        case 0: return buf0;
        case 1: return buf1;
        case 2: return buf2;
        default: throw std::out_of_range("Invalid buffer id");
    }
};

const char* GPU_Sim::op_name(Op op) {
    switch (op) {
        case Op::LD: return "LD";
        case Op::ST: return "ST";
        case Op::VADD: return "VADD";
        case Op::CMP_LT: return "CMP_LT";
        case Op::SEL: return "SEL";
        case Op::BRA: return "BRA";
        case Op::JMP: return "JMP";
        case Op::JOIN: return "JOIN";
        case Op::HALT: return "HALT";
        default: return "?";
    }
}

std::vector<int32_t> GPU_Sim::compute_bra_join_map(const std::vector<Instr>& program) const {
    std::vector<int32_t> bra_to_join(program.size(), -1);
    std::vector<uint32_t> stack; // holds BRA pcs waiting for a JOIN

    for (uint32_t pc = 0; pc < (uint32_t)program.size(); pc++) {
        if (program[pc].op == Op::BRA) {
            stack.push_back(pc);
        } else if (program[pc].op == Op::JOIN) {
            if (!stack.empty()) {
                uint32_t bra_pc = stack.back();
                stack.pop_back();
                bra_to_join[bra_pc] = (int32_t)pc;
            }
        }
    }

    return bra_to_join;
}


void GPU_Sim::init_warp(Warp_state& w, uint32_t warp_base_tid, uint32_t n_threads) {
    w.base_mask = 0;
    for (int lane = 0; lane < warp_size; lane++) {
        uint32_t tid = warp_base_tid + (uint32_t)lane;
        if (tid < n_threads) w.base_mask |= (1u << lane);
    }

    w.active_mask = w.base_mask;
    w.pc = 0;
    w.halted = false;
    w.stack.clear();

    for (int lane = 0; lane < warp_size; lane++) {
        w.pred[lane] = false;
        for (int r = 0; r < 16; r++) w.regs[lane][r] = 0;
    }
}

void GPU_Sim::step_warp(Warp_state& w,
                        const std::vector<Instr>& program,
                        const std::vector<int32_t>& bra_to_join,
                        Buffer& mem,
                        uint32_t warp_base_tid,
                        Metrics& m,
                        bool trace) {
    if (w.halted) return;
    if (w.active_mask == 0) { w.halted = true; return; }
    if (w.pc >= program.size()) { w.halted = true; return; }

    const Instr& ins = program[w.pc];

    m.warp_cycles++;
    m.active_lane_cycles += (uint64_t)popcount32(w.active_mask);

    if (trace) {
        std::cout << "pc=" << w.pc
                  << " op=" << op_name(ins.op)
                  << " mask=0x" << std::hex << w.active_mask << std::dec
                  << " stack=" << w.stack.size()
                  << "\n";
    }

    auto lane_active = [&](int lane) -> bool {
        return ((w.active_mask >> lane) & 1u) != 0;
    };

    switch (ins.op) {
        case Op::LD: {
            m.mem_lane_ops += (uint64_t)popcount32(w.active_mask);
            const auto& B = mem.get(ins.buf);

            for (int lane = 0; lane < warp_size; lane++) {
                if (!lane_active(lane)) continue;

                uint32_t tid = warp_base_tid + (uint32_t)lane;
                int64_t addr64 = (int64_t)tid + (int64_t)ins.imm;
                if (addr64 < 0) continue;
                uint32_t addr = (uint32_t)addr64;

                if (addr < B.size()) w.regs[lane][ins.dst] = B[addr];
            }
            w.pc++;
            break;
        }

        case Op::ST: {
            m.mem_lane_ops += (uint64_t)popcount32(w.active_mask);
            auto& B = mem.get(ins.buf);

            for (int lane = 0; lane < warp_size; lane++) {
                if (!lane_active(lane)) continue;

                uint32_t tid = warp_base_tid + (uint32_t)lane;
                int64_t addr64 = (int64_t)tid + (int64_t)ins.imm;
                if (addr64 < 0) continue;
                uint32_t addr = (uint32_t)addr64;

                if (addr < B.size()) B[addr] = w.regs[lane][ins.a];
            }
            w.pc++;
            break;
        }

        case Op::VADD: {
            for (int lane = 0; lane < warp_size; lane++) {
                if (!lane_active(lane)) continue;
                w.regs[lane][ins.dst] = w.regs[lane][ins.a] + w.regs[lane][ins.b];
            }
            w.pc++;
            break;
        }

        case Op::CMP_LT: {
            for (int lane = 0; lane < warp_size; lane++) {
                if (!lane_active(lane)) continue;
                w.pred[lane] = (w.regs[lane][ins.a] < w.regs[lane][ins.b]);
            }
            w.pc++;
            break;
        }

        case Op::SEL: {
            for (int lane = 0; lane < warp_size; lane++) {
                if (!lane_active(lane)) continue;
                uint32_t A = w.regs[lane][ins.a];
                uint32_t B = w.regs[lane][ins.b];
                w.regs[lane][ins.dst] = w.pred[lane] ? A : B;
            }
            w.pc++;
            break;
        }

        case Op::BRA: {
            uint32_t taken = 0;
            uint32_t not_taken = 0;

            for (int lane = 0; lane < warp_size; lane++) {
                if (!lane_active(lane)) continue;
                if (w.pred[lane]) taken |= (1u << lane);
                else              not_taken |= (1u << lane);
            }

            bool diverged = (taken != 0) && (not_taken != 0);
            if (diverged) m.divergent_branches++;

            uint32_t fallthrough_pc = w.pc + 1;
            uint32_t target_pc = (uint32_t)ins.imm;

            int32_t join_pc = (w.pc < bra_to_join.size()) ? bra_to_join[w.pc] : -1;

            // If no JOIN exists, behave like uniform branch
            if (join_pc < 0 || !diverged) {
                if (taken) w.pc = target_pc;
                else       w.pc = fallthrough_pc;
                break;
            }

            // Diverged : execute taken now, defer not-taken
            StackFrame fr;
            fr.deferred_mask = not_taken;
            fr.deferred_pc   = fallthrough_pc;
            fr.join_pc       = (uint32_t)join_pc;
            w.stack.push_back(fr);


            w.active_mask = taken;
            w.pc = target_pc;
            break;
        }

        case Op::JMP: {
            w.pc = (uint32_t)ins.imm;
            break;
        }

        case Op::JOIN: {
            if (!w.stack.empty() && w.stack.back().join_pc == w.pc) {
                StackFrame fr = w.stack.back();
                w.stack.pop_back();

                w.active_mask = fr.deferred_mask;
                w.pc = fr.deferred_pc;
                m.reconverges++;
            } else {
                w.pc++;
            }
            break;
        }

        case Op::HALT:
        default:
            w.halted = true;
            break;
    }
}

Metrics GPU_Sim::run(const std::vector<Instr>& program, Buffer& mem, uint32_t n_threads, bool trace) {
    Metrics m;
    if (n_threads == 0) return m;

    uint32_t n_warps = ((n_threads - 1) / warp_size) + 1;
    std::vector<Warp_state> warps(n_warps);

    auto bra_to_join = compute_bra_join_map(program);

    for (uint32_t wid = 0; wid < n_warps; wid++) {
        init_warp(warps[wid], wid * warp_size, n_threads);
    }

    bool any_running = true;
    while (any_running) {
        any_running = false;

        for (uint32_t wid = 0; wid < n_warps; wid++) {
            if (warps[wid].halted) continue;
            any_running = true;

            step_warp(warps[wid], program, bra_to_join, mem, wid * warp_size, m, trace);
        }
    }

    return m;
}
