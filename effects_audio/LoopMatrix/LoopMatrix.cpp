//
// Created by ferluht on 03/12/2022.
//

#include <LoopMatrix/LoopMatrix.h>

LoopMatrix::LoopMatrix() : AudioEffect("TAPES") {
    focus_tape = 0;
    copy_from = nullptr;
    sync = new Sync();

    for (int i = 0; i < m*n; i ++)
        scene_copying_status[i] = false;

    master_effects = new Rack("AUDIO FX", Rack::SEQUENTIAL);
    master_effects->add(new Delay());
//    master_effects->add(new Plateau());

    for (int i = 0; i < m*n; i ++) {
        sends[i][0].value = 0.0;
        sends[i][1].value = 0.0;
    }

    for (int i = 0; i < m*n; i ++)
        tapes[i] = new Tape(sync);

    for (int i = 0; i < m * n; i++)
        saved[i] = false;

    addMIDIHandler(0, SCREENS::MAX_SCREENS - 1,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + m * n,
                   MIDI::UI::TAPE::START, MIDI::UI::TAPE::STOP,
                   [this](MData &cmd, Sync &sync) -> MIDISTATUS {
                       copy_from = nullptr;
                       focus_tape = cmd.status - MIDI::GENERAL::LOOP_HEADER;
                       if (current_screen == 0) {
                           cmd.status = MIDI::GENERAL::LOOP_HEADER;
                           return tapes[focus_tape]->midiIn(cmd, sync);
                       } else {
                           return MIDISTATUS::DONE;
                       }
                   });

    addMIDIHandler(0, SCREENS::MAX_SCREENS - 1,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + m * n,
                   MIDI::UI::LOOPMATRIX::COPY, MIDI::UI::LOOPMATRIX::COPY,
                   [this](MData &cmd, Sync &sync) -> MIDISTATUS {
                       if (cmd.data2 == 0) return MIDISTATUS::DONE;
                       focus_tape = cmd.status - MIDI::GENERAL::LOOP_HEADER;
                       cmd.status = MIDI::GENERAL::LOOP_HEADER;
                       if (copy_from) {
                           auto copy_to = tapes[focus_tape];
                           auto ret = copy_from->copy(copy_to, 0);
                           if (ret == MIDISTATUS::DONE) copy_from = nullptr;
                           return ret;
                       }
                       copy_from = tapes[focus_tape];
                       return MIDISTATUS::DONE;
                   });

    addMIDIHandler(0, SCREENS::MAX_SCREENS - 1,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + m * n,
                   MIDI::UI::LOOPMATRIX::SELECT_SCENE, MIDI::UI::LOOPMATRIX::SELECT_SCENE,
                   [this](MData &cmd, Sync &sync) -> MIDISTATUS {
                       int scene_idx = cmd.status - MIDI::GENERAL::LOOP_HEADER;
                       for (int i = 0; i < m * n; i ++) tapes[i]->select_scene(scene_idx);
                       return MIDISTATUS::DONE;
                   });

    addMIDIHandler(0, SCREENS::MAX_SCREENS - 1,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + m * n,
                   MIDI::UI::LOOPMATRIX::COPY_SCENE, MIDI::UI::LOOPMATRIX::COPY_SCENE,
                   [this](MData &cmd, Sync &sync) -> MIDISTATUS {
                       int scene_idx = cmd.status - MIDI::GENERAL::LOOP_HEADER;
                       for (int i = 0; i < m * n; i ++) {
                           if (scene_copying_status[i] == false) {
                               auto ret = tapes[i]->copy(tapes[i], scene_idx);
                               if (ret == MIDISTATUS::DONE) scene_copying_status[i] = true;
                               break;
                           }
                       }
                       for (int i = 0; i < m * n; i ++)
                           if (scene_copying_status[i] == false) return MIDISTATUS::WAITING;
                       for (int i = 0; i < m * n; i ++) {
                           scene_copying_status[i] = false;
                           tapes[i]->copy_status(scene_idx);
                       }
                       return MIDISTATUS::DONE;
                   });

    addMIDIHandler({}, {MIDI::GENERAL::CC_HEADER}, {CC_E1, CC_E2}, [this](MData &cmd, Sync &sync) -> MIDISTATUS {
        if (current_screen == 1) {
            if (cmd.data1 == CC_E1) sends[focus_tape][0].update(cmd.data2);
            if (cmd.data1 == CC_E2) sends[focus_tape][1].update(cmd.data2);
        }
        return MIDISTATUS::DONE;
    });

    addDrawHandler({SCREENS::LOOP_VIEW}, [this](GFXcanvas1 * screen) -> void {
        for (int i = 0; i < 4; i ++) {
            screen->drawRect(111, 11 + 5 * i, 15, 4, 1);
            float width = tapes[i]->getPosition() * 15;
            if (width > 1)
                screen->drawRect(111, 11 + 5 * i + 1, width, 2, 1);

            int max_br = 50;
            switch (tapes[i]->getState()) {
                case (Tape::TAPE_STATE::REC):
                    screen->setLed(i + 2, max_br, 0, 0);
                case (Tape::TAPE_STATE::OVERDUB):
//                    if (sync->isWaiting(tapes[i]) && draw_counter > 5) break;
                    screen->drawCircle(103, 11 + 5 * i + 1, 1, 1);
                    if (tapes[i]->getState() == Tape::TAPE_STATE::OVERDUB)
                        screen->setLed(i + 2, 0, 0, tapes[i]->getAmp() * max_br);
                    break;
                case (Tape::TAPE_STATE::STOP):
//                    if (sync->isWaiting(tapes[i]) && draw_counter > 5) break;
                    screen->drawRect(98, 11 + 5 * i, 3, 3,1);
                    screen->setLed(i + 2, 0, 0, 0);
                    break;
                case (Tape::TAPE_STATE::OVERWRITE):
//                    if (sync->isWaiting(tapes[i]) && draw_counter > 5) break;
                    screen->drawFastHLine(102, 11 + 5 * i + 1, 3, 1);
                    screen->setLed(i + 2, tapes[i]->getAmp() * max_br, 0, 0);
                    break;
                case (Tape::TAPE_STATE::PLAY):
//                    if (sync->isWaiting(tapes[i]) && draw_counter > 5) break;
                    screen->drawTriangle(106, 11 + 5 * i, 106, 11 + 5 * i + 2, 109, 11 + 5 * i + 1,1);
                    screen->setLed(i + 2, 0, tapes[i]->getAmp() * max_br, 0);
                    break;
                default:
                    break;
            }
        }
        draw_counter = (draw_counter + 1) % 10;
    });

    addDrawHandler({SCREENS::MASTER_EFFECTS_AUDIO}, [this](GFXcanvas1 * screen) -> void {
        for (int i = 0; i < m; i ++) {
            for (int j = 0; j < n; j ++) {
                screen->drawCircle(105 + i * 13, 15 + j * 11, 3, 1);
                if (i + j*m == focus_tape) screen->drawCircle(105 + i * 13, 15 + j * 11, 1, 1);
            }
        }
        draw_counter = (draw_counter + 1) % 10;

        for (int i = 0; i < 2; i ++) {
            int xoffset = (int)(i / 2) * 46;
            int yoffset = (i % 2) * 10;
            screen->setCursor(4 + xoffset, 17 + yoffset);
            screen->setTextSize(1);
            screen->print(master_effects->get_item(i)->getName());

            screen->setCursor(36 + xoffset, 17 + yoffset);
            screen->setTextSize(1);
            screen->print("->");

            screen->drawRect(48 + xoffset, 14 + yoffset, 20, 4, 1);
            screen->drawRect(48 + xoffset, 15 + yoffset, sends[focus_tape][i].value * 19 + 1, 2, 1);
        }
    });
}

