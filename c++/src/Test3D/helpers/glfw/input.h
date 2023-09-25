#ifndef __WASTELADNS_INPUT_GLFW_H__
#define __WASTELADNS_INPUT_GLFW_H__

#ifndef UNITYBUILD
#include <cstring>
#include "../input.h"
#include "core.h"
#endif

namespace Input {
namespace Gamepad {
    
//    namespace Analog {
//        const f32 novalue = 404.f;
//        const f32 digitalThreshold = 0.5f;
//    }
    
    struct KeyboardMapping {
        s32 mapping[Keys::COUNT];
    };
    
    void load(KeyboardMapping& mapping) {
        memset(mapping.mapping, GLFW_KEY_UNKNOWN, sizeof(s32) * Keys::COUNT);
        // Hardcoded for now
        mapping.mapping[Keys::B_U] = GLFW_KEY_I;
        mapping.mapping[Keys::B_D] = GLFW_KEY_K;
        mapping.mapping[Keys::B_L] = GLFW_KEY_J;
        mapping.mapping[Keys::B_R] = GLFW_KEY_L;
        mapping.mapping[Keys::D_U] = GLFW_KEY_W;
        mapping.mapping[Keys::D_D] = GLFW_KEY_S;
        mapping.mapping[Keys::D_L] = GLFW_KEY_A;
        mapping.mapping[Keys::D_R] = GLFW_KEY_D;
    }
    
    void pollState(State& pad, GLFWwindow* window, const KeyboardMapping& mapping) {
        pad.active = true;
        pad.keys.last = pad.keys.current;
        pad.keys.current = 0;
        for (int i = 0; i < Keys::COUNT; i++) {
            s32 glfw_id = mapping.mapping[i];
            if (glfw_id != GLFW_KEY_UNKNOWN) {
                s32 keyState = glfwGetKey(window, glfw_id);
                s32 keyOn = keyState != GLFW_RELEASE;
                pad.keys.current = pad.keys.current | (keyOn << i);
            }
        }
        
        // TODO: handle analog only if requested?
    }
    
//    void pollState(Joypad& joypad, s32 joyId, GLFWwindow* window) {
//        joypad.active = glfwJoystickPresent(joyId) != 0;
//        if (joypad.active) {
//
//            if (!pad.mapping.set) {
//                loadPreset(pad.mapping, glfwGetJoystickName(joyId));
//                pad.mapping.set = true;
//            }
//
//            // Analogs (axis and triggers)
//            for (s32 i = 0; i < Gamepad::Analog::COUNT; i++) {
//                pad.analogs.values[i] = Gamepad::Analog::novalue;
//            }
//            s32 axesCount;
//            const f32* axes = glfwGetJoystickAxes(joyId, &axesCount);
//            for (s32 i = 0; i < axesCount; i++) {
//                if (i < pad.mapping.a_mappingCount) {
//                    const Gamepad::Analog::Enum axisIndex = pad.mapping.a_mapping[i];
//                    if (axisIndex != Gamepad::Analog::INVALID) {
//                        f32 v = axes[i];
//                        const f32 deadzone = 0.25f;
//                        if (Math::abs(v) < deadzone) {
//                            v = 0.f;
//                        }
//                        pad.analogs.values[axisIndex] = v;
//                    }
//                }
//            }
//
//            bool digitalTrigger_L = false;
//            bool digitalTrigger_R = false;
//
//            pad.buttons.last = pad.buttons.current;
//            pad.buttons.current = (Gamepad::Digital::Enum) 0;
//            s32 buttonCount;
//            const u8* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
//            for (s32 i = 0; i < buttonCount; i++) {
//                if (i < pad.mapping.b_mappingCount) {
//                    const Gamepad::Digital::Enum buttonIndex = pad.mapping.b_mapping[i];
//                    if (buttonIndex != Gamepad::Digital::INVALID) {
//                        pad.buttons.current = pad.buttons.current | ((buttons[i] != 0) << buttonIndex);
//                    }
//                    if (buttonIndex == Gamepad::Digital::L2) {
//                        digitalTrigger_L = true;
//                    } else if (buttonIndex == Gamepad::Digital::R2) {
//                        digitalTrigger_R = true;
//                    }
//                }
//            }
//
//            // Manually set digital trigger values, if needed
//            if (!digitalTrigger_L) {
//                const f32 trigger_l = pad.analogs.values[Gamepad::Analog::Trigger_L];
//                if (trigger_l != Gamepad::Analog::novalue) {
//                    bool triggerPressed = trigger_l > Gamepad::Analog::digitalThreshold;
//                    pad.buttons.current = (Gamepad::Digital::Enum) (pad.buttons.current | (triggerPressed << Gamepad::Digital::L2));
//                    digitalTrigger_L = true;
//                }
//            }
//            if (!digitalTrigger_R) {
//                const f32 trigger_r = pad.analogs.values[Gamepad::Analog::Trigger_R];
//                if (trigger_r != Gamepad::Analog::novalue) {
//                    bool triggerPressed = trigger_r > Gamepad::Analog::digitalThreshold;
//                    pad.buttons.current = (Gamepad::Digital::Enum) (pad.buttons.current | (triggerPressed << Gamepad::Digital::R2));
//                    digitalTrigger_R = true;
//                }
//            }
//        }
//    }
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
