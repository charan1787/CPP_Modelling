#include<iostream>
#include<cstdint>

int main(){
    int warp = 32; // 32 threads
    int true_lanes = 16; // 16 threads for true
    int false_lanes = 16; // 16 threads for false

    int pre = 5; // cycles needed for pre work
    int true_case = 10; // cycles needed for true work
    int false_case = 10; // cycles needed for false work
    int post = 5; // cycles needed for post work

    int cycles_no_div = pre + true_case + post;
    int cycles_div = pre + true_case + false_case + post;

    double util_no_div = 1.0;
    double util_div = double((pre + post) * warp + true_case * true_lanes + false_case * false_lanes)/ double(warp * cycles_div);

    // utilisation = (total pre work + total post work + total true false work) / ( warps + total cycles)
    // total pre or post work = respenctive cycles * threads working 

    std::cout << "No divergence cycles: " << cycles_no_div << "  util: " << util_no_div << "\n";
    std::cout << "Divergence cycles:    " << cycles_div    << "  util: " << util_div    << "\n";



}