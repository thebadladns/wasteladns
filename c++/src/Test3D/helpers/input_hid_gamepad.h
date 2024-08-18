#ifndef __WASTELADNS_INPUT_HID_GAMEPAD_H__
#define __WASTELADNS_INPUT_HID_GAMEPAD_H__

namespace Input {
namespace Gamepad {

    struct Type { enum Enum { NES_8BITDO, XBOX360, MAPPINGCOUNT, DUALSHOCK4 = MAPPINGCOUNT, TOTALCOUNT }; }; // no mapping for dualshock4 (special case)
    struct Mapping {
        DeviceInfo deviceInfo;
        const s32 keys_map[64]; // a bunch of them, just in case (todo: improve)
        const s32 dpad_map[9]; // todo: improve
        const s32 sliders_map[9]; // from HID_USAGE_GENERIC_X to HID_USAGE_GENERIC_WHEEL
    };
    Mapping mappings[Type::Enum::MAPPINGCOUNT] = {
        {     {} // NES_8BITDO
            , { KeyMask::BUTTON_E, KeyMask::BUTTON_S, 0, KeyMask::BUTTON_N, KeyMask::BUTTON_W, 0, KeyMask::L2, KeyMask::R2, KeyMask::L1, KeyMask::R1, KeyMask::SELECT, KeyMask::START, 0, KeyMask::LEFT_THUMB, KeyMask::RIGHT_THUMB } // only fill these, rest will be filled with 0
            , { KeyMask::DPAD_UP, KeyMask::DPAD_UP | KeyMask::DPAD_RIGHT, KeyMask::DPAD_RIGHT, KeyMask::DPAD_RIGHT | KeyMask::DPAD_DOWN, KeyMask::DPAD_DOWN, KeyMask::DPAD_DOWN | KeyMask::DPAD_LEFT, KeyMask::DPAD_LEFT, KeyMask::DPAD_LEFT | KeyMask::DPAD_UP }
            , { Sliders::AXIS_X_LEFT, Sliders::AXIS_Y_LEFT, Sliders::AXIS_X_RIGHT, -1, -1, Sliders::AXIS_Y_RIGHT, -1, -1, -1 }
        }
        , {   {} // XBOX // todo: test properly
            , { KeyMask::BUTTON_S, KeyMask::BUTTON_E, KeyMask::BUTTON_W, KeyMask::BUTTON_N, KeyMask::L1, KeyMask::R1, KeyMask::SELECT, KeyMask::START, KeyMask::LEFT_THUMB, KeyMask::RIGHT_THUMB, KeyMask::LEFT_THUMB } // only fill these, rest will be filled with 0
            , { 0, KeyMask::DPAD_UP, KeyMask::DPAD_UP | KeyMask::DPAD_RIGHT, KeyMask::DPAD_RIGHT, KeyMask::DPAD_RIGHT | KeyMask::DPAD_DOWN, KeyMask::DPAD_DOWN, KeyMask::DPAD_DOWN | KeyMask::DPAD_LEFT, KeyMask::DPAD_LEFT, KeyMask::DPAD_LEFT | KeyMask::DPAD_UP }
            , { Sliders::AXIS_X_LEFT, Sliders::AXIS_Y_LEFT, Sliders::AXIS_X_RIGHT, -1, -1, Sliders::AXIS_Y_RIGHT, -1, -1, -1 } 
        }
    };

#if __DEBUG
    const char* names[Type::Enum::TOTALCOUNT] = { "8Bitdo", "Xbox360", "Dualshock4" };
#endif

    Type::Enum ComputeControllerType(unsigned long vendorID, unsigned long productID) {
        const unsigned long sonyVendorID = 0x054C;
        const unsigned long ds4Gen1ProductID = 0x05C4;
        const unsigned long ds4Gen2ProductID = 0x09CC;
        const unsigned long microsoftVendorID = 0x045e;
        const unsigned long xbox360wireless3rdparty1 = 0x02a9;
        const unsigned long xbox360wireless3rdparty2 = 0x0291;
        const unsigned long xbox360wireless = 0x028e;
        if (vendorID == sonyVendorID && (productID == ds4Gen1ProductID || productID == ds4Gen2ProductID)) {
            return Type::DUALSHOCK4;
        }
        else if (vendorID == microsoftVendorID && (productID == xbox360wireless3rdparty1 || productID == xbox360wireless3rdparty2 || productID == xbox360wireless)) {
            return Type::XBOX360;
        }
        else {
            return Type::NES_8BITDO;
        }
    }
    f32 TranslateRawSliderValue(SliderInfo& sliderInfo, const unsigned long value, const Sliders::Enum type) {
        // Perform auto calibration
        if (value < sliderInfo.min) sliderInfo.min = value;
        if (value > sliderInfo.max) sliderInfo.max = value;
        const unsigned long size = sliderInfo.max - sliderInfo.min;
        f32 slider = 0.f;
        if (size > 0) slider = (2.f * (value - sliderInfo.min) / size) - 1.f;
        if (Math::abs(slider) < 0.05f) slider = 0.f;
        if (type == Sliders::AXIS_Y_LEFT || type == Sliders::AXIS_Y_RIGHT) slider = -slider;
        return slider;
    }

