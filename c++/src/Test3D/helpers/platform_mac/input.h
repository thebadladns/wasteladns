#ifndef __WASTELADNS_INPUT_MACOS_H__
#define __WASTELADNS_INPUT_MACOS_H__

#ifndef UNITYBUILD
#include <cstring>
#include "../input.h"
#include "core.h"
#endif

namespace Input {
namespace Keyboard {
    // Use an explicit state queue in case message handling needs
    // to be moved outside the frame loop
    struct Queue {
        bool keyStates[Input::Keyboard::Keys::COUNT];
    };
};
namespace Gamepad {

//    void load(KeyboardMapping& mapping) {
//        memset(mapping.mapping, GLFW_KEY_UNKNOWN, sizeof(s32) * Keys::COUNT);
//        // Hardcoded for now
//        mapping.mapping[Keys::B_U] = GLFW_KEY_I;
//        mapping.mapping[Keys::B_D] = GLFW_KEY_K;
//        mapping.mapping[Keys::B_L] = GLFW_KEY_J;
//        mapping.mapping[Keys::B_R] = GLFW_KEY_L;
//        mapping.mapping[Keys::D_U] = GLFW_KEY_W;
//        mapping.mapping[Keys::D_D] = GLFW_KEY_S;
//        mapping.mapping[Keys::D_L] = GLFW_KEY_A;
//        mapping.mapping[Keys::D_R] = GLFW_KEY_D;
//    }
//
//    void pollState(State& pad, GLFWwindow* window, const KeyboardMapping& mapping) {
//        pad.active = true;
//        pad.keys.last = pad.keys.current;
//        pad.keys.current = 0;
//        for (int i = 0; i < Keys::COUNT; i++) {
//            s32 glfw_id = mapping.mapping[i];
//            if (glfw_id != GLFW_KEY_UNKNOWN) {
//                s32 keyState = glfwGetKey(window, glfw_id);
//                s32 keyOn = keyState != GLFW_RELEASE;
//                pad.keys.current = pad.keys.current | (keyOn << i);
//            }
//        }
//
//        // TODO: handle analog only if requested?
//    }
    
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
    struct PollData {
        Queue queue;
    };

    void pollState(State& keyboard, const Queue& queue) {
        memcpy(keyboard.last, keyboard.current, sizeof(u8) * Keys::COUNT);
        memset(keyboard.current, 0, sizeof(u8) * Keys::COUNT);
        for (int i = 0; i < Keys::COUNT; i++) {
            keyboard.current[i] = queue.keyStates[i];
        }
    }
};

namespace Mouse
{
    struct PollData {
        f32 x, y;
        f32 scrollx, scrolly;
        u8 keyStates[Keys::COUNT];
    };
    void resetState(PollData& queue) {
        queue.scrollx = queue.scrolly = 0.0;
    }
    void pollState(State& mouse, const PollData& queue) {

        memcpy(mouse.last, mouse.current, sizeof(u8) * Keys::COUNT);
        memset(mouse.current, 0, sizeof(u8) * Keys::COUNT);
        for (int i = 0; i < Keys::COUNT; i++) {
            mouse.current[i] = queue.keyStates[i];
        }
        mouse.scrolldx = queue.scrollx;
        mouse.scrolldy = queue.scrolly;
        mouse.dx = queue.x - mouse.x;
        mouse.dy = queue.y - mouse.y;
        mouse.x = queue.x;
        mouse.y = queue.y;
    }
};

};

#endif // __WASTELADNS_INPUT_MACOS_H__
