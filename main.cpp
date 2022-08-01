#include <rtaudio/RtAudio.h>
#include <rtmidi/RtMidi.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <chrono>
#include <wiringSerial.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <nanovg/src/nanovg.h>
#define NANOVG_GLES2_IMPLEMENTATION
#include <nanovg/src/nanovg_gl.h>
#include <nanovg/src/nanovg_gl_utils.h>

#define BUTTON_HEADER 10
#define ENCODER_HEADER 11

#include "Daw.h"
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

DAW * daw;

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


int fd;
uint8_t uartbuffer[256];
uint8_t uartit = 0;

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

EGLDisplay display;
int major, minor;
int desiredWidth, desiredHeight;
GLuint program, vert, frag, vbo;
GLint posLoc, colorLoc, result;
EGLContext context;
EGLSurface surface;
NVGcontext* vg;
unsigned char* image;

static const EGLint pbufferAttribs[] = {
EGL_WIDTH,
SCREEN_WIDTH,
EGL_HEIGHT,
SCREEN_HEIGHT,
EGL_NONE,
};

static const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                        EGL_NONE};

static const EGLint configAttribs[] = {
EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8,
EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 8,
EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE
};

int init_graphics() {

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) return EXIT_FAILURE;

    if (eglInitialize(display, &major, &minor) == EGL_FALSE) {
        eglTerminate(display);
        return EXIT_FAILURE;
    }

    std::cout << "Initialized EGL version: " << major << "." << minor << std::endl;

    EGLint numConfigs;
    EGLConfig config;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs)) {
        eglTerminate(display);
        return EXIT_FAILURE;
    }

    surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
    if (surface == EGL_NO_SURFACE)
    {
        eglTerminate(display);
        return EXIT_FAILURE;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        eglDestroySurface(display, surface);
        eglTerminate(display);
        return EXIT_FAILURE;
    }

    eglMakeCurrent(display, surface, surface, context);

    eglSwapInterval(display, 0);

    // The desired width and height is defined inside of pbufferAttribs
    // Check top of this file for EGL_WIDTH and EGL_HEIGHT
    desiredWidth = pbufferAttribs[1];  // 800
    desiredHeight = pbufferAttribs[3]; // 600

    // Set GL Viewport size, always needed!
    glViewport(0, 0, desiredWidth, desiredHeight);

    // Get GL Viewport size and test if it is correct.
    // NOTE! DO NOT UPDATE EGL LIBRARY ON RASPBERRY PI AS IT WILL INSTALL FAKE
    // EGL! If you have fake/faulty EGL library, the glViewport and
    // glGetIntegerv won't work! The following piece of code checks if the gl
    // functions are working as intended!
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // viewport[2] and viewport[3] are viewport width and height respectively
    std::cout << "GL Viewport size: " << viewport[2] << "x" << viewport[3] << std::endl;

    // Test if the desired width and height match the one returned by
    // glGetIntegerv
    if (desiredWidth != viewport[2] || desiredHeight != viewport[3])
    {
        std::cout << "Error! The glViewport/glGetIntegerv are not working! " << std::endl;
    }

    vg = nvgCreateGLES2(NULL);
    if (vg == NULL) {
        printf("Could not init nanovg.\n");
        return -1;
    }

    image = (unsigned char*)malloc(desiredWidth*desiredHeight*4);
    if (image == NULL)
        return -1;
}

void draw() {
    vg = nvgCreateGLES2(NULL);
    if (vg == NULL) {
        printf("Could not init nanovg.\n");
    }

    nvgCreateFont(vg, "sans", "/home/pi/rpidaw/3rdparty/nanovg/example/Arial MT Std Light.ttf");

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    nvgBeginFrame(vg, desiredWidth, desiredHeight, 1);

    nvgFontFace(vg, "sans");
    nvgFontSize(vg, 13);
    nvgStrokeWidth(vg, 0.1);
    nvgScale(vg, 1, -1);
    nvgTranslate(vg, 0, -SCREEN_HEIGHT);
    nvgFontBlur(vg, 0);
    nvgTextLetterSpacing(vg, 2);
    nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 255));
    daw->draw(vg);
    nvgEndFrame(vg);

    glReadPixels(0, 0, desiredWidth, desiredHeight, GL_RGBA, GL_UNSIGNED_BYTE, image);

    for (int i = 0; i < desiredWidth*desiredHeight*4; i += 8*4) {
        unsigned char px8 = 0;
        for (int j = 0; j < 8; j ++) {
            px8 |= (image[i+j*4+1] > 0) << (7-j);
        }
        serialPutchar(fd, px8);
    }

    nvgDeleteGLES2(vg);
}

int main( int argc, char *argv[] )
{
    init_graphics();

    unsigned int bufferFrames, fs = SAMPLERATE, device = 0, offset = 0;
    daw = new DAW();

    RtMidiIn midiin;
    midiin.setErrorCallback(&midiErrorCallback);
    midiin.setCallback(&midicallback);
    midiin.ignoreTypes(false, false, false);

    RtAudio::StreamOptions options;

    // Specify our own error callback function.
    RtAudio dac( RtAudio::LINUX_ALSA, &audioErrorCallback );

    std::vector<unsigned int> deviceIds = dac.getDeviceIds();
    if ( deviceIds.size() < 1 ) {
        std::cout << "\nNo audio devices found!\n";
        exit( 1 );
    }

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

    if((fd=serialOpen("/dev/ttyS0", 115200))<0){
        fprintf(stderr,"Unable to open serial device: %s\n",strerror(errno));
        return 1;
    }

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
        draw();
        SLEEP(50);
    }

    cleanup:
    if ( dac.isStreamOpen() ) dac.closeStream();
    free( data );
    midiin.closePort();

    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);

    return 0;
}