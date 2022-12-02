//
// Created by ferluht on 02/12/2022.
//

#ifndef RPIDAW_EFFECT_H
#define RPIDAW_EFFECT_H

#include <AMG.h>

class Effect : public AMG {
public:
    Effect(char const * name) : AMG(name) {

    }
};

class AudioEffect : public Effect {
public:
    AudioEffect(char const * name) : Effect(name) {

    }
};

class MIDIEffect : public Effect {
public:
    MIDIEffect(char const * name) : Effect(name) {

    }
};

#endif //RPIDAW_EFFECT_H