void LoopMatrix::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {
    float buf[m*n][BUF_SIZE * 2];
    float emptybuf[BUF_SIZE * 2];

    for (unsigned int i = 0; i < 2*nBufferFrames; i ++)
        emptybuf[i] = 0;

    for (int j = 0; j < m * n; j++) {
        if (j == focus_tape)
            tapes[j]->process(buf[j], inputBuffer, nBufferFrames, 0);
        else
            tapes[j]->process(buf[j], emptybuf, nBufferFrames, 0);
    }

    for (unsigned int k=0; k<2*nBufferFrames; k++)
        outputBuffer[k] = 0;

    for (int e = 0; e < master_effects->get_size(); e ++) {
        for (int i = 0; i < m * n; i++)
            for (unsigned int k = 0; k < 2 * nBufferFrames; k++) {
                emptybuf[k] += buf[i][k] * sends[i][e].value;
                outputBuffer[k] += buf[i][k] * (1 - sends[i][e].value);
            }

        master_effects->get_item(e)->process(emptybuf, emptybuf, nBufferFrames, 0);

        for (unsigned int k = 0; k < 2 * nBufferFrames; k++) {
            outputBuffer[k] += emptybuf[k];
            emptybuf[k] = 0;
        }
    }

    for (unsigned int k = 0; k < 2 * nBufferFrames; k++)
        outputBuffer[k] /= master_effects->get_size();
}

bool LoopMatrix::save() {
    bool ret = true;

    if (saveiterations == 0) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
        auto timestr = oss.str();

        if (saveiterations == 0)
            savedir = "../res/saved/" + timestr;

        mkdir(savedir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    for (int i = 0; i < m * n; i++) {
        if (!saved[i]) saved[i] = tapes[i]->save(savedir + "/" + std::to_string(i) + ".wav");
        ret = ret && saved[i];
    }
    if (ret) for (int i = 0; i < m * n; i++)
            saved[i] = false;

    if (ret) saveiterations = 0;
    else saveiterations ++;

    return ret;
}