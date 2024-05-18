cmake_minimum_required(VERSION 3.7.2)
project(loopa)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

if (MACOSX)
    set(BUILD_TESTING OFF)
    add_subdirectory(3rdparty/rtmidi)
    add_subdirectory(3rdparty/rtaudio)
else()
    FIND_LIBRARY(rtaudio NAMES RtAudio)
    FIND_LIBRARY(rtmidi NAMES RtMidi)
    FIND_LIBRARY(wiringPi NAMES wiringPi)
endif()

set(CMAKE_CXX_STANDARD 14)

include_directories(common)
include_directories(core)
include_directories(effects_audio)
include_directories(effects_midi)
include_directories(effects_master)
include_directories(instruments)
include_directories(utils)
include_directories(arduino/loopa)
include_directories(3rdparty)
include_directories(3rdparty/nanogui/include)
include_directories(3rdparty/nanogui/ext/eigen)
include_directories(3rdparty/nanogui/ext/glfw/include)
include_directories(3rdparty/nanogui/ext/nanovg/src)
include_directories(3rdparty/tinyxml2)

set(LIBSAMPLERATE_TESTS OFF)
set(LIBSAMPLERATE_EXAMPLES OFF)
set(LIBSAMPLERATE_INSTALL OFF)
add_subdirectory(3rdparty/libsamplerate)

if(NOT MACOS)
include_directories(/opt/vc/include)
link_directories(/opt/vc/lib)
endif()

#include_directories(/opt/vc/include)
#link_directories(/opt/vc/lib)
#
#FIND_LIBRARY(brcmEGL NAMES brcmEGL)
#FIND_LIBRARY(brcmGLESv2 NAMES brcmGLESv2)

add_executable(loopa
        main.cpp
        DawProcessing.cpp
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
#        effects_audio/Tape/Tape.cpp
        effects_audio/Tape/NewTape.cpp
        common/Utils.cpp
        core/AMG.cpp
        instruments/SingleTone/SingleTone.cpp
        core/Sync.cpp
        effects_midi/Scale.cpp
        effects_midi/Arpeggiator.cpp
        core/Rack.cpp
        utils/WavFile.cpp
        utils/Filesystem.cpp
        DawMidiHandlers.cpp
        DawScreens.cpp
        instruments/Sampler/Sampler.cpp
        instruments/Sampler/SampleKit.cpp
        instruments/MicInput/MicInput.cpp
        3rdparty/Adafruit-GFX-offscreen/Adafruit_GFX.cpp
        utils/SerialPort.cpp
#        effects_audio/LoopMatrix/LoopMatrix.cpp
        effects_audio/Tanhx/Tanhx.cpp
        3rdparty/tinyxml2/tinyxml2.cpp)

set(CMAKE_CXX_FLAGS "-O3 -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -Wno-cpp") # -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -Wall -Wextra")
#target_compile_options(rpidaw PRIVATE -Ofast)

if (MACOSX)
    set(CMAKE_CXX_FLAGS "-O0 -Wno-cpp")
    target_link_libraries(loopa rtaudio rtmidi samplerate)
else()
    set(CMAKE_CXX_FLAGS "-O3 -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -Wno-cpp")
    target_link_libraries(loopa rtaudio rtmidi samplerate wiringPi)
endif()