#include "GLFW/glfw3.h"

// C libs
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits>

// Core
#include "helpers/TypeDefinitions.h"

// Rest
#include "helpers/GrcPrimitives.h"

// Truetype, no asserts
#define STBTT_assert
#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb/stb_truetype.h"

namespace Input {
    
    struct DebugSet {
        enum Keys : u8 {
            ESC = 0, EXIT = ESC,
            COUNT,
            INVALID
        };
        
        static Keys mapKey(const s32 key) {
            switch (key) {
                case GLFW_KEY_ESCAPE: return ESC;
                default: return INVALID;
            }
        }
    };
    
    template<typename _Set>
    struct KeyboardState {
        static const u8 kPreviousStateShift = 1;
        static const u8 kCurrentStateMask = 0x01;
        static const u8 kPreviousStateMask = 0x02;
        static const u8 kRepeatShift = 7;
        static const u8 kRepeatMask = 0x80;
        
        bool down(typename _Set::Keys key) const {
            return values[(u8)key] & kCurrentStateMask;
        }
        
        bool wasDown(typename _Set::Keys key) const {
            return values[(u8)key] & kPreviousStateMask;
        }
        
        bool released(typename _Set::Keys key) const {
            return !down(key) && wasDown(key);
        }
        
        bool pressed(typename _Set::Keys key) const {
            return down(key) && !wasDown(key);
        }

        bool repeated(typename _Set::Keys key) const {
            return values[(u8)key] & kRepeatMask;
        }
        
        void handleEvent(const s32 key, const s32 action) {
            const typename _Set::Keys mappedKey = _Set::mapKey(key);
            if (mappedKey != _Set::Keys::INVALID) {
                u8& value = values[(u8)mappedKey];
                
                value = (value & ~kCurrentStateMask) | ((action != GLFW_RELEASE) & kCurrentStateMask);
                value = (value & ~kRepeatMask) | ((action == GLFW_REPEAT) << kRepeatShift);
            }
        }
        
        void update() {
            for (u8& value : values) {
                u8 state = value;
                state = (state << kPreviousStateShift) | (state & kCurrentStateMask);
                value = (value & kRepeatMask) | (state & ~kRepeatMask);
            }
        }
        
        u8 values[(u8)_Set::COUNT];
    };
}

namespace Math3D {
    // CM = Column Major
    typedef f32 TransformMatrixCM32[16];
    typedef f64 TransformMatrixCM64[16];
};

namespace Camera {
    
    // frustum.fov = 60.0;
    // frustum.aspect = 1.0;
    // frustum.near = 1.0;
    // frustum.far = 200.0;
    struct FrustumParams {
        f32 fov;
        f32 aspect;
        f32 near;
        f32 far;
    };
    
    void computeProjectionMatrix(const FrustumParams& params, Math3D::TransformMatrixCM64& matrix) {
        const f64 xMax = params.near * tanf(params.fov * M_PI / 360.0);
        const f64 xMin = -xMax;
        const f64 yMin = xMin / params.aspect;
        const f64 yMax = xMax / params.aspect;
        
        memset(matrix, 0, sizeof(matrix));
        matrix[0] = (2.f * params.near) / (xMax - xMin);
        matrix[5] = (2.f * params.near) / (yMax - yMin);
        matrix[10] = -(params.far + params.near) / (params.far - params.near);
        matrix[8] = (xMax + xMin) / (xMax - xMin);
        matrix[9] = (yMax + yMin) / (yMax - yMin);
        matrix[11] = -1.f;
        matrix[14] = -(2.0 * params.far * params.near) / (params.far - params.near);
    }
    
    // ortho.right = app.mainWindow.width * 0.5f;
    // ortho.top = app.mainWindow.height * 0.5f;
    // ortho.left = -ortho.right;
    // ortho.bottom = -ortho.top;
    // ortho.near = -1.f;
    // ortho.far = 200.f;
    struct OrthoParams {
        f32 left;
        f32 right;
        f32 top;
        f32 bottom;
        f32 near;
        f32 far;
    };
    
    void computeProjectionMatrix(const OrthoParams& params, Math3D::TransformMatrixCM32& matrix) {
        memset(matrix, 0, sizeof(matrix));
        matrix[0] = 2.f / (params.right - params.left);
        matrix[5] = 2.f / (params.top - params.bottom);
        matrix[10] = -2.f / (params.far - params.near);
        matrix[12] = -(params.right + params.left) / (params.right - params.left);
        matrix[13] = -(params.top + params.bottom) / (params.top - params.bottom);
        matrix[14] = -(params.far + params.near) / (params.far - params.near);
        matrix[15] = 1.f;
    }
};

struct App {
    
    struct Time {
    
        struct Config {
            f64 targetFrameRate;
            f64 maxFrameLength;
        };

        static void startLoop(Time& time) {
            time.lastUpdate = 0.0;
            time.now = time.start = time.lastUpdate = glfwGetTime();
            time.frame = 0;
            time.tillUpdate = 0.0;
        }
        
