#ifndef __WASTELADNS_INPUT_DX11_H__
#define __WASTELADNS_INPUT_DX11_H__

#ifndef UNITYBUILD
#include <cstring>
#include "../input.h"
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

    struct KeyboardMapping {
        Keyboard::Keys::Enum mapping[Keys::COUNT];
    };

    void load(KeyboardMapping& mapping) {
        memset(mapping.mapping, Keyboard::Keys::INVALID, sizeof(s32) * Keys::COUNT);
        // Hardcoded for now
        mapping.mapping[Keys::B_U] = Keyboard::Keys::I;
        mapping.mapping[Keys::B_D] = Keyboard::Keys::K;
        mapping.mapping[Keys::B_L] = Keyboard::Keys::J;
        mapping.mapping[Keys::B_R] = Keyboard::Keys::L;
    }

    void pollState(State& pad, const Keyboard::Queue& queue, const KeyboardMapping& mapping) {
        pad.active = true;
        pad.keys.last = pad.keys.current;
        pad.keys.current = 0;
        for (int i = 0; i < Keys::COUNT; i++) {
            Keyboard::Keys::Enum keyId = mapping.mapping[i];
            if (keyId != Keyboard::Keys::INVALID) {
                bool keyState = queue.keyStates[keyId];
                pad.keys.current = pad.keys.current | ((s32)keyState << i);
            }
        }

        // TODO: handle analog only if requested?
    }
}

namespace Keyboard {
    struct Mapping {
        ::Input::Keyboard::Keys::Enum mapping[512];
    };
    struct PollData {
        Queue queue;
        Mapping mapping;
    };

