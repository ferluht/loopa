#include <rtaudio/RtAudio.h>
#include <rtmidi/RtMidi.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <chrono>

#ifndef __APPLE__
    #include <wiringSerial.h>
#endif

#include "Daw.h"
#include "Adafruit-GFX-offscreen/Adafruit_GFX.h"

#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

int fd;
uint8_t uartbuffer[256];
uint8_t uartit = 0;
DAW * daw;
GFXcanvas1 screen(SCREEN_WIDTH, SCREEN_HEIGHT);

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
        cmd.status = message->at(0);
        cmd.data1 = message->at(1);
        cmd.data2 = message->at(2);
        daw->midiIn(cmd);
    }
}

int audiocallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                   double streamTime, RtAudioStreamStatus status, void *data )
{
    if ( status )
        std::cout << "Stream underflow detected!" << std::endl;

//    std::cout << nBufferFrames << std::endl;

//    auto t1 = std::chrono::high_resolution_clock::now();

    float *outBufferFloat = (float *) outputBuffer;
    float *inBufferFloat = (float *) inputBuffer;
    daw->process(outBufferFloat, inBufferFloat, nBufferFrames, streamTime);
//    if (outBufferFloat[0] > 0.01) std::cout << outBufferFloat[0] << std::endl;
//    auto t2 = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
//
//    double newcpuusage = ms_double.count() / 10 / (256.f / SAMPLERATE);
//    if (newcpuusage > cpuusage) cpuusage = newcpuusage;
//    prevStreamTime = streamTime;

#ifndef __APPLE__
    while (serialDataAvail(fd) > 0) {
        uartbuffer[uartit] = serialGetchar(fd);
        if (uartbuffer[uartit] == '\n') {
            MData cmd;
            switch (uartbuffer[0]) {
                case CC_HEADER:
                case NOTEON_HEADER:
                case NOTEOFF_HEADER:
                    cmd.status = uartbuffer[0];
                    cmd.data1 = uartbuffer[1];
                    cmd.data2 = uartbuffer[2];
                    daw->midiIn(cmd);
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
#endif

    return 0;
}

bool checkMidi(RtMidiIn * midiin) {
    if (midiin->isPortOpen()) {
        unsigned int nPorts = midiin->getPortCount();
        if (nPorts < 2) {
            midiin->closePort();
            std::cout << "midi port closed" << std::endl;
        }
    } else {
        unsigned int nPorts = midiin->getPortCount();
        if (nPorts < 2) return false;
        try {
            midiin->openPort(1);
            std::cout << "midi port opened" << std::endl;
        } catch (RtMidiError &error) {
            error.printMessage();
        }
    }
    return true;
}

void draw() {

    screen.fillScreen(0x00);

    daw->draw(&screen);

#ifndef __APPLE__
    for (int i = 0; i < 128*32/8; i ++) {
        serialPutchar(fd, screen.getBuffer()[i]);
    }
#endif
}

int main( int argc, char *argv[] )
{
    unsigned int bufferFrames, fs = SAMPLERATE, device = 0, offset = 0;
    daw = new DAW();

    RtMidiIn midiin;
    midiin.setErrorCallback(&midiErrorCallback);
    midiin.setCallback(&midicallback);
    midiin.ignoreTypes(false, false, false);

    RtAudio::StreamOptions options;

    // Specify our own error callback function.
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

    // Tell RtAudio to output all messages, even warnings.
    dac.showWarnings( false );

    // Set our stream parameters for output only.
    bufferFrames = BUF_SIZE;
    RtAudio::StreamParameters oParams;
    oParams.nChannels = 2;
    oParams.firstChannel = offset;

    oParams.deviceId = dac.getDefaultOutputDevice();

    options.flags = RTAUDIO_HOG_DEVICE;
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;

#ifndef __APPLE__
    if((fd=serialOpen("/dev/ttyS0", 115200))<0){
        fprintf(stderr,"Unable to open serial device: %s\n",strerror(errno));
        return 1;
    }
#endif

    // An error in the openStream() function can be detected either by
    // checking for a non-zero return value OR by a subsequent call to
    // isStreamOpen().
    if ( dac.openStream( &oParams, NULL, RTAUDIO_FLOAT32, fs, &bufferFrames, &audiocallback, (void *)data, &options ) ) {
        std::cout << dac.getErrorText() << std::endl;
        goto cleanup;
    }
    if ( dac.isStreamOpen() == false ) goto cleanup;

    std::cout << "Stream latency = " << dac.getStreamLatency() << "\n" << std::endl;

    // Stream is open ... now start it.
    if ( dac.startStream() ) {
        std::cout << dac.getErrorText() << std::endl;
        goto cleanup;
    }

    MData cmd;

    std::srand(std::time(nullptr));

    while ( dac.isStreamRunning() == true ) {
        checkMidi(&midiin);
        cmd.status = NOTEON_HEADER;
        cmd.data1 = 60;
        cmd.data2 = 100;
        daw->midiIn(cmd);
        draw();
        SLEEP(1500);
    }

    cleanup:
    if ( dac.isStreamOpen() ) dac.closeStream();
    free( data );
    midiin.closePort();

    return 0;
}