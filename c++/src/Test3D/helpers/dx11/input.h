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

    void processPads(Allocator::Arena scratchArena, ::Input::Gamepad::State* pads, u32& padCount, const u32 maxPadCount, const HRAWINPUT lParam) {
        namespace Pad = Input::Gamepad;

        UINT bufferSize;
        const ptrdiff_t align = 16;

        GetRawInputData(lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));
        RAWINPUT* input = (RAWINPUT*)Allocator::alloc_arena(scratchArena, bufferSize, align);
        bool gotInput = GetRawInputData(lParam, RID_INPUT, input, &bufferSize, sizeof(RAWINPUTHEADER)) > 0;
        if (gotInput) {
            s32 currentpad = -1;
            for (s32 pad = 0; pad < (s32)padCount; pad++) {
                if (pads[pad].deviceHandle == input->header.hDevice) {
                    currentpad = pad;
                    break;
                }
            }
            if (currentpad < 0 && padCount < maxPadCount) {
                currentpad = padCount++;
                ::Input::Gamepad::State& padToCreate = pads[currentpad];
                padToCreate = {};
                padToCreate.deviceHandle = input->header.hDevice;

                RID_DEVICE_INFO deviceInfo;
                UINT deviceInfoSize = sizeof(deviceInfo);
                bool gotInfo = GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize) > 0;
                const DWORD sonyVendorID = 0x054C;
                const DWORD ds4Gen1ProductID = 0x05C4;
                const DWORD ds4Gen2ProductID = 0x09CC;
                bool dualshock = deviceInfo.hid.dwVendorId == sonyVendorID
                    || (deviceInfo.hid.dwProductId == ds4Gen1ProductID || (deviceInfo.hid.dwProductId == ds4Gen2ProductID));
                padToCreate.type = dualshock ? ::Input::Gamepad::Type::DUALSHOCK4 : ::Input::Gamepad::Type::GENERIC;

#if __DEBUG
                u32 deviceNameSize = sizeof(padToCreate.name);
                GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICENAME, padToCreate.name, &deviceNameSize);

                Platform::debuglog("Registered new pad %s\n", padToCreate.name);
