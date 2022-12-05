//
// Created by ferluht on 03/12/2022.
//

#ifndef RPIDAW_LOOPMATRIX_H
#define RPIDAW_LOOPMATRIX_H

#include <Effect.h>
#include <Tape/Tape.h>

template <uint8_t m, uint8_t n>
class LoopMatrix : public AudioEffect {

    Tape * tapes[m*n];
    int focus_tape;
    Tape * copy_from;
    Sync * sync;
    int draw_counter = 0;

public:

    LoopMatrix() : AudioEffect("TAPES") {
        focus_tape = 0;
        copy_from = nullptr;
        sync = new Sync();

        for (int i = 0; i < m*n; i ++)
                tapes[i] = new Tape(sync);

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::GENERAL::CC_HEADER + m * n,
                       MIDI::UI::TAPE::START, MIDI::UI::TAPE::STOP,
                       [this](MData &cmd) -> MIDISTATUS {
                           copy_from = nullptr;
                           focus_tape = cmd.status - MIDI::GENERAL::CC_HEADER;
                           cmd.status = MIDI::GENERAL::CC_HEADER;
                           return tapes[focus_tape]->midiIn(cmd);
                       });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::GENERAL::CC_HEADER + m * n,
                       MIDI::UI::LOOPMATRIX::COPY, MIDI::UI::LOOPMATRIX::COPY,
                       [this](MData &cmd) -> MIDISTATUS {
                           if (cmd.data2 == 0) return MIDISTATUS::DONE;
                           focus_tape = cmd.status - MIDI::GENERAL::CC_HEADER;
                           cmd.status = MIDI::GENERAL::CC_HEADER;
                           if (copy_from) {
                               auto copy_to = tapes[focus_tape];
                               auto ret = copy_from->copy(copy_to);
                               if (ret == MIDISTATUS::DONE) copy_from = nullptr;
                               return ret;
                           }
                           copy_from = tapes[focus_tape];
                           return MIDISTATUS::DONE;
                       });
    }

    void process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) override {
        float buf[m*n][BUF_SIZE * 2];
        float emptybuf[BUF_SIZE * 2];

        for (unsigned int i = 0; i < 2*nBufferFrames; i ++)
            emptybuf[i] = 0;

        for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
            for (int j = 0; j < m * n; j++) {
                if (j == focus_tape)
                    tapes[j]->process(&buf[j][i], &inputBuffer[i], 1, 0);
                else
                    tapes[j]->process(&buf[j][i], &emptybuf[i], 1, 0);
            }
            sync->process();
        }

        for (unsigned int k=0; k<2*nBufferFrames; k++)
            outputBuffer[k] = 0;

        for (int i = 0; i < m*n; i++)
            for (unsigned int k=0; k<2*nBufferFrames; k++)
                outputBuffer[k] += buf[i][k];
    }

    void draw(GFXcanvas1 * screen) override {
        for (int i = 0; i < 4; i ++) {
            screen->drawRect(111, 11 + 5 * i, 15, 4, 1);
            float width = tapes[i]->getPosition() * 15;
            if (width > 1)
                screen->drawRect(111, 11 + 5 * i + 1, width, 2, 1);

            switch (tapes[i]->getState()) {
                case (Tape::TAPE_STATE::REC):
                case (Tape::TAPE_STATE::OVERDUB):
                    if (sync->isWaiting(tapes[i]) && draw_counter > 5) break;
                    screen->drawCircle(103, 11 + 5 * i + 1, 1, 1);
                    break;
                case (Tape::TAPE_STATE::STOP):
                    if (sync->isWaiting(tapes[i]) && draw_counter > 5) break;
                    screen->drawRect(98, 11 + 5 * i, 3, 3,1);
                    break;
                case (Tape::TAPE_STATE::PLAY):
                    if (sync->isWaiting(tapes[i]) && draw_counter > 5) break;
                    screen->drawTriangle(106, 11 + 5 * i, 106, 11 + 5 * i + 2, 109, 11 + 5 * i + 1,1);
                    break;
                default:
                    break;
            }
        }
        draw_counter = (draw_counter + 1) % 10;
    }
};


#endif //RPIDAW_LOOPMATRIX_H
