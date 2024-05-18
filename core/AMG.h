//
// Created by ferluht on 21/07/2022.
//

#pragma once

#include "A.h"
#include "M.h"
#include "G.h"
#include <tinyxml2.h>
#include <cstring>
#include <unordered_map>
#include <typeinfo>
#include <mutex>
#include <math.h>

extern float GLOBAL_SPEED;

class Parameter {

    float step;
    float min;
    float max;
    std::vector<std::string> values;

public:

    bool continuous;
    std::string name;
    float value;

    Parameter() : Parameter("", 0) {};

    Parameter(std::string name_, float value_) {
        value = value_;
        name = name_;
        max = 1;
        min = 0;
        step = 0.01;
        continuous = true;
    }

    Parameter(std::string name_, float min_, float max_, float value_, float step_) :
            Parameter(name_, value_) {
        continuous = true;
        step = step_;
        min = min_;
        max = max_;
    }

    Parameter(std::string name_, std::vector<std::string> values_, int default_value) :
            Parameter(name_, default_value) {
        continuous = false;
        values = values_;
        min = 0;
        max = values.size() - 1;
        step = 1;
    }

    void update(uint8_t v) {
        float inc = v > 64 ? 4*step : -4*step;
        if (value + inc > max) value = max;
        else if (value + inc < min) value = min;
        else value += inc;
    }

    inline float getVal() {
        return value;
    }

    inline float getVal0to1() {
        return (value - min) / (max - min);
    }

    inline std::string getStringVal() {
        if (!continuous) return values[(int)value];
        return "default";
    }
};

class AudioEffect;
class MIDIEffect;
class Instrument;

class DeviceFactory {

public:

    enum DEVICE_TYPE {
        MIDI_FX,
        INSTRUMENT,
        AUDIO_FX
    };

    using create_f = DeviceFactory*();

    static void registerDevice(std::string const & name, create_f * fp, DEVICE_TYPE dtype)
    {
        registry()[dtype][name] = fp;
    }

    static DeviceFactory* instantiate(DEVICE_TYPE dtype, std::string const & name)
    {
        auto it1 = registry().find(dtype);
        if (it1 == registry().end()) return nullptr;
        auto it2 = it1->second.find(name);
        return it2 == it1->second.end() ? nullptr : (it2->second)();
    }

    template <typename D>
    struct AddToRegistry
    {
        explicit AddToRegistry(std::string name)
        {
            D * d = new D;
            if (dynamic_cast<Instrument*>(d) != nullptr) {
                DeviceFactory::registerDevice(name, &D::create, DEVICE_TYPE::INSTRUMENT);
                return;
            }
            if (dynamic_cast<AudioEffect*>(d) != nullptr) {
                DeviceFactory::registerDevice(name, &D::create, DEVICE_TYPE::AUDIO_FX);
                return;
            }
            if (dynamic_cast<MIDIEffect*>(d) != nullptr) {
                DeviceFactory::registerDevice(name, &D::create, DEVICE_TYPE::MIDI_FX);
                return;
            }
        }
        // make non-copyable, etc.
    };

    static std::vector<std::string> getDeviceList(DEVICE_TYPE dtype) {
        std::vector<std::string> list;

        auto it = registry().find(dtype);
        if (it == registry().end()) return list;
        auto devices_list = it->second;

        for (const auto &d : devices_list) {
            list.push_back(d.first);
        }

        return list;
    }


    static std::unordered_map<DEVICE_TYPE, std::unordered_map<std::string, create_f *>> & registry();
private:
//    static std::unordered_map<DEVICE_TYPE, std::unordered_map<std::string, create_f *>> & registry();
};

/**
 * AMG stands for Audio+MIDI+Graphics. The base class for all audio+midi+graphic objects.
*/
class AMG : public A, public M, public G, public DeviceFactory{
    std::string classname;
public:

    AMG() {
        classname = "UNDEFINED";
    }

    AMG(const char * classname_) : A(), M(), G() {
        classname = classname_;
    }

    virtual const char * getName() {
        return classname.c_str();
    }

    inline const char * getClassName() {
        return classname.c_str();
    }

    virtual void save(tinyxml2::XMLDocument * xmlDoc, tinyxml2::XMLElement * state) {};
    virtual void load(tinyxml2::XMLElement * state) {};
};

class DeviceWithParameters : public AMG {
    std::vector<Parameter *> inputparams;
    int parampage = 0;

public:

    DeviceWithParameters(const char * name_);

    Parameter * addParameter(std::string name) {
        return addParameter(name, 0);
    }

    Parameter * addParameter(std::string name, float default_value) {
        Parameter * p = new Parameter(name, default_value);
        inputparams.push_back(p);
        return p;
    }

    Parameter * addParameter(std::string name, float min, float max, float default_value, float step) {
        Parameter * p = new Parameter(name, min, max, default_value, step);
        inputparams.push_back(p);
        return p;
    }

    Parameter * addParameter(std::string name, std::vector<std::string> values, int default_value) {
        Parameter * p = new Parameter(name, values, default_value);
        inputparams.push_back(p);
        return p;
    }
};

