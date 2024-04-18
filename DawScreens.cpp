//
// Created by ferluht on 06/01/2024.
//

#include "Daw.h"

inline void draw_fx_screen_header(GFXcanvas1 * screen, int idx, int size, std::string prefix, const char * fx_name) {
    std::string title;

    if (size > 0)
        title = prefix + " " + std::to_string(idx + 1) + "/" + std::to_string(size);
    else
        title = prefix;

    int16_t x1, y1;
    uint16_t w, h;
    screen->getTextBounds(fx_name, 0, 0, &x1, &y1, &w, &h);
    screen->setCursor(122 - w, 6);
    screen->print(fx_name);

    for (int i = 0; i < 18; i ++) screen->leds[i] = 0;
    screen->drawFastHLine(0, 8, 128, 1);
    screen->setCursor(4, 6);
    screen->setTextSize(1);
    screen->print(title.c_str());
}

inline void draw_waiting_message(GFXcanvas1 * screen, bool draw) {
    if (draw) {
        screen->fillRect(16, 4, 96, 24, 0);
        screen->drawRect(16, 4, 96, 24, 1);
        screen->setCursor(32, 20);
        screen->setTextSize(2);
        screen->print("WAITING...");
    }
}

void DAW::initScreens() {

    addDrawHandler({}, [this](GFXcanvas1 * screen) -> void {
        for (int i = 0; i < 18; i ++) screen->leds[i] = 0;

        screen->setCursor(6, 6);
        screen->setTextSize(1);
        screen->print("NO SCREEN");
    });

    addDrawHandler({SCREENS::LOOP_VIEW}, [this](GFXcanvas1 * screen) -> void {
        for (int i = 0; i < 18; i ++) screen->leds[i] = 0;

        if (recording_blink_counter > 15)
            screen->setLed(1, recording_master * 10, 0, 0);
        recording_blink_counter = (recording_blink_counter + 1) % 30;

        screen->drawFastVLine(96, 0, 32, 1);
        screen->drawFastHLine(0, 8, 128, 1);

        screen->setFont(&Picopixel);
        screen->setTextSize(1);

        auto selected_track = (Rack *) (tracks->get_focus());
        screen->setCursor(6, 6);
        screen->setTextSize(1);
        screen->print(("TRACK " + std::to_string(tracks->get_focus_index())).c_str());

        auto selected_instrument_rack = (Rack *) (selected_track->get_item(1));
        screen->setTextSize(1);
        auto instr_name = selected_instrument_rack->get_focus()->getName();
        int16_t x1, y1;
        uint16_t w, h;
        screen->getTextBounds(instr_name, 0, 0, &x1, &y1, &w, &h);
        screen->setCursor(90 - w, 6);
        screen->print(instr_name);

        getCurrentTrackInstrument()->draw(screen);

        screen->setCursor(103, 6);
        screen->setTextSize(1);
        screen->print("TAPES");

        tapes->draw(screen);
    });

    addDrawHandler({SCREENS::TAPE_VIEW}, [this](GFXcanvas1 * screen) -> void {
        for (int i = 0; i < 18; i ++) screen->leds[i] = 0;

        if (recording_blink_counter > 15)
            screen->setLed(1, recording_master * 10, 0, 0);
        recording_blink_counter = (recording_blink_counter + 1) % 30;

        screen->setFont(&Picopixel);
        screen->setTextSize(1);

        auto selected_track = (Rack *) (tracks->get_focus());
        auto selected_instrument_rack = (Rack *) (selected_track->get_item(1));
        screen->setCursor(50, 7);
        screen->setTextSize(1);
        auto s = "TRACK " + std::to_string(tracks->get_focus_index()) + ": " + selected_instrument_rack->get_focus()->getName();
        screen->print(s.c_str());

        tapes->draw(screen);
    });

    addDrawHandler({SCREENS::TRACK_VIEW}, [this](GFXcanvas1 * screen) -> void {
        auto rack = getCurrentTrackInstrument();
        for (int i = 0; i < 18; i ++) screen->leds[i] = 0;
        draw_fx_screen_header(screen, tracks->get_focus_index(), tracks->get_size(),
                              "TRACK", rack->getName());
        rack->draw(screen);
    });

    addDrawHandler({SCREENS::MASTER_EFFECTS_MIDI}, [this](GFXcanvas1 * screen) -> void {
        draw_fx_screen_header(screen, master_midi_fx->get_focus_index(), master_midi_fx->get_size(),
                              "MASTER MIDI FX", master_midi_fx->get_focus()->getName());
        master_midi_fx->get_focus()->draw(screen);
    });

    addDrawHandler({SCREENS::MASTER_EFFECTS_AUDIO}, [this](GFXcanvas1 * screen) -> void {
        draw_fx_screen_header(screen, master_audio_fx->get_focus_index(), master_audio_fx->get_size(),
                              "MASTER AUDIO FX", master_audio_fx->get_focus()->getName());
        master_audio_fx->get_focus()->draw(screen);
    });

    addDrawHandler({SCREENS::TRACK_EFFECTS_MIDI}, [this](GFXcanvas1 * screen) -> void {
        auto rack = getCurrentTrackMIDIFXRack();
        draw_fx_screen_header(screen, rack->get_focus_index(), rack->get_size(),
                              "TRACK MIDI FX", rack->get_focus()->getName());

        rack->get_focus()->draw(screen);
    });

    addDrawHandler({SCREENS::TRACK_EFFECTS_AUDIO}, [this](GFXcanvas1 * screen) -> void {
        auto rack = getCurrentTrackAudioFXRack();
        draw_fx_screen_header(screen, rack->get_focus_index(), rack->get_size(),
                              "TRACK AUDIO FX", rack->get_focus()->getName());

        rack->get_focus()->draw(screen);
    });

    addDrawHandler({SCREENS::PROJECT}, [this](GFXcanvas1 * screen) -> void {
        for (int i = 0; i < 18; i ++) screen->leds[i] = 0;

        draw_fx_screen_header(screen, -1, -1, "PROJECT SETTINGS", "projectname");

        screen->setFont(&Picopixel);
        screen->setTextSize(1);

        screen->setCursor(4, 16);
        screen->setTextSize(1);

        std::ostringstream oss;
        oss << std::setprecision(4) << sync.getBPM();
        std::string s = "BPM=" + oss.str();
        screen->print(s.c_str());

        screen->setCursor(4, 24);
        if (line_in) screen->print("IN: LINE");
        else screen->print("IN: MIC");
    });
}