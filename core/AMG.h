//
// Created by ferluht on 21/07/2022.
//

#pragma once

#include "A.h"
#include "M.h"
#include "G.h"
#include <cstring>

class AMG : public A, public M, public G {
public:
    char name[50];

    AMG() {
        std::strcpy(name, "name");
    }

    AMG(const char * name_) : A(), M(), G() {
        std::strcpy(name, name_);
    }
};