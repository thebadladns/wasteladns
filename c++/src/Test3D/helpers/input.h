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
        struct Mapping {
            DeviceInfo deviceInfo;
            const s32 keys_map[64];
            const s32 dpad_map[8];
            const s32 sliders_map[9];
        };
        struct Type { enum Enum { NES_8BITDO, XBOX360, MAPPINGCOUNT, DUALSHOCK4 = MAPPINGCOUNT, TOTALCOUNT }; }; // no mapping for dualshock4 (special case)
        Mapping mappings[Type::Enum::MAPPINGCOUNT] = {
            {     {} // NES_8BITDO
                , { KeyMask::BUTTON_E, KeyMask::BUTTON_S, 0, KeyMask::BUTTON_N, KeyMask::BUTTON_W, 0, KeyMask::L2, KeyMask::R2, KeyMask::L1, KeyMask::R1, KeyMask::SELECT, KeyMask::START, 0, KeyMask::LEFT_THUMB, KeyMask::RIGHT_THUMB } // only fill these, rest will be filled with 0
                , { KeyMask::DPAD_UP, KeyMask::DPAD_UP|KeyMask::DPAD_RIGHT, KeyMask::DPAD_RIGHT, KeyMask::DPAD_RIGHT|KeyMask::DPAD_DOWN, KeyMask::DPAD_DOWN, KeyMask::DPAD_DOWN|KeyMask::DPAD_LEFT, KeyMask::DPAD_LEFT, KeyMask::DPAD_LEFT|KeyMask::DPAD_UP }
                , { Sliders::AXIS_X_LEFT, Sliders::AXIS_Y_LEFT, Sliders::AXIS_X_RIGHT, -1, -1, Sliders::AXIS_Y_RIGHT, -1, -1, -1 } // from HID_USAGE_GENERIC_X to HID_USAGE_GENERIC_WHEEL
             }
            , {   {} // XBOX
                , { KeyMask::BUTTON_S, KeyMask::BUTTON_E, KeyMask::BUTTON_W, KeyMask::BUTTON_N, KeyMask::L1, KeyMask::R1, KeyMask::SELECT, KeyMask::START, KeyMask::LEFT_THUMB, KeyMask::RIGHT_THUMB, KeyMask::LEFT_THUMB } // only fill these, rest will be filled with 0
                , { KeyMask::DPAD_UP, KeyMask::DPAD_UP | KeyMask::DPAD_RIGHT, KeyMask::DPAD_RIGHT, KeyMask::DPAD_RIGHT | KeyMask::DPAD_DOWN, KeyMask::DPAD_DOWN, KeyMask::DPAD_DOWN | KeyMask::DPAD_LEFT, KeyMask::DPAD_LEFT, KeyMask::DPAD_LEFT | KeyMask::DPAD_UP }
                , { Sliders::AXIS_X_LEFT, Sliders::AXIS_Y_LEFT, Sliders::AXIS_X_RIGHT, -1, -1, Sliders::AXIS_Y_RIGHT, -1, -1, -1 } // from HID_USAGE_GENERIC_X to HID_USAGE_GENERIC_WHEEL
             }
        };
        #if __DEBUG
        const char* names[Type::Enum::TOTALCOUNT] = { "8Bitdo", "Xbox360", "Dualshock4" };
        #endif

        struct State {
            Gamepad::DeviceHandle deviceHandle;
            u16 last_keys;
            u16 curr_keys;
            f32 sliders[Sliders::COUNT];
            Type::Enum type;
            __DEBUGDEF(char name[128]);
        };
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

//#include "input_mappings.h"

#endif // __WASTELADNS_INPUT_H__
