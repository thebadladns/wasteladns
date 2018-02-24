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

// Core
#include "helpers/types.h"
#define __WASTELADNS_MATH_IMPL__
#include "helpers/math.h"
#define __WASTELADNS_ANGLE_IMPL__
#include "helpers/Angle.h"
#define __WASTELADNS_VEC_IMPL__
#include "helpers/vec.h"
#define __WASTELADNS_COLOR_IMPL__
#include "helpers/color.h"
#define __WASTELADNS_DEBUGDRAW_IMPL__
#define __WASTELADNS_DEBUGDRAW_TEXT__
#include "helpers/debugdraw.h"
#include "helpers/camera.h"
#include "helpers/input.h"


/*
controller

<path id="base" class="cls-1" d="M89,155L59,199l-29,1L19,178,33,78,49,43H75l8,15H212l8-15h26l16,35,14,100-11,22-29-1-30-44H89Z"/>
<g id="buttons">
<path id="_" data-name="^" class="cls-1" d="M241.44,102.635l0.47-8.239-5-5.4h-8.491l-5.268,5.4,0.058,8.239,5.012,4.917h7.963Z"/>
<path id="x" class="cls-1" d="M239.354,139.259l0.355-6.231-4.365-4.056h-7.352l-4.567,4.056,0.044,6.231L227.841,143H234.8Z"/>
<path id="o" class="cls-1" d="M261.075,122.206l0.965-7.13-4.3-4.654h-7.881l-5.257,4.654-0.508,7.13,4.336,4.27h7.424Z"/>
<path id="_2" data-name="[]" class="cls-1" d="M219.575,122.206l-0.152-7.13-5.025-4.654h-7.881l-4.528,4.654,0.609,7.13,5,4.27h7.424Z"/>
</g>
<g id="axis-r">
<path id="axis-r-base" class="cls-2" d="M175.009,155.626l-2-10.876L178.877,134H196.12l5.865,10.75-3.378,12.028L193.428,162H181.569Z"/>
<path id="axis-r-mov" class="cls-1" d="M177.965,161.76l-1.924-7.646,4.4-8.114H194.6l4.4,8.114-3.01,8.425L191.96,166h-8.889Z"/>
</g>
<g id="axis-l">
<path id="axis-l-base" class="cls-2" d="M96.009,155.626l-2-10.876L99.877,134H117.12l5.865,10.75-3.378,12.028L114.428,162H102.569Z"/>
<path id="axis-l-mov" class="cls-1" d="M98.965,161.76l-1.923-7.646L101.436,146H115.6l4.395,8.114-3.01,8.425L112.96,166h-8.889Z"/>
</g>
<g id="dpad">
<path id="u" class="cls-1" d="M52.532,95l10.407,16.661L72.371,95H52.532Z"/>
<path id="r" class="cls-1" d="M87.152,112.677l-15.278,8.672,13.636,7.91Z"/>
<path id="d" class="cls-1" d="M71.5,141l-8.05-11.741L56.093,141H71.5Z"/>
<path id="l" class="cls-1" d="M41.4,129.259l13.172-7.91-15.786-8.672Z"/>
</g>
<path id="select" class="cls-1" d="M104.077,84.1H95l2.441,12.8h4.923Z"/>
<path id="start" class="cls-1" d="M190.986,84H200L197.576,97h-4.89Z"/>
<path id="r1" class="cls-1" d="M250,69H221l4-19h19Z"/>
<path id="r2" class="cls-1" d="M247,45H219l7-13h14Z"/>
<path id="l1" class="cls-1" d="M45,69H74L70,50H51Z"/>
<path id="l2" class="cls-1" d="M48,45H76L69,32H55Z"/>
<path id="tpad" class="cls-1" d="M112,65l-4,22,11,38h57l11-38-4-22H112Z"/>

*/

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
						f32 timeDelta = (f32)game.time.config.targetFramerate;

						using namespace Game;

						glfwPollEvents();
						app.input.keyboardSet.pollState(window.handle);

						// Logic
						if (app.input.keyboardSet.released(GLFW_KEY_ESCAPE)) {
							glfwSetWindowShouldClose(app.mainWindow.handle, 1);
						}

						// Render update
						glClearColor(0.f, 0.f, 0.f, 1.f);
						glClear(GL_COLOR_BUFFER_BIT);
						{
							glMatrixMode(GL_PROJECTION);
							glLoadIdentity();
							glMultMatrixf(game.view.orthoTransformCM);

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
									strncat(buffer, axisBuff, sizeof(axisBuff));
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
									strncat(buffer, buttonBuff, sizeof(buttonBuff));
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