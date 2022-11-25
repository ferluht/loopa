//
// Created by ferluht on 25/11/2022.
//

#include "Gui.h"

extern DAW * daw;

class KbdScreen : public Screen {

public:

    KbdScreen(DAW * mdaw_) : Screen() {
        mdaw = mdaw_;
    }

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override {

//        std::cout << key << " " << action << std::endl;

        if (65 <= key && key <= 90) {
            cmd.status = action ? NOTEON_HEADER : NOTEOFF_HEADER;
            cmd.data1 = key;
            cmd.data2 = action ? 100 : 0;
            daw->midiIn(cmd);
        } else if (key > 261) {
            cmd.status = CC_HEADER;
            cmd.data1 = S2;
            cmd.data2 = 127;
            daw->midiIn(cmd);
            cmd.status = CC_HEADER;
            cmd.data2 = action ? 100 : 0;
            switch (key) {
                case 262:
                    cmd.data1 = K3;
                    break;
                case 263:
                    cmd.data1 = K1;
                    break;
                case 264:
                    cmd.data1 = K2;
                    break;
                case 265:
                    cmd.data1 = K4;
                    break;
                default:
                    break;
            }
            daw->midiIn(cmd);
            cmd.status = CC_HEADER;
            cmd.data1 = S2;
            cmd.data2 = 0;
            daw->midiIn(cmd);
        } else {
            return true;
        }

        daw->midiIn(cmd);
    }

private:
    DAW * mdaw;
    MData cmd;
};

#ifndef __APPLE__
    int fd;
    uint8_t uartbuffer[256];
    uint8_t uartit = 0;
#else
    KbdScreen *nguiscreen = nullptr;
    GLFWwindow* window = nullptr;
#endif

class PixelDisplay : public nanogui::GLCanvas {
public:

    PixelDisplay(Widget *parent, GFXcanvas1 * screen, float pixsize, float pixratio) : nanogui::GLCanvas(parent) {
        mScreen = screen;

        indices = new MatrixXu(3, SCREEN_HEIGHT * SCREEN_WIDTH * 2);
        positions = new MatrixXf(3, SCREEN_HEIGHT * SCREEN_WIDTH * 4);
        colors = new MatrixXf(3, SCREEN_HEIGHT * SCREEN_WIDTH * 4);

        for (int i = 0; i < SCREEN_HEIGHT; i ++) {
            for (int j = 0; j < SCREEN_WIDTH; j ++) {
                int n = i * SCREEN_WIDTH + j;
                indices->col( n * 2 + 0) << n * 4 + 0, n * 4 + 1, n * 4 + 2;
                indices->col( n * 2 + 1) << n * 4 + 3, n * 4 + 0, n * 4 + 2;

                float pmx = pixsize * j;
                float pmy = pixsize * i * pixratio;
                positions->col(n * 4 + 0) << pmx - pixsize / 2 - SCREEN_WIDTH * pixsize / 2,  - pmy + pixsize * pixratio / 2 + SCREEN_HEIGHT * pixsize * pixratio / 2, 0;
                positions->col(n * 4 + 1) << pmx + pixsize / 2 - SCREEN_WIDTH * pixsize / 2,  - pmy + pixsize * pixratio / 2 + SCREEN_HEIGHT * pixsize * pixratio / 2, 0;
                positions->col(n * 4 + 2) << pmx + pixsize / 2 - SCREEN_WIDTH * pixsize / 2,  - pmy - pixsize * pixratio / 2 + SCREEN_HEIGHT * pixsize * pixratio / 2, 0;
                positions->col(n * 4 + 3) << pmx - pixsize / 2 - SCREEN_WIDTH * pixsize / 2,  - pmy - pixsize * pixratio / 2 + SCREEN_HEIGHT * pixsize * pixratio / 2, 0;
            }
        }

        mShader.init(
            /* An identifying name */
            "a_simple_shader",

            /* Vertex shader */
            "#version 330\n"
            "in vec3 position;\n"
            "in vec3 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "    frag_color = vec4(color, 1.0);\n"
            "    gl_Position = vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 color;\n"
            "in vec4 frag_color;\n"
            "void main() {\n"
            "    color = frag_color;\n"
            "}"
        );

        mShader.bind();
        mShader.uploadIndices(*indices);
        mShader.uploadAttrib("position", *positions);
    }

    ~PixelDisplay() {
        mShader.free();
    }

