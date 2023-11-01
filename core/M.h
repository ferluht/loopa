//
// Created by ferluht on 08/07/2022.
//

#pragma once

#include <stdlib.h>
#include <vector>
#include <queue>
#include <cmath>
#include <list>
#include <string>
#include <map>
#include <functional>
#include <algorithm>

#define CC_E1 105
#define CC_E2 106

#define E1_B 107
#define E2_B 108

enum MIDISTATUS {
    WAITING,
    DONE
};

namespace MIDI {

    namespace GENERAL {
        const uint8_t NOTEOFF_HEADER = 128;
        const uint8_t NOTEON_HEADER = 144;
        const uint8_t POLYPRESSURE_HEADER = 160;
        const uint8_t CC_HEADER = 176;
        const uint8_t PITCHWHEEL_HEADER = 224;
    }

    namespace UI {

        namespace DAW {
            enum {
                START = 0,
                LEFT,
                RIGHT,
                UP,
                DOWN,
                UPUP,
                DOWNDOWN,
                SAVE,
                REC,
                END = 10
            };
        }

        namespace TAPE {
            enum {
                START = 10,
                TRIG,
                CLEAR,
                DOUBLE,
                STOP,
                END = 20
            };
        }

        namespace LOOPMATRIX {
            enum {
                START = 20,
                COPY,
                COPY_SCENE,
                SELECT_SCENE,
                END = 30
            };
        }

        namespace SCREEN {
            enum {
                START = 30,
                LOOP_SCREEN,
                ALT_PARAMS,
                END = 40
            };
        }
    }

}

struct MData {
    double beat;
    unsigned char status;
    unsigned char data1;
    unsigned char data2;
};

/**
 * M stands for MIDI. The base class for all MIDI objects.
*/
class M {

    std::map<uint8_t, std::map<uint8_t, int>> handlers;
    std::vector<std::function<MIDISTATUS(MData&)>> handlers_array;

public:

    M() {

    }

    /**
     * Add MIDI handler for specific header
     *
     * @param header status (first) byte of midi command
     * @param handler callback to call when matching midi header is received
     */
    void addMIDIHandler(uint8_t header, std::function<MIDISTATUS(MData&)> handler) {
        addMIDIHandler(header, 0, 255, handler);
    }

    /**
     * Add MIDI handler for set of headers
     *
     * @param headers set of midi headers
     * @param handler callback to call when matching midi header is received
     */
    void addMIDIHandler(std::vector<uint8_t> headers, std::function<MIDISTATUS(MData&)> handler) {
        addMIDIHandler(headers, 0, 255, handler);
    }

    /**
     * Add MIDI handler for set of headers
     *
     * @param headers set of midi headers
     * @param handler callback to call when matching midi header is received
     */
    void addMIDIHandler(uint8_t header, uint8_t code_start, uint8_t code_end, std::function<MIDISTATUS(MData&)> handler) {
        std::vector<uint8_t> headers;
        headers.push_back(header);
        addMIDIHandler(headers, code_start, code_end, handler);
    }

    /**
     * Add MIDI handler for set of headers and range of codes [code_start .. code_end]
     *
     * @param headers set of midi headers
     * @param code_start first midi code to include in handler
     * @param code_end last midi code to include in handler
     * @param handler callback to call when matching midi header and code are received
     */
    void addMIDIHandler(std::vector<uint8_t> headers, uint8_t code_start, uint8_t code_end, std::function<MIDISTATUS(MData&)> handler) {
        std::vector<uint8_t> codes;
        for (uint8_t c = code_start; c < code_end; c ++)
            codes.push_back(c);
        codes.push_back(code_end);
        addMIDIHandler(headers, codes, handler);
    }

    /**
     * Add MIDI handler for range of headers [header_start .. header_end] and range of codes [code_start .. code_end]
     *
     * @param header_start first midi header to include in handler
     * @param header_end last midi header to include in handler
     * @param code_start first midi code to include in handler
     * @param code_end last midi code to include in handler
     * @param handler callback to call when matching midi header and code are received
     */
    void addMIDIHandler(uint8_t header_start, uint8_t header_end,
                        uint8_t code_start, uint8_t code_end, std::function<MIDISTATUS(MData&)> handler) {
        std::vector<uint8_t> headers;
        for (uint8_t h = header_start; h < header_end; h ++)
            headers.push_back(h);
        headers.push_back(header_end);

        std::vector<uint8_t> codes;
        for (uint8_t c = code_start; c < code_end; c ++)
            codes.push_back(c);
        codes.push_back(code_end);
        addMIDIHandler(headers, codes, handler);
    }

    /**
     * Add MIDI handler for set of headers and set of codes
     *
     * @param headers set of midi headers
     * @param codes set of midi codes
     * @param handler callback to call when matching midi header and code are received
     */
    void addMIDIHandler(std::vector<uint8_t> headers, std::vector<uint8_t> codes, std::function<MIDISTATUS(MData&)> handler) {
        handlers_array.push_back(handler);
        int handler_idx = handlers_array.size() - 1;
        for (auto ith = headers.begin(); ith < headers.end(); ith ++)
            for (auto itc = codes.begin(); itc < codes.end(); itc ++)
                handlers[*ith][*itc] = handler_idx;
    }

    /**
     * Add MIDI handler for just one header and one code
     *
     * @param header header
     * @param code code
     * @param handler callback to call when matching midi header and code are received
     */
    void addMIDIHandler(uint8_t header, uint8_t code, std::function<MIDISTATUS(MData&)> handler) {
        handlers_array.push_back(handler);
        int handler_idx = handlers_array.size() - 1;
        handlers[header][code] = handler_idx;
    }

