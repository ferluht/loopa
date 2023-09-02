//
// Created by ferluht on 13/06/2023.
//

#ifndef RPIDAW_MASTEREFFECTS_H
#define RPIDAW_MASTEREFFECTS_H

#include <Effect.h>
#include <MidiEffects.h>

class MasterMIDIEffects : public MIDIEffect {

public:

    Scale scale;

    MasterMIDIEffects();
    void draw(GFXcanvas1 * screen) override;
};


#endif //RPIDAW_MASTEREFFECTS_H
