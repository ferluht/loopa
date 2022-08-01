//
// Created by ferluht on 10/07/2022.
//
#include "Utils.hpp"

float soft_clip(float in) {
    if (in >  1) return  1;
    if (in < -1) return -1;
    return in - in * in * in / 3;
}