#endif
            }
            ::Input::Gamepad::State& pad = pads[currentpad];

            if (pad.type == ::Input::Gamepad::Type::DUALSHOCK4) { // handle Dualshock4 separately, since it doesn't specify HID gamepad usages in the report
                BYTE report_id = input->data.hid.bRawData[0];
                BYTE* rawdata = &input->data.hid.bRawData[1];
                if (report_id == 0x11) { // Bluetooth, USB is 0x01
                    rawdata = &input->data.hid.bRawData[3];
                }

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

                ControllerData& controller = *(ControllerData*)rawdata;
                u16 keys = 0;
                if (controller.button_square) keys |= Pad::Keys::BUTTON_W;
                if (controller.button_cross) keys |= Pad::Keys::BUTTON_S;
                if (controller.button_circle) keys |= 1 << Pad::Keys::BUTTON_E;
                if (controller.button_triangle) keys |= 1 << Pad::Keys::BUTTON_N;
                if (controller.button_options) keys |= 1 << Pad::Keys::START;
                if (controller.button_share) keys |= 1 << Pad::Keys::SELECT;
                if (controller.button_left_3) keys |= 1 << Pad::Keys::LEFT_THUMB;
                if (controller.button_right_3) keys |= 1 << Pad::Keys::RIGHT_THUMB;
                if (controller.button_left_1) keys |= 1 << Pad::Keys::L1;
                if (controller.button_right_1) keys |= 1 << Pad::Keys::R1;
                if (controller.button_left_2) keys |= 1 << Pad::Keys::L2;
                if (controller.button_right_2) keys |= 1 << Pad::Keys::R2;
                switch (controller.axis_dpad) { // todo: simplify?
                case 0: keys |= (1 << Pad::Keys::DPAD_UP); break;
                case 1: keys |= (1 << Pad::Keys::DPAD_UP) | (1 << Pad::Keys::DPAD_RIGHT); break;
                case 2: keys |= (1 << Pad::Keys::DPAD_RIGHT); break;
                case 3: keys |= (1 << Pad::Keys::DPAD_RIGHT) | (1 << Pad::Keys::DPAD_DOWN); break;
                case 4: keys |= (1 << Pad::Keys::DPAD_DOWN); break;
                case 5: keys |= (1 << Pad::Keys::DPAD_DOWN) | (1 << Pad::Keys::DPAD_LEFT); break;
                case 6: keys |= (1 << Pad::Keys::DPAD_LEFT); break;
                case 7: keys |= (1 << Pad::Keys::DPAD_LEFT) | (1 << Pad::Keys::DPAD_UP); break;
                }
                pad.last_keys = pad.curr_keys;
                pad.curr_keys = keys;

                auto normalizeAxis = [](u8 v) {
                    f32 rawAxis = (2.0f * v / 255.f) - 1.0f;
                    if (Math::abs(rawAxis) < 0.1f) rawAxis = 0.f;
                    return rawAxis;
                    };
                pad.sliders[Pad::Sliders::AXIS_X_LEFT] = normalizeAxis(controller.axis_left_x);
                pad.sliders[Pad::Sliders::AXIS_Y_LEFT] = -normalizeAxis(controller.axis_left_y);
                pad.sliders[Pad::Sliders::AXIS_X_RIGHT] = normalizeAxis(controller.axis_right_x);
                pad.sliders[Pad::Sliders::AXIS_Y_RIGHT] = -normalizeAxis(controller.axis_right_y);
                pad.sliders[Pad::Sliders::TRIGGER_LEFT] = controller.axis_left_2 / 255.f;
                pad.sliders[Pad::Sliders::TRIGGER_RIGHT] = controller.axis_right_2 / 255.f;

            }
            else { // TODO: handle mappings other than 8Bitdo
                GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, 0, &bufferSize);
                _HIDP_PREPARSED_DATA* data = (_HIDP_PREPARSED_DATA*)Allocator::alloc_arena(scratchArena, bufferSize, align);
                GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, data, &bufferSize);

                HIDP_CAPS caps;
                HidP_GetCaps(data, &caps);
                HIDP_BUTTON_CAPS* buttonCaps = (HIDP_BUTTON_CAPS*)Allocator::alloc_arena(scratchArena, caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS), align);
                HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, data);
                ULONG usageCount = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
                USAGE* usages = (USAGE*)Allocator::alloc_arena(scratchArena, sizeof(USAGE) * usageCount, align);
                HidP_GetUsages(HidP_Input, buttonCaps->UsagePage, 0, usages, &usageCount, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
                u16 keys = 0;
                pad.sliders[Pad::Sliders::TRIGGER_RIGHT] = 0.f;
                pad.sliders[Pad::Sliders::TRIGGER_LEFT] = 0.f;
                for (ULONG i = 0; i < usageCount; i++) {
                    USAGE buttonidx = usages[i] - buttonCaps->Range.UsageMin;
                    switch (buttonidx) {
                    case 0: keys |= 1 << Pad::Keys::BUTTON_E; break;
                    case 1: keys |= 1 << Pad::Keys::BUTTON_S; break;
                    case 3: keys |= 1 << Pad::Keys::BUTTON_N; break;
                    case 4: keys |= 1 << Pad::Keys::BUTTON_W; break;
                    case 6: keys |= 1 << Pad::Keys::L2; pad.sliders[Pad::Sliders::TRIGGER_LEFT] = 1.f; break;
                    case 7: keys |= 1 << Pad::Keys::R2; pad.sliders[Pad::Sliders::TRIGGER_RIGHT] = 1.f; break;
                    case 8: keys |= 1 << Pad::Keys::L1; break;
                    case 9: keys |= 1 << Pad::Keys::R1; break;
                    case 10: keys |= 1 << Pad::Keys::SELECT; break;
                    case 11: keys |= 1 << Pad::Keys::START; break;
                    case 13: keys |= 1 << Pad::Keys::LEFT_THUMB; break;
                    case 14: keys |= 1 << Pad::Keys::RIGHT_THUMB; break;
                    };
                }

                HIDP_VALUE_CAPS* valueCaps = (HIDP_VALUE_CAPS*)Allocator::alloc_arena(scratchArena, caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS), align);
                HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, data);
                for (ULONG i = 0; i < caps.NumberInputValueCaps; i++) {
                    ULONG value;
                    HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);

                    auto normalizeAxis = [](ULONG v) {
                        f32 rawAxis = (v - 127.f) / 127.f;
                        if (Math::abs(rawAxis) < 0.1f) rawAxis = 0.f;
                        return rawAxis;
                        };
                    f32 offset = 128.f, scale = 128.f, v;
                    switch (valueCaps[i].Range.UsageMin)
                    {
                    case HID_USAGE_GENERIC_X: v = normalizeAxis(value); pad.sliders[Pad::Sliders::AXIS_X_LEFT] = v; break;
                    case HID_USAGE_GENERIC_Y: v = normalizeAxis(value); pad.sliders[Pad::Sliders::AXIS_Y_LEFT] = -v; break;
                    case HID_USAGE_GENERIC_Z: v = normalizeAxis(value); pad.sliders[Pad::Sliders::AXIS_X_RIGHT] = v; break;
                    case HID_USAGE_GENERIC_RZ: v = normalizeAxis(value); pad.sliders[Pad::Sliders::AXIS_Y_RIGHT] = v; break;
                    case HID_USAGE_GENERIC_HATSWITCH: {
                        switch (value) {
                        case 0: keys |= (1 << Pad::Keys::DPAD_UP); break;
                        case 1: keys |= (1 << Pad::Keys::DPAD_UP) | (1 << Pad::Keys::DPAD_RIGHT); break;
                        case 2: keys |= (1 << Pad::Keys::DPAD_RIGHT); break;
                        case 3: keys |= (1 << Pad::Keys::DPAD_RIGHT) | (1 << Pad::Keys::DPAD_DOWN); break;
                        case 4: keys |= (1 << Pad::Keys::DPAD_DOWN); break;
                        case 5: keys |= (1 << Pad::Keys::DPAD_DOWN) | (1 << Pad::Keys::DPAD_LEFT); break;
                        case 6: keys |= (1 << Pad::Keys::DPAD_LEFT); break;
                        case 7: keys |= (1 << Pad::Keys::DPAD_LEFT) | (1 << Pad::Keys::DPAD_UP); break;
                        }
                        break;
                    } break;
                    default:  v = (value - offset) / scale; Platform::debuglog(" GENERIC %.3f", value); break;
                    }
                }

                pad.last_keys = pad.curr_keys;
                pad.curr_keys = keys;
            }
        }
    }
}

};
#endif // __WASTELADNS_INPUT_DX11_H__
