//
// Created by ferluht on 13/06/2023.
//

#include "MasterEffects.h"

MasterMIDIEffects::MasterMIDIEffects() : MIDIEffect("MASTERFX") {

}

void MasterMIDIEffects::draw(GFXcanvas1 *screen) {
    screen->setCursor(4, 17);
    screen->print("SCALE");
    screen->setCursor(28, 17);
    std::string s = scale.getScale();
    std::string r = scale.getRoot();
    screen->print((r + " " + s).c_str());
}