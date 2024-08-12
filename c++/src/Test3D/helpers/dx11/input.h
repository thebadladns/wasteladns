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
        if (!gotInput) return;

        s32 padToUpdate = padCount;
        for (s32 padidx = 0; padidx < (s32)padCount; padidx++) {
            if (pads[padidx].deviceHandle == input->header.hDevice) {
                padToUpdate = padidx;
                break;
            }
        }
        if (padToUpdate >= (s32)maxPadCount) return;
        Pad::State& pad = pads[padToUpdate];

        // this will be a new pad, check the type
        if (padToUpdate == padCount) {
             RID_DEVICE_INFO deviceInfo;
             UINT deviceInfoSize = sizeof(deviceInfo);
             bool gotInfo = GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize) > 0;
             if (!gotInfo) return;
             const DWORD sonyVendorID = 0x054C;
             const DWORD ds4Gen1ProductID = 0x05C4;
             const DWORD ds4Gen2ProductID = 0x09CC;
             if (deviceInfo.hid.dwVendorId == sonyVendorID && (deviceInfo.hid.dwProductId == ds4Gen1ProductID || (deviceInfo.hid.dwProductId == ds4Gen2ProductID))) {
                 pad.type = Pad::Type::DUALSHOCK4;
             } else {
                 pad.type = Pad::Type::NES_8BITDO;
             }
        }
        
        // handle Dualshock4 separately, since it doesn't specify HID gamepad usages in the report
        if (pad.type == ::Input::Gamepad::Type::DUALSHOCK4) {
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
            if (controller.button_square) keys |= Pad::KeyMask::BUTTON_W;
            if (controller.button_cross) keys |= Pad::KeyMask::BUTTON_S;
            if (controller.button_circle) keys |= Pad::KeyMask::BUTTON_E;
            if (controller.button_triangle) keys |= Pad::KeyMask::BUTTON_N;
            if (controller.button_options) keys |= Pad::KeyMask::START;
            if (controller.button_share) keys |= Pad::KeyMask::SELECT;
            if (controller.button_left_3) keys |= Pad::KeyMask::LEFT_THUMB;
            if (controller.button_right_3) keys |= Pad::KeyMask::RIGHT_THUMB;
            if (controller.button_left_1) keys |= Pad::KeyMask::L1;
            if (controller.button_right_1) keys |= Pad::KeyMask::R1;
            if (controller.button_left_2) keys |= Pad::KeyMask::L2;
            if (controller.button_right_2) keys |= Pad::KeyMask::R2;
            switch (controller.axis_dpad) { // todo: simplify?
            case 0: keys |= (Pad::KeyMask::DPAD_UP); break;
            case 1: keys |= (Pad::KeyMask::DPAD_UP) | (Pad::KeyMask::DPAD_RIGHT); break;
            case 2: keys |= (Pad::KeyMask::DPAD_RIGHT); break;
            case 3: keys |= (Pad::KeyMask::DPAD_RIGHT) | (Pad::KeyMask::DPAD_DOWN); break;
            case 4: keys |= (Pad::KeyMask::DPAD_DOWN); break;
            case 5: keys |= (Pad::KeyMask::DPAD_DOWN) | (Pad::KeyMask::DPAD_LEFT); break;
            case 6: keys |= (Pad::KeyMask::DPAD_LEFT); break;
            case 7: keys |= (Pad::KeyMask::DPAD_LEFT) | (Pad::KeyMask::DPAD_UP); break;
            }
            pad.last_keys = pad.curr_keys;
            pad.curr_keys = keys;

            auto normalizeAxis = [](u8 v) {
                f32 rawAxis = (2.0f * v / 255.f) - 1.0f;
                if (Math::abs(rawAxis) < 0.05f) rawAxis = 0.f;
                return rawAxis;
                };
            pad.sliders[Pad::Sliders::AXIS_X_LEFT] = normalizeAxis(controller.axis_left_x);
            pad.sliders[Pad::Sliders::AXIS_Y_LEFT] = -normalizeAxis(controller.axis_left_y);
            pad.sliders[Pad::Sliders::AXIS_X_RIGHT] = normalizeAxis(controller.axis_right_x);
            pad.sliders[Pad::Sliders::AXIS_Y_RIGHT] = -normalizeAxis(controller.axis_right_y);
            pad.sliders[Pad::Sliders::TRIGGER_LEFT] = controller.axis_left_2 / 255.f;
            pad.sliders[Pad::Sliders::TRIGGER_RIGHT] = controller.axis_right_2 / 255.f;
        }
        else {
            const Pad::Mapping& mapping = Pad::mappings[pad.type];

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
            const ULONG mappedUsageCount = Math::min(usageCount, (ULONG)COUNT_OF(mapping.keys_map));
            for (ULONG i = 0; i < mappedUsageCount; i++) {
                USAGE buttonidx = usages[i] - buttonCaps->Range.UsageMin;
                s32 keyMask = mapping.keys_map[buttonidx];
                if (keyMask < 0) continue;
                else if (keyMask == Pad::KeyMask::L2) pad.sliders[Pad::Sliders::TRIGGER_LEFT] = 1.f;
                else if (keyMask == Pad::KeyMask::R2) pad.sliders[Pad::Sliders::TRIGGER_RIGHT] = 1.f;
                keys |= keyMask;
            }

            HIDP_VALUE_CAPS* valueCaps = (HIDP_VALUE_CAPS*)Allocator::alloc_arena(scratchArena, caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS), align);
            HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, data);
            const USHORT mappedCapCount = (USHORT)COUNT_OF(mapping.sliders_map);
            for (USHORT i = 0; i < caps.NumberInputValueCaps; i++) {
                if (valueCaps[i].Range.UsageMin == HID_USAGE_GENERIC_HATSWITCH) {
                    ULONG value;
                    HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
                    if (value >= COUNT_OF(mapping.dpad_map)) continue;
                    keys |= mapping.dpad_map[value];
                } else {
                    u32 mapIdx = valueCaps[i].Range.UsageMin - HID_USAGE_GENERIC_X;
                    if (mapIdx >= mappedCapCount) continue;
                    s32 sliderIdx = mapping.sliders_map[mapIdx];
                    if (sliderIdx < 0) continue;
                    ULONG value;
                    HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, data, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
                    f32 slider = (value - 128.f) / 128.f;
                    if (Math::abs(slider) < 0.02f) slider = 0.f;
                    if (sliderIdx == Pad::Sliders::AXIS_Y_LEFT || sliderIdx == Pad::Sliders::AXIS_Y_RIGHT) slider = -slider;
                    pad.sliders[sliderIdx] = slider;
                }
            }
            pad.last_keys = pad.curr_keys;
            pad.curr_keys = keys;
        }

        
        // If the pad is new, and we got button input from it, finish creation
        if (pad.curr_keys && padToUpdate == padCount) {
            padCount++; // acknowledge the current pad
            pad.deviceHandle = input->header.hDevice;
            __DEBUGEXP(strncpy_s(pad.name, Pad::names[pad.type], sizeof(pad.name)));

            //#if __DEBUG
            //                u32 deviceNameSize = sizeof(padToCreate.name);
            //                GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICENAME, padToCreate.name, &deviceNameSize);
            //
            //                Platform::debuglog("Registered new pad %s\n", padToCreate.name);
            //#endif
        }
    }
}

};
#endif // __WASTELADNS_INPUT_DX11_H__
