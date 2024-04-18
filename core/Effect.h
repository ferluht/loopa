//
// Created by ferluht on 02/12/2022.
//

#ifndef RPIDAW_EFFECT_H
#define RPIDAW_EFFECT_H

#include <AMG.h>

class Effect : public DeviceWithParameters {
public:
    explicit Effect(char const * name) : DeviceWithParameters(name) {}
};

class AudioEffect : public Effect {
public:
    explicit AudioEffect(char const * name) : Effect(name) {}
};

class MIDIEffect : public Effect {
public:

    Parameter * ison;

    explicit MIDIEffect(char const * name) : Effect(name) {
        ison = addParameter("ENBL", {"FALSE", "TRUE"}, 1);
    }

    inline void enable(bool enabled) {
        ison->value = enabled;
    }
};

#endif //RPIDAW_EFFECT_H
