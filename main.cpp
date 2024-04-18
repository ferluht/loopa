#include <rtaudio/RtAudio.h>
#include <rtmidi/RtMidi.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "Daw.h"
#include <CRC8.h>

#ifdef __arm__
#include <wiringSerial.h>
#else
#include <SerialPort.h>
#endif

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

DAW * daw;
GFXcanvas1 * screen = new GFXcanvas1(SCREEN_WIDTH, SCREEN_HEIGHT);
int ardport, echoport=-1;
uint8_t uartbuffer[256];
uint8_t uartit = 0;
bool debug_mode = false;

void init_serial() {
#ifdef __arm__
    if((ardport=serialOpen("/dev/ttyS0", 115200))<0){
        fprintf(stderr,"Unable to open serial device: %s\n",strerror(errno));
        return;
    }
    if((echoport=serialOpen("/dev/ttyGS0", 115200))<0){
        fprintf(stderr,"Unable to open serial device: %s\n",strerror(errno));
        return;
    }
#else
    if((ardport=openAndConfigureSerialPort("/dev/tty.usbmodem14401", 115200))<0){
        fprintf(stderr,"Unable to open serial device: %s\n",strerror(errno));
        return;
    }
#endif
}

void process_gui() {
#ifdef __arm__
    if (echoport > -1 && (serialDataAvail(echoport) > 0 || debug_mode)) {
        debug_mode = true;
        while (serialDataAvail(echoport) > 0) {
            const char c = serialGetchar(echoport);
            serialPutchar(ardport, c);
        }
    } else {
        for (int i = 0; i < 128 * 32 / 8; i++)
            serialPutchar(ardport, screen->getBuffer()[i]);

        for (unsigned char led: screen->leds)
            serialPutchar(ardport, led);

        uint8_t crc = getCrc(0, screen->getBuffer(), 128 * 32 / 8);
        crc = getCrc(crc, screen->leds, 18);
        serialPutchar(ardport, crc);
    }
#else
    for (int i = 6; i < 18; i ++) {
        if (screen->leds[i] < 245) screen->leds[i] += 1;
    }
    writeSerialData(screen->getBuffer(), 128 * 32 / 8);
    writeSerialData(screen->leds, 18);

    uint8_t crc = getCrc(0, screen->getBuffer(), 128 * 32 / 8);
    crc = getCrc(crc, screen->leds, 18);
    writeSerialData(&crc, 1);
#endif
}

void scan_buttons() {
#ifdef __arm__
    while (serialDataAvail(ardport) > 0) {
        uartbuffer[uartit] = serialGetchar(ardport);
        if (echoport > -1 && debug_mode) {
            serialPutchar(echoport, uartbuffer[uartit]);
        }
#else
    char buf;
    while (readSerialData(&buf, 1) > 0) {
        uartbuffer[uartit] = buf;
#endif
        if (uartbuffer[uartit] == '\n') {
            MData cmd;
            switch (uartbuffer[0]) {
                case MIDI::GENERAL::CC_HEADER:
                case MIDI::GENERAL::NOTEON_HEADER:
                case MIDI::GENERAL::NOTEOFF_HEADER:
                    cmd.status = uartbuffer[0];
                    cmd.data1 = uartbuffer[1];
                    cmd.data2 = uartbuffer[2];
                    if (!debug_mode) daw->midiIn(cmd);
                    break;
                default:
                    break;
            }
            uartit = 0;
        } else {
            uartit++;
        }
        if (uartit > 255) uartit = 0;
    }
//    flushSerialData();
}

void midiErrorCallback( RtMidiError::Type /*type*/, const std::string &errorText, void *) {
    std::cerr << "\nMidi Error: " << errorText << "\n\n";
}

void audioErrorCallback( RtAudioErrorType /*type*/, const std::string &errorText )
{
    std::cerr << "\nAudio Error: " << errorText << "\n\n";
}

void midicallback( double deltatime, std::vector< unsigned char > *message, void */*userData*/ )
{
    unsigned int nBytes = message->size();
    if (nBytes == 3) {
        MData cmd;
        cmd.status = message->at(0) & 0xF0;
        cmd.data1 = message->at(1);
        cmd.data2 = message->at(2);
        daw->midiIn(cmd);
    }
}

int audiocallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                   double streamTime, RtAudioStreamStatus status, void *data )
{
    if (debug_mode) return 0;

    if ( status )
        std::cout << "Stream underflow detected!" << std::endl;

    Sync sync;
    float *outBufferFloat = (float *) outputBuffer;
    float *inBufferFloat = (float *) inputBuffer;
    daw->process(outBufferFloat, inBufferFloat, nBufferFrames);

    return 0;
}