    /**
     * MIDI receiver method.
     * When received midi command calls matching callback if exists
     * If no callback exists just returns MIDISTATUS::DONE
     * @param cmd midi command
     */
    virtual MIDISTATUS midiIn(MData& cmd) {
        auto x = handlers.find(cmd.status);
        if (x == handlers.end()) return MIDISTATUS::DONE;
        auto xy = x->second.find(cmd.data1);
        if (xy == x->second.end()) return MIDISTATUS::DONE;
        int idx = xy->second;
        return handlers_array[idx](cmd);
    }
};

/**
 * Hardware control class. Used in MIDIMap for mapping hardware knobs to virtual midi commands.
 * @private
*/
class HardwareControl {

public:

    /**
     * HardwareControl constructor
     *
     * @param label knob label
     * @param header first byte of assigned midi command
     * @param code second byte of assigned midi command
     */
    HardwareControl(std::string label, uint8_t header, uint8_t code) {
        value = 0;
        this->header = header;
        this->code = code;
        this->label = label;
    }

private:
    uint8_t value;
    uint8_t header;
    uint8_t code;
    std::string label;
    std::vector<std::pair<std::vector<std::pair<uint8_t, uint8_t>>, std::pair<uint8_t, uint8_t>>> ctrl_seqs;

    void midiIn(MData &cmd) {
        value = cmd.data2;
    }

    inline bool isPressed() {
        return value > 0;
    }

    inline uint8_t getValue() {
        return value;
    }

    inline void addCtrlSequence(std::vector<std::pair<uint8_t, uint8_t>> ctrl_seq, uint8_t mheader, uint8_t mcode) {
        ctrl_seqs.push_back({ctrl_seq, {mheader, mcode}});

        std::sort(ctrl_seqs.begin(), ctrl_seqs.end(),
                  [](std::pair<std::vector<std::pair<unsigned char, unsigned char>>, std::pair<unsigned char, unsigned char>> a,
                           std::pair<std::vector<std::pair<unsigned char, unsigned char>>, std::pair<unsigned char, unsigned char>> b) -> bool
             {
                 return a.first.size() > b.first.size();
             });
    }

    friend class MIDIMap;
};

/**
 * Middleware midi handler that maps hardware knobs to virtual midi commands.
 *
 * Example of usage:
 * @code
 * MIDIMap * midiMap = new MIDIMap();
 * midiMap->addHardwareControl("SHIFT", CC_HEADER, 101);
 * midiMap->addHardwareControl("K1", CC_HEADER, 110);
 * midiMap->addMapping({"K1"}, CC_HEADER + 5, 102);
 * midiMap->addMapping({"SHIFT", "K1"}, CC_HEADER, 150);
 *
 * ...
 *
 * MIDIMap->midiIn(cmd);
 * @endcode
 *
 * When pressed SHIFT, midiIn will not change cmd
 *
 * When pressed K1 while holding SHIFT cmd.status will change to CC_HEADER and cmd.data1 will change to 150
*/
class MIDIMap : public M {

    std::map<uint8_t, std::map<uint8_t, HardwareControl *>> imap;
    std::map<std::string, HardwareControl*> knobs_by_names;

public:
    MIDIMap() : M() {

    }

    /**
     * Add hardware knob
     *
     * @param label knob label
     * @param header first byte of assigned midi command
     * @param code second byte of assigned midi command
     *
     * Usage example:
     * @code
     * addHardwareControl("K1", CC_HEADER, 60);
     * @endcode
     */
    void addHardwareControl(std::string label, uint8_t header, uint8_t code) {
        HardwareControl * ctrl = new HardwareControl(label, header, code);
        imap[ctrl->header][ctrl->code] = ctrl;
        knobs_by_names[ctrl->label] = ctrl;
    }

    /**
     * Add mapping of control sequence of knobs to virtual midi command (preserving last byte of midi command)
     *
     * @param ctrl_seq sequence of hardware knob names which triggers midi command
     * @param mheader first byte of assigned midi command
     * @param mcode second byte of assigned midi command
     *
     * Usage example:
     * @code
     * addMapping({"K1", "K2"}, CC_HEADER, 100);
     * @endcode
     */
    void addMapping(std::vector<std::string> ctrl_seq, uint8_t mheader, uint8_t mcode) {
        std::vector<std::pair<uint8_t, uint8_t>> ctrl_seq_num;
        for (auto it = ctrl_seq.begin(); it < ctrl_seq.end() - 1; it ++) {
            HardwareControl * ctrl = knobs_by_names.find(*it)->second;
            ctrl_seq_num.emplace_back(ctrl->header, ctrl->code);
        }
        HardwareControl * ctrl = knobs_by_names.find(*(ctrl_seq.end()-1))->second;
        ctrl->addCtrlSequence(ctrl_seq_num, mheader, mcode);
    }

    /**
     * @private
     */
    MIDISTATUS midiIn(MData& cmd) override {
        auto x = imap.find(cmd.status);
        if (x == imap.end()) return MIDISTATUS::DONE;
        auto xy = x->second.find(cmd.data1);
        if (xy == x->second.end()) return MIDISTATUS::DONE;
        HardwareControl * ctrl = xy->second;
        if (ctrl) {
            ctrl->midiIn(cmd);
            for (auto it = ctrl->ctrl_seqs.begin(); it < ctrl->ctrl_seqs.end(); it ++) {
                bool flag = true;
                for (auto xy = it->first.begin(); xy < it->first.end(); xy ++) {
                    HardwareControl * c = imap.find(xy->first)->second.find(xy->second)->second;
                    if (!c->isPressed())
                        flag = false;
                }
                if (flag) {
                    cmd.status = it->second.first;
                    cmd.data1 = it->second.second;
                    break;
                }
            }
        }
        return MIDISTATUS::DONE;
    }
};