        static void processLoop(Time& time) {
            time.now = glfwGetTime();
            time.running = time.now - time.start;
            time.tillUpdate = time.config.targetFrameRate - (time.now - time.lastUpdate);
        }
        
        static void processFrame(Time& time) {
            time.lastFrameDelta = fminf(time.now - time.lastUpdate, time.config.maxFrameLength);
            time.lastUpdate = time.now;
            time.frame++;
        }
        
        Config config;
        
        // In seconds
        f64 running;
        f64 start;
        f64 now;
        f64 lastUpdate;
        f64 lastFrameDelta;
        f64 tillUpdate;
        
        s64 frame;
    };
    
    struct Window {
        GLFWwindow* handle;
        f32 desiredRatio;
        u32 width;
        u32 height;
        bool fullscreen = false;
    };
    
    struct Input {
        ::Input::KeyboardState<::Input::DebugSet> debugSet;
    };
    
    Window mainWindow;
    Camera::OrthoParams orthoParams;
    Time time;
    Time::Config timeConfig;
    Input input;
};
App app;

struct MainLevel {
  
    struct TextSet {
        enum Keys : u8 {
            BACKPSPACE = 0,
            ENTER = 1,
            COUNT,
            INVALID
        };
        
        static Keys mapKey(const s32 key) {
            switch (key) {
                case GLFW_KEY_BACKSPACE: return BACKPSPACE;
                case GLFW_KEY_ENTER: return ENTER;
                default: return INVALID;
            }
        }
    };
    
    struct Text {
        enum { glyphCount = 96, bitmapSize = 512, firstChar = 32 };
        char buffer[256];
        s32 bufferCount;
        f32 pixelHeight;
        // ASCII 32..126 is 95 glyphs
        stbtt_bakedchar bakedFont[glyphCount];
        GLuint ftext;
    };
    
    Text text;
    ::Input::KeyboardState<TextSet> textInputSet;
};
MainLevel mainLevel;