    virtual void drawGL() override {

        mShader.bind();
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            for (int j = 0; j < SCREEN_WIDTH; j++) {
                int n = i * SCREEN_WIDTH + j;

                int c = 0;
                if (mScreen->getPixel(j, i) > 0) c = 250;

                colors->col(n * 4 + 0) << c, c, c;
                colors->col(n * 4 + 1) << c, c, c;
                colors->col(n * 4 + 2) << c, c, c;
                colors->col(n * 4 + 3) << c, c, c;
            }
        }

        mShader.uploadAttrib("color", *colors);
        glEnable(GL_DEPTH_TEST);
        /* Draw 12 triangles starting at index 0 */
        mShader.drawIndexed(GL_TRIANGLES, 0, SCREEN_HEIGHT * SCREEN_WIDTH * 2);
        glDisable(GL_DEPTH_TEST);
    }

private:
    nanogui::GLShader mShader;
    MatrixXu * indices;
    MatrixXf * positions;
    MatrixXf * colors;
    GFXcanvas1 * mScreen;
};


GFXcanvas1 * screen = new GFXcanvas1(SCREEN_WIDTH, SCREEN_HEIGHT);

void init_gui() {
#ifndef __APPLE__
    if((fd=serialOpen("/dev/ttyS0", 115200))<0){
        fprintf(stderr,"Unable to open serial device: %s\n",strerror(errno));
        return 1;
    }
#else
    glfwInit();

    glfwSetTime(0);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create a GLFWwindow object
    window = glfwCreateWindow(800, 400, "rpidaw emulator", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    nguiscreen = new KbdScreen(daw);
    nguiscreen->initialize(window, true);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(0);
    glfwSwapBuffers(window);

    PixelDisplay * dspl = nguiscreen->add<PixelDisplay>(screen, 0.016, 4);
    dspl->setFixedSize(Eigen::Vector2i(400, 100));
    dspl->setPosition(Eigen::Vector2i(50, 50));

    Button * ctrl = nguiscreen->add<Button>();
    ctrl->setFlags(Button::NormalButton);
    ctrl->setFixedSize(Eigen::Vector2i(50, 50));
    ctrl->setPosition(Eigen::Vector2i(50, 250));
    ctrl->setCaption("ctrl");
    ctrl->setCallback([ctrl]() {
        MData cmd;
        cmd.status = NOTEON_HEADER;
        cmd.data1 = 70;
        cmd.data2 = 100;
        daw->midiIn(cmd);
        SLEEP(5);
        cmd.status = NOTEOFF_HEADER;
        cmd.data1 = 70;
        cmd.data2 = 0;
        daw->midiIn(cmd);
    });


    nguiscreen->setVisible(true);
    nguiscreen->performLayout();

    glfwSetCursorPosCallback(window,
                             [](GLFWwindow *, double x, double y) {
                                 nguiscreen->cursorPosCallbackEvent(x, y);
                             }
    );

    glfwSetMouseButtonCallback(window,
                               [](GLFWwindow *, int button, int action, int modifiers) {
                                   nguiscreen->mouseButtonCallbackEvent(button, action, modifiers);
                               }
    );

    glfwSetKeyCallback(window,
                       [](GLFWwindow *, int key, int scancode, int action, int mods) {
                           nguiscreen->keyCallbackEvent(key, scancode, action, mods);
                       }
    );

    glfwSetCharCallback(window,
                        [](GLFWwindow *, unsigned int codepoint) {
                            nguiscreen->charCallbackEvent(codepoint);
                        }
    );

    glfwSetDropCallback(window,
                        [](GLFWwindow *, int count, const char **filenames) {
                            nguiscreen->dropCallbackEvent(count, filenames);
                        }
    );

    glfwSetScrollCallback(window,
                          [](GLFWwindow *, double x, double y) {
                              nguiscreen->scrollCallbackEvent(x, y);
                          }
    );

    glfwSetFramebufferSizeCallback(window,
                                   [](GLFWwindow *, int width, int height) {
                                       nguiscreen->resizeCallbackEvent(width, height);
                                   }
    );
#endif
}


bool process_gui() {
#ifdef __APPLE__
    if (glfwWindowShouldClose(window)) return false;
    glfwPollEvents();

    glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    nguiscreen->drawContents();
    nguiscreen->drawWidgets();

    glfwSwapBuffers(window);
#else
    for (int i = 0; i < 128*32/8; i ++) {
        serialPutchar(fd, screen.getBuffer()[i]);
    }
#endif
    return true;
}

void scan_buttons() {
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
}

void close_gui() {
#ifdef __APPLE__
    glfwTerminate();
#endif
}