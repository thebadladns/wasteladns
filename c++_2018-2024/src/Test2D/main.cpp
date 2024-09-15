#include "GLFW/glfw3.h"

// C libs
#include <stdlib.h>
#include <limits>

// Core
#include "helpers/TypeDefinitions.h"

// Rest
#include "helpers/GrcPrimitives.h"

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
        static const u8 kCurrentStateMask = 0x01;
        static const u8 kPreviousStateMask = 0x02;
        
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
        
        void handleEvent(const s32 key, const s32 action) {
            const typename _Set::Keys mappedKey = _Set::mapKey(key);
            if (mappedKey != _Set::Keys::INVALID) {
                u8& value = values[(u8)mappedKey];
                
                value = ( ((action == GLFW_PRESS || action == GLFW_REPEAT) & (value | kCurrentStateMask))
                         | (~(action != GLFW_RELEASE) & (value & ~kCurrentStateMask)) );
            }
        }
        
        void update() {
            for (u8& value : values) {
                value = (value << 1) | (value & kCurrentStateMask);
            }
        }
        
        u8 values[(u8)_Set::COUNT];
    };
}

namespace Random {
    f32 normalized() {
        return rand() / (f32) RAND_MAX;
    }

    f32 ranged(f32 low, f32 high) {
        return normalized() * (high - low) + low;
    }
    
    f32 rangedSafe(f32 low, f32 high) {
        f32 width = high - low;
        if (width == 0.f) {
            return high;
        }
        return normalized() * (width) + low;
    }
    
    f32 max(f32 high) {
        return normalized() * high;
    }
    
    s32 ranged(s32 low, s32 high) {
        return (rand() % (high - low)) + low;
    }
    
    s32 rangedSafe(s32 low, s32 high) {
        s32 width = high - low;
        if (width == 0) {
            return high;
        }
        return (rand() % (width)) + low;
    }
    
    s32 max(s32 high) {
        return rand() % high;
    }
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

namespace Game {
    
    struct Entity {
        struct SceneComponent {
            f32 x, y;
        };
        struct GraphicsComponent {
            void (*customRender)(f32 x, f32 y);
        };
        
        SceneComponent sceneComp;
        GraphicsComponent graphicsComp;
    };
    
    struct Scene {
        Game::Entity* entityList;
        s32 entityListCount;
    };
};

struct App {
    
    struct Time {
        // In seconds
        f64 running;
        f64 start;
        f64 now;
        
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
        
        struct Gameplay {
            struct Set {
                enum Keys : u8 {
                    UP = 0,
                    DOWN,
                    LEFT,
                    RIGHT,
                    COUNT,
                    INVALID
                };
                
                static Keys mapKey(const s32 key) {
                    switch (key) {
                        case GLFW_KEY_W: return UP;
                        case GLFW_KEY_S: return DOWN;
                        case GLFW_KEY_A: return LEFT;
                        case GLFW_KEY_D: return RIGHT;
                        default: return INVALID;
                    }
                }
            };
            
            ::Input::KeyboardState<Set> set;
            s8 xdir;
            s8 ydir;
        };
        
        Gameplay gameplay;
        ::Input::KeyboardState<::Input::DebugSet> debugSet;
    };
    
    Window mainWindow;
    Time time;
    Input input;
};

App app;

struct MainLevel {
    
    static void processControl(Game::Scene& scene, App::Input& input) {
        
        using GameplaySet = App::Input::Gameplay::Set;
        App::Input::Gameplay& gameplay = input.gameplay;
        const auto& gameplaySet = input.gameplay.set;
        
        const bool down = gameplaySet.down(GameplaySet::DOWN);
        const bool downPressed = gameplaySet.pressed(GameplaySet::DOWN);
        const bool up = gameplaySet.down(GameplaySet::UP);
        const bool upPressed = gameplaySet.pressed(GameplaySet::UP);
        const bool left = gameplaySet.down(GameplaySet::LEFT);
        const bool leftPressed = gameplaySet.pressed(GameplaySet::LEFT);
        const bool right = gameplaySet.down(GameplaySet::RIGHT);
        const bool rightPressed = gameplaySet.pressed(GameplaySet::RIGHT);
        
        if (downPressed) {
            gameplay.ydir = -1;
        }
        else if (upPressed) {
            gameplay.ydir = 1;
        }
        else if (down && !up) {
            gameplay.ydir = -1;
        }
        else if (up && !down) {
            gameplay.ydir = 1;
        }
        else if (!up && !down) {
            gameplay.ydir = 0;
        }
        
        if (leftPressed) {
            gameplay.xdir = -1;
        }
        else if (rightPressed) {
            gameplay.xdir = 1;
        }
        else if (left && !right) {
            gameplay.xdir = -1;
        }
        else if (right && !left) {
            gameplay.xdir = 1;
        }
        else if (!right && !left) {
            gameplay.xdir = 0;
        }
        
        Vec2 dir2D(gameplay.xdir, gameplay.ydir);
        f32 speed = 2.f;
        normalizeSafe(dir2D);
        Vec2 speed2D = dir2D * speed;
        scene.entityList[0].sceneComp.x += speed2D.x;
        scene.entityList[0].sceneComp.y += speed2D.y;
    };
    
