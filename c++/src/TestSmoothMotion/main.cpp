#include "GLFW/glfw3.h"

#define GRC_PRIMITIVES_ENABLE_TEXT 1
#include "lib/stb/stb_easy_font.h"

// C libs
#include <stdlib.h>
#include <stdio.h>
#include <limits>

#include <cstring>

// Core
#include "helpers/Types.h"
#define __WASTELADNS_MATH_IMPL__
#include "helpers/Math.h"
#define __WASTELADNS_ANGLE_IMPL__
#include "helpers/Angle.h"
#define __WASTELADNS_VEC_IMPL__
#include "helpers/Vec.h"
#define __WASTELADNS_COLOR_IMPL__
#include "helpers/Color.h"
#define __WASTELADNS_ARRAY_IMPL__
#include "helpers/Array.h"

// Rest
#define __WASTELADNS_DEBUGDRAW_IMPL__
#define __WASTELADNS_DEBUGDRAW_TEXT__
#include "helpers/debugdraw.h"

namespace Input {
    
    namespace DebugSet {
        enum Keys : u8 {
              ESC = 1 << 0
            , EXIT = ESC
            , SPACE = 1 << 1
            , COUNT = 3
            , CLEAR = 0
        };
        typedef s32 Mapping[Keys::COUNT];
        const Mapping mapping = {
              GLFW_KEY_ESCAPE
            , GLFW_KEY_SPACE
        };
    };
    
    template<typename _Set, const s32* _mapping>
    struct DigitalState {
        bool down(_Set key) const {
            return (current & key) != 0;
        }
        bool up(_Set key) const {
            return (current & key) == 0;
        }
        bool released(_Set key) const {
            return (current & key) == 0 && (last & key) != 0;
        }
        bool pressed(_Set key) const {
            return (current & key) != 0 && (last & key) == 0;
        }
        void pollState(GLFWwindow* window) {
            last = current;
            current = _Set::CLEAR;
            for (int i = 0; i < _Set::COUNT; i++) {
                s32 keyState = glfwGetKey(window, _mapping[i]);
                s32 keyOn = (keyState == GLFW_PRESS) || (keyState == GLFW_REPEAT);
                current = (_Set) (current | (keyOn << i));
            }
        }
        
        _Set last;
        _Set current;
    };
    
    struct KeyboardState {
        bool down(s32 key) {
            return current[key] != 0;
        }
        bool up(s32 key) {
            return current[key] == 0;
        }
        bool released(s32 key) {
            return current[key] == 0 && last[key] != 0;
        }
        bool pressed(s32 key) {
            return current[key] != 0 && last[key] == 0;
        }
        void pollState(GLFWwindow* window) {
            memcpy(last, current, GLFW_KEY_LAST);
            memset(current, 0, GLFW_KEY_LAST);
            for (int i = 0; i < GLFW_KEY_LAST; i++) {
                current[i] = glfwGetKey(window, i);
            }
        }
        
        u8 last[GLFW_KEY_LAST];
        u8 current[GLFW_KEY_LAST];
    };
};

namespace Math3D {
    // CM = Column Major
    typedef f32 TransformMatrixCM32[16];
    typedef f64 TransformMatrixCM64[16];
};

namespace EaseCurves {
    
    template <typename _T>
    _T lappr(_T current, _T target, _T rate, _T timeDelta) {
        if (current < target) {
            return Math<_T>::min(current + rate * timeDelta, target);
        } else {
            return Math<_T>::max(current - rate * timeDelta, target);
        }
    }
    template f32 lappr<f32>(f32 current, f32 target, f32 rate, f32 timeDelta);
    template f64 lappr<f64>(f64 current, f64 target, f64 rate, f64 timeDelta);

    template <typename _T>
    _T eappr(_T curr, _T target, _T timeHorizon, _T timeDelta) {
        
        curr = target + (curr - target) / Math<_T>::exp_taylor( timeDelta * Math<_T>::e / timeHorizon );
        
        return curr;
    };
    template f32 eappr<f32>(f32 curr, f32 target, f32 timeHorizon, f32 timeDelta);
    template f64 eappr<f64>(f64 curr, f64 target, f64 timeHorizon, f64 timeDelta);
    
