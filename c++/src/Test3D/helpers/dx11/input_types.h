#ifndef __WASTELADNS_INPUT_TYPES_DX11_H__
#define __WASTELADNS_INPUT_TYPES_DX11_H__

namespace Input {
    
    namespace Keyboard {
        namespace Keys { enum Enum : s32 {
              SPACE = 0x039
            , APOSTROPHE = 0x028 /* ' */
            , COMMA = 0x033 /* , */
            , MINUS = 0x00C /* - */
            , PERIOD = 0x034 /* . */
            , SLASH = 0x035 /* / */
            , SEMICOLON = 0x027 /* ; */
            , EQUAL = 0x00D /* = */
            , NUM0 = 0x00B
            , NUM1 = 0x002
            , NUM2 = 0x003
            , NUM3 = 0x004
            , NUM4 = 0x005
            , NUM5 = 0x006
            , NUM6 = 0x007
            , NUM7 = 0x008
            , NUM8 = 0x009
            , NUM9 = 0x00A
            , A = 0x01E
            , B = 0x030
            , C = 0x02E
            , D = 0x020
            , E = 0x012
            , F = 0x021
            , G = 0x022
            , H = 0x023
            , I = 0x017
            , J = 0x024
            , K = 0x025
            , L = 0x026
            , M = 0x032
            , N = 0x031
            , O = 0x018
            , P = 0x019
            , Q = 0x010
            , R = 0x013
            , S = 0x01F
            , T = 0x014
            , U = 0x016
            , V = 0x02F
            , W = 0x011
            , X = 0x02D
            , Y = 0x015
            , Z = 0x02C
            , LEFT_BRACKET = 0x01A /* [ */
            , BACKSLASH = 0x02B /* \ */
            , RIGHT_BRACKET = 0x01B /* ] */
            , GRAVE_ACCENT = 0x029 /* ` */
            , WORLD_2 = 0x056
            , ESCAPE = 0x001
            , ENTER = 0x01C
            , TAB = 0x00F
            , BACKSPACE = 0x00E
            , INSERT = 0x152
            , DELETE = 0x153
            , RIGHT = 0x14D
            , LEFT = 0x14B
            , DOWN = 0x150
            , UP = 0x148
            , PAGE_UP = 0x149
            , PAGE_DOWN = 0x151
            , HOME = 0x147
            , END = 0x14F
            , CAPS_LOCK = 0x03A
            , SCROLL_LOCK = 0x046
            , NUM_LOCK = 0x145
            , PRINT_SCREEN = 0x137
            , F1 = 0x03B
            , F2 = 0x03C
            , F3 = 0x03D
            , F4 = 0x03E
            , F5 = 0x03F
            , F6 = 0x040
            , F7 = 0x041
            , F8 = 0x042
            , F9 = 0x043
            , F10 = 0x044
            , F11 = 0x057
            , F12 = 0x058
            , F13 = 0x064
            , F14 = 0x065
            , F15 = 0x066
            , F16 = 0x067
            , F17 = 0x068
            , F18 = 0x069
            , F19 = 0x06A
            , F20 = 0x06B
            , F21 = 0x06C
            , F22 = 0x06D
            , F23 = 0x06E
            , F24 = 0x076
            , KP_0 = 0x052
            , KP_1 = 0x04F
            , KP_2 = 0x050
            , KP_3 = 0x051
            , KP_4 = 0x04B
            , KP_5 = 0x04C
            , KP_6 = 0x04D
            , KP_7 = 0x047
            , KP_8 = 0x048
            , KP_9 = 0x049
            , KP_DECIMAL = 0x053
            , KP_DIVIDE = 0x135
            , KP_ADD = 0x049
            , KP_SUBTRACT = 0x04A
            , KP_ENTER = 0x11C
            , KP_MULTIPLY = 0x037
            , LEFT_SHIFT = 0x02A
            , LEFT_CONTROL = 0x01D
            , LEFT_ALT = 0x038
            , LEFT_SUPER = 0x15B
            , RIGHT_SHIFT = 0x036
            , RIGHT_CONTROL = 0x11D
            , RIGHT_ALT = 0x138
            , RIGHT_SUPER = 0x15C
            , MENU = 0x15D
            , PAUSE = 0x045
            , COUNT
            , INVALID = -1
        }; };
    };

    namespace Mouse {
        namespace Keys { enum Enum : s32 {
              BUTTON_LEFT = 0
            , BUTTON_RIGHT
            , BUTTON_MIDDLE
            , COUNT
        }; };
    };

    namespace Gamepad {

        struct SliderInfo {
            USAGE usage;
            USAGE usagePage;
            u32 min;
            u32 max;
        };

        struct DeviceInfo {
            SliderInfo sliders_info[10]; // from HID_USAGE_GENERIC_X to HID_USAGE_GENERIC_HATSWITCH
            _HIDP_PREPARSED_DATA* preparsedData;
            USAGE keys_usage_page;
            USAGE keys_usage_min;
            u32 keys_count;
            u32 sliders_count;
            bool loaded;
        };

        typedef HANDLE DeviceHandle;
    };
    //namespace Gamepad {
    //    typedef LPDIRECTINPUTDEVICE8 Device;
    //};
};

#endif // __WASTELADNS_INPUT_TYPES_DX11_H__
