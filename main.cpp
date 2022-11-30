#include <rtaudio/RtAudio.h>
#include <rtmidi/RtMidi.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>

#include "Gui.h"
#include "Daw.h"

DAW * daw;
extern GFXcanvas1 * screen;

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
    if ( status )
        std::cout << "Stream underflow detected!" << std::endl;

    float *outBufferFloat = (float *) outputBuffer;
    float *inBufferFloat = (float *) inputBuffer;
    daw->process(outBufferFloat, inBufferFloat, nBufferFrames, streamTime);

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

int main( int argc, char *argv[] )
{
    unsigned int bufferFrames, fs = SAMPLERATE, device = 0, offset = 0;
    daw = new DAW(screen);

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

    init_gui();

    int it = 0;
    const int div = 50;
    const int midi_refresh_time = 1;

    // An error in the openStream() function can be detected either by
    // checking for a non-zero return value OR by a subsequent call to
    // isStreamOpen().
    if ( dac.openStream( &oParams, NULL, RTAUDIO_FLOAT32, fs, &bufferFrames, &audiocallback, (void *)data, &options ) ) {
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
            checkMidi(&midiin);
            screen->fillScreen(0x00);
            daw->draw(screen);
            if (!process_gui()) break;
        }
        scan_buttons();
        it = (it + 1) % div;
        SLEEP(midi_refresh_time);
    }

    close_gui();

    cleanup:
    if ( dac.isStreamOpen() ) dac.closeStream();
    free( data );
    midiin.closePort();

    return 0;
}