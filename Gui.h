//
// Created by ferluht on 25/11/2022.
//

#ifndef RPIDAW_GUI_H
#define RPIDAW_GUI_H

#ifdef __APPLE__
    #define GLFW_INCLUDE_GLCOREARB
#else
    #define GL_GLEXT_PROTOTYPES
#endif

#ifndef __APPLE__
    int fd;
    uint8_t uartbuffer[256];
    uint8_t uartit = 0;
#else
    #define GLFW_INCLUDE_GLU
    #include <GLFW/glfw3.h>
    #include <nanogui/nanogui.h>
    using namespace nanogui;
#endif

#include "Daw.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#include "Adafruit-GFX-offscreen/Adafruit_GFX.h"

void init_gui();
bool process_gui();
void scan_buttons();
void close_gui();

#endif //RPIDAW_GUI_H