int main(int argc, char** argv) {
    
    if (glfwInit())
    {
        App::Window& window = app.mainWindow;
        
        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
        window.width = 640;
        window.height = 480;
        window.desiredRatio = window.width / (f32) window.height;
        window.fullscreen = false;
        window.handle = glfwCreateWindow(
              window.width
            , window.height
            , "text test"
            , nullptr /*monitor*/
            , nullptr /*share*/
        );
        if (window.handle)
        {
            glfwSetKeyCallback(app.mainWindow.handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                app.input.debugSet.handleEvent(key, action);
                mainLevel.textInputSet.handleEvent(key, action);
            });
            
            // Render setup
            glfwMakeContextCurrent(window.handle);
            glfwSwapInterval(1);
            glMatrixMode(GL_PROJECTION);
            {
                glLoadIdentity();
                Math3D::TransformMatrixCM32 projectionTransform;
                Camera::OrthoParams& ortho = app.orthoParams;
                ortho.right = app.mainWindow.width * 0.5f;
                ortho.top = app.mainWindow.height * 0.5f;
                ortho.left = -ortho.right;
                ortho.bottom = -ortho.top;
                ortho.near = -1.f;
                ortho.far = 200.f;
                Camera::computeProjectionMatrix(ortho, projectionTransform);
                glMultMatrixf(projectionTransform);
            }
            glMatrixMode(GL_MODELVIEW);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            { // Scene setup
                using Text = MainLevel::Text;
                
                MainLevel::Text& text = mainLevel.text;
                
                unsigned char bitmap[Text::bitmapSize*Text::bitmapSize];
                
                FILE* file = fopen("assets/fonts/adventurePixels.ttf", "r");
				if (!file) {
					goto windowclose;
				}

                fseek(file, 0, SEEK_END);
                const s64 bufferSize = ftell(file);
                rewind(file);
                unsigned char* fileBuffer = (unsigned char*)malloc(sizeof(unsigned char)*bufferSize);
                fread(fileBuffer, 1, bufferSize, file);
                if (!fileBuffer) {
                    goto windowclose;
                }
                
                text.pixelHeight = 16.f;
                stbtt_BakeFontBitmap(
                      fileBuffer, 0 /*font location and offset*/
                    , text.pixelHeight /*pixel height*/
                    , bitmap, Text::bitmapSize, Text::bitmapSize /*bitmap data*/
                    , MainLevel::Text::firstChar
                    , Text::glyphCount /*num chars*/
                    , text.bakedFont
                );
                free(fileBuffer);
                fclose(file);
                
                glGenTextures(1, &text.ftext);
                glBindTexture(GL_TEXTURE_2D, text.ftext);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, Text::bitmapSize, Text::bitmapSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                text.bufferCount = 0;
                glfwSetCharCallback(app.mainWindow.handle, [](GLFWwindow* window, unsigned int codepoint) {
                    
					MainLevel::Text& text = mainLevel.text;

                    static char result[6 + 1];
                    int length = wctomb(result, codepoint);
                    if (length == -1)
                        length = 0;
                    result[length] = '\0';
                    
                    if (length + text.bufferCount < sizeof(text.buffer)/sizeof(text.buffer[0])) {
                        memcpy(&text.buffer[text.bufferCount], result, length);
                        text.bufferCount += length;
                    }
                });
            }
            
            app.time.config.maxFrameLength = 0.1;
            app.time.config.targetFrameRate = 1.0 / 60.0;
            App::Time::startLoop(app.time);
            do {
                if (app.time.tillUpdate <= 0.0001) {
                    App::Time::processFrame(app.time);
                    
                    // Input
                    app.input.debugSet.update();
                    mainLevel.textInputSet.update();
                    glfwPollEvents();
                    
                    // Processing
                    if (app.input.debugSet.released(Input::DebugSet::EXIT)) {
                        glfwSetWindowShouldClose(app.mainWindow.handle, 1);
                    }
                    if (mainLevel.textInputSet.pressed(MainLevel::TextSet::BACKPSPACE)
                    || mainLevel.textInputSet.repeated(MainLevel::TextSet::BACKPSPACE)) {
                        mainLevel.text.bufferCount = fmax(mainLevel.text.bufferCount - 1, 0);
                    }
                    if (mainLevel.textInputSet.pressed(MainLevel::TextSet::ENTER)
                        || mainLevel.textInputSet.repeated(MainLevel::TextSet::ENTER)) {
                        mainLevel.text.buffer[mainLevel.text.bufferCount++] = '\n';
                    }
                    
                    // Render
                    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    {
                        {
                            f32 scale = 5.f * sin(app.time.running * 4.f) + 20.f;
                            const Col colorA(0.7f, 0.4f, 1.0f, 1.0f);
                            const Col colorB(0.78f, 0.0f, 0.6f, 1.0f);
                            const Col colorC(0.7f, 0.7f, 0.6f, 1.0f);
                            GrcPrimitives::segment(Vec3(-scale, -scale, 0.f), Vec3(0.f, scale, 0.f), colorA);
                            GrcPrimitives::segment(Vec3(0.f, scale, 0.f), Vec3(scale, -scale, 0.f), colorB);
                            GrcPrimitives::segment(Vec3(scale, -scale, 0.f), Vec3(-scale, -scale, 0.f), colorC);
                        }
                        
                        {
                            using Text = MainLevel::Text;
                            MainLevel::Text& text = mainLevel.text;
                            
                            f32 xorigin = app.orthoParams.left;
                            f32 yorigin = app.orthoParams.top - text.pixelHeight;
                            f32 x = xorigin, y = yorigin;
                            
                            glEnable(GL_TEXTURE_2D);
                            glBindTexture(GL_TEXTURE_2D, text.ftext);
                            glBegin(GL_QUADS);
                            const Col color(0.88f, 0.83f, 0.73f, 1.0f);
                            glColor4f(RGBA_PARAMS(color));
                            
                            glTexCoord2f(0.0, 1.0); glVertex2f(-200,-300);
                            glTexCoord2f(1.0, 1.0); glVertex2f(200,-300);
                            glTexCoord2f(1.0, 0.0); glVertex2f(200,100);
                            glTexCoord2f(0.0, 0.0); glVertex2f(-200,100);
                            
                            
                            for (s32 i = 0; i < text.bufferCount; i++) {
                                char c = text.buffer[i];
                                
                                if (c == '\n') {
                                    y -= text.pixelHeight;
                                    x = xorigin;
                                }
                                else if (c >= Text::firstChar) {
                                    stbtt_aligned_quad q;
                                    {
                                        float ipw = 1.0f / Text::bitmapSize, iph = 1.0f / Text::bitmapSize;
                                        const stbtt_bakedchar* b = text.bakedFont + (c - Text::firstChar);
                                        
                                        // Text wrap
                                        if (x + b->xadvance > app.orthoParams.right) {
                                            y -= text.pixelHeight;
                                            x = xorigin;
                                        }
                                        
                                        int round_x = floor((x + b->xoff) + 0.5f);
                                        int round_y = floor((y - b->yoff) + 0.5f);
                                        
                                        q.x0 = round_x;
                                        q.y0 = round_y;
                                        q.x1 = round_x + b->x1 - b->x0;
                                        q.y1 = round_y - b->y1 + b->y0;
                                        
                                        q.s0 = b->x0 * ipw;
                                        q.t0 = b->y1 * iph;
                                        q.s1 = b->x1 * ipw;
                                        q.t1 = b->y0 * iph;
                                        
                                        x += b->xadvance;
                                    }

                                    glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y0);
                                    glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y0);
                                    glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y1);
                                    glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y1);
                                }
                            }
                            glEnd();
                            glBindTexture(GL_TEXTURE_2D, 0);
                        }
                    }
                    glfwSwapBuffers(app.mainWindow.handle);
                }
                
                App::Time::processLoop(app.time);
                
            } while (!glfwWindowShouldClose(app.mainWindow.handle));
            
        windowclose:
            glfwDestroyWindow(window.handle);
        }
        
        glfwTerminate();
    }
    
    return 0;
}