    void load(Mapping& mapping) {
        memset(mapping.mapping, 0, sizeof(mapping.mapping));
        mapping.mapping[0x00B] = Keys::NUM0;
        mapping.mapping[0x002] = Keys::NUM1;
        mapping.mapping[0x003] = Keys::NUM2;
        mapping.mapping[0x004] = Keys::NUM3;
        mapping.mapping[0x005] = Keys::NUM4;
        mapping.mapping[0x006] = Keys::NUM5;
        mapping.mapping[0x007] = Keys::NUM6;
        mapping.mapping[0x008] = Keys::NUM7;
        mapping.mapping[0x009] = Keys::NUM8;
        mapping.mapping[0x00A] = Keys::NUM9;
        mapping.mapping[0x01E] = Keys::A;
        mapping.mapping[0x030] = Keys::B;
        mapping.mapping[0x02E] = Keys::C;
        mapping.mapping[0x020] = Keys::D;
        mapping.mapping[0x012] = Keys::E;
        mapping.mapping[0x021] = Keys::F;
        mapping.mapping[0x022] = Keys::G;
        mapping.mapping[0x023] = Keys::H;
        mapping.mapping[0x017] = Keys::I;
        mapping.mapping[0x024] = Keys::J;
        mapping.mapping[0x025] = Keys::K;
        mapping.mapping[0x026] = Keys::L;
        mapping.mapping[0x032] = Keys::M;
        mapping.mapping[0x031] = Keys::N;
        mapping.mapping[0x018] = Keys::O;
        mapping.mapping[0x019] = Keys::P;
        mapping.mapping[0x010] = Keys::Q;
        mapping.mapping[0x013] = Keys::R;
        mapping.mapping[0x01F] = Keys::S;
        mapping.mapping[0x014] = Keys::T;
        mapping.mapping[0x016] = Keys::U;
        mapping.mapping[0x02F] = Keys::V;
        mapping.mapping[0x011] = Keys::W;
        mapping.mapping[0x02D] = Keys::X;
        mapping.mapping[0x015] = Keys::Y;
        mapping.mapping[0x02C] = Keys::Z;

        mapping.mapping[0x028] = Keys::APOSTROPHE;
        mapping.mapping[0x02B] = Keys::BACKSLASH;
        mapping.mapping[0x033] = Keys::COMMA;
        mapping.mapping[0x00D] = Keys::EQUAL;
        mapping.mapping[0x029] = Keys::GRAVE_ACCENT;
        mapping.mapping[0x01A] = Keys::LEFT_BRACKET;
        mapping.mapping[0x00C] = Keys::MINUS;
        mapping.mapping[0x034] = Keys::PERIOD;
        mapping.mapping[0x01B] = Keys::RIGHT_BRACKET;
        mapping.mapping[0x027] = Keys::SEMICOLON;
        mapping.mapping[0x035] = Keys::SLASH;
        mapping.mapping[0x056] = Keys::WORLD_2;

        mapping.mapping[0x00E] = Keys::BACKSPACE;
        mapping.mapping[0x153] = Keys::DELETE;
        mapping.mapping[0x14F] = Keys::END;
        mapping.mapping[0x01C] = Keys::ENTER;
        mapping.mapping[0x001] = Keys::ESCAPE;
        mapping.mapping[0x147] = Keys::HOME;
        mapping.mapping[0x152] = Keys::INSERT;
        mapping.mapping[0x15D] = Keys::MENU;
        mapping.mapping[0x151] = Keys::PAGE_DOWN;
        mapping.mapping[0x149] = Keys::PAGE_UP;
        mapping.mapping[0x045] = Keys::PAUSE;
        mapping.mapping[0x146] = Keys::PAUSE;
        mapping.mapping[0x039] = Keys::SPACE;
        mapping.mapping[0x00F] = Keys::TAB;
        mapping.mapping[0x03A] = Keys::CAPS_LOCK;
        mapping.mapping[0x145] = Keys::NUM_LOCK;
        mapping.mapping[0x046] = Keys::SCROLL_LOCK;
        mapping.mapping[0x03B] = Keys::F1;
        mapping.mapping[0x03C] = Keys::F2;
        mapping.mapping[0x03D] = Keys::F3;
        mapping.mapping[0x03E] = Keys::F4;
        mapping.mapping[0x03F] = Keys::F5;
        mapping.mapping[0x040] = Keys::F6;
        mapping.mapping[0x041] = Keys::F7;
        mapping.mapping[0x042] = Keys::F8;
        mapping.mapping[0x043] = Keys::F9;
        mapping.mapping[0x044] = Keys::F10;
        mapping.mapping[0x057] = Keys::F11;
        mapping.mapping[0x058] = Keys::F12;
        mapping.mapping[0x064] = Keys::F13;
        mapping.mapping[0x065] = Keys::F14;
        mapping.mapping[0x066] = Keys::F15;
        mapping.mapping[0x067] = Keys::F16;
        mapping.mapping[0x068] = Keys::F17;
        mapping.mapping[0x069] = Keys::F18;
        mapping.mapping[0x06A] = Keys::F19;
        mapping.mapping[0x06B] = Keys::F20;
        mapping.mapping[0x06C] = Keys::F21;
        mapping.mapping[0x06D] = Keys::F22;
        mapping.mapping[0x06E] = Keys::F23;
        mapping.mapping[0x076] = Keys::F24;
        mapping.mapping[0x038] = Keys::LEFT_ALT;
        mapping.mapping[0x01D] = Keys::LEFT_CONTROL;
        mapping.mapping[0x02A] = Keys::LEFT_SHIFT;
        mapping.mapping[0x15B] = Keys::LEFT_SUPER;
        mapping.mapping[0x137] = Keys::PRINT_SCREEN;
        mapping.mapping[0x138] = Keys::RIGHT_ALT;
        mapping.mapping[0x11D] = Keys::RIGHT_CONTROL;
        mapping.mapping[0x036] = Keys::RIGHT_SHIFT;
        mapping.mapping[0x15C] = Keys::RIGHT_SUPER;
        mapping.mapping[0x150] = Keys::DOWN;
        mapping.mapping[0x14B] = Keys::LEFT;
        mapping.mapping[0x14D] = Keys::RIGHT;
        mapping.mapping[0x148] = Keys::UP;

        mapping.mapping[0x052] = Keys::KP_0;
        mapping.mapping[0x04F] = Keys::KP_1;
        mapping.mapping[0x050] = Keys::KP_2;
        mapping.mapping[0x051] = Keys::KP_3;
        mapping.mapping[0x04B] = Keys::KP_4;
        mapping.mapping[0x04C] = Keys::KP_5;
        mapping.mapping[0x04D] = Keys::KP_6;
        mapping.mapping[0x047] = Keys::KP_7;
        mapping.mapping[0x048] = Keys::KP_8;
        mapping.mapping[0x049] = Keys::KP_9;
        mapping.mapping[0x04E] = Keys::KP_ADD;
        mapping.mapping[0x053] = Keys::KP_DECIMAL;
        mapping.mapping[0x135] = Keys::KP_DIVIDE;
        mapping.mapping[0x11C] = Keys::KP_ENTER;
        mapping.mapping[0x037] = Keys::KP_MULTIPLY;
        mapping.mapping[0x04A] = Keys::KP_SUBTRACT;
    }

    void pollState(State& keyboard, Queue& queue) {
        memcpy(keyboard.last, keyboard.current, sizeof(u8) * Keys::COUNT);
        memset(keyboard.current, 0, sizeof(u8) * Keys::COUNT);
        for (int i = 0; i < Keys::COUNT; i++) {
            keyboard.current[i] = queue.keyStates[i];
        }
    }
};
};
#endif // __WASTELADNS_INPUT_DX11_H__
