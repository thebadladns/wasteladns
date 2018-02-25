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
#include "helpers/controller_render.h"

namespace Motion {

    struct Agent {

		struct Config {
			f32 maxBoost = 1.25f;
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
        Input::Gamepad::State* pad;
        f32 timeDelta;
    };
    static void updateMovement(UpdateMovementParams& params) {
        
        using namespace Input::Gamepad;
        
        Agent& agent = *params.agent;
        State& pad = *params.pad;
        
        Vec2& inputDirWS = agent.inputDirWS;
        f32 inputMag = 0.f;
        f32 inputBoost = 1.f;
        
        // TODO: handle deadzone internally
        const f32 deadzone = 0.075f;
        
        if (pad.active) {
            Vec2 axis_l(pad.analogs.values[Analog::AxisLH], -pad.analogs.values[Analog::AxisLV]);
			inputMag = Vec::mag(axis_l);
            if (inputMag > deadzone) {
                inputDirWS = Vec::scale(axis_l, 1.f/ inputMag);
                agent.analog = true;
            } else {
				inputMag = 0.f;
            }
            
            const f32 trigger_r = pad.analogs.values[Analog::Trigger_R];
            if (trigger_r != Analog::novalue) {
                const f32 triggerNormalized = Math::bias(trigger_r);
				inputBoost = (1.f + triggerNormalized * (agent.config.maxBoost - 1.f));
            } else if (pad.buttons.down(Digital::R2)) {
				inputBoost = agent.config.maxBoost;
            }
        }
        
        if (inputMag < deadzone) {

            // Need to reset upong transition to analog: digital state depends on status of button presses
            if (agent.analog) {
                inputDirWS.x = 0.f;
                inputDirWS.y = 0.f;
                agent.analog = false;
            }
            
            inputDirWS.y -= 3 * pad.buttons.pressed(Digital::D_D) + pad.buttons.down(Digital::D_D) - pad.buttons.released(Digital::D_D);
            inputDirWS.y += 3 * pad.buttons.pressed(Digital::D_U) + pad.buttons.down(Digital::D_U) - pad.buttons.released(Digital::D_U);
            inputDirWS.x -= 3 * pad.buttons.pressed(Digital::D_L) + pad.buttons.down(Digital::D_L) - pad.buttons.released(Digital::D_L);
            inputDirWS.x += 3 * pad.buttons.pressed(Digital::D_R) + pad.buttons.down(Digital::D_R) - pad.buttons.released(Digital::D_R);
            inputDirWS.x = Math::clamp(inputDirWS.x, -1.f, 1.f);
            inputDirWS.y = Math::clamp(inputDirWS.y, -1.f, 1.f);
            inputMag = Vec::mag(inputDirWS);
        }
        
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

	struct View {
		Camera::OrthoParams orthoParams;
		Camera::FrustumParams frustumParams;
		Math3D::Transform32 orthoTransformCM;
		Math3D::Transform64 frustumTransformCM;
	};
    
    struct Player {
        Motion::Agent motion;
    };

	struct Instance {
		Time time;
		View view;
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

	struct Input {
        ::Input::DebugState debugSet;
	};

	struct Instance {
		Window mainWindow;
		Time time;
		Game::Instance game;
		Input input;
	};
};

App::Instance app;
int main(int argc, char** argv) {

    ControllerVertex::RenderBuffers controllerBuffers;
    ControllerVertex::parsetree_svg(controllerBuffers, ControllerVertex::svg);
    
	if (glfwInit())
	{
		App::Window& window = app.mainWindow;

		glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
		window.width = 640;
		window.height = 480;
		window.desiredRatio = window.width / (f32)window.height;
		window.fullscreen = false;
		window.handle = glfwCreateWindow(
			window.width
			, window.height
			, "Gamepad test"
			, nullptr /*monitor*/
			, nullptr /*share*/
		);
		if (window.handle)
		{
			Game::Instance& game = app.game;

			// Input setup
			glfwSetInputMode(app.mainWindow.handle, GLFW_STICKY_KEYS, 1);

			// Render setup
			glfwMakeContextCurrent(window.handle);
			glfwSwapInterval(1);
			{
				Camera::OrthoParams& ortho = game.view.orthoParams;
				ortho.right = app.mainWindow.width * 0.5f;
				ortho.top = app.mainWindow.height * 0.5f;
				ortho.left = -ortho.right;
				ortho.bottom = -ortho.top;
				ortho.near = -1.f;
				ortho.far = 200.f;
				Math3D::Transform32& orthoTransformCM = game.view.orthoTransformCM;
				Camera::computeProjectionMatrix(ortho, orthoTransformCM);

				Camera::FrustumParams& frustum = game.view.frustumParams;
				frustum.fov = 60.0;
				frustum.aspect = 1.0;
				frustum.near = 1.0;
				frustum.far = 600.0;
                Math3D::Transform64& perspectiveTransformCM = game.view.frustumTransformCM;
                Camera::computeProjectionMatrix(frustum, perspectiveTransformCM);
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
                        Input::pollState(app.input.debugSet, Input::DebugSet::mapping, window.handle);
                        Input::pollState(game.pad, GLFW_JOYSTICK_1, window.handle);

						// Logic
                        if (app.input.debugSet.released(Input::DebugSet::Keys::ESC)) {
							glfwSetWindowShouldClose(app.mainWindow.handle, 1);
						}
                        
                        // Locomotion update
                        {
                            using namespace Motion;
                            
                            UpdateMovementParams locoParams;
                            locoParams.agent = &game.player.motion;
                            locoParams.pad = &game.pad;
                            locoParams.timeDelta = (f32)game.time.config.targetFramerate;
                            updateMovement(locoParams);
                        }

						// Render update
						glClearColor(0.f, 0.f, 0.f, 1.f);
						glClear(GL_COLOR_BUFFER_BIT);
						{
                            // ORTHO
                            {
                                using namespace ControllerVertex;
                                
                                glMatrixMode(GL_PROJECTION);
                                glLoadIdentity();
                                glMultMatrixf(game.view.orthoTransformCM);
                                
                                glMatrixMode(GL_MODELVIEW);
                                glPushMatrix();
                                {
                                    const f32 scale = 0.4f;
                                    const f32 controllerHeight = controllerBuffers.max.y - controllerBuffers.min.y;
                                    const f32 scaledControllerHeight = scale * controllerHeight;
                                    const f32 ypadding = scale * 20.f;
                                    glTranslatef(0.0f, game.view.orthoParams.bottom + 0.5f * scaledControllerHeight + ypadding, 0.f);
                                    glScalef(scale, scale, 1.f);
                                    
                                    const Col padColor(1.0f, 1.0f, 1.0f, 1.0f);
                                    glColor4f(RGBA_PARAMS(padColor));
                                    
                                    glEnableClientState(GL_VERTEX_ARRAY);
                                    
                                    // Main shape
                                    for (RenderBuffer& buffer : controllerBuffers.sbuffers) {
                                        glVertexPointer(2, GL_FLOAT, 0, buffer.vertex);
                                        glDrawArrays(GL_LINE_LOOP, 0, buffer.count);
                                    }
                                    
                                    // Digital buttons
                                    for (u32 i = 0; i < (s32) DynamicShape::ButtonEnd; i++) {
                                        const RenderBuffer& buffer = controllerBuffers.dbuffers[i];

                                        s32 primitive = GL_LINE_LOOP;
                                        if (game.pad.buttons.down(shape2button_mapping[i])) {
                                            primitive = GL_TRIANGLE_FAN;
                                        }

                                        glVertexPointer(2, GL_FLOAT, 0, buffer.vertex);
                                        glDrawArrays(primitive, 0, buffer.count);
                                    }
                                    
                                    // Axis
                                    const f32 axisMovementMag = 5.f;
                                    for (u32 i = (s32)DynamicShape::AxisStart; i < (s32)DynamicShape::AxisEnd; i++) {
                                        
                                        const s32 offset_i = i - (s32)DynamicShape::AxisStart;
                                        const f32 axis_l = game.pad.analogs.values[(s32)axis2analog_mapping[offset_i]];
                                        const f32 axis_r = game.pad.analogs.values[(s32)axis2analog_mapping[offset_i] + 1];
                                        if (axis_l == Input::Gamepad::Analog::novalue) {
                                            continue;
                                        }
                                        if (axis_r == Input::Gamepad::Analog::novalue) {
                                            continue;
                                        }
                                        
                                        const RenderBuffer& buffer = controllerBuffers.dbuffers[i];
                                        f32 xoffset = axisMovementMag * axis_l;
                                        f32 yoffset = - axisMovementMag * axis_r;
                                        
                                        s32 primitive = GL_LINE_LOOP;
                                        if (game.pad.buttons.down(shape2button_mapping[i])) {
                                            primitive = GL_TRIANGLE_FAN;
                                        }
                                        
                                        glMatrixMode(GL_MODELVIEW);
                                        glPushMatrix();
                                        {
                                            glTranslatef(xoffset, yoffset, 0.f);
                                            glVertexPointer(2, GL_FLOAT, 0, buffer.vertex);
                                            glDrawArrays(primitive, 0, buffer.count);
                                        }
                                        glPopMatrix();
                                    }
                                    
                                    // Triggers
                                    for (u32 i = (s32)DynamicShape::TriggerStart; i < (s32)DynamicShape::TriggerEnd; i++) {

                                        const s32 offset_i = i - (s32)DynamicShape::TriggerStart;
                                        const f32 trigger_raw = game.pad.analogs.values[(s32)trigger2analog_mapping[offset_i]];
                                        f32 trigger = 0.f;
                                        if (trigger_raw != Input::Gamepad::Analog::novalue) {
                                            trigger = Math::bias(trigger_raw);
                                        }
                                        
                                        const RenderBuffer& buffer = controllerBuffers.dbuffers[i];
                                        const Vec2 center = Vec::scale(Vec::add(buffer.min, buffer.max), 0.5f);
                                        
                                        
                                        s32 primitive = GL_LINE_LOOP;
                                        if (game.pad.buttons.down(shape2button_mapping[i])) {
                                            primitive = GL_TRIANGLE_FAN;
                                        }
                                        
                                        glMatrixMode(GL_MODELVIEW);
                                        glPushMatrix();
                                        {
                                            f32 yscale = Math::lerp(trigger, 1.f, 0.4f);
                                            f32 height = buffer.max.y - buffer.min.y;
                                            f32 scaledHeight = yscale * height;
                                            glTranslatef(center.x, center.y, 0.f);
                                            glScalef(1.f, yscale, 1.f);
                                            glTranslatef(-center.x, -center.y - (height - scaledHeight), 0.f);
                                            
                                            glVertexPointer(2, GL_FLOAT, 0, buffer.vertex);
                                            glDrawArrays(primitive, 0, buffer.count);
                                        }
                                        glPopMatrix();
                                    }
                                    glDisableClientState(GL_VERTEX_ARRAY);
                                }
                                glPopMatrix();

                                Vec3 debugPos = Vec3(game.view.orthoParams.left + 10.f, game.view.orthoParams.top - 10.f, -50);
                                const Col textColor(1.0f, 1.0f, 1.0f, 1.0f);
                                
                                DebugDraw::TextParams textParams;
                                textParams.scale = 1.f;
                                textParams.pos = debugPos;
                                textParams.text = "Joystick data";
                                DebugDraw::text(textParams);
                                debugPos.y -= 15.f * textParams.scale;

                                const s32 joystickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1);
                                if (joystickPresent) {
                                    char buffer[128];

                                    textParams.pos = debugPos;
                                    textParams.text = glfwGetJoystickName(GLFW_JOYSTICK_1);
                                    DebugDraw::text(textParams);
                                    debugPos.y -= 15.f * textParams.scale;

                                    s32 axesCount;
                                    const f32* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
                                    snprintf(buffer, sizeof(buffer), "Axes: ");
                                    for (s32 i = 0; i < axesCount; i++) {
                                        char axisBuff[32];
                                        snprintf(axisBuff, sizeof(axisBuff), " %d:%.3f", i, axes[i]);
                                        strncat(buffer, axisBuff, Math::min(sizeof(buffer)-strlen(axisBuff), strlen(axisBuff)));
                                    }
                                    textParams.pos = debugPos;
                                    textParams.text = buffer;
                                    DebugDraw::text(textParams);
                                    debugPos.y -= 15.f * textParams.scale;

                                    s32 buttonCount;
                                    const u8* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
                                    snprintf(buffer, sizeof(buffer), "Buttons: ");
                                    for (s32 i = 0; i < buttonCount; i++) {
                                        char buttonBuff[32];
                                        snprintf(buttonBuff, sizeof(buttonBuff), " %d:%x", i, buttons[i]);
                                        strncat(buffer, buttonBuff, Math::min(sizeof(buffer)-strlen(buttonBuff), strlen(buttonBuff)));
                                    }
                                    textParams.pos = debugPos;
                                    textParams.text = buffer;
                                    DebugDraw::text(textParams);
                                    debugPos.y -= 15.f * textParams.scale;
                                }
                            }
                            
                            // PERSPECTIVE
                            {
                                glMatrixMode(GL_PROJECTION);
                                glLoadIdentity();
                                glMultMatrixd(game.view.frustumTransformCM);
                                
                                glMatrixMode(GL_MODELVIEW);
                                glPushMatrix();
                                {
                                    glRotatef(-10.f, 1.f, 0.f, 0.f);
                                    glTranslatef(0.f, 0.f, -200.f);
                                    
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
			glfwDestroyWindow(app.mainWindow.handle);
		}
		glfwTerminate();
	}

	return 0;
}
