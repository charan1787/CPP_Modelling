#include <gtest/gtest.h>
#include "model.h"
#include <random>
#include <vector>

static void ref_add(const std::vector<uint32_t>& A,
                    const std::vector<uint32_t>& B,
                    std::vector<uint32_t>& C,
                    uint32_t n) {
    for (uint32_t i = 0; i < n; i++) C[i] = (uint32_t)(A[i] + B[i]);
}

TEST(Random, VADD_MatchesReference) {
    std::mt19937 rng(1234567u);

    std::uniform_int_distribution<uint32_t> dist_u32;
    std::uniform_int_distribution<uint32_t> dist_n(1, 2000);

    for (int trial = 0; trial < 50; trial++) {
        uint32_t n = dist_n(rng);

        Buffer mem;
        mem.buf0.resize(n);
        mem.buf1.resize(n);
        mem.buf2.resize(n, 0);

        for (uint32_t i = 0; i < n; i++) {
            mem.buf0[i] = dist_u32(rng);
            mem.buf1[i] = dist_u32(rng);
        }

        std::vector<Instr> prog = {
            {Op::LD,   0,0,0, 0,0},
            {Op::LD,   1,0,0, 1,0},
            {Op::VADD, 2,0,1, 0,0},
            {Op::ST,   0,2,0, 2,0},
            {Op::HALT, 0,0,0, 0,0}
        };

        GPU_Sim sim;
        sim.run(prog, mem, n);

        std::vector<uint32_t> refC(n);
        ref_add(mem.buf0, mem.buf1, refC, n);

        for (uint32_t i = 0; i < n; i++) {
            ASSERT_EQ(mem.buf2[i], refC[i]);
        }
    }
}
