#include "model.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    uint32_t N = 64; // No.of threads

    Buffer mem;
    mem.buf0.resize(N); // as each thread has its own memory it is of size N
    mem.buf1.resize(N);
    mem.buf2.resize(N, 0);

    for (uint32_t i = 0; i < N; i++) {
        mem.buf0[i] = i;
        mem.buf1[i] = 50 + i / 2;
    }

    vector<Instr> prog = {
        {Op::LD,     0,0,0, 0, 0},
        {Op::LD,     1,0,0, 1, 0},
        {Op::CMP_LT, 0,0,1, 0, 0},
        {Op::SEL,    2,0,1, 0, 0},
        {Op::ST,     0,2,0, 2, 0},
        {Op::HALT,   0,0,0, 0, 0}
    };

    GPU_Sim sim;
    sim.run(prog, mem, N);

    cout << "T-ID  buf0  buf1  buf2(min)\n";
    for (uint32_t i = 0; i < N; i++) {
        cout << " " << i << "     " << mem.buf0[i]
             << "    " << mem.buf1[i]
             << "     " << mem.buf2[i] << "\n";
    }
}
