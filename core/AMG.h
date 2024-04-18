//
// Created by ferluht on 21/07/2022.
//

#pragma once

#include "A.h"
#include "M.h"
#include "G.h"
#include <cstring>

extern float GLOBAL_SPEED;

class Parameter {

    float step;
    float min;
    float max;
    std::vector<std::string> values;

public:

    bool continuous;
    std::string name;
    float value;

    Parameter() : Parameter("", 0) {};

    Parameter(std::string name_, float value_) {
        value = value_;
        name = name_;
        max = 1;
        min = 0;
        step = 0.01;
        continuous = true;
    }

    Parameter(std::string name_, float min_, float max_, float value_, float step_) :
            Parameter(name_, value_) {
        continuous = true;
        step = step_;
        min = min_;
        max = max_;
    }

    Parameter(std::string name_, std::vector<std::string> values_, int default_value) :
            Parameter(name_, default_value) {
        continuous = false;
        values = values_;
        min = 0;
        max = values.size() - 1;
        step = 1;
    }

    void update(uint8_t v) {
        float inc = (float) (v - 64) * step;
        if (value + inc > max) value = max;
        else if (value + inc < min) value = min;
        else value += inc;
    }

    inline float getVal() {
        return value;
    }

    inline float getVal0to1() {
        return (value - min) / (max - min);
    }

    inline std::string getStringVal() {
        if (!continuous) return values[(int)value];
        return "default";
    }
};


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

class DeviceWithParameters : public AMG {
    std::vector<Parameter *> inputparams;
    int parampage = 0;

public:

    DeviceWithParameters(const char * name_);

    Parameter * addParameter(std::string name) {
        return addParameter(name, 0);
    }

    Parameter * addParameter(std::string name, float default_value) {
        Parameter * p = new Parameter(name, default_value);
        inputparams.push_back(p);
        return p;
    }

    Parameter * addParameter(std::string name, float min, float max, float default_value, float step) {
        Parameter * p = new Parameter(name, min, max, default_value, step);
        inputparams.push_back(p);
        return p;
    }

    Parameter * addParameter(std::string name, std::vector<std::string> values, int default_value) {
        Parameter * p = new Parameter(name, values, default_value);
        inputparams.push_back(p);
        return p;
    }
};