#include "Utils.hpp"
#include <iostream>

float soft_clip(float in) {
    if (in >  1) std::cout << in << "\n";
    if (in < -1) std::cout << in << "\n";
    if (in == 0) std::cout << in << "\n";

    return in;
    if (in >  1) return  1;
    if (in < -1) return -1;
    return 1.5 * in - in * in * in * 0.5;
}