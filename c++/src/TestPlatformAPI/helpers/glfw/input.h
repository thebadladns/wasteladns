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
    struct Mapping {
        s32 mapping[Keys::COUNT];
    };
    
    void load(Mapping& mapping) {
        memset(mapping.mapping, GLFW_KEY_UNKNOWN, sizeof(s32) * Keys::COUNT);
        mapping.mapping[Keys::SPACE] = GLFW_KEY_SPACE;
        mapping.mapping[Keys::APOSTROPHE] = GLFW_KEY_APOSTROPHE;
        mapping.mapping[Keys::COMMA] = GLFW_KEY_COMMA;
        mapping.mapping[Keys::MINUS ] = GLFW_KEY_MINUS;
        mapping.mapping[Keys::PERIOD] = GLFW_KEY_PERIOD;
        mapping.mapping[Keys::SLASH] = GLFW_KEY_SLASH;
        mapping.mapping[Keys::NUM0] = GLFW_KEY_0;
        mapping.mapping[Keys::NUM1] = GLFW_KEY_1;
        mapping.mapping[Keys::NUM2] = GLFW_KEY_2;
        mapping.mapping[Keys::NUM3] = GLFW_KEY_3;
        mapping.mapping[Keys::NUM4] = GLFW_KEY_4;
        mapping.mapping[Keys::NUM5] = GLFW_KEY_5;
        mapping.mapping[Keys::NUM6] = GLFW_KEY_6;
        mapping.mapping[Keys::NUM7] = GLFW_KEY_7;
        mapping.mapping[Keys::NUM8] = GLFW_KEY_8;
        mapping.mapping[Keys::NUM9] = GLFW_KEY_9;
        mapping.mapping[Keys::SEMICOLON] = GLFW_KEY_SEMICOLON;
        mapping.mapping[Keys::EQUAL] = GLFW_KEY_EQUAL;
        mapping.mapping[Keys::A] = GLFW_KEY_A;
        mapping.mapping[Keys::B] = GLFW_KEY_B;
        mapping.mapping[Keys::C] = GLFW_KEY_C;
        mapping.mapping[Keys::D] = GLFW_KEY_D;
        mapping.mapping[Keys::E] = GLFW_KEY_E;
        mapping.mapping[Keys::F] = GLFW_KEY_F;
        mapping.mapping[Keys::G] = GLFW_KEY_G;
        mapping.mapping[Keys::H] = GLFW_KEY_H;
        mapping.mapping[Keys::I] = GLFW_KEY_I;
        mapping.mapping[Keys::J] = GLFW_KEY_J;
        mapping.mapping[Keys::K] = GLFW_KEY_K;
        mapping.mapping[Keys::L] = GLFW_KEY_L;
        mapping.mapping[Keys::M] = GLFW_KEY_M;
        mapping.mapping[Keys::N] = GLFW_KEY_N;
        mapping.mapping[Keys::O] = GLFW_KEY_O;
        mapping.mapping[Keys::P] = GLFW_KEY_P;
        mapping.mapping[Keys::Q] = GLFW_KEY_Q;
        mapping.mapping[Keys::R] = GLFW_KEY_R;
        mapping.mapping[Keys::S] = GLFW_KEY_S;
        mapping.mapping[Keys::T] = GLFW_KEY_T;
        mapping.mapping[Keys::U] = GLFW_KEY_U;
        mapping.mapping[Keys::V] = GLFW_KEY_V;
        mapping.mapping[Keys::W] = GLFW_KEY_W;
        mapping.mapping[Keys::X] = GLFW_KEY_X;
        mapping.mapping[Keys::Y] = GLFW_KEY_Y;
        mapping.mapping[Keys::Z] = GLFW_KEY_Z;
        mapping.mapping[Keys::LEFT_BRACKET] = GLFW_KEY_LEFT_BRACKET;
        mapping.mapping[Keys::BACKSLASH] = GLFW_KEY_BACKSLASH;
        mapping.mapping[Keys::RIGHT_BRACKET] = GLFW_KEY_RIGHT_BRACKET;
        mapping.mapping[Keys::GRAVE_ACCENT] = GLFW_KEY_GRAVE_ACCENT;
        mapping.mapping[Keys::WORLD_1] = GLFW_KEY_WORLD_1;
        mapping.mapping[Keys::WORLD_2] = GLFW_KEY_WORLD_2;
        mapping.mapping[Keys::ESCAPE] = GLFW_KEY_ESCAPE;
        mapping.mapping[Keys::ENTER] = GLFW_KEY_ENTER;
        mapping.mapping[Keys::TAB] = GLFW_KEY_TAB;
        mapping.mapping[Keys::BACKSPACE] = GLFW_KEY_BACKSPACE;
        mapping.mapping[Keys::INSERT] = GLFW_KEY_INSERT;
        mapping.mapping[Keys::DELETE] = GLFW_KEY_DELETE;
        mapping.mapping[Keys::RIGHT] = GLFW_KEY_RIGHT;
        mapping.mapping[Keys::LEFT] = GLFW_KEY_LEFT;
        mapping.mapping[Keys::DOWN] = GLFW_KEY_DOWN;
        mapping.mapping[Keys::UP] = GLFW_KEY_UP;
        mapping.mapping[Keys::PAGE_UP] = GLFW_KEY_PAGE_UP;
        mapping.mapping[Keys::PAGE_DOWN] = GLFW_KEY_PAGE_DOWN;
        mapping.mapping[Keys::HOME] = GLFW_KEY_HOME;
        mapping.mapping[Keys::END] = GLFW_KEY_END;
        mapping.mapping[Keys::CAPS_LOCK] = GLFW_KEY_CAPS_LOCK;
        mapping.mapping[Keys::SCROLL_LOCK] = GLFW_KEY_SCROLL_LOCK;
        mapping.mapping[Keys::NUM_LOCK] = GLFW_KEY_NUM_LOCK;
        mapping.mapping[Keys::PRINT_SCREEN] = GLFW_KEY_PRINT_SCREEN;
        mapping.mapping[Keys::PAUSE] = GLFW_KEY_PAUSE;
        mapping.mapping[Keys::F1] = GLFW_KEY_F1;
        mapping.mapping[Keys::F2] = GLFW_KEY_F2;
        mapping.mapping[Keys::F3] = GLFW_KEY_F3;
        mapping.mapping[Keys::F4] = GLFW_KEY_F4;
        mapping.mapping[Keys::F5] = GLFW_KEY_F5;
        mapping.mapping[Keys::F6] = GLFW_KEY_F6;
        mapping.mapping[Keys::F7] = GLFW_KEY_F7;
        mapping.mapping[Keys::F8] = GLFW_KEY_F8;
        mapping.mapping[Keys::F9] = GLFW_KEY_F9;
        mapping.mapping[Keys::F10] = GLFW_KEY_F10;
        mapping.mapping[Keys::F11] = GLFW_KEY_F11;
        mapping.mapping[Keys::F12] = GLFW_KEY_F12;
        mapping.mapping[Keys::F13] = GLFW_KEY_F13;
        mapping.mapping[Keys::F14] = GLFW_KEY_F14;
        mapping.mapping[Keys::F15] = GLFW_KEY_F15;
        mapping.mapping[Keys::F16] = GLFW_KEY_F16;
        mapping.mapping[Keys::F17] = GLFW_KEY_F17;
        mapping.mapping[Keys::F18] = GLFW_KEY_F18;
        mapping.mapping[Keys::F19] = GLFW_KEY_F19;
        mapping.mapping[Keys::F20] = GLFW_KEY_F20;
        mapping.mapping[Keys::F21] = GLFW_KEY_F21;
        mapping.mapping[Keys::F22] = GLFW_KEY_F22;
        mapping.mapping[Keys::F23] = GLFW_KEY_F23;
        mapping.mapping[Keys::F24] = GLFW_KEY_F24;
        mapping.mapping[Keys::F25] = GLFW_KEY_F25;
        mapping.mapping[Keys::KP_0] = GLFW_KEY_KP_0;
        mapping.mapping[Keys::KP_1] = GLFW_KEY_KP_1;
        mapping.mapping[Keys::KP_2] = GLFW_KEY_KP_2;
        mapping.mapping[Keys::KP_3] = GLFW_KEY_KP_3;
        mapping.mapping[Keys::KP_4] = GLFW_KEY_KP_4;
        mapping.mapping[Keys::KP_5] = GLFW_KEY_KP_5;
        mapping.mapping[Keys::KP_6] = GLFW_KEY_KP_6;
        mapping.mapping[Keys::KP_7] = GLFW_KEY_KP_7;
        mapping.mapping[Keys::KP_8] = GLFW_KEY_KP_8;
        mapping.mapping[Keys::KP_9] = GLFW_KEY_KP_9;
        mapping.mapping[Keys::KP_DECIMAL] = GLFW_KEY_KP_DECIMAL;
        mapping.mapping[Keys::KP_DIVIDE] = GLFW_KEY_KP_DIVIDE;
        mapping.mapping[Keys::KP_MULTIPLY] = GLFW_KEY_KP_MULTIPLY;
        mapping.mapping[Keys::KP_SUBTRACT] = GLFW_KEY_KP_SUBTRACT;
        mapping.mapping[Keys::KP_ADD] = GLFW_KEY_KP_ADD;
        mapping.mapping[Keys::KP_ENTER] = GLFW_KEY_KP_ENTER;
        mapping.mapping[Keys::KP_EQUAL] = GLFW_KEY_KP_EQUAL;
        mapping.mapping[Keys::LEFT_SHIFT] = GLFW_KEY_LEFT_SHIFT;
        mapping.mapping[Keys::LEFT_CONTROL] = GLFW_KEY_LEFT_CONTROL;
        mapping.mapping[Keys::LEFT_ALT] = GLFW_KEY_LEFT_ALT;
        mapping.mapping[Keys::LEFT_SUPER] = GLFW_KEY_LEFT_SUPER;
        mapping.mapping[Keys::RIGHT_SHIFT] = GLFW_KEY_RIGHT_SHIFT;
        mapping.mapping[Keys::RIGHT_CONTROL] = GLFW_KEY_RIGHT_CONTROL;
        mapping.mapping[Keys::RIGHT_ALT] = GLFW_KEY_RIGHT_ALT;
        mapping.mapping[Keys::RIGHT_SUPER] = GLFW_KEY_RIGHT_SUPER;
        mapping.mapping[Keys::MENU] = GLFW_KEY_MENU;
    };
    
    void pollState(State& keyboard, GLFWwindow* window, const Mapping& mapping) {
        memcpy(keyboard.last, keyboard.current, sizeof(u8) * Keys::COUNT);
        memset(keyboard.current, 0, sizeof(u8) * Keys::COUNT);
        for (int i = 0; i < Keys::COUNT; i++) {
            s32 glfw_id = mapping.mapping[i];
            if (glfw_id != GLFW_KEY_UNKNOWN) {
                keyboard.current[i] = glfwGetKey(window, glfw_id) != GLFW_RELEASE;
            }
        }
    }
};
};

#endif // __WASTELADNS_INPUT_GLFW_H__
