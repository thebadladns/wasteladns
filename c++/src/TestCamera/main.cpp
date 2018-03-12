#include "GLFW/glfw3.h"
#define __WASTELADNS_GLFW3_H__

// C libs
#include <stdlib.h>
#define __WASTELADNS_C_STDLIB_H__
#include <math.h>
#define __WASTELADNS_C_MATH_H__
#include <stdio.h>
#define __WASTELADNS_C_STDIO_H__
#include <limits>
#define __WASTELADNS_C_LIMITS_H__
#include <cstring>
#define __WASTELADNS_C_CSTRING_H__
#include <string>
#define __WASTELADNS_C_STRING_H__
#include <map>
#define __WASTELADNS_C_MAP_H__
#include <functional>
#define __WASTELADNS_C_FUNCTIONAL_H__

// Core
#include "helpers/types.h"
#define __WASTELADNS_MATH_IMPL__
#include "helpers/math.h"
#define __WASTELADNS_EASING_IMPL__
#include "helpers/easing.h"
#define __WASTELADNS_ANGLE_IMPL__
#include "helpers/angle.h"
#define __WASTELADNS_VEC_IMPL__
#include "helpers/vec.h"
#define __WASTELADNS_COLOR_IMPL__
#include "helpers/color.h"
#define __WASTELADNS_DEBUGDRAW_IMPL__
#define __WASTELADNS_DEBUGDRAW_TEXT__
#include "helpers/debugdraw.h"
#include "helpers/camera.h"

// TODO: understand this and find a proper spot for this
namespace Hash {
    u32 fnv(const char* name) {
        const u8* data = (const u8*)name;
        u32 val = 3759247821;
        while(*data){
            val ^= *data++;
            val *= 0x01000193;
        }
        val &= 0x7fffffff;
        val |= val==0;
        return val;
    }
}

#include "helpers/input.h"

namespace Motion {

    struct Agent {

		struct Config {
			f32 maxBoost = 1.5f;
			f32 maxSpeed = 200.f;

			f32 accHorizon = 0.5f;
			f32 accHorizonStop = 0.25f;
			f32 turnHorizon = 0.14f;
		};

		Config config;
        Vec2 pos;
        Vec2 dir;
        Vec2 inputDirWS;
        f32 orientation;
		f32 speed;
        bool analog = true;
    };
    
