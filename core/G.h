//
// Created by ferluht on 19/07/2022.
//

#pragma once

#include <cstdint>
#include <Adafruit-GFX-offscreen/Adafruit_GFX.h>
#include <Screens.h>
#include <iostream>

/**
 * G stands for Graphics. The base class for all graphic objects.
*/
class G {

    std::map<uint8_t, int> handlers;
    std::vector<std::function<void(GFXcanvas1 *)>> handlers_array;

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
    void draw(GFXcanvas1 * screen) {
        auto h = handlers.find(SCREEN_IDX);
        if (h != handlers.end()) handlers_array[h->second](screen);
    }

    void addDrawHandler(std::vector<uint8_t> screen_indices, std::function<void(GFXcanvas1 *)> drawer) {
        if (screen_indices.empty())
            for (int i = 0; i < MAX_SCREENS; i ++)
                screen_indices.push_back(i);

        handlers_array.push_back(drawer);
        for (uint8_t idx : screen_indices)
            handlers[idx] = handlers_array.size() - 1;
    }
};
