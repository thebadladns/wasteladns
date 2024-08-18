#ifndef __WASTELADNS_INPUT_DX11_H__
#define __WASTELADNS_INPUT_DX11_H__

#ifndef UNITYBUILD
#include <cstring>
#include "../input.h"
#endif

#include "../input_hid_gamepad.h"

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

        u32 padToUpdate = padCount;
        for (u32 padidx = 0; padidx < padCount; padidx++) {
            if (pads[padidx].deviceHandle == input->header.hDevice) {
                padToUpdate = padidx;
                break;
            }
        }
        if (padToUpdate >= maxPadCount) return;
        Pad::State& pad = pads[padToUpdate];

        // this will be a new pad, check the type
        if (padToUpdate == padCount) {
            pad = {};
            RID_DEVICE_INFO deviceInfo;
            UINT deviceInfoSize = sizeof(deviceInfo);
            bool gotInfo = GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize) > 0;
            if (!gotInfo) return;
            DWORD vendorID = deviceInfo.hid.dwVendorId;
            DWORD productID = deviceInfo.hid.dwProductId;
            pad.type = ComputeControllerType(vendorID, productID);
        }
        
        // handle Dualshock4 separately, since it doesn't specify HID gamepad usages in the report
        if (pad.type == ::Input::Gamepad::Type::DUALSHOCK4) {
            BYTE reportID = input->data.hid.bRawData[0];
            BYTE* rawdata = &input->data.hid.bRawData[1];
            Dualshock4::parseDualshock4(pad, reportID, rawdata);
        }
        else {
            Pad::Mapping& mapping = Pad::mappings[pad.type];
            if (!mapping.deviceInfo.loaded) {
                GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, 0, &bufferSize);
                // todo: handle our own persistent data instead of malloc?
                mapping.deviceInfo.preparsedData = (_HIDP_PREPARSED_DATA*)malloc(bufferSize);
                GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, mapping.deviceInfo.preparsedData, &bufferSize);

                HIDP_CAPS caps;
                HidP_GetCaps(mapping.deviceInfo.preparsedData, &caps);
                HIDP_BUTTON_CAPS* buttonCaps = (HIDP_BUTTON_CAPS*)Allocator::alloc_arena(scratchArena, caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS), align);
                HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, mapping.deviceInfo.preparsedData);
                HIDP_VALUE_CAPS* valueCaps = (HIDP_VALUE_CAPS*)Allocator::alloc_arena(scratchArena, caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS), align);
                HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, mapping.deviceInfo.preparsedData);

                u32 mapped_key_count = Math::min((u32)(buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1), (u32)COUNT_OF(mapping.keys_map));
                mapping.deviceInfo.keys_count = mapped_key_count;
                mapping.deviceInfo.keys_usage_min = buttonCaps->Range.UsageMin;
                mapping.deviceInfo.keys_usage_page = buttonCaps->UsagePage;
                const USHORT sliderInfoMaxCount = COUNT_OF(mapping.deviceInfo.sliders_info);
                for (ULONG i = 0; i < caps.NumberInputValueCaps; i++) {
                    USAGE usage = valueCaps[i].Range.UsageMin;
                    if (usage < HID_USAGE_GENERIC_X || usage > HID_USAGE_GENERIC_HATSWITCH) continue;
                    if (mapping.deviceInfo.sliders_count >= sliderInfoMaxCount) break;
                    Pad::SliderInfo& sliderInfo = mapping.deviceInfo.sliders_info[mapping.deviceInfo.sliders_count++];
                    sliderInfo.usage = valueCaps[i].Range.UsageMin;
                    sliderInfo.usagePage = valueCaps[i].UsagePage;
                    sliderInfo.min = valueCaps[i].LogicalMin;
                    sliderInfo.max = valueCaps[i].LogicalMax;
                }
                mapping.deviceInfo.loaded = true;
            }

            memset(pad.sliders, 0, sizeof(pad.sliders));

            ULONG usageCount = mapping.deviceInfo.keys_count;
            USAGE* usages = (USAGE*)Allocator::alloc_arena(scratchArena, sizeof(USAGE) * usageCount, align);
            HidP_GetUsages(HidP_Input, mapping.deviceInfo.keys_usage_page, 0, usages, &usageCount, mapping.deviceInfo.preparsedData, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
            u16 keys = 0;
            for (ULONG i = 0; i < usageCount; i++) {
                USAGE buttonidx = usages[i] - mapping.deviceInfo.keys_usage_min;
                s32 keyMask = mapping.keys_map[buttonidx];
                if (keyMask < 0) continue;
                else if (keyMask == Pad::KeyMask::L2) pad.sliders[Pad::Sliders::TRIGGER_LEFT] = 1.f;
                else if (keyMask == Pad::KeyMask::R2) pad.sliders[Pad::Sliders::TRIGGER_RIGHT] = 1.f;
                keys |= keyMask;
            }

            const USHORT mappedCapCount = (USHORT)COUNT_OF(mapping.sliders_map);
            const USHORT valueCount = mapping.deviceInfo.sliders_count;
            for (USHORT i = 0; i < valueCount; i++) {
                Pad::SliderInfo& sliderInfo = mapping.deviceInfo.sliders_info[i];
                USAGE usage = sliderInfo.usage;
                USAGE usagePage = sliderInfo.usagePage;
                if (usage == HID_USAGE_GENERIC_HATSWITCH) {
                    ULONG value;
                    HidP_GetUsageValue(HidP_Input, usagePage, 0, usage, &value, mapping.deviceInfo.preparsedData, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
                    if (value >= COUNT_OF(mapping.dpad_map)) continue;
                    keys |= mapping.dpad_map[value];
                } else {
                    u32 mapIdx = usage - HID_USAGE_GENERIC_X;
                    if (mapIdx >= mappedCapCount) continue;
                    s32 sliderIdx = mapping.sliders_map[mapIdx];
                    if (sliderIdx < 0) continue;
                    ULONG value;
                    HidP_GetUsageValue(HidP_Input, usagePage, 0, usage, &value, mapping.deviceInfo.preparsedData, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
                    pad.sliders[sliderIdx] = TranslateRawSliderValue(sliderInfo, value, (Sliders::Enum)sliderIdx);
                }
            }
            pad.last_keys = pad.curr_keys;
            pad.curr_keys = keys;
        }
        
        // If the pad is new, and we got button input from it, finish creation
        if (padToUpdate == padCount) {
            bool validpad = hasValidInput(pad);
            if (validpad) {
                padCount++; // acknowledge the current pad
                pad.deviceHandle = input->header.hDevice;
                __DEBUGEXP(strncpy_s(pad.name, Pad::names[pad.type], sizeof(pad.name)));

                #if __DEBUG
                char name[256];
                u32 deviceNameSize = sizeof(name);
                GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICENAME, name, &deviceNameSize);
                
                Platform::debuglog("Registered new pad %s\n", name);
                #endif
            }
        }
    }
}

};
#endif // __WASTELADNS_INPUT_DX11_H__
