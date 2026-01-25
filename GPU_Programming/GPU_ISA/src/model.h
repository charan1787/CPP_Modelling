# pragma once
# include <vector>
# include <array>
# include <cstdint>
# include "isa.h"

struct Buffer{
    std::vector <uint32_t> buf0, buf1, buf2;
    std::vector <uint32_t> &get (uint8_t id);
};

struct Warp_state{
    std::array<std::array<uint32_t,16>,warp_size> regs{};
    std::array<bool,warp_size> pred;
    std::array<bool,warp_size> active;
};

class GPU_Sim{
public:
    void run(const std::vector<Instr>& program,Buffer &mem, uint32_t n_threads);
private:
    void exec_warp(const Instr &ins,Warp_state &warp, Buffer &mem, 
        uint32_t warp_base_id, uint32_t n_thread);
};