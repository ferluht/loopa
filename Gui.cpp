//
// Created by ferluht on 25/11/2022.
//

#include "Gui.h"

#ifndef __APPLE__
    int fd;
    uint8_t uartbuffer[256];
    uint8_t uartit = 0;
#else
    Screen *nguiscreen = nullptr;
    GLFWwindow* window = nullptr;
#endif

class ImageCanvas : public nanogui::GLCanvas {
public:

    ImageCanvas(Widget *parent, GFXcanvas1 * screen) : nanogui::GLCanvas(parent) {
        using namespace nanogui;
        mScreen = screen;

        mShader.init(
                /* An identifying name */
                "a_simple_shader",

                /* Vertex shader */
                "#version 330\n"
                "uniform mat4 modelViewProj;\n"
                "in vec3 position;\n"
                "in vec3 color;\n"
                "out vec4 frag_color;\n"
                "void main() {\n"
                "    frag_color = 3.0 * modelViewProj * vec4(color, 1.0);\n"
                "    gl_Position = modelViewProj * vec4(position, 1.0);\n"
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
    }

    void drawGL() override {

        mShader.bind();
//        glEnable(GL_DEPTH_TEST);
        /* Draw 12 triangles starting at index 0 */
        glDrawPixels(10, 10, GL_RGBA, GL_UNSIGNED_BYTE, mScreen->getBuffer());
//        glDisable(GL_DEPTH_TEST);
//        glClear(GL_COLOR_BUFFER_BIT);
//        glMatrixMode( GL_PROJECTION );
//        glLoadIdentity();
//        gluOrtho2D( 0.0, 500.0, 500.0,0.0 );
//
//        glBegin(GL_POINTS);
//            glColor3f(1,1,1);
//            glVertex2i(100,100);
//        glEnd();
//        glDrawPixels(10, 10, GL_RGBA, GL_UNSIGNED_BYTE, mScreen->getBuffer());

//        glBindTexture(GL_TEXTURE_2D, mImageID);
//        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 32, GL_COLOR_INDEX, GL_BITMAP, (GLvoid*)mScreen->getBuffer());
    }

private:
    nanogui::GLShader mShader;
    GFXcanvas1 * mScreen;
};

class MyGLCanvas : public nanogui::GLCanvas {
public:

    MyGLCanvas(Widget *parent, GFXcanvas1 * screen) : nanogui::GLCanvas(parent) {
        mScreen = screen;

        indices = new MatrixXu(3, SCREEN_HEIGHT * SCREEN_WIDTH * 2);
        positions = new MatrixXf(3, SCREEN_HEIGHT * SCREEN_WIDTH * 4);
        colors = new MatrixXf(3, SCREEN_HEIGHT * SCREEN_WIDTH * 4);
        float pixsize = 0.015;
        for (int i = 0; i < SCREEN_HEIGHT; i ++) {
            for (int j = 0; j < SCREEN_WIDTH; j ++) {
                int n = i * SCREEN_WIDTH + j;
                indices->col( n * 2 + 0) << n * 4 + 0, n * 4 + 1, n * 4 + 2;
                indices->col( n * 2 + 1) << n * 4 + 3, n * 4 + 0, n * 4 + 2;

                float pmx = pixsize * j;
                float pmy = pixsize * i;
                positions->col(n * 4 + 0) << - pmx + pixsize / 2 + SCREEN_WIDTH * pixsize / 2,  - pmy + pixsize / 2 + SCREEN_HEIGHT * pixsize / 2, 0;
                positions->col(n * 4 + 1) << - pmx - pixsize / 2 + SCREEN_WIDTH * pixsize / 2,  - pmy + pixsize / 2 + SCREEN_HEIGHT * pixsize / 2, 0;
                positions->col(n * 4 + 2) << - pmx - pixsize / 2 + SCREEN_WIDTH * pixsize / 2,  - pmy - pixsize / 2 + SCREEN_HEIGHT * pixsize / 2, 0;
                positions->col(n * 4 + 3) << - pmx + pixsize / 2 + SCREEN_WIDTH * pixsize / 2,  - pmy - pixsize / 2 + SCREEN_HEIGHT * pixsize / 2, 0;
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

    ~MyGLCanvas() {
        mShader.free();
    }

    virtual void drawGL() override {

        mShader.bind();
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            for (int j = 0; j < SCREEN_WIDTH / 8; j++) {
                for (int k = 0; k < 8; k++) {
                    int n = i * (int)(SCREEN_WIDTH / 8) + j;
                    int m = i * SCREEN_WIDTH + j * 8 + k;

                    int c = 0;
                    if ((mScreen->getBuffer()[n] >> k) % 2) c = 250;

                    colors->col(m * 4 + 0) << c, c, c;
                    colors->col(m * 4 + 1) << c, c, c;
                    colors->col(m * 4 + 2) << c, c, c;
                    colors->col(m * 4 + 3) << c, c, c;
                }
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
    window = glfwCreateWindow(800, 800, "rpidaw", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    nguiscreen = new Screen();
    nguiscreen->initialize(window, true);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(0);
    glfwSwapBuffers(window);

//    glDrawPixels(10, 10, GL_COLOR_INDEX, GL_BITMAP, screen->getBuffer());

    enum test_enum {
        Item1 = 0,
        Item2,
        Item3
    };

    bool bvar = true;
    int ivar = 12345678;
    double dvar = 3.1415926;
    float fvar = (float)dvar;
    std::string strval = "A string";
    test_enum enumval = Item2;
    nanogui::Color colval(0.5f, 0.5f, 0.7f, 1.f);

    // Create nanogui gui
    bool enabled = true;
    FormHelper *gui = new FormHelper(nguiscreen);

    ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");

    gui->addGroup("Basic types");
    gui->addVariable("bool", bvar)->setTooltip("Test tooltip.");
    gui->addVariable("string", strval);

//    gui->addWidget("test", new ImageCanvas(nanoguiWindow, screen));
    gui->addWidget("test", new MyGLCanvas(nanoguiWindow, screen));

    gui->addGroup("Validating fields");
    gui->addVariable("int", ivar)->setSpinnable(true);
    gui->addVariable("float", fvar)->setTooltip("Test.");
    gui->addVariable("double", dvar)->setSpinnable(true);

    gui->addGroup("Complex types");
    gui->addVariable("Enumeration", enumval, enabled)->setItems({ "Item 1", "Item 2", "Item 3" });
    gui->addVariable("Color", colval)
            ->setFinalCallback([](const Color &c) {
                std::cout << "ColorPicker Final Callback: ["
                          << c.r() << ", "
                          << c.g() << ", "
                          << c.b() << ", "
                          << c.w() << "]" << std::endl;
            });

    gui->addGroup("Other widgets");
    gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");;

    nguiscreen->setVisible(true);
    nguiscreen->performLayout();
    nanoguiWindow->center();

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

    // Draw nanogui
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

void close_gui() {
#ifdef __APPLE__
    glfwTerminate();
#endif
}