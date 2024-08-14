#ifndef __WASTELADNS_INPUT_GLFW_H__
#define __WASTELADNS_INPUT_GLFW_H__

#ifndef UNITYBUILD
#include <cstring>
#include "../input.h"
#include "core.h"
#endif

namespace Input {
namespace Gamepad {

    void pollState(::Input::Gamepad::State* pads, u32& padCount, const u32 maxPadCount) {
        namespace Pad = ::Input::Gamepad;

        for (u32 joyId = GLFW_JOYSTICK_1; joyId < GLFW_JOYSTICK_LAST; joyId++) {
            if (!glfwJoystickPresent(joyId)) continue;
            if (!glfwJoystickIsGamepad(joyId)) continue;

            const char* guid = glfwGetJoystickGUID(joyId);
            u32 padToUpdate = padCount;
            for (u32 padidx = 0; padidx < padCount; padidx++) {
                if (pads[padidx].deviceHandle == guid) { // hopefully checking the pointers is enough
                    padToUpdate = padidx;
                    break;
                }
            }
            if (padToUpdate >= maxPadCount) return;
            Pad::State& pad = pads[padToUpdate];

            // this will be a new pad
            if (padToUpdate == padCount) {
                pad = {};
            }

            u16 keys = 0;
            GLFWgamepadstate state;
            if (glfwGetGamepadState(joyId, &state))
            {
                u32 keyMask[GLFW_GAMEPAD_BUTTON_LAST + 1] =
                { Pad::KeyMask::BUTTON_S, Pad::KeyMask::BUTTON_E, Pad::KeyMask::BUTTON_W, Pad::KeyMask::BUTTON_N,
                  Pad::KeyMask::L1, Pad::KeyMask::R1, Pad::KeyMask::SELECT, Pad::KeyMask::START, 0,
                  Pad::KeyMask::LEFT_THUMB, Pad::KeyMask::RIGHT_THUMB,
                  Pad::KeyMask::DPAD_UP, Pad::KeyMask::DPAD_RIGHT, Pad::KeyMask::DPAD_DOWN, Pad::KeyMask::DPAD_LEFT
                };
                for (u32 i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) {
                    if (state.buttons[i] & GLFW_PRESS) { keys |= keyMask[i]; }
                }
                u32 sliderMask[GLFW_GAMEPAD_AXIS_LAST + 1] =
                { Pad::Sliders::AXIS_X_LEFT, Pad::Sliders::AXIS_Y_LEFT,
                  Pad::Sliders::AXIS_X_RIGHT, Pad::Sliders::AXIS_Y_RIGHT,
                  Pad::Sliders::TRIGGER_LEFT, Pad::Sliders::TRIGGER_RIGHT
                };
                for (u32 i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; i++) {
                    f32 slider = state.axes[i];
                    u32 sliderIdx = sliderMask[i];
                    if (sliderIdx == Pad::Sliders::AXIS_Y_LEFT || sliderIdx == Pad::Sliders::AXIS_Y_RIGHT) slider = -slider;
                    if (Math::abs(slider) < 0.02f) slider = 0.f;
                    pad.sliders[sliderIdx] = slider;
                }
                if (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.9) { keys |= Pad::KeyMask::L2; }
                if (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.9) { keys |= Pad::KeyMask::R2; }

                pad.last_keys = pad.curr_keys;
                pad.curr_keys = keys;
            }

            // If the pad is new, and we got button input from it, finish creation
            if (padToUpdate == padCount) {
                bool validpad = pad.curr_keys != 0;
                for (u32 slider = 0; validpad && slider < Pad::Sliders::COUNT; slider++) {
                    if (Math::abs(pad.sliders[slider]) > 2.f) { // TODO: properly ignore bad inputs
                        validpad = false;
                    }
                }
                if (validpad) {
                    pad.deviceHandle = guid;
                    #if __DEBUG
                    const char* name = glfwGetGamepadName(joyId);
                    strncpy(pad.name, name, sizeof(pad.name));
                    #endif
                    padCount++;
                }
            }
        }
    }
};
    
namespace Keyboard
{
    void pollState(State& keyboard, GLFWwindow* window) {
        memcpy(keyboard.last, keyboard.current, sizeof(u8) * Keys::COUNT);
        memset(keyboard.current, 0, sizeof(u8) * Keys::COUNT);
        for (int i = 0; i < Keys::COUNT; i++) {
            if (i != GLFW_KEY_UNKNOWN) {
                keyboard.current[i] = glfwGetKey(window, i) != GLFW_RELEASE;
            }
        }
    }
};

namespace Mouse
{
    void resetState(State& mouse) {
        mouse.scrolldx = mouse.scrolldy = 0.0;
    }
	void pollState(State& mouse, GLFWwindow* window) {
		memcpy(mouse.last, mouse.current, sizeof(u8) * Keys::COUNT);
		memset(mouse.current, 0, sizeof(u8) * Keys::COUNT);
		for (int i = 0; i < Keys::COUNT; i++) {
			if (i != GLFW_KEY_UNKNOWN) {
				mouse.current[i] = glfwGetMouseButton(window, i) != GLFW_RELEASE;
			}
		}

        f64 x, y;
        glfwGetCursorPos(window, &x, &y);
        mouse.dx = (f32)x - mouse.x;
        mouse.dy = (f32)y - mouse.y;
        mouse.x = (f32)x;
        mouse.y = (f32)y;
	}
};

};

#endif // __WASTELADNS_INPUT_GLFW_H__