    Game::Scene scene;
    Game::Entity player;
};

MainLevel mainLevel;

int main(int argc, char **argv) {
    
    {
        glfwInit();
    }
    
    { // Window init
        App::Window& window = app.mainWindow;
        
        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
        
        window.width = 640;
        window.height = 480;
        {
            GLFWmonitor* monitor = nullptr;
            GLFWwindow* share = nullptr;
            const char* title = "2D test";
            window.handle = glfwCreateWindow(window.width, window.height, title, monitor, share);
        }
        window.desiredRatio = window.width / (f32) window.height;
        window.fullscreen = false;
    }
    
    { // Render init
        App::Window& window = app.mainWindow;
        
        glfwMakeContextCurrent(window.handle);
        glfwSwapInterval(1);

        {
            glMatrixMode(GL_PROJECTION);

            // Ortho
            {
                glLoadIdentity();

                Math3D::TransformMatrixCM32 projectionTransform;
                Camera::OrthoParams ortho;
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
        }
    }
    
    {
        App::Input& appInput = app.input;
        glfwSetKeyCallback(app.mainWindow.handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {

			app.input.debugSet.handleEvent(key, action);
			app.input.gameplay.set.handleEvent(key, action);
        });
    }
    
    { // Scene init
        
        Game::Entity& debug = mainLevel.player;
        debug.graphicsComp.customRender = [](f32 x, f32 y) {
            const Col gridbottom(1.0f, 1.0f, 1.0f, 1.0f);
            GrcPrimitives::segment(Vec3(x - 10.f, y - 10.f, 0.f), Vec3(x, y + 10.f, 0.f), gridbottom);
            GrcPrimitives::segment(Vec3(x, y + 10.f, 0.f), Vec3(x + 10.f, y - 10.f, 0.f), gridbottom);
            GrcPrimitives::segment(Vec3(x + 10.f, y - 10.f, 0.f), Vec3(x - 10.f, y - 10.f, 0.f), gridbottom);
        };
        debug.sceneComp.x = 0;
        debug.sceneComp.y = 0;
        
        mainLevel.scene.entityListCount = 1;
        mainLevel.scene.entityList = &mainLevel.player;
    }
    
    const u32 targetFramesPerSecond = 60;
    const f64 targetFrameRate = 1.0 / (f64)targetFramesPerSecond;
    
    const f64 maxFrameLength = 0.1;
    const f64 buffer = 0.0001;
    
    app.time.start = glfwGetTime();
    app.time.frame = 0;
    f64 lastFrameTimeStamp = app.time.start - targetFrameRate;
    while (!glfwWindowShouldClose(app.mainWindow.handle)) {
        app.time.now = glfwGetTime();
        app.time.running = app.time.now - app.time.start;
        const f64 frameDelta = fminf(app.time.now - lastFrameTimeStamp, maxFrameLength);
        
        if (frameDelta + buffer > targetFrameRate) {
            lastFrameTimeStamp = app.time.now;
            app.time.frame++;
            
            { // Tick
                
                { // Input update
                    app.input.debugSet.update();
                    app.input.gameplay.set.update();
                    
                    glfwPollEvents();
                }
                if (app.input.debugSet.released(Input::DebugSet::EXIT)) {
                    glfwSetWindowShouldClose(app.mainWindow.handle, 1);
                }
                
                { // Scene update
                    mainLevel.processControl(mainLevel.scene, app.input);
                }
                
                { // Render update
                    glClearColor(0.f, 0.f, 0.f, 1.f);
                    glClear(GL_COLOR_BUFFER_BIT);
                    
                    for (s32 i = 0; i < mainLevel.scene.entityListCount; i++) {
                        Game::Entity& entity = mainLevel.scene.entityList[i];
                        if (entity.graphicsComp.customRender) {
                            entity.graphicsComp.customRender(entity.sceneComp.x, entity.sceneComp.y);
                        }
                    }
                    
                    // Needed since GLFW_DOUBLEBUFFER is GL_TRUE
                    glfwSwapBuffers(app.mainWindow.handle);
                }
            }
        }
    }
    
    glfwDestroyWindow(app.mainWindow.handle);
    glfwTerminate();
    
    return 0;
}
