//
// Created by ferluht on 20/07/2022.
//

#include "Scale.h"

void Scale::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 17);
    screen->print("SCALE");
    screen->setCursor(28, 17);
    std::string scale = selected_scale->first;
    screen->print(((*selected_root) + " " + scale).c_str());
}