#include <gtest/gtest.h>
#include "model.h"
#include <vector>

TEST(Conformance, PartialWarpDoesNotWrite) {
    Buffer mem;
    mem.buf0.resize(64, 1);
    mem.buf1.resize(64, 2);
    mem.buf2.resize(64, 0xDEADBEEFu);

    std::vector<Instr> prog = {
        {Op::LD,   0,0,0, 0,0},
        {Op::LD,   1,0,0, 1,0},
        {Op::VADD, 2,0,1, 0,0},
        {Op::ST,   0,2,0, 2,0},
        {Op::HALT, 0,0,0, 0,0}
    };

    GPU_Sim sim;
    sim.run(prog, mem, 33);

    for (int i = 0; i < 33; i++) EXPECT_EQ(mem.buf2[i], 3u);
    for (int i = 33; i < 64; i++) EXPECT_EQ(mem.buf2[i], 0xDEADBEEFu);
}
