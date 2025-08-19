#ifndef __WASTELADNS_INPUT_DEFS_MACOS_H__
#define __WASTELADNS_INPUT_DEFS_MACOS_H__

namespace input {
    
namespace keyboard {
    struct Keys { enum Enum : s32 {
          SPACE = 0x31
        , APOSTROPHE = 0x27 /* ' */
        , COMMA = 0x2B /* , */
        , MINUS = 0x1B /* - */
        , PERIOD = 0x2F /* . */
        , SLASH = 0x2C /* / */
        , SEMICOLON = 0x29 /* ; */
        , EQUAL = 0x18 /* = */
        , NUM0 = 0x1D
        , NUM1 = 0x12
        , NUM2 = 0x13
        , NUM3 = 0x14
        , NUM4 = 0x15
        , NUM5 = 0x17
        , NUM6 = 0x16
        , NUM7 = 0x1A
        , NUM8 = 0x1C
        , NUM9 = 0x19
        , A = 0x00
        , B = 0x0B
        , C = 0x08
        , D = 0x02
        , E = 0x0E
        , F = 0x03
        , G = 0x05
        , H = 0x04
        , I = 0x22
        , J = 0x26
        , K = 0x28
        , L = 0x25
        , M = 0x2E
        , N = 0x2D
        , O = 0x1F
        , P = 0x23
        , Q = 0x0C
        , R = 0x0F
        , S = 0x01
        , T = 0x11
        , U = 0x20
        , V = 0x09
        , W = 0x0D
        , X = 0x07
        , Y = 0x10
        , Z = 0x06
        , LEFT_BRACKET = 0x21 /* [ */
        , BACKSLASH = 0x2A /* \ */
        , RIGHT_BRACKET = 0x1E /* ] */
        , GRAVE_ACCENT = 0x32 /* ` */
        , WORLD_1 = 0x0A
        , ESCAPE = 0x35
        , ENTER = 0x24
        , TAB = 0x30
        , BACKSPACE = 0x33
        , INSERT = 0x72
        , DELETE = 0x75
        , RIGHT = 0x7C
        , LEFT = 0x7B
        , DOWN = 0x7D
        , UP = 0x7E
        , PAGE_UP = 0x74
        , PAGE_DOWN = 0x79
        , HOME = 0x73
        , END = 0x77
        , CAPS_LOCK = 0x39
        , NUM_LOCK = 0x47
        , PRINT_SCREEN = 0x69
        , F1 = 0x7A
        , F2 = 0x78
        , F3 = 0x63
        , F4 = 0x76
        , F5 = 0x60
        , F6 = 0x61
        , F7 = 0x62
        , F8 = 0x64
        , F9 = 0x65
        , F10 = 0x6D
        , F11 = 0x67
        , F12 = 0x6F
        , F14 = 0x6B
        , F15 = 0x71
        , F16 = 0x6A
        , F17 = 0x40
        , F18 = 0x4F
        , F19 = 0x50
        , F20 = 0x5A
        , KP_0 = 0x52
        , KP_1 = 0x53
        , KP_2 = 0x54
        , KP_3 = 0x55
        , KP_4 = 0x56
        , KP_5 = 0x57
        , KP_6 = 0x58
        , KP_7 = 0x59
        , KP_8 = 0x5B
        , KP_9 = 0x5C
        , KP_DECIMAL = 0x41
        , KP_DIVIDE = 0x4B
        , KP_SUBTRACT = 0x4E
        , KP_ENTER = 0x4C
        , KP_EQUAL = 0x43
        , LEFT_SHIFT = 0x38
        , LEFT_CONTROL = 0x3B
        , LEFT_ALT = 0x3A
        , LEFT_SUPER = 0x37
        , RIGHT_SHIFT = 0x3C
        , RIGHT_CONTROL = 0x3E
        , RIGHT_ALT = 0x3D
        , RIGHT_SUPER = 0x36
        , MENU = 0x6E
        , COUNT = 0x7F
        , INVALID = -1
    }; };
} // keyboard
namespace mouse {
    struct Keys { enum Enum : s32 {
          BUTTON_LEFT = 0
        , BUTTON_RIGHT
        , BUTTON_MIDDLE
        , COUNT
    }; };
} // mouse
} //input

#endif // __WASTELADNS_INPUT_DEFS_MACOS_H__