class Effect : public DeviceWithParameters {
public:
    explicit Effect(char const * name) : DeviceWithParameters(name) {}
};

class AudioEffect : public Effect {
public:
    explicit AudioEffect(char const * name) : Effect(name) {}
};

class MIDIEffect : public Effect {
public:

    Parameter * ison;

    explicit MIDIEffect(char const * name) : Effect(name) {
        ison = addParameter("ENBL", {"FALSE", "TRUE"}, 1);
    }

    inline void enable(bool enabled) {
        ison->value = enabled;
    }
};








class VoiceState {

    unsigned char key;
    bool active;

public:

    VoiceState() {
        active = false;
    };

    inline void enable() { active = true; }
    inline void disable() { active = false; }
    inline bool isActive() { return active; }

    template <class VoiceState> friend class PolyInstrument;
};

class Instrument : public DeviceWithParameters {
public:
    explicit Instrument(const char * name) : DeviceWithParameters(name) {}
};

template <class TVoiceState>
class PolyInstrument : public Instrument {

    unsigned int num_voices = 6;

    std::mutex keyPressedLock;

//    bool damper_pedal;
//    bool portamento;
//    bool sostenuto;
//    bool soft_pedal;
//    bool chorus;
//    bool celeste;
//    bool phaser;

    int _prev_active_voices = 0;

public:

    std::list<TVoiceState *> voices;

    float base_note = 57.0;
    float power_base = 2.0;
    float semitones = 12.0;
    float instrument_volume = 1;
    double base_frequency = 440.0;

    float pitch = 0;
    float pitch_distance = 2.0;

    bool clear_input = true;

    explicit PolyInstrument(const char * name) : Instrument(name){
        pitch = 0;
        for (int i = 0; i < num_voices; i++) voices.push_back(new TVoiceState());

        addMIDIHandler({}, {MIDI::GENERAL::NOTEON_HEADER, MIDI::GENERAL::NOTEOFF_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {
            keyPressed(cmd);
        });

        addMIDIHandler({}, {MIDI::GENERAL::PITCHWHEEL_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {
            pitch = ((float)((cmd.data2 << 7) + cmd.data1) / 16384.f - 0.5) * 2 * pitch_distance;
        });
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) override;

    void keyPressed(MData& md);

    inline float getFrequency(float note)
    {
        return GLOBAL_SPEED*base_frequency*(powf(power_base, (note - base_note + pitch)/semitones));
    }

    inline float getPhaseIncrement(float note)
    {
        return 2*M_PI*getFrequency(note) / SAMPLERATE;
    }

    void set_voices(uint8_t nvoices) {
        while (voices.size() > nvoices) voices.pop_front();
        while (voices.size() < nvoices) voices.push_back(new TVoiceState());
    }

    virtual void updateVoice(TVoiceState * voiceState, MData &cmd) {};

    virtual void processVoice(TVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                              unsigned int nBufferFrames, Sync & sync, uint8_t nvoices) {};

};

template <class TVoiceState>
void PolyInstrument<TVoiceState>::keyPressed(MData &md) {
    keyPressedLock.lock();

    for (auto it = voices.begin(); it != voices.end(); it++ )
    {
        if ((*it)->key == md.data1){
            TVoiceState * state = (*it);
            voices.erase(it);
            updateVoice(state, md);
            state->key = md.data1;
            voices.push_back(state);
            keyPressedLock.unlock();
            return;
        }
    }

    for (auto it = voices.begin(); it != voices.end(); it++ )
    {
        if (!(*it)->isActive()){
            TVoiceState * state = (*it);
            voices.erase(it);
            updateVoice(state, md);
            state->key = md.data1;
            voices.push_back(state);
            keyPressedLock.unlock();
            return;
        }
    }

    if (voices.empty()) {
        updateVoice(nullptr, md);
    } else {
        TVoiceState *state = *voices.begin();
        updateVoice(state, md);
        state->key = md.data1;
        voices.pop_front();
        voices.push_back(state);
    }

    keyPressedLock.unlock();
}

template <class TVoiceState>
void PolyInstrument<TVoiceState>::process(float *outputBuffer, float * inputBuffer,
                                          unsigned int nBufferFrames, Sync & sync)
{
    keyPressedLock.lock();

    if (clear_input) {
        float j = -1;
        for (unsigned int i = 0; i < 2 * nBufferFrames; i += 2) {
            inputBuffer[i + 0] = 0.0001f * j;
            inputBuffer[i + 1] = 0.0001f * j;
            j *= -1;
        }
    }

    if (voices.empty()) {
        processVoice(nullptr, outputBuffer, inputBuffer, nBufferFrames, sync, num_voices);
    } else {
        for (auto it = voices.begin(); it != voices.end(); it++) {
            if (PERF_TESTING || (*it)->isActive())
                processVoice(*it, outputBuffer, inputBuffer, nBufferFrames, sync, num_voices);
        }
    }

    keyPressedLock.unlock();
}