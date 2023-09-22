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
        mapping.mapping[Keys::D_U] = Keyboard::Keys::W;
        mapping.mapping[Keys::D_D] = Keyboard::Keys::S;
        mapping.mapping[Keys::D_L] = Keyboard::Keys::A;
        mapping.mapping[Keys::D_R] = Keyboard::Keys::D;
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
#endif // __WASTELADNS_INPUT_DX11_H__
