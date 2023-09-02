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
        max = values.size();
        step = 1;
    }

    void update(uint8_t v) {
        float inc = (float) (v - 64) * step;
        value += inc;
        if (value > max) value = max;
        if (value < min) value = min;
    }

    inline float getVal() {
        return value;
    }

    inline float getVal0to1() {
        return (value - min) / (max - min);
    }

    inline std::string getStringVal() {
        if (continuous) return values[(int)value];
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

    DeviceWithParameters(const char * name_) : AMG(name_) {
//        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::SCREEN::ALT_PARAMS, [this](MData &cmd) -> MIDISTATUS {
//            this->altparams = cmd.data2 > 0;
//            return MIDISTATUS::DONE;
//        });

        addMIDIHandler({MIDI::GENERAL::CC_HEADER}, {CC_E1, CC_E2}, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data1 == CC_E1 && inputparams.size() > parampage * 2 + 0) inputparams[parampage * 2 + 0]->update(cmd.data2);
            if (cmd.data1 == CC_E2 && inputparams.size() > parampage * 2 + 1) inputparams[parampage * 2 + 1]->update(cmd.data2);
            return MIDISTATUS::DONE;
        });
    }

    void draw(GFXcanvas1 * screen) override {
        for (int i = parampage * 2; i < parampage * 2 + 2; i ++) {
            if (inputparams.size() <= i) continue;
            int xoffset = (int)(i / 2) * 46;
            int yoffset = (i % 2) * 10;
            screen->setCursor(4 + xoffset, 17 + yoffset);
            screen->setTextSize(1);
            screen->print(inputparams[i]->name.c_str());
            screen->drawRect(26 + xoffset, 14 + yoffset, 20, 4, 1);
            screen->drawRect(26 + xoffset, 15 + yoffset, inputparams[i]->getVal0to1() * 19 + 1, 2, 1);
        }
    }

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