    struct UpdateMovementParams {
        Agent* agent;
        Vec2 analog_dir;
        f32 timeDelta;
        f32 analog_mag;
        f32 boost;
        bool input_up_down;
        bool input_up_released;
        bool input_up_pressed;
        bool input_down_down;
        bool input_down_released;
        bool input_down_pressed;
        bool input_right_down;
        bool input_right_released;
        bool input_right_pressed;
        bool input_left_down;
        bool input_left_released;
        bool input_left_pressed;
        bool analog;
    };
    static void updateMovement(UpdateMovementParams& params) {
        
        Agent& agent = *params.agent;
        
        Vec2& inputDirWS = agent.inputDirWS;
        f32 inputMag = 0.f;
        f32 inputBoost = (1.f + params.boost * (agent.config.maxBoost - 1.f));
        
        if (params.analog) {
            inputDirWS = params.analog_dir;
            inputMag = params.analog_mag;
        } else {
            // Need to reset upong transition to analog: digital state depends on status of button presses
            if (agent.analog) {
                inputDirWS.x = 0.f;
                inputDirWS.y = 0.f;
                agent.analog = false;
            }
            
            inputDirWS.y -= 3 * params.input_down_pressed + params.input_down_down - params.input_down_released;
            inputDirWS.y += 3 * params.input_up_pressed + params.input_up_down - params.input_up_released;
            inputDirWS.x -= 3 * params.input_left_pressed + params.input_left_down - params.input_left_released;
            inputDirWS.x += 3 * params.input_right_pressed + params.input_right_down - params.input_right_released;
            
            inputDirWS.x = Math::clamp(inputDirWS.x, -1.f, 1.f);
            inputDirWS.y = Math::clamp(inputDirWS.y, -1.f, 1.f);
            inputMag = Vec::mag(inputDirWS);
        }
        agent.analog = params.analog;
        
        inputMag = Math::min(1.f, inputMag);
        f32 desiredSpeed = inputMag * agent.config.maxSpeed * inputBoost;
        f32 accHorizon = agent.config.accHorizon;
        if (inputMag <= Math::eps<f32>) {
            accHorizon = agent.config.accHorizonStop;
        }
        
        f32 currentOrientationLS = 0.f, inputOrientationLS = 0.f;
		if (inputMag > 0.001f) {
			f32 inputHeadingWS = Angle::orientation(inputDirWS);

			currentOrientationLS = 0.f;
			inputOrientationLS = Angle::subtractShort(inputHeadingWS, agent.orientation);

			currentOrientationLS = Math::eappr(currentOrientationLS, inputOrientationLS, agent.config.turnHorizon, params.timeDelta);
			agent.orientation = Angle::wrap(agent.orientation + currentOrientationLS);
			agent.dir = Angle::direction(agent.orientation);

			const f32 minReductionAngle1 = 40.f * Angle::d2r<f32>;
			const f32 maxReductionAngle1 = 120.f * Angle::d2r<f32>;
            const f32 minReductionAngle2 = maxReductionAngle1;
            const f32 maxReductionAngle2 = 150.f * Angle::d2r<f32>;
            const f32 angleDelta = Math::abs(inputOrientationLS);
            f32 turnFactor = 0.f;
            f32 turnSpeedReduction = 1.f;
            if (angleDelta < maxReductionAngle1) {
                turnFactor = Math::clamp((angleDelta - minReductionAngle1) / (maxReductionAngle1 - minReductionAngle1), 0.f, 1.f);
                turnSpeedReduction = Math::lerp(turnFactor, 1.f, 0.6f);
            } else {
                turnFactor = Math::clamp((angleDelta - minReductionAngle2) / (maxReductionAngle2 - minReductionAngle2), 0.f, 1.f);
                turnSpeedReduction = Math::lerp(turnFactor, 0.6f, 0.25f);
            }
			
			desiredSpeed *= turnSpeedReduction;
		}
        
        agent.speed = Math::eappr(agent.speed, desiredSpeed, accHorizon, params.timeDelta);
        
        if (Math::abs(inputOrientationLS) * Angle::r2d<f32> > 150.f) {
            agent.speed = 0.f;
        }
        
        agent.pos = Vec::add(agent.pos, Vec::scale(agent.dir, agent.speed * params.timeDelta));
    }
}

namespace Game {

	struct Time {
		struct Config {
			f64 targetFramerate;
			f64 maxFrameLength;
		};

		Config config;
		f64 lastFrame;
		f64 lastFrameDelta;
		f64 nextFrame;
		s64 frame;
	};

    struct Player {
        Motion::Agent motion;
    };

	struct Instance {
		Time time;
        Camera::ProjectionConfig projection;
        Camera::Instance camera;
        Input::Gamepad::State pad;
        
        Player player;
	};
};

namespace App {

	struct Time {
		f64 running;
		f64 start;
		f64 now;
	};

	struct Window {
		GLFWwindow* handle;
		f32 desiredRatio;
		u32 width;
		u32 height;
		bool fullscreen = false;
	};

	namespace Input {
        
        struct Keys {
            enum Enum {
                  FLYCAM_UP
                , FLYCAM_DOWN
                , FLYCAM_LEFT
                , FLYCAM_RIGHT
                , ESC
                , COUNT
            };
        };
        typedef s32 Mapping[Keys::COUNT];
        const Mapping mapping = {
              GLFW_KEY_W
            , GLFW_KEY_S
            , GLFW_KEY_A
            , GLFW_KEY_D
            , GLFW_KEY_ESCAPE
        };
        
        typedef ::Input::DigitalState<Keys::COUNT> DebugSet;
	};

	struct Instance {
		Window mainWindow;
		Time time;
		Game::Instance game;
        Input::DebugSet input;
	};
};

