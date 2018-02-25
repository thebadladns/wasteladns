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
#include "helpers/input.h"
#include "helpers/controller_render.h"

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

	struct Instance {
		Time time;
		View view;
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
		::Input::KeyboardState keyboardSet;
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
				Math3D::Transform32& projectionTransformCM = game.view.orthoTransformCM;
				Camera::computeProjectionMatrix(ortho, projectionTransformCM);

				Camera::FrustumParams& frustum = game.view.frustumParams;
				frustum.fov = 60.0;
				frustum.aspect = 1.0;
				frustum.near = 1.0;
				frustum.far = 200.0;
			}

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
//                        f32 timeDelta = (f32)game.time.config.targetFramerate;

						using namespace Game;

						glfwPollEvents();
						app.input.keyboardSet.pollState(window.handle);

						// Logic
						if (app.input.keyboardSet.released(GLFW_KEY_ESCAPE)) {
							glfwSetWindowShouldClose(app.mainWindow.handle, 1);
						}
                        
						f32 axisStates[(s32)ControllerVertex::Analog::Count] = {};
                        bool buttonStates[(s32) ControllerVertex::Digital::Count] = {};
                        
                        const s32 joystickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1);
                        if (joystickPresent) {
                            
                            u32 name = Hash::fnv(glfwGetJoystickName(GLFW_JOYSTICK_1));
                            const ControllerVertex::Digital* b_mapping = ControllerVertex::b_mapping_default;
                            s32 b_mappingCount = ControllerVertex::b_mapping_defaultCount;
                            const ControllerVertex::Analog* a_mapping = ControllerVertex::a_mapping_default;
                            s32 a_mappingCount = ControllerVertex::a_mapping_defaultCount;
                            
                            if (name == ControllerVertex::mapping_ps4Name) {
                                b_mapping = ControllerVertex::b_mapping_ps4;
                                b_mappingCount = ControllerVertex::b_mapping_ps4Count;
                                a_mapping = ControllerVertex::a_mapping_ps4;
                                a_mappingCount = ControllerVertex::a_mapping_ps4Count;
                            } else if (name == ControllerVertex::mapping_8bitdoName) {
                                b_mapping = ControllerVertex::b_mapping_8bitdo;
                                b_mappingCount = ControllerVertex::b_mapping_8bitdoCount;
                                a_mapping = ControllerVertex::a_mapping_8bitdo;
                                a_mappingCount = ControllerVertex::a_mapping_8bitdoCount;
                            } else if (name == ControllerVertex::mapping_winbluetoothwirelessName) {
								b_mapping = ControllerVertex::b_mapping_winbluetoothwireless;
								b_mappingCount = ControllerVertex::b_mapping_winbluetoothwirelessCount;
								a_mapping = ControllerVertex::a_mapping_winbluetoothwireless;
								a_mappingCount = ControllerVertex::a_mapping_winbluetoothwirelessCount;
							} else if (name == ControllerVertex::mapping_xboxName) {
								b_mapping = ControllerVertex::b_mapping_xbox;
								b_mappingCount = ControllerVertex::b_mapping_xboxCount;
								a_mapping = ControllerVertex::a_mapping_xbox;
								a_mappingCount = ControllerVertex::a_mapping_xboxCount;
							}

                            s32 axesCount;
                            const f32* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
                            for (s32 i = 0; i < axesCount; i++) {
                                if (i < a_mappingCount) {
                                    const ControllerVertex::Analog axisIndex = a_mapping[i];
                                    if (axisIndex != ControllerVertex::Analog::Invalid) {
                                        axisStates[(s32)axisIndex] = axes[i];
                                    }
                                }
                            }
                            
                            s32 buttonCount;
                            const u8* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
                            for (s32 i = 0; i < buttonCount; i++) {
                                if (i < b_mappingCount) {
                                    const ControllerVertex::Digital buttonIndex = b_mapping[i];
                                    if (buttonIndex != ControllerVertex::Digital::Invalid) {
                                        buttonStates[(s32)buttonIndex] = buttons[i];
                                    }
                                }
                            }
                        }

						// Render update
						glClearColor(0.f, 0.f, 0.f, 1.f);
						glClear(GL_COLOR_BUFFER_BIT);
						{
                            using namespace ControllerVertex;
                            
							glMatrixMode(GL_PROJECTION);
							glLoadIdentity();
							glMultMatrixf(game.view.orthoTransformCM);
                            
                            {
                                const Col padColor(1.0f, 1.0f, 1.0f, 1.0f);
                                glColor4f(RGBA_PARAMS(padColor));
                                
                                glEnableClientState(GL_VERTEX_ARRAY);
                                for (RenderBuffer& buffer : controllerBuffers.sbuffers) {
                                    glVertexPointer(2, GL_FLOAT, 0, buffer.vertex);
                                    glDrawArrays(GL_LINE_LOOP, 0, buffer.count);
                                }
                                for (u32 i = 0; i < (s32) DynamicShape::ButtonEnd; i++) {
                                    const RenderBuffer& buffer = controllerBuffers.dbuffers[i];

                                    s32 primitive = GL_LINE_LOOP;
                                    if (buttonStates[(s32)shape2button_mapping[i]]) {
                                        primitive = GL_TRIANGLE_FAN;
                                    }

                                    glVertexPointer(2, GL_FLOAT, 0, buffer.vertex);
                                    glDrawArrays(primitive, 0, buffer.count);
                                }
                                const f32 axisMovementMag = 5.f;
                                for (u32 i = (s32)DynamicShape::AxisStart; i < (s32)DynamicShape::AxisEnd; i++) {
                                    
                                    const s32 offset_i = i - (s32)DynamicShape::AxisStart;
                                    const RenderBuffer& buffer = controllerBuffers.dbuffers[i];
                                    f32 xoffset = axisMovementMag * axisStates[(s32)axis2analog_mapping[offset_i]];
                                    f32 yoffset = - axisMovementMag * axisStates[(s32)axis2analog_mapping[offset_i] + 1];
                                    
                                    s32 primitive = GL_LINE_LOOP;
                                    if (buttonStates[(s32)shape2button_mapping[i]]) {
                                        primitive = GL_TRIANGLE_FAN;
                                    }
                                    
                                    glPushMatrix();
                                    {
                                        glTranslatef(xoffset, yoffset, 0.f);
                                        glVertexPointer(2, GL_FLOAT, 0, buffer.vertex);
                                        glDrawArrays(primitive, 0, buffer.count);
                                    }
                                    glPopMatrix();
                                }
                                
                                for (u32 i = (s32)DynamicShape::TriggerStart; i < (s32)DynamicShape::TriggerEnd; i++) {

                                    const s32 offset_i = i - (s32)DynamicShape::TriggerStart;
                                    const RenderBuffer& buffer = controllerBuffers.dbuffers[i];
                                    const Vec2 center = Vec::scale(Vec::add(buffer.min, buffer.max), 0.5f);
                                    
                                    f32 axis = Math::bias(axisStates[(s32)trigger2analog_mapping[offset_i]]);
                                    
                                    s32 primitive = GL_LINE_LOOP;
                                    if (buttonStates[(s32)shape2button_mapping[i]]) {
                                        primitive = GL_TRIANGLE_FAN;
                                    }
                                    
                                    glPushMatrix();
                                    {
                                        f32 yscale = Math::lerp(axis, 1.f, 0.4f);
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
