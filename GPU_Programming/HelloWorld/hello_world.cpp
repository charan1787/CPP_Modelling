#include<iostream>
#include<array>

int main(){
    constexpr int Lanes = 32; 
    // used for constants during compile time
    // we did not use const as it is used for runtime constants

    std::array<int, Lanes> data; // 32 threads
    // lane means each thread

    for(int lane = 0;lane < Lanes;lane++){
        data[lane] = lane * 2; // one instruction applied to all lanes
        std::cout<< "Hello from lane" << lane << ", value " << data[lane] << std::endl;

    }

    return 0;

}