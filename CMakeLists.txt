cmake_minimum_required(VERSION 3.7.2)
project(rpidaw)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

set(BUILD_TESTING OFF)
add_subdirectory(3rdparty/rtmidi)
add_subdirectory(3rdparty/rtaudio)

add_subdirectory(3rdparty/nanogui)

FIND_LIBRARY(wiringPi NAMES wiringPi)

set(CMAKE_CXX_STANDARD 14)

include_directories(common)
include_directories(core)
include_directories(effects_audio)
include_directories(effects_midi)
include_directories(instruments)
include_directories(ui)
include_directories(utils)
include_directories(3rdparty)
include_directories(3rdparty/nanogui/include)
include_directories(3rdparty/nanogui/ext/eigen)
include_directories(3rdparty/nanogui/ext/glfw/include)
include_directories(3rdparty/nanogui/ext/nanovg/src)

set(LIBSAMPLERATE_TESTS OFF)
set(LIBSAMPLERATE_EXAMPLES OFF)
set(LIBSAMPLERATE_INSTALL OFF)
add_subdirectory(3rdparty/libsamplerate)

#include_directories(/opt/vc/include)
#link_directories(/opt/vc/lib)
#
#FIND_LIBRARY(brcmEGL NAMES brcmEGL)
#FIND_LIBRARY(brcmGLESv2 NAMES brcmGLESv2)

add_executable(rpidaw
        main.cpp
        Daw.cpp
        effects_audio/Limiter/SoftClipper.cpp
        instruments/SimpleInstrument/SimpleInstrument.cpp
        common/ADSR.cpp
        effects_audio/Plateau/OnePoleFilters.cpp
        effects_audio/Plateau/Dattorro.cpp
        effects_audio/Plateau/Utilities.cpp
        effects_audio/Plateau/Plateau.cpp
        effects_audio/Delay/Delay.cpp
        effects_audio/Chronoblob/PingPong.cpp
        effects_audio/Chronoblob/Chronoblob.cpp
        effects_audio/Tape/Tape.cpp
        common/Utils.cpp
        instruments/SingleTone/SingleTone.cpp
        3rdparty/nanovg/src/nanovg.c
        effects_midi/Scale.cpp
        ui/Rack.cpp
        ui/TapeRack.cpp
        utils/WavFile.cpp
        instruments/Sampler/Sampler.cpp
        instruments/Sampler/SampleKit.cpp
        3rdparty/Adafruit-GFX-offscreen/Adafruit_GFX.cpp Gui.cpp Gui.h)

set(CMAKE_CXX_FLAGS "-O3 -Wno-cpp") # -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -Wall -Wextra")
#target_compile_options(rpidaw PRIVATE -Ofast)

if (MACOSX)
    target_link_libraries(rpidaw rtaudio rtmidi samplerate nanogui)
else()
    target_link_libraries(rpidaw rtaudio rtmidi samplerate wiringPi brcmEGL brcmGLESv2)
endif()