#ifndef __WASTELADNS_INPUT_H__
#define __WASTELADNS_INPUT_H__

#ifndef UNITYBUILD
#include "hash.h"
#endif

namespace Input {
    
    namespace Keyboard {
        
        struct State {
            bool down(Keys::Enum key) const {
                return current[key] != 0;
            }
            bool up(Keys::Enum key) const {
                return current[key] == 0;
            }
            bool released(Keys::Enum key) const {
                return current[key] == 0 && last[key] != 0;
            }
            bool pressed(Keys::Enum key) const {
                return current[key] != 0 && last[key] == 0;
            }

            u8 last[Keys::COUNT];
            u8 current[Keys::COUNT];
        };
    };

    namespace Gamepad {
        struct KeyMask { enum Enum {
              BUTTON_N = 1 << 0
            , BUTTON_S = 1 << 1
            , BUTTON_W = 1 << 2
            , BUTTON_E = 1 << 3
            , DPAD_UP = 1 << 4
            , DPAD_DOWN = 1 << 5
            , DPAD_LEFT = 1 << 6
            , DPAD_RIGHT = 1 << 7
            , START = 1 << 8
            , SELECT = 1 << 9
            , L1 = 1 << 10
            , R1 = 1 << 11
            , L2 = 1 << 12
            , R2 = 1 << 13
            , LEFT_THUMB = 1 << 14
            , RIGHT_THUMB = 1 << 15
            , COUNT = 16
        }; };
        struct Sliders { enum Enum {
              AXIS_X_LEFT = 0
            , AXIS_Y_LEFT = 1
            , AXIS_X_RIGHT = 2
            , AXIS_Y_RIGHT = 3
            , TRIGGER_LEFT = 4
            , TRIGGER_RIGHT = 5
            , COUNT = 6
        }; };

        struct State {
            bool down(KeyMask::Enum keyMask) const {
                return (curr_keys & keyMask) != 0;
            }
            bool up(KeyMask::Enum keyMask) const {
                return (curr_keys & keyMask) == 0;
            }
            bool released(KeyMask::Enum keyMask) const {
                return (curr_keys & keyMask) == 0 && (last_keys & keyMask) != 0;
            }
            bool pressed(KeyMask::Enum keyMask) const {
                return (curr_keys & keyMask) != 0 && (last_keys & keyMask) == 0;
            }
            u64 deviceHandle;
            u16 last_keys;
            u16 curr_keys;
            f32 sliders[Sliders::COUNT];
            u32 type;
            __DEBUGDEF(char name[128]);
        };

        bool hasValidInput(State& pad) {
            bool validpad = pad.curr_keys != 0;
            for (u32 slider = 0; slider < Sliders::COUNT; slider++) {
                if (Math::abs(pad.sliders[slider]) > 2.f) { // TODO: properly ignore bad inputs
                    validpad = false;
                }
                else if (Math::abs(pad.sliders[slider]) > 0.9f) {
                    validpad = validpad || true;
                }
            }
            return validpad;
        }
    }

	namespace Mouse {
    
		struct State {
			bool down(Keys::Enum key) const {
				return current[key] != 0;
			}
			bool up(Keys::Enum key) const {
				return current[key] == 0;
			}
			bool released(Keys::Enum key) const {
				return current[key] == 0 && last[key] != 0;
			}
			bool pressed(Keys::Enum key) const {
				return current[key] != 0 && last[key] == 0;
			}

			u8 last[Keys::COUNT];
			u8 current[Keys::COUNT];
			f32 dx;
			f32 dy;
			f32 x;
			f32 y;
			f32 scrolldx;
			f32 scrolldy;
		};
	};

    // Queue up to two key down / key up events per frame (to handle the cases where a key is pressed and released within the same frame)
    struct KeyEvent { enum Enum { None = 0b00, Released = 0b01, Pressed = 0b10, FrameShift = 2, FrameMask = 0b11, FullMask = 0b1111 }; };
    inline void queueKeyUp(u8* queue, const u8* current, const u32 keycode) {
        if (!current[keycode] && (queue[keycode] & KeyEvent::FrameMask) == KeyEvent::Pressed) {
            // we're already doing a button press this frame, release it next frame
            queue[keycode] |= KeyEvent::Released << KeyEvent::FrameShift;
        } else {
            queue[keycode] = KeyEvent::Released;
        }
    }
    inline void queueKeyDown(u8* queue, const u8*, const u32 keycode) {
        queue[keycode] = KeyEvent::Pressed; // also clears next frame with (KeyEvent::None << KeyEvent::FrameShift);
    }
    inline void unqueueKey(u8* current, u8* queue, const u32 keycode) {
        u8 queuedValueThisFrame = (queue[keycode] & KeyEvent::FrameMask);
        if (queuedValueThisFrame) {
            current[keycode] = queuedValueThisFrame - 1;
        }
        queue[keycode] >>= KeyEvent::FrameShift;
    }
};

#endif // __WASTELADNS_INPUT_H__
