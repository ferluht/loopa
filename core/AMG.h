//
// Created by ferluht on 21/07/2022.
//

#pragma once

#include "A.h"
#include "M.h"
#include "G.h"

class AMG : public A, public M, public G {
public:
    char name[50];

    AMG() {
        strcpy(name, "name");
    }

    AMG(const char * name_) : A(), M(), G() {
        strcpy(name, name_);
    }
};