	namespace Dualshock4 {
        struct ControllerData {
            u8 axis_left_x;
            u8 axis_left_y;
            u8 axis_right_x;
            u8 axis_right_y;
            u8 axis_dpad : 4;
            bool button_square : 1;
            bool button_cross : 1;
            bool button_circle : 1;
            bool button_triangle : 1;
            bool button_left_1 : 1;
            bool button_right_1 : 1;
            bool button_left_2 : 1;
            bool button_right_2 : 1;
            bool button_share : 1;
            bool button_options : 1;
            bool button_left_3 : 1;
            bool button_right_3 : 1;
            bool button_ps : 1;
            bool button_touch : 1;
            u8 sequence_number : 6;
            u8 axis_left_2;
            u8 axis_right_2;
            // for remaining buttons check https://github.com/chromium/chromium/blob/main/device/gamepad/dualshock4_controller.cc#L44
        };
        void parseDualshock4(State& pad, const u8 reportID, const u8* data) {
            const u8* rawdata = data;
            if (reportID == 0x11) { // Bluetooth, USB is 0x01
                rawdata = &(data[2]);
            }

            ControllerData& controller = *(ControllerData*)rawdata;
            u16 keys = 0;
            if (controller.button_square) keys |= KeyMask::BUTTON_W;
            if (controller.button_cross) keys |= KeyMask::BUTTON_S;
            if (controller.button_circle) keys |= KeyMask::BUTTON_E;
            if (controller.button_triangle) keys |= KeyMask::BUTTON_N;
            if (controller.button_options) keys |= KeyMask::START;
            if (controller.button_share) keys |= KeyMask::SELECT;
            if (controller.button_left_3) keys |= KeyMask::LEFT_THUMB;
            if (controller.button_right_3) keys |= KeyMask::RIGHT_THUMB;
            if (controller.button_left_1) keys |= KeyMask::L1;
            if (controller.button_right_1) keys |= KeyMask::R1;
            if (controller.button_left_2) keys |= KeyMask::L2;
            if (controller.button_right_2) keys |= KeyMask::R2;
            switch (controller.axis_dpad) { // todo: simplify?
            case 0: keys |= (KeyMask::DPAD_UP); break;
            case 1: keys |= (KeyMask::DPAD_UP) | (KeyMask::DPAD_RIGHT); break;
            case 2: keys |= (KeyMask::DPAD_RIGHT); break;
            case 3: keys |= (KeyMask::DPAD_RIGHT) | (KeyMask::DPAD_DOWN); break;
            case 4: keys |= (KeyMask::DPAD_DOWN); break;
            case 5: keys |= (KeyMask::DPAD_DOWN) | (KeyMask::DPAD_LEFT); break;
            case 6: keys |= (KeyMask::DPAD_LEFT); break;
            case 7: keys |= (KeyMask::DPAD_LEFT) | (KeyMask::DPAD_UP); break;
            }
            pad.last_keys = pad.curr_keys;
            pad.curr_keys = keys;

            auto normalizeAxis = [](u8 v) {
                f32 rawAxis = (2.0f * v / 255.f) - 1.0f;
                if (Math::abs(rawAxis) < 0.05f) rawAxis = 0.f;
                return rawAxis;
                };
            pad.sliders[Sliders::AXIS_X_LEFT] = normalizeAxis(controller.axis_left_x);
            pad.sliders[Sliders::AXIS_Y_LEFT] = -normalizeAxis(controller.axis_left_y);
            pad.sliders[Sliders::AXIS_X_RIGHT] = normalizeAxis(controller.axis_right_x);
            pad.sliders[Sliders::AXIS_Y_RIGHT] = -normalizeAxis(controller.axis_right_y);
            pad.sliders[Sliders::TRIGGER_LEFT] = controller.axis_left_2 / 255.f;
            pad.sliders[Sliders::TRIGGER_RIGHT] = controller.axis_right_2 / 255.f;
        }
	}
}
}

#endif // __WASTELADNS_INPUT_HID_GAMEPAD_H__