bool checkMidiIn(RtMidiIn * midiin) {
    if (midiin->isPortOpen()) {
        unsigned int nPorts = midiin->getPortCount();
        if (nPorts < 2) {
            midiin->closePort();
            std::cout << "midi in port closed" << std::endl;
        }
    } else {
        unsigned int nPorts = midiin->getPortCount();
        if (nPorts < 2) return false;
        try {
            midiin->openPort(1);
            std::cout << "midi in port opened" << std::endl;
        } catch (RtMidiError &error) {
            error.printMessage();
        }
    }
    return true;
}

int main( int argc, char *argv[] )
{
    unsigned int bufferFrames, fs = SAMPLERATE, device = 0, offset = 0;
    daw = new DAW(screen);

    RtMidiIn midiin;
    midiin.setErrorCallback(&midiErrorCallback);
    midiin.setCallback(&midicallback);
    midiin.ignoreTypes(false, false, false);

    RtAudio::StreamOptions options;

#ifdef __APPLE__
    RtAudio dac( RtAudio::MACOSX_CORE, &audioErrorCallback );
#else
    RtAudio dac( RtAudio::LINUX_ALSA, &audioErrorCallback );
#endif

    std::vector<unsigned int> deviceIds = dac.getDeviceIds();
    if ( deviceIds.size() < 1 ) {
        std::cout << "\nNo audio devices found!\n";
        exit(1);
    }

    auto names = dac.getDeviceNames();
    std::cout << "device = " << names[0] << std::endl;

    double *data = (double *) calloc( 2, sizeof( double ) );

    dac.showWarnings( false );

    bufferFrames = BUF_SIZE;
    RtAudio::StreamParameters oParams;
    oParams.nChannels = 2;
    oParams.firstChannel = offset;
    oParams.deviceId = dac.getDefaultOutputDevice();

    RtAudio::StreamParameters iParams;
#ifndef __APPLE__
    iParams.nChannels = 2;
#else
    iParams.nChannels = 1;
#endif
    iParams.firstChannel = 0;
    iParams.deviceId = dac.getDefaultInputDevice();

    options.flags = RTAUDIO_HOG_DEVICE;
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;

    init_serial();

    int it = 0;
    const int div = 100;
    const int midi_refresh_time = 1;

    // An error in the openStream() function can be detected either by
    // checking for a non-zero return value OR by a subsequent call to
    // isStreamOpen().
    if ( dac.openStream( &oParams, &iParams, RTAUDIO_FLOAT32, fs, &bufferFrames, &audiocallback, (void *)data, &options ) ) {
        std::cout << dac.getErrorText() << std::endl;
        goto cleanup;
    }
    if ( dac.isStreamOpen() == false ) goto cleanup;

    std::cout << "Stream latency = " << dac.getStreamLatency() << "\n" << std::endl;

    if ( dac.startStream() ) {
        std::cout << dac.getErrorText() << std::endl;
        goto cleanup;
    }

    while ( dac.isStreamRunning() == true ) {

        if (it == 0) {
            checkMidiIn(&midiin);
            screen->fillScreen(0x00);
            if (!debug_mode) daw->draw(screen);
            process_gui();
        }
        scan_buttons();
        it = (it + 1) % div;
        SLEEP(midi_refresh_time);
    }

    cleanup:
    if ( dac.isStreamOpen() ) dac.closeStream();
    free( data );
    midiin.closePort();

    return 0;
}