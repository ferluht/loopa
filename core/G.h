//
// Created by ferluht on 19/07/2022.
//

#pragma once

#include <cstdint>
#include <Adafruit-GFX-offscreen/Adafruit_GFX.h>

/**
 * G stands for Graphics. The base class for all graphic objects.
*/
class G {
public:

    G() {

    }

    /**
     * Virtual draw method
     *
     * @param screen GFXcanvas1
     *
     * Example of usage in child class:
     * @code
     * void draw(GFXcanvas1 * screen) override {
     *   screen->setCursor(0, 0);
     *   screen->setTextSize(1);
     *   screen->print("SAMPLE TEXT");
     * }
     * @endcode
     */
    virtual void draw(GFXcanvas1 * screen) {}
};