    template <typename _T>
    _T subsampled_eappr(_T curr, _T target, _T timeHorizon, _T timeDelta, _T stepTime) {
        
        s32 steps = (s32)(0.5f + timeDelta / stepTime);

        _T t = 0.f;
        for (s32 i = 0; i < steps; i++) {
            curr = target + (curr - target) / Math<_T>::exp_taylor(stepTime * Math<_T>::e / (timeHorizon - t));
            t += stepTime;
        }
        curr = target + (curr - target) / Math<_T>::exp_taylor((timeDelta - t) * Math<_T>::e / (timeHorizon - timeDelta));

        return curr;
    };
    template f32 subsampled_eappr<f32>(f32 curr, f32 target, f32 timeHorizon, f32 timeDelta, f32 stepTime);
    template f64 subsampled_eappr<f64>(f64 curr, f64 target, f64 timeHorizon, f64 timeDelta, f64 stepTime);
}

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
        const f64 xMax = params.near * tan(params.fov * 2.f * Angle<f64>::d2r);
        const f64 xMin = -xMax;
        const f64 yMin = xMin / params.aspect;
        const f64 yMax = xMax / params.aspect;
        
		memset(matrix, 0, sizeof(matrix));
        matrix[0] = (2.0 * params.near) / (xMax - xMin);
        matrix[5] = (2.0 * params.near) / (yMax - yMin);
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

namespace App {
    
    struct Time {
        
        struct Config {
            f64 targetFrameRate;
            f64 maxFrameLength;
        };
        
        Config config;
        
        // In seconds
        f64 running;
        f64 start;
        f64 now;
        f64 lastFrame;
        f64 lastFrameDelta;
        f64 tillNextFrame;
        
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
        ::Input::KeyboardState keyboardSet;
    };
    
    struct Instance {
        Window mainWindow;
        Camera::OrthoParams orthoParams;
        Time time;
        Input input;
    };
};
App::Instance app;

namespace Game {
    
    namespace PlayerSet {
        enum Keys : u8 {
              UP = 1 << 0
            , DOWN = 1 << 1
            , LEFT = 1 << 2
            , RIGHT = 1 << 3
            , COUNT = 4
            , CLEAR = 0
        };
        typedef s32 Mapping[Keys::COUNT];
        extern const Mapping mapping = {
              GLFW_KEY_W
            , GLFW_KEY_S
            , GLFW_KEY_A
            , GLFW_KEY_D
        };
    };
    typedef ::Input::DigitalState<PlayerSet::Keys, PlayerSet::mapping> PlayerInput;
    
    namespace Motion {

        namespace Private {
            enum { historyMaxCount = 200 };
            typedef f32 History[historyMaxCount];
            History inputDirWSHistory = {};
            History dirHistory = {};
            s32 historyCount = historyMaxCount;
            
            enum EaseTypes : s32 { Linear, ExpSimple, ExpNumerical, ExpTest, None, Count };
            s32 easeType = EaseTypes::ExpTest;
        }
        
        struct Controller {
            Vec2 pos;
            Vec2 dir;
            Vec2 inputDirWS;
            f32 heading;
        };

        struct UpdateMovementParams {
            Controller* controller;
            PlayerInput* inputSet;
            f32 timeDelta;
        };
        static void updateMovement(UpdateMovementParams& params) {

            using namespace Game::PlayerSet;
            
            Controller& controller = *params.controller;
            auto& input = *params.inputSet;
            
            Vec2& inputDirWS = controller.inputDirWS;
            inputDirWS.y -= 3 * input.pressed(DOWN) + input.down(DOWN) - input.released(DOWN);
            inputDirWS.y += 3 * input.pressed(UP) + input.down(UP) - input.released(UP);
            inputDirWS.x -= 3 * input.pressed(LEFT) + input.down(LEFT) - input.released(LEFT);
            inputDirWS.x += 3 * input.pressed(RIGHT) + input.down(RIGHT) - input.released(RIGHT);
            inputDirWS.x = Math<f32>::clamp(inputDirWS.x, -1.f, 1.f);
            inputDirWS.y = Math<f32>::clamp(inputDirWS.y, -1.f, 1.f);
            
            f32 speed = 0.f;
            if (inputDirWS.x != 0 || inputDirWS.y != 0) {
                f32 inputHeadingWS = Angle<f32>::heading(inputDirWS);
                
                f32 currentHeadingLS = 0.f;
                f32 inputHeadingLS = Angle<f32>::shortestDelta(inputHeadingWS, controller.heading);
                
                switch (Private::easeType) {
                    default:
                    case Private::EaseTypes::Linear:
                        currentHeadingLS = EaseCurves::lappr(currentHeadingLS, inputHeadingLS, 10.f, params.timeDelta);
                        break;
                    case Private::EaseTypes::ExpSimple:
                        currentHeadingLS = currentHeadingLS + (inputHeadingLS - currentHeadingLS) * 0.3f;
                        break;
                    case Private::EaseTypes::ExpNumerical:
                        currentHeadingLS = EaseCurves::subsampled_eappr(currentHeadingLS, inputHeadingLS, 0.14f, params.timeDelta, 1 / 60.f);
                        break;
                    case Private::EaseTypes::ExpTest:
                        currentHeadingLS = EaseCurves::eappr(currentHeadingLS, inputHeadingLS, 0.14f, params.timeDelta);
                        break;
                    case Private::EaseTypes::None:
                        currentHeadingLS = inputHeadingLS;
                        break;
                }

                controller.heading = Angle<f32>::modpi(controller.heading + currentHeadingLS);
                controller.dir = Angle<f32>::direction(controller.heading);
                speed = 160.f;
            }
            
            controller.pos = Vec::add(controller.pos, Vec::scale(controller.dir, (speed * params.timeDelta)));
            
            f32 historyData[] = { Angle<f32>::heading(inputDirWS), controller.heading };
            Array::Queue::BatchPush<f32>((f32**)&(Private::inputDirWSHistory), 2, Private::historyCount, Private::historyMaxCount, historyData);
        }
    };
    
