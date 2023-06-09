//
// Created by ferluht on 21/07/2022.
//

#pragma once

#include "A.h"
#include "M.h"
#include "G.h"
#include <cstring>

/**
 * AMG stands for Audio+MIDI+Graphics. The base class for all audio+midi+graphic objects.
*/
class AMG : public A, public M, public G {
    char name[50];
public:

    AMG() {
        std::strcpy(name, "name");
    }

    AMG(const char * name_) : A(), M(), G() {
        std::strcpy(name, name_);
    }

    inline const char * getName() {
        return name;
    }
};

class Parameter {

public:

    Parameter() {
        value = 0;
    }

    void set(uint8_t v) {
        value = (float)v / 128.0;
    }

    void update(uint8_t v) {
        float inc = (float)(v - 64) / 128.0;
        value += inc;
        if (value > 1) value = 1;
        if (value < 0) value = 0;
    }

    std::string name;
    bool enabled;
    float value;
};