//
// Created by ferluht on 14/06/2023.
//

#include <AMG.h>

float GLOBAL_SPEED = 1.0;
uint8_t SCREEN_IDX = SCREENS::TAPE_VIEW;

std::unordered_map<DeviceFactory::DEVICE_TYPE, std::unordered_map<std::string, DeviceFactory::create_f *>> & DeviceFactory::registry()
{
    static std::unordered_map<DeviceFactory::DEVICE_TYPE, std::unordered_map<std::string, DeviceFactory::create_f *>> impl;
    return impl;
}

DeviceWithParameters::DeviceWithParameters(const char *name_) : AMG(name_) {

    addMIDIHandler({}, {MIDI::GENERAL::CC_HEADER}, {CC_E1, CC_E2}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data1 == CC_E1 && inputparams.size() > parampage * 2 + 0) inputparams[parampage * 2 + 0]->update(cmd.data2);
        if (cmd.data1 == CC_E2 && inputparams.size() > parampage * 2 + 1) inputparams[parampage * 2 + 1]->update(cmd.data2);
        cmd.status = MIDI::GENERAL::INVALIDATED;
    });

    addMIDIHandler({}, {MIDI::GENERAL::CC_HEADER}, {CC_E3, CC_E4}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data1 == CC_E3 && cmd.data2 > 0 && parampage > 0) parampage --;
        if (cmd.data1 == CC_E4 && cmd.data2 > 0 && parampage < inputparams.size() / 2 - 1) parampage ++;
        cmd.status = MIDI::GENERAL::INVALIDATED;
    });

    addDrawHandler({}, [this](GFXcanvas1 * screen) -> void {
        for (int i = parampage * 2; i < parampage * 2 + 2; i ++) {
            if (inputparams.size() <= i) continue;
            int xoffset = 6;
            int yoffset = (i % 2) * 10;
            screen->setCursor(xoffset, 17 + yoffset);
            screen->setTextSize(1);
            screen->print(inputparams[i]->name.c_str());
            screen->drawRect(22 + xoffset, 14 + yoffset, 20, 4, 1);
            screen->drawRect(22 + xoffset, 15 + yoffset, inputparams[i]->getVal0to1() * 19 + 1, 2, 1);

//            int xoffset = 12 + (i % 2) * 46;
//            int yoffset = 2;
//            screen->setCursor(xoffset - 4, 23 + yoffset);
//            screen->setTextSize(1);
//
//            screen->print((inputparams[i]->name + ": " + std::to_string(inputparams[i]->getVal0to1())).c_str());
//            screen->drawRect(xoffset - 4, 12 + yoffset, 28, 4, 1);
//            screen->drawRect(xoffset - 4, 13 + yoffset, inputparams[i]->getVal0to1() * 27 + 1, 2, 1);
        }
    });
}