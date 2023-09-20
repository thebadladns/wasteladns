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
    
//    namespace Analog {
//        const f32 novalue = 404.f;
//        const f32 digitalThreshold = 0.5f;
//    }
    
    struct KeyboardMapping {
        s32 mapping[Keys::COUNT];
    };
    
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
    struct Mapping {
        Keyboard::Keys::Enum mapping[0x7F];
    };
    struct PollData {
        Queue queue;
        Mapping mapping;
    };
    void load(Mapping& mapping) {
        memset(mapping.mapping, 0, sizeof(mapping.mapping));
        
        mapping.mapping[0x1D] = Keys::NUM0;
        mapping.mapping[0x12] = Keys::NUM1;
        mapping.mapping[0x13] = Keys::NUM2;
        mapping.mapping[0x14] = Keys::NUM3;
        mapping.mapping[0x15] = Keys::NUM4;
        mapping.mapping[0x17] = Keys::NUM5;
        mapping.mapping[0x16] = Keys::NUM6;
        mapping.mapping[0x1A] = Keys::NUM7;
        mapping.mapping[0x1C] = Keys::NUM8;
        mapping.mapping[0x19] = Keys::NUM9;
        mapping.mapping[0x00] = Keys::A;
        mapping.mapping[0x0B] = Keys::B;
        mapping.mapping[0x08] = Keys::C;
        mapping.mapping[0x02] = Keys::D;
        mapping.mapping[0x0E] = Keys::E;
        mapping.mapping[0x03] = Keys::F;
        mapping.mapping[0x05] = Keys::G;
        mapping.mapping[0x04] = Keys::H;
        mapping.mapping[0x22] = Keys::I;
        mapping.mapping[0x26] = Keys::J;
        mapping.mapping[0x28] = Keys::K;
        mapping.mapping[0x25] = Keys::L;
        mapping.mapping[0x2E] = Keys::M;
        mapping.mapping[0x2D] = Keys::N;
        mapping.mapping[0x1F] = Keys::O;
        mapping.mapping[0x23] = Keys::P;
        mapping.mapping[0x0C] = Keys::Q;
        mapping.mapping[0x0F] = Keys::R;
        mapping.mapping[0x01] = Keys::S;
        mapping.mapping[0x11] = Keys::T;
        mapping.mapping[0x20] = Keys::U;
        mapping.mapping[0x09] = Keys::V;
        mapping.mapping[0x0D] = Keys::W;
        mapping.mapping[0x07] = Keys::X;
        mapping.mapping[0x10] = Keys::Y;
        mapping.mapping[0x06] = Keys::Z;

        mapping.mapping[0x27] = Keys::APOSTROPHE;
        mapping.mapping[0x2A] = Keys::BACKSLASH;
        mapping.mapping[0x2B] = Keys::COMMA;
        mapping.mapping[0x18] = Keys::EQUAL;
        mapping.mapping[0x32] = Keys::GRAVE_ACCENT;
        mapping.mapping[0x21] = Keys::LEFT_BRACKET;
        mapping.mapping[0x1B] = Keys::MINUS;
        mapping.mapping[0x2F] = Keys::PERIOD;
        mapping.mapping[0x1E] = Keys::RIGHT_BRACKET;
        mapping.mapping[0x29] = Keys::SEMICOLON;
        mapping.mapping[0x2C] = Keys::SLASH;
        mapping.mapping[0x0A] = Keys::WORLD_1;

        mapping.mapping[0x33] = Keys::BACKSPACE;
        mapping.mapping[0x39] = Keys::CAPS_LOCK;
        mapping.mapping[0x75] = Keys::DELETE;
        mapping.mapping[0x7D] = Keys::DOWN;
        mapping.mapping[0x77] = Keys::END;
        mapping.mapping[0x24] = Keys::ENTER;
        mapping.mapping[0x35] = Keys::ESCAPE;
        mapping.mapping[0x7A] = Keys::F1;
        mapping.mapping[0x78] = Keys::F2;
        mapping.mapping[0x63] = Keys::F3;
        mapping.mapping[0x76] = Keys::F4;
        mapping.mapping[0x60] = Keys::F5;
        mapping.mapping[0x61] = Keys::F6;
        mapping.mapping[0x62] = Keys::F7;
        mapping.mapping[0x64] = Keys::F8;
        mapping.mapping[0x65] = Keys::F9;
        mapping.mapping[0x6D] = Keys::F10;
        mapping.mapping[0x67] = Keys::F11;
        mapping.mapping[0x6F] = Keys::F12;
        mapping.mapping[0x69] = Keys::PRINT_SCREEN;
        mapping.mapping[0x6B] = Keys::F14;
        mapping.mapping[0x71] = Keys::F15;
        mapping.mapping[0x6A] = Keys::F16;
        mapping.mapping[0x40] = Keys::F17;
        mapping.mapping[0x4F] = Keys::F18;
        mapping.mapping[0x50] = Keys::F19;
        mapping.mapping[0x5A] = Keys::F20;
        mapping.mapping[0x73] = Keys::HOME;
        mapping.mapping[0x72] = Keys::INSERT;
        mapping.mapping[0x7B] = Keys::LEFT;
        mapping.mapping[0x3A] = Keys::LEFT_ALT;
        mapping.mapping[0x3B] = Keys::LEFT_CONTROL;
        mapping.mapping[0x38] = Keys::LEFT_SHIFT;
        mapping.mapping[0x37] = Keys::LEFT_SUPER;
        mapping.mapping[0x6E] = Keys::MENU;
        mapping.mapping[0x47] = Keys::NUM_LOCK;
        mapping.mapping[0x79] = Keys::PAGE_DOWN;
        mapping.mapping[0x74] = Keys::PAGE_UP;
        mapping.mapping[0x7C] = Keys::RIGHT;
        mapping.mapping[0x3D] = Keys::RIGHT_ALT;
        mapping.mapping[0x3E] = Keys::RIGHT_CONTROL;
        mapping.mapping[0x3C] = Keys::RIGHT_SHIFT;
        mapping.mapping[0x36] = Keys::RIGHT_SUPER;
        mapping.mapping[0x31] = Keys::SPACE;
        mapping.mapping[0x30] = Keys::TAB;
        mapping.mapping[0x7E] = Keys::UP;

        mapping.mapping[0x52] = Keys::KP_0;
        mapping.mapping[0x53] = Keys::KP_1;
        mapping.mapping[0x54] = Keys::KP_2;
        mapping.mapping[0x55] = Keys::KP_3;
        mapping.mapping[0x56] = Keys::KP_4;
        mapping.mapping[0x57] = Keys::KP_5;
        mapping.mapping[0x58] = Keys::KP_6;
        mapping.mapping[0x59] = Keys::KP_7;
        mapping.mapping[0x5B] = Keys::KP_8;
        mapping.mapping[0x5C] = Keys::KP_9;
        mapping.mapping[0x45] = Keys::KP_ADD;
        mapping.mapping[0x41] = Keys::KP_DECIMAL;
        mapping.mapping[0x4B] = Keys::KP_DIVIDE;
        mapping.mapping[0x4C] = Keys::KP_ENTER;
        mapping.mapping[0x51] = Keys::KP_EQUAL;
        mapping.mapping[0x43] = Keys::KP_MULTIPLY;
        mapping.mapping[0x4E] = Keys::KP_SUBTRACT;
    }
    
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
