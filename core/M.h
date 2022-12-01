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
#include <algorithm>

#define NOTEOFF_HEADER 128
#define NOTEON_HEADER 144
#define POLYPRESSURE_HEADER 160
#define CC_HEADER 176
#define PITCHWHEEL_HEADER 224
#define DAW_CTRL_HEADER 100


#define CC_MODWHEEL 1
#define CC_BREATH_CONTROLLER 2
#define CC_AFTERTOUCH 3
#define CC_FOOT_CONTROLLER 4
#define CC_PORTAMENTO_TIME 5
#define CC_DATA_ENTRY 6
#define CC_MAIN_VOLUME 7
#define CC_DAMPER_PEDAL 64
#define CC_PORTAMENTO 65
#define CC_SOSTENUTO 66
#define CC_SOFT_PEDAL 67
#define CC_CHORUS 93

#define CC_E1 105
#define CC_E2 106

enum MIDISTATUS {
    WAITING,
    DONE
};

enum MIDICC {
    NONE,
    DAW_ = 40,
    DAW_ALT1,
    DAW_ALT2,
    DAW_LEFT,
    DAW_RIGHT,
    DAW_UP,
    DAW_DOWN,
    DAW_END,
    TAPE = 50,
    TAPE_TRIG,
    TAPE_CLEAR,
    TAPE_DOUBLE,
    TAPE_STOP,
    TAPE_END
};

struct MData {
    double beat;
    unsigned char status;
    unsigned char data1;
    unsigned char data2;
};

class M {

    std::map<uint8_t, std::map<uint8_t, int>> handlers;
    std::vector<std::function<MIDISTATUS(MData&)>> handlers_array;

public:

    M() {

    }

    void addHandler(uint8_t header, std::function<MIDISTATUS(MData&)> handler) {
        addHandler(header, 0, 255, handler);
    }

    void addHandler(std::vector<uint8_t> &headers, std::function<MIDISTATUS(MData&)> handler) {
        addHandler(headers, 0, 255, handler);
    }

    void addHandler(uint8_t header, uint8_t code_start, uint8_t code_end, std::function<MIDISTATUS(MData&)> handler) {
        std::vector<uint8_t> headers;
        headers.push_back(header);
        addHandler(headers, code_start, code_end, handler);
    }

    void addHandler(std::vector<uint8_t> &headers, uint8_t code_start, uint8_t code_end, std::function<MIDISTATUS(MData&)> handler) {
        std::vector<uint8_t> codes;
        for (uint8_t c = code_start; c < code_end; c ++)
            codes.push_back(c);
        codes.push_back(code_end);
        addHandler(headers, codes, handler);
    }

    void addHandler(std::vector<uint8_t> &headers, std::vector<uint8_t> &codes, std::function<MIDISTATUS(MData&)> handler) {
        handlers_array.push_back(handler);
        int handler_idx = handlers_array.size() - 1;
        for (auto ith = headers.begin(); ith < headers.end(); ith ++)
            for (auto itc = codes.begin(); itc < codes.end(); itc ++)
                handlers[*ith][*itc] = handler_idx;
    }

    void addHandler(uint8_t header, uint8_t code, std::function<MIDISTATUS(MData&)> handler) {
        handlers_array.push_back(handler);
        int handler_idx = handlers_array.size() - 1;
        handlers[header][code] = handler_idx;
    }

    virtual MIDISTATUS midiIn(MData& cmd) {
        auto x = handlers.find(cmd.status);
        if (x == handlers.end()) return MIDISTATUS::DONE;
        auto xy = x->second.find(cmd.data1);
        if (xy == x->second.end()) return MIDISTATUS::DONE;
        int idx = xy->second;
        return handlers_array[idx](cmd);
    }
};


class HardwareControl {
public:
    uint8_t header;
    uint8_t code;
    std::string name;
    std::vector<std::pair<std::vector<std::pair<uint8_t, uint8_t>>, std::pair<uint8_t, uint8_t>>> ctrl_seqs;

    HardwareControl(std::string name_, uint8_t header_, uint8_t code_) {
        value = 0;
        header = header_;
        code = code_;
        name = name_;
    }

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

private:
    uint8_t value;
};

class MIDIMap : public M {

    std::map<uint8_t, std::map<uint8_t, HardwareControl *>> imap;
    std::map<std::string, HardwareControl*> knobs_by_names;

public:
    MIDIMap() : M() {

    }

    void addHardwareControl(std::string name_, uint8_t header_, uint8_t code_) {
        HardwareControl * ctrl = new HardwareControl(name_, header_, code_);
        imap[ctrl->header][ctrl->code] = ctrl;
        knobs_by_names[ctrl->name] = ctrl;
    }

    void addMapping(std::vector<std::string> ctrl_seq, uint8_t mheader, uint8_t mcode) {
        std::vector<std::pair<uint8_t, uint8_t>> ctrl_seq_num;
        for (auto it = ctrl_seq.begin(); it < ctrl_seq.end() - 1; it ++) {
            HardwareControl * ctrl = knobs_by_names.find(*it)->second;
            ctrl_seq_num.emplace_back(ctrl->header, ctrl->code);
        }
        HardwareControl * ctrl = knobs_by_names.find(*(ctrl_seq.end()-1))->second;
        ctrl->addCtrlSequence(ctrl_seq_num, mheader, mcode);
    }

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
