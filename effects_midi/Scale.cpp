//
// Created by ferluht on 20/07/2022.
//

#include "Scale.h"

void Scale::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 16);
    screen->setTextSize(1);
    screen->print(selected_scale->first);
}