    struct Player {
        Motion::Controller motion;
    };
    
    struct Instance {
        PlayerInput playerSet;
        Player player;
        f32 timerate = 1.f;
    };
};

enum {trailMaxCount = 100 };
Vec2 trail[trailMaxCount];
s32 trailCount = 0;

Game::Instance game;

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
                                         , "smooth motion test"
                                         , nullptr /*monitor*/
                                         , nullptr /*share*/
                                         );
        if (window.handle)
        {
            // Input setup
            glfwSetInputMode(app.mainWindow.handle, GLFW_STICKY_KEYS, 1);
            
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
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Scene set up
            game.player.motion.pos = Vec2(0.f, 0.f);
            game.player.motion.dir = game.player.motion.inputDirWS = Vec2(0.f, 0.f);
            
            app.time.config.maxFrameLength = 0.1;
            app.time.config.targetFrameRate = 1.0 / 60.0;
            app.time.now = app.time.start = app.time.lastFrame = glfwGetTime();
            app.time.frame = 0;
            app.time.tillNextFrame = 0.0;
            do {
                if (app.time.tillNextFrame <= 0.0001) {
                    app.time.lastFrameDelta = Math<f64>::min(app.time.now - app.time.lastFrame, app.time.config.maxFrameLength);
                    app.time.lastFrame = app.time.now;
                    app.time.frame++;
                    
                    using namespace Game;
                    
                    glfwPollEvents();
                    app.input.keyboardSet.pollState(window.handle);
                    game.playerSet.pollState(window.handle);
                    
                    // Processing
                    {
                        if (app.input.keyboardSet.released(GLFW_KEY_ESCAPE)) {
                            glfwSetWindowShouldClose(app.mainWindow.handle, 1);
                        }
                        if (app.input.keyboardSet.pressed(GLFW_KEY_SPACE)) {
                            
                            s32 inc = 1;
                            if (app.input.keyboardSet.down(GLFW_KEY_LEFT_SHIFT)) {
                                inc = -inc;
                            }
                            Motion::Private::easeType = (Motion::Private::easeType + inc) % (int) (Game::Motion::Private::EaseTypes::Count);
                        }
                        if (app.input.keyboardSet.pressed(GLFW_KEY_0)) {
                            game.timerate = 1.f;
                        }
                        if (app.input.keyboardSet.pressed(GLFW_KEY_8)) {
                            game.timerate -= 0.1f;
                        }
                        if (app.input.keyboardSet.pressed(GLFW_KEY_9)) {
                            game.timerate += 0.1f;
                            game.timerate = Math<f32>::max(game.timerate, 0.01f);
                        }
                        
                        // Locomotion update
                        {
                            using namespace Motion;
                            
                            UpdateMovementParams locoParams;
                            locoParams.controller = &game.player.motion;
                            locoParams.inputSet = &game.playerSet;
                            locoParams.timeDelta = (f32)(app.time.lastFrameDelta * game.timerate);
                            updateMovement(locoParams);
                            
                            if ( trailCount == 0 || Vec::mag(Vec::subtract(trail[trailCount - 1], game.player.motion.pos)) > Math<f32>::eps) {
                                Array::Params posParams;
                                posParams.data = (void*)trail;
                                posParams.count = &trailCount;
                                posParams.maxCount = trailMaxCount;
                                posParams.stride = sizeof(Vec2);
                                Array::Queue::Push(posParams, &game.player.motion.pos);
                            }
                        }
                    }
                    
                    { // Render update
                        glClearColor(0.f, 0.f, 0.f, 1.f);
                        glClear(GL_COLOR_BUFFER_BIT);
                        
                        {
                            using namespace Motion;
                            
                            Controller& motion = game.player.motion;
                            
                            char easeNames[Private::EaseTypes::Count][64] = {
                                  "lappr"
                                , "Simple Exp (frame dependent)"
                                , "Numerical Exp (frame independent)"
                                , "eappr (frame independent?)"
                                , "None"
                            };
                            
                            // Debug text
                            char buffer[128];
                            sprintf(buffer, "player inputDir:%.3f, currentDir:%.3f rate:%.3f",
                                    Angle<f32>::modpi(Angle<f32>::heading(motion.inputDirWS)), motion.heading, game.timerate );
                            
                            Vec3 debugPos = Vec3(app.orthoParams.left + 10.f, app.orthoParams.top - 10.f, 0.f);
                            const Col textColor(1.0f, 1.0f, 1.0f, 1.0f);
                            
                            DebugDraw::TextParams textParams;
                            textParams.scale = 2.f;
                            textParams.pos = debugPos;
                            textParams.text = buffer;
                            DebugDraw::text(textParams);
                            debugPos.y -= 10.f * textParams.scale;
                            
							sprintf(buffer, "Ease:%s", easeNames[Private::easeType]);
                            textParams.pos = debugPos;
                            DebugDraw::text(textParams);
                            debugPos.y -= 20.f * textParams.scale;
                            
                            DebugDraw::GraphParams graphParams;
                            graphParams.topLeft = debugPos;
                            graphParams.w = 600;
                            graphParams.max = Angle<f32>::pi;
                            graphParams.min = -Angle<f32>::pi;
                            graphParams.count = Private::historyCount;
                            
                            graphParams.values = Private::inputDirWSHistory;
                            graphParams.color = Col(0.5f, 0.8f, 0.f, 0.2f);
                            DebugDraw::graph(graphParams);
                            
                            graphParams.values = Private::dirHistory;
                            graphParams.color = Col(0.5f, 0.8f, 0.f, 0.6f);
                            DebugDraw::graph(graphParams);
                            
                            Col trailColor(0.3f, 0.3f, 0.3f, 1.f);
                            for (int i = 1; i < trailCount; i++) {
                                DebugDraw::segment(Vec3(trail[i-1], 0.f), Vec3(trail[i], 0.f), trailColor);
                            }
                        }
                        
                        glMatrixMode(GL_MODELVIEW);
                        glPushMatrix();
                        {
                            Game::Motion::Controller& motion = game.player.motion;
                            
                            f32 x = motion.pos.x;
                            f32 y = motion.pos.y;
                            
                            glTranslatef(x, y, 0.f);
                            glRotatef(motion.heading * Angle<f32>::r2d, 0.f, 0.f, -1.f);
                            
                            f32 w = 5.f;
                            f32 h = 10.f;
                            const Col playerColor(1.0f, 1.0f, 1.0f, 1.0f);
                            DebugDraw::segment(Vec3(-w, -h, 0.f), Vec3(0.f, h, 0.f), playerColor);
                            DebugDraw::segment(Vec3(0.f, h, 0.f), Vec3(w, -h, 0.f), playerColor);
                            DebugDraw::segment(Vec3(w, -h, 0.f), Vec3(-w, -h, 0.f), playerColor);
                        }
                        glPopMatrix();
                        
                        // Needed since GLFW_DOUBLEBUFFER is GL_TRUE
                        glfwSwapBuffers(app.mainWindow.handle);
                    }
                }
                app.time.now = glfwGetTime();
                app.time.running = app.time.now - app.time.start;
                app.time.tillNextFrame = app.time.config.targetFrameRate - (app.time.now - app.time.lastFrame);
            
            } while (!glfwWindowShouldClose(app.mainWindow.handle));
        }
    }
    glfwDestroyWindow(app.mainWindow.handle);
    glfwTerminate();
    
    return 0;
}