App::Instance app;
int main(int argc, char** argv) {
    
	if (!glfwInit()) {
        return 0;
    }
    
    App::Window& window = app.mainWindow;

    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    window.width = 640;
    window.height = 480;
    window.desiredRatio = window.width / (f32)window.height;
    window.fullscreen = false;
    window.handle = glfwCreateWindow(
        window.width
        , window.height
        , "Camera test"
        , nullptr /*monitor*/
        , nullptr /*share*/
    );
    if (!window.handle) {
        return 0;
    }
        
    Game::Instance& game = app.game;

    // Input setup
    glfwSetInputMode(app.mainWindow.handle, GLFW_STICKY_KEYS, 1);

    // Render setup
    glfwMakeContextCurrent(window.handle);
    glfwSwapInterval(1);
    // Projection
    {
        Camera::OrthoParams& ortho = game.projection.orthoParams;
        ortho.right = app.mainWindow.width * 0.5f;
        ortho.top = app.mainWindow.height * 0.5f;
        ortho.left = -ortho.right;
        ortho.bottom = -ortho.top;
        ortho.near = -1.f;
        ortho.far = 200.f;
        Math3D::Transform32& orthoTransformCM = game.projection.orthoTransformCM;
        Camera::computeProjectionMatrix(ortho, orthoTransformCM);

        Camera::FrustumParams& frustum = game.projection.frustumParams;
        frustum.fov = 60.0;
        frustum.aspect = 1.0;
        frustum.near = 1.0;
        frustum.far = 600.0;
        Math3D::Transform64& perspectiveTransformCM = game.projection.frustumTransformCM;
        Camera::computeProjectionMatrix(frustum, perspectiveTransformCM);
    }
    // Camera
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glRotatef(-10.f, 1.f, 0.f, 0.f);
        glTranslatef(0.f, 0.f, -200.f);
        glGetFloatv(GL_MODELVIEW_MATRIX, game.camera.matrixCM);
        glPopMatrix();
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
    // Scene set up
    game.player.motion.pos = Vec2(0.f, 0.f);
    game.player.motion.dir = game.player.motion.inputDirWS = Vec2(0.f, 0.f);
    game.player.motion.speed = 0.f;

    game.time.config.maxFrameLength = 0.1;
    game.time.config.targetFramerate = 1.0 / 60.0;
    app.time.now = app.time.start = glfwGetTime();
    game.time.lastFrame = game.time.nextFrame = app.time.now;
    game.time.frame = 0;
    do {
        if (app.time.now >= game.time.nextFrame) {
            game.time.lastFrameDelta = Math::min(app.time.now - game.time.lastFrame, game.time.config.maxFrameLength);
            game.time.lastFrame = app.time.now;
            game.time.nextFrame = app.time.now + game.time.config.targetFramerate;

            {
                using namespace Game;

                // Input
                glfwPollEvents();
                Input::pollState(app.input, App::Input::mapping, window.handle);
                Input::pollState(game.pad, GLFW_JOYSTICK_1, window.handle);
                
                // Logic
                if (app.input.released(App::Input::Keys::ESC)) {
                    glfwSetWindowShouldClose(app.mainWindow.handle, 1);
                }
                
                // Locomotion update
                {
                    using namespace Motion;
                    using namespace Input::Gamepad;
                    
                    UpdateMovementParams motionParams;
                    motionParams.agent = &game.player.motion;
                    motionParams.timeDelta = (f32)game.time.config.targetFramerate;
                    
                    if (game.pad.active) {
                        motionParams.analog_dir = Vec2(game.pad.analogs.values[Analog::AxisLH], -game.pad.analogs.values[Analog::AxisLV]);
                        motionParams.analog_mag = Vec::mag(motionParams.analog_dir);
                        motionParams.analog = motionParams.analog_mag > Math::eps<f32>;
                        if (motionParams.analog) {
                            motionParams.analog_dir = Vec::scale(motionParams.analog_dir, 1.f/ motionParams.analog_mag);
                        } else {
                            const auto& buttons = game.pad.buttons;
                            motionParams.input_up_down = buttons.down(Digital::D_U);
                            motionParams.input_up_released = buttons.released(Digital::D_U);
                            motionParams.input_up_pressed = buttons.pressed(Digital::D_U);
                            motionParams.input_down_down = buttons.down(Digital::D_D);
                            motionParams.input_down_released = buttons.released(Digital::D_D);
                            motionParams.input_down_pressed = buttons.released(Digital::D_D);
                            motionParams.input_right_down = buttons.down(Digital::D_R);
                            motionParams.input_right_released = buttons.released(Digital::D_R);
                            motionParams.input_right_pressed = buttons.pressed(Digital::D_R);
                            motionParams.input_left_down = buttons.down(Digital::D_L);
                            motionParams.input_left_released = buttons.released(Digital::D_L);
                            motionParams.input_left_pressed = buttons.pressed(Digital::D_L);
                        }
                        f32 trigger_r = game.pad.analogs.values[Analog::Trigger_R];
                        if (trigger_r != Analog::novalue) {
                            motionParams.boost = Math::bias(game.pad.analogs.values[Analog::Trigger_R]);
                        } else {
                            motionParams.boost = game.pad.buttons.down(Digital::R2) ? 1.f : 0.f;
                        }
                    } else {
                        // Stop with current dir
                        motionParams.analog_dir = game.player.motion.inputDirWS;
                        motionParams.analog_mag = 0.f;
                        motionParams.analog = true;
                        motionParams.boost = 0.f;
                    }
                    updateMovement(motionParams);
                }
                
                // Camera update
                {
                    Camera::UpdateCameraParams cameraParams;
                    cameraParams.instance = &game.camera;
                    cameraParams.input_up = app.input.down(App::Input::Keys::FLYCAM_UP);
                    cameraParams.input_down = app.input.down(App::Input::Keys::FLYCAM_DOWN);
                    cameraParams.input_left = app.input.down(App::Input::Keys::FLYCAM_LEFT);
                    cameraParams.input_right = app.input.down(App::Input::Keys::FLYCAM_RIGHT);
                    Camera::UpdateCamera(cameraParams);
                }

                // Render update
                glClearColor(0.f, 0.f, 0.f, 1.f);
                glClear(GL_COLOR_BUFFER_BIT);
                {
                    // ORTHO
                    {
                        glMatrixMode(GL_PROJECTION);
                        glLoadMatrixf(game.projection.orthoTransformCM);

                        DebugDraw::TextParams textParams;
                        textParams.scale = 1.f;
                        textParams.pos = Vec3(game.projection.orthoParams.left + 10.f, game.projection.orthoParams.top - 10.f, -50);
                        textParams.text = "Camera data";
                        textParams.color = Col(1.0f, 1.0f, 1.0f, 1.0f);
                        DebugDraw::text(textParams);
                        textParams.pos.y -= 15.f * textParams.scale;
                        
                        char buff[128];
                        Vec3 cameraPos = *((Vec3*)&game.camera.matrixCM[12]);
                        snprintf(buff, sizeof(buff), "Pos " VEC3_FORMAT_LITE, VEC3_PARAMS(cameraPos));
                        textParams.text = buff;
                        DebugDraw::text(textParams);
                        textParams.pos.y -= 15.f * textParams.scale;
                    }
                    
                    // PERSPECTIVE
                    {
                        glMatrixMode(GL_PROJECTION);
                        glLoadMatrixd(game.projection.frustumTransformCM);
                        
                        glMatrixMode(GL_MODELVIEW);
                        glPushMatrix();
                        {
                            glLoadMatrixf(game.camera.matrixCM);
                            
                            // Tiled floor
                            const f32 l = -200.;
                            const f32 r = -l;
                            const f32 b = -200.;
                            const f32 t = -b;
                            const f32 z = -0.f;
                            const f32 separation = 20.f;
                            const Col gridColor(1.0f, 1.0f, 1.0f, 0.25f);
                            for (f32 x = l; x < r + 0.001; x += separation) {
                                DebugDraw::segment(Vec3(x, b, z), Vec3(x, t, z), gridColor);
                            }
                            for (f32 y = b; y < t + 0.001; y += separation) {
                                DebugDraw::segment(Vec3(l, y, z), Vec3(r, y, z), gridColor);
                            }
                            
                            Motion::Agent& motion = game.player.motion;
                            
                            f32 x = motion.pos.x;
                            f32 y = motion.pos.y;
                            
                            glTranslatef(x, y, 0.f);
                            glRotatef(motion.orientation * Angle::r2d<f32>, 0.f, 0.f, -1.f);
                            
                            f32 w = 5.f;
                            f32 h = 10.f;
                            const Col playerColor(1.0f, 1.0f, 1.0f, 1.0f);
                            DebugDraw::segment(Vec3(-w, -h, 0.f), Vec3(0.f, h, 0.f), playerColor);
                            DebugDraw::segment(Vec3(0.f, h, 0.f), Vec3(w, -h, 0.f), playerColor);
                            DebugDraw::segment(Vec3(w, -h, 0.f), Vec3(-w, -h, 0.f), playerColor);
                        }
                        glPopMatrix();
                    }
                    
                }
                // Needed since GLFW_DOUBLEBUFFER is GL_TRUE
                glfwSwapBuffers(app.mainWindow.handle);
            }
        }

        app.time.now = glfwGetTime();
        app.time.running = app.time.now - app.time.start;
    } while (!glfwWindowShouldClose(app.mainWindow.handle));
    
    // Internally handles all glfwDestroyWindow
    glfwTerminate();
    
	return 0;
}
