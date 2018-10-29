#ifndef __WASTELADNS_INPUT_H__
#define __WASTELADNS_INPUT_H__

#ifndef UNITYBUILD
#include "hash.h"
#endif

namespace Input {
    
    namespace Keyboard {
        namespace Keys { enum Enum : s32 {
              SPACE = 0
            , APOSTROPHE /* ' */
            , COMMA /* , */
            , MINUS /* - */
            , PERIOD /* . */
            , SLASH /* / */
            , NUM0
            , NUM1
            , NUM2
            , NUM3
            , NUM4
            , NUM5
            , NUM6
            , NUM7
            , NUM8
            , NUM9
            , SEMICOLON /* ; */
            , EQUAL /* = */
            , A
            , B
            , C
            , D
            , E
            , F
            , G
            , H
            , I
            , J
            , K
            , L
            , M
            , N
            , O
            , P
            , Q
            , R
            , S
            , T
            , U
            , V
            , W
            , X
            , Y
            , Z
            , LEFT_BRACKET /* [ */
            , BACKSLASH /* \ */
            , RIGHT_BRACKET /* ] */
            , GRAVE_ACCENT /* ` */
            , WORLD_1
            , WORLD_2
            , ESCAPE
            , ENTER
            , TAB
            , BACKSPACE
            , INSERT
            , DELETE
            , RIGHT
            , LEFT
            , DOWN
            , UP
            , PAGE_UP
            , PAGE_DOWN
            , HOME
            , END
            , CAPS_LOCK
            , SCROLL_LOCK
            , NUM_LOCK
            , PRINT_SCREEN
            , PAUSE
            , F1
            , F2
            , F3
            , F4
            , F5
            , F6
            , F7
            , F8
            , F9
            , F10
            , F11
            , F12
            , F13
            , F14
            , F15
            , F16
            , F17
            , F18
            , F19
            , F20
            , F21
            , F22
            , F23
            , F24
            , F25
            , KP_0
            , KP_1
            , KP_2
            , KP_3
            , KP_4
            , KP_5
            , KP_6
            , KP_7
            , KP_8
            , KP_9
            , KP_DECIMAL
            , KP_DIVIDE
            , KP_MULTIPLY
            , KP_SUBTRACT
            , KP_ADD
            , KP_ENTER
            , KP_EQUAL
            , LEFT_SHIFT
            , LEFT_CONTROL
            , LEFT_ALT
            , LEFT_SUPER
            , RIGHT_SHIFT
            , RIGHT_CONTROL
            , RIGHT_ALT
            , RIGHT_SUPER
            , MENU
            , COUNT
            , INVALID = -1
        }; };
        
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
        
        namespace Keys { enum Enum : s32 {
              B_U = 0
            , B_D = 1
            , B_L = 2
            , B_R = 3
            , D_U = 4
            , D_D = 5
            , D_L = 6
            , D_R = 7
            , START = 8
            , SELECT = 9
            , L1 = 10
            , R1 = 11
            , A_L = 12
            , A_R = 13
            , L2 = 14
            , R2 = 15
            , TOUCH = 16
            , COUNT
            , INVALID = -1
        }; };

        namespace Analog { enum Enum : s8 {
              AxisLH = 0
            , AxisLV
            , AxisRH
            , AxisRV
            , Trigger_L
            , Trigger_R
            , COUNT
            , INVALID = -1
        }; };

//        struct Mapping {
//            const Digital::Enum* b_mapping;
//            const Analog::Enum* a_mapping;
//            s32 b_mappingCount;
//            s32 a_mappingCount;
//            u32 name;
//            bool set = false;
//        };

//        bool loadPreset(Gamepad::Mapping& mapping, const char* name);

        struct State {
            
            struct KeyState {
                u32 last;
                u32 current;
            };
            
            bool down(Keys::Enum key) const {
                return (keys.current & (1 << key)) != 0;
            }
            bool up(Keys::Enum key) const {
                return (keys.current & (1 << key)) == 0;
            }
            bool released(Keys::Enum key) const {
                const u32 mask = (1 << key);
                return (keys.current & mask) == 0 && (keys.last & mask) != 0;
            }
            bool pressed(Keys::Enum key) const {
                const u32 mask = (1 << key);
                return (keys.current & mask) != 0 && (keys.last & mask) == 0;
            }
            
            KeyState keys;
            f32 analogs[Analog::COUNT];
            bool active = false;
        };
    };
};

//#include "input_mappings.h"

#endif // __WASTELADNS_INPUT_H__
