//
// Created by ferluht on 25/11/2022.
//

#pragma once

#include <stdint.h>

#ifdef __APPLE__
    #define GLFW_INCLUDE_GLCOREARB
#else
    #define GL_GLEXT_PROTOTYPES
#endif

#ifndef __APPLE__
    #include <wiringSerial.h>
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

