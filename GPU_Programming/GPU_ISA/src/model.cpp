#include "model.h"
#include <stdexcept>

std::vector<uint32_t> & Buffer::get(uint8_t id){
    switch(id){
        case 0 : return buf0;
        case 1 : return buf1;
        case 2 : return buf2;
        default : throw std::out_of_range("Invalid buffer id");
    }
};

void GPU_Sim::exec_warp(const Instr &ins,Warp_state &warp, Buffer &mem, 
        uint32_t warp_base_id, uint32_t n_thread){
        
    for(int lane = 0; lane < warp_size ; lane++){
        uint32_t tid = warp_base_id + lane;
        warp.active[lane] = tid < n_thread;
    }

    for(int lane = 0; lane < warp_size ; lane++){
        if (!warp.active[lane]) continue;

        uint32_t tid = warp_base_id + lane;
        
        // addr = tid + imm (supports negative imm safely)
        int64_t addr64 = (int64_t)tid + ins.imm;
        if (addr64 < 0) continue;
        uint32_t addr = (uint32_t)addr64;

        switch (ins.op) {
            case Op::LD: {
                const auto& B = mem.get(ins.buf);
                if (addr < B.size()) warp.regs[lane][ins.dst] = B[addr];
                break;
            }

            case Op::ST: {
                auto& B = mem.get(ins.buf);
                if (addr < B.size()) B[addr] = warp.regs[lane][ins.a];
                break;
            }

            case Op::VADD: {
                uint32_t A = warp.regs[lane][ins.a];
                uint32_t B = warp.regs[lane][ins.b];
                warp.regs[lane][ins.dst] = (uint32_t)(A + B);
                break;
            }

            case Op::CMP_LT: {
                uint32_t A = warp.regs[lane][ins.a];
                uint32_t B = warp.regs[lane][ins.b];
                warp.pred[lane] = (A < B);
                break;
            }

            case Op::SEL: {
                uint32_t A = warp.regs[lane][ins.a];
                uint32_t B = warp.regs[lane][ins.b];
                warp.regs[lane][ins.dst] = warp.pred[lane] ? A : B;
                break;
            }

            case Op::HALT:
            default:
                break;
        }


    }

}

void GPU_Sim::run(const std::vector<Instr>& program, Buffer& mem, uint32_t n_threads){
    

    uint32_t n_warps = ((n_threads - 1)/warp_size) + 1; // classic upperbound operartion of a number.
    std::vector<Warp_state> warp(n_warps);
    
    for(auto & instr : program){
        
        if(instr.op == Op::HALT) break;
        
        for(uint32_t wid = 0; wid < n_warps ; wid++){
            
            exec_warp(instr,warp[wid],mem,wid*warp_size,n_threads);
        }
    }
}