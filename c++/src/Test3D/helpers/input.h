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
        namespace Keys { enum Enum {
              BUTTON_N = 0
            , BUTTON_S = 1
            , BUTTON_W = 2
            , BUTTON_E = 3
            , DPAD_UP = 4
            , DPAD_DOWN = 5
            , DPAD_LEFT = 6
            , DPAD_RIGHT = 7
            , START = 8
            , SELECT = 9
            , LEFT_THUMB = 10
            , RIGHT_THUMB = 11
            , L1 = 12
            , R1 = 13
            , L2 = 14
            , R2 = 15
            , COUNT
        }; };
        namespace Sliders {enum Enum {
              AXIS_X_LEFT = 0
            , LX = AXIS_X_LEFT
            , AXIS_Y_LEFT = 1
            , LY = AXIS_Y_LEFT
            , AXIS_X_RIGHT = 2
            , RX = AXIS_X_RIGHT
            , AXIS_Y_RIGHT = 3
            , RY = AXIS_Y_RIGHT
            , TRIGGER_LEFT = 4
            , LT = TRIGGER_LEFT
            , TRIGGER_RIGHT = 5
            , RT = TRIGGER_RIGHT
            , COUNT = 6
        }; };
        namespace Type { enum Enum { GENERIC, DUALSHOCK4, COUNT }; };
        struct State {
            Gamepad::DeviceHandle deviceHandle;
            u16 last_keys;
            u16 curr_keys;
            f32 sliders[Sliders::COUNT];
            Type::Enum type;
            __DEBUGDEF(char name[128]);
        };
    }
//    namespace Gamepad {
//        
//        namespace Keys { enum Enum : s32 {
//              B_U = 0
//            , B_D = 1
//            , B_L = 2
//            , B_R = 3
//            , D_U = 4
//            , D_D = 5
//            , D_L = 6
//            , D_R = 7
//            , START = 8
//            , SELECT = 9
//            , L1 = 10
//            , R1 = 11
//            , A_L = 12
//            , A_R = 13
//            , L2 = 14
//            , R2 = 15
//            , TOUCH = 16
//            , COUNT
//            , INVALID = -1
//        }; };
//
//        namespace Analog { enum Enum : s8 {
//              AxisLH = 0
//            , AxisLV
//            , AxisRH
//            , AxisRV
//            , Trigger_L
//            , Trigger_R
//            , COUNT
//            , INVALID = -1
//        }; };
//
////        struct Mapping {
////            const Digital::Enum* b_mapping;
////            const Analog::Enum* a_mapping;
////            s32 b_mappingCount;
////            s32 a_mappingCount;
////            u32 name;
////            bool set = false;
////        };
//
////        bool loadPreset(Gamepad::Mapping& mapping, const char* name);
//
//        struct State {
//            
//            struct KeyState {
//                u32 last;
//                u32 current;
//            };
//            
//            bool down(Keys::Enum key) const {
//                return (keys.current & (1 << key)) != 0;
//            }
//            bool up(Keys::Enum key) const {
//                return (keys.current & (1 << key)) == 0;
//            }
//            bool released(Keys::Enum key) const {
//                const u32 mask = (1 << key);
//                return (keys.current & mask) == 0 && (keys.last & mask) != 0;
//            }
//            bool pressed(Keys::Enum key) const {
//                const u32 mask = (1 << key);
//                return (keys.current & mask) != 0 && (keys.last & mask) == 0;
//            }
//            
//            KeyState keys;
//            f32 analogs[Analog::COUNT];
//            bool active = false;
//        };
//    };

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
        queue[keycode] = KeyEvent::Pressed + (KeyEvent::None << KeyEvent::FrameShift);
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
