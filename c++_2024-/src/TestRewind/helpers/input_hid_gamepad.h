#ifndef __WASTELADNS_INPUT_HID_GAMEPAD_H__
#define __WASTELADNS_INPUT_HID_GAMEPAD_H__

#if __WIN64

#include <hidsdi.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

namespace input {
namespace gamepad {
struct SliderInfo {
    USAGE usage;
    USAGE usagePage; // todo: do we need to do anything for HID_USAGE_GENERIC_START, or HID_USAGE_PAGE_CONSUMER?
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
}
}
#elif __MACOS
namespace input {
namespace gamepad {
struct KeyInfo {
    IOHIDElementRef native;
    u32 usage;
    u64 min;
    u64 max;
};
struct SliderInfo {
    IOHIDElementRef native;
    u32 usage;
    u64 min;
    u64 max;
};
struct HatInfo {
    IOHIDElementRef native;
    u32 usage;
    u64 min;
    u64 max;
};

struct DeviceInfo {
    KeyInfo keys_info[64]; // a bunch of them, same as Mapping::keys_map // todo: improve, and figure out correspondence with windows usages
    SliderInfo sliders_info[9]; // from kHIDUsage_GD_X to kHIDUsage_GD_Wheel
    HatInfo hat_info;
    u32 keys_count;
    u32 sliders_count;
    bool loaded;
};
typedef IOHIDDeviceRef DeviceHandle;
}
}
#endif 

namespace input {
namespace gamepad {

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
    if (math::abs(slider) < 0.05f) slider = 0.f;
    if (type == Sliders::AXIS_Y_LEFT || type == Sliders::AXIS_Y_RIGHT) slider = -slider;
    return slider;
}

void parseDualshock4(State& pad, const u8 reportID, const u8* data) {
    const u8* rawdata = data;
    if (reportID == 0x11) { // full-feature input reports add 2 extra bytes to the header
        rawdata = &(data[2]);
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
    switch (controller.axis_dpad) {
    case 0: keys |= (KeyMask::DPAD_UP); break;
    case 1: keys |= (KeyMask::DPAD_UP) | (KeyMask::DPAD_RIGHT); break;
    case 2: keys |= (KeyMask::DPAD_RIGHT); break;
    case 3: keys |= (KeyMask::DPAD_RIGHT) | (KeyMask::DPAD_DOWN); break;
    case 4: keys |= (KeyMask::DPAD_DOWN); break;
    case 5: keys |= (KeyMask::DPAD_DOWN) | (KeyMask::DPAD_LEFT); break;
    case 6: keys |= (KeyMask::DPAD_LEFT); break;
    case 7: keys |= (KeyMask::DPAD_LEFT) | (KeyMask::DPAD_UP); break;
    }
    pad.curr_keys = keys;

    auto normalizeAxis = [](u8 v) {
        f32 rawAxis = (2.0f * v / 255.f) - 1.0f;
        if (math::abs(rawAxis) < 0.05f) rawAxis = 0.f;
        return rawAxis;
        };
    pad.sliders[Sliders::AXIS_X_LEFT] = normalizeAxis(controller.axis_left_x);
    pad.sliders[Sliders::AXIS_Y_LEFT] = -normalizeAxis(controller.axis_left_y);
    pad.sliders[Sliders::AXIS_X_RIGHT] = normalizeAxis(controller.axis_right_x);
    pad.sliders[Sliders::AXIS_Y_RIGHT] = -normalizeAxis(controller.axis_right_y);
    pad.sliders[Sliders::TRIGGER_LEFT] = controller.axis_left_2 / 255.f;
    pad.sliders[Sliders::TRIGGER_RIGHT] = controller.axis_right_2 / 255.f;
}

#if __WIN64
void process_hid_pads_win(allocator::Arena scratchArena, State* pads, u32& padCount, const u32 maxPadCount, const HRAWINPUT lParam) {
    UINT bufferSize;
    const ptrdiff_t align = 16;

    GetRawInputData(lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));
    RAWINPUT* input = (RAWINPUT*)allocator::alloc_arena(scratchArena, bufferSize, align);
    bool gotInput = GetRawInputData(lParam, RID_INPUT, input, &bufferSize, sizeof(RAWINPUTHEADER)) > 0;
    if (!gotInput) return;

    u32 padToUpdate = padCount;
    for (u32 padidx = 0; padidx < padCount; padidx++) {
        if ((DeviceHandle)(pads[padidx].deviceHandle) == input->header.hDevice) {
            padToUpdate = padidx;
            break;
        }
    }
    if (padToUpdate >= maxPadCount) return;
    State& pad = pads[padToUpdate];

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

    // handle Dualshock4 separately, since it doesn't always specify HID gamepad usages in the report
    // see https://chromium-review.googlesource.com/c/chromium/src/+/1478406
    if (pad.type == Type::DUALSHOCK4) {
        BYTE reportID = input->data.hid.bRawData[0];
        BYTE* rawdata = &input->data.hid.bRawData[1];
        parseDualshock4(pad, reportID, rawdata);
    }
    else {
        Mapping& mapping = mappings[pad.type];
        if (!mapping.deviceInfo.loaded) {
            GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, 0, &bufferSize);
            // todo: handle our own persistent data instead of malloc?
            mapping.deviceInfo.preparsedData = (_HIDP_PREPARSED_DATA*)malloc(bufferSize);
            GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, mapping.deviceInfo.preparsedData, &bufferSize);

            HIDP_CAPS caps;
            HidP_GetCaps(mapping.deviceInfo.preparsedData, &caps);
            HIDP_BUTTON_CAPS* buttonCaps = (HIDP_BUTTON_CAPS*)allocator::alloc_arena(scratchArena, caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS), align);
            HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, mapping.deviceInfo.preparsedData);
            HIDP_VALUE_CAPS* valueCaps = (HIDP_VALUE_CAPS*)allocator::alloc_arena(scratchArena, caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS), align);
            HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, mapping.deviceInfo.preparsedData);

            u32 mapped_key_count = math::min((u32)(buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1), (u32)countof(mapping.keys_map));
            mapping.deviceInfo.keys_count = mapped_key_count;
            mapping.deviceInfo.keys_usage_min = buttonCaps->Range.UsageMin;
            mapping.deviceInfo.keys_usage_page = buttonCaps->UsagePage;
            const USHORT sliderInfoMaxCount = countof(mapping.deviceInfo.sliders_info);
            for (ULONG i = 0; i < caps.NumberInputValueCaps; i++) {
                USAGE usage = valueCaps[i].Range.UsageMin;
                if (usage < HID_USAGE_GENERIC_X || usage > HID_USAGE_GENERIC_HATSWITCH) continue;
                if (mapping.deviceInfo.sliders_count >= sliderInfoMaxCount) break;
                SliderInfo& sliderInfo = mapping.deviceInfo.sliders_info[mapping.deviceInfo.sliders_count++];
                sliderInfo.usage = valueCaps[i].Range.UsageMin;
                sliderInfo.usagePage = valueCaps[i].UsagePage;
                sliderInfo.min = valueCaps[i].LogicalMin;
                sliderInfo.max = valueCaps[i].LogicalMax;
            }
            mapping.deviceInfo.loaded = true;
        }

        memset(pad.sliders, 0, sizeof(pad.sliders));

        ULONG usageCount = mapping.deviceInfo.keys_count;
        USAGE* usages = (USAGE*)allocator::alloc_arena(scratchArena, sizeof(USAGE) * usageCount, align);
        HidP_GetUsages(HidP_Input, mapping.deviceInfo.keys_usage_page, 0, usages, &usageCount, mapping.deviceInfo.preparsedData, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
        u16 keys = 0;
        for (ULONG i = 0; i < usageCount; i++) {
            USAGE buttonidx = usages[i] - mapping.deviceInfo.keys_usage_min;
            s32 keyMask = mapping.keys_map[buttonidx];
            if (keyMask < 0) continue;
            else if (keyMask == KeyMask::L2) pad.sliders[Sliders::TRIGGER_LEFT] = 1.f;
            else if (keyMask == KeyMask::R2) pad.sliders[Sliders::TRIGGER_RIGHT] = 1.f;
            keys |= keyMask;
        }

        const USHORT mappedCapCount = (USHORT)countof(mapping.sliders_map);
        const USHORT valueCount = mapping.deviceInfo.sliders_count;
        for (USHORT i = 0; i < valueCount; i++) {
            SliderInfo& sliderInfo = mapping.deviceInfo.sliders_info[i];
            USAGE usage = sliderInfo.usage;
            USAGE usagePage = sliderInfo.usagePage;
            if (usage == HID_USAGE_GENERIC_HATSWITCH) {
                ULONG value;
                HidP_GetUsageValue(HidP_Input, usagePage, 0, usage, &value, mapping.deviceInfo.preparsedData, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
                if (value >= countof(mapping.dpad_map)) continue;
                keys |= mapping.dpad_map[value];
            }
            else {
                u32 mapIdx = usage - HID_USAGE_GENERIC_X;
                if (mapIdx >= mappedCapCount) continue;
                s32 sliderIdx = mapping.sliders_map[mapIdx];
                if (sliderIdx < 0) continue;
                ULONG value;
                HidP_GetUsageValue(HidP_Input, usagePage, 0, usage, &value, mapping.deviceInfo.preparsedData, (PCHAR)input->data.hid.bRawData, input->data.hid.dwSizeHid);
                pad.sliders[sliderIdx] = TranslateRawSliderValue(sliderInfo, value, (Sliders::Enum)sliderIdx);
            }
        }
        pad.curr_keys = keys;
    }

    // If the pad is new, and we got button input from it, finish creation
    if (padToUpdate == padCount) {
        bool validpad = hasValidInput(pad);
        if (validpad) {
            padCount++; // acknowledge the current pad
            pad.deviceHandle = (u64)input->header.hDevice;
            __DEBUGEXP(platform::strncpy(pad.name, names[pad.type], sizeof(pad.name)));

            #if __DEBUG
            char name[256];
            u32 deviceNameSize = sizeof(name);
            GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICENAME, name, &deviceNameSize);

            platform::debuglog("Registered new pad %s\n", name);
            #endif
        }
    }
}
void init_hid_pads_win(HWND hWnd) {
    RAWINPUTDEVICE rid;
    rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid.usUsage = HID_USAGE_GENERIC_GAMEPAD;
    rid.dwFlags = 0;
    rid.hwndTarget = hWnd;
    RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));
}
#elif __MACOS

void process_hid_pads_mac(void* context, IOReturn result, void* sender, IOHIDReportType type, uint32_t reportID, uint8_t* report, CFIndex reportLength) {

    ::platform::State& ctx = *(::platform::State*)context;
    ::platform::Input& input = ctx.input;

    IOHIDDeviceRef device = (IOHIDDeviceRef)sender;

    u32 padToUpdate = input.padCount;
    for (u32 padidx = 0; padidx < input.padCount; padidx++) {
        if ((DeviceHandle)(input.pads[padidx].deviceHandle) == device) {
            padToUpdate = padidx;
            break;
        }
    }

    if (padToUpdate >= countof(input.pads)) return;
    State& pad = input.pads[padToUpdate];

    // this will be a new pad, check the type
    if (padToUpdate == input.padCount) {
        pad = {};

        u64 vendorID = [(__bridge NSNumber*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
        u64 productID = [(__bridge NSNumber*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];

        pad.type = ComputeControllerType(vendorID, productID);
    }

    // handle Dualshock4 separately, since it doesn't always specify HID gamepad usages in the report
    // see https://chromium-review.googlesource.com/c/chromium/src/+/1478406
    if (pad.type == Type::DUALSHOCK4) {
        Dualshock4::parseDualshock4(pad, report[0], &(report[1]));
    }
    else {
        Mapping& mapping = mappings[pad.type];

        if (!mapping.deviceInfo.loaded) {
            CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
            for (CFIndex i = 0; i < CFArrayGetCount(elements); i++) {
                IOHIDElementRef native = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
                if (CFGetTypeID(native) != IOHIDElementGetTypeID())
                    continue;

                const IOHIDElementType type = IOHIDElementGetType(native);
                if ((type != kIOHIDElementTypeInput_Axis) && (type != kIOHIDElementTypeInput_Button) && (type != kIOHIDElementTypeInput_Misc)) {
                    continue;
                }

                const uint32_t usage = IOHIDElementGetUsage(native);
                const uint32_t page = IOHIDElementGetUsagePage(native);
                if (page == kHIDPage_GenericDesktop) {
                    switch (usage) {
                        case kHIDUsage_GD_X:
                        case kHIDUsage_GD_Y:
                        case kHIDUsage_GD_Z:
                        case kHIDUsage_GD_Rx:
                        case kHIDUsage_GD_Ry:
                        case kHIDUsage_GD_Rz:
                        case kHIDUsage_GD_Slider:
                        case kHIDUsage_GD_Dial:
                        case kHIDUsage_GD_Wheel: {
                            if (mapping.deviceInfo.sliders_count >= countof(mapping.deviceInfo.sliders_info)) break;
                            SliderInfo& sliderInfo = mapping.deviceInfo.sliders_info[mapping.deviceInfo.sliders_count++];
                            sliderInfo.native = native;
                            sliderInfo.usage = usage;

                            sliderInfo.min = IOHIDElementGetLogicalMin(native);
                            sliderInfo.max = IOHIDElementGetLogicalMax(native);
                        }
                        break;
                        case kHIDUsage_GD_Hatswitch: {
                            HatInfo& hatInfo = mapping.deviceInfo.hat_info;
                            hatInfo.native = native;
                            hatInfo.usage = usage;
                            hatInfo.min = IOHIDElementGetLogicalMin(native);
                            hatInfo.max = IOHIDElementGetLogicalMax(native);
                        }
                        break;
                        // todo: figure out mappings with pads that use kHIDUsage_GD_DPadUp to kHIDUsage_GD_Start
                        /*case kHIDUsage_GD_DPadUp:
                        case kHIDUsage_GD_DPadRight:
                        case kHIDUsage_GD_DPadDown:
                        case kHIDUsage_GD_DPadLeft:
                        case kHIDUsage_GD_SystemMainMenu:
                        case kHIDUsage_GD_Select:
                        case kHIDUsage_GD_Start: {
                            if (mapping.deviceInfo.keys_count >= countof(mapping.deviceInfo.keys_info)) break;
                            KeyInfo& keyInfo = mapping.deviceInfo.keys_info[mapping.deviceInfo.keys_count++];
                            keyInfo.native = native;
                            keyInfo.usage = usage;
                            keyInfo.min = IOHIDElementGetLogicalMin(native);
                            keyInfo.max = IOHIDElementGetLogicalMax(native);
                        }
                        break;
                         */
                    }
                }
                else if (page == kHIDPage_Button /* || page == kHIDPage_Consumer */) { // todo: see https://github.com/libsdl-org/SDL/issues/1973
                    if (mapping.deviceInfo.keys_count >= countof(mapping.deviceInfo.keys_info)) continue;
                    KeyInfo& keyInfo = mapping.deviceInfo.keys_info[mapping.deviceInfo.keys_count++];
                    keyInfo.native = native;
                    keyInfo.usage = usage;
                    keyInfo.min = IOHIDElementGetLogicalMin(native);
                    keyInfo.max = IOHIDElementGetLogicalMax(native);
                }
            }
            CFRelease(elements);
            mapping.deviceInfo.loaded = true;
        }

        memset(pad.sliders, 0, sizeof(pad.sliders));
        // Buttons
        u16 keys = 0;
        const u32 mappedKeysCount = (u32)countof(mapping.keys_map);
        for (u32 i = 0; i < mapping.deviceInfo.keys_count; i++) {
            KeyInfo& keysInfo = mapping.deviceInfo.keys_info[i];
            IOHIDValueRef valueRef;
            if (IOHIDDeviceGetValue(device, keysInfo.native, &valueRef) == kIOReturnSuccess) {
                if (IOHIDValueGetIntegerValue(valueRef) == 0) continue;
                u8 buttonIdx = keysInfo.usage - kHIDUsage_Button_1;
                if (buttonIdx >= mappedKeysCount) continue;
                s32 keyMask = mapping.keys_map[buttonIdx];
                if (keyMask == KeyMask::L2) pad.sliders[Sliders::TRIGGER_LEFT] = 1.f;
                else if (keyMask == KeyMask::R2) pad.sliders[Sliders::TRIGGER_RIGHT] = 1.f;
                keys |= keyMask;
            }
        }
        // Dpad
        IOHIDValueRef valueRef;
        if (IOHIDDeviceGetValue(device, mapping.deviceInfo.hat_info.native, &valueRef) == kIOReturnSuccess) {
            u64 value = IOHIDValueGetIntegerValue(valueRef) - mapping.deviceInfo.hat_info.min;
            if (value < countof(mapping.dpad_map)) {
                keys |= mapping.dpad_map[value];
            }
        }
        // Axes
        for (u32 i = 0; i < mapping.deviceInfo.sliders_count; i++) {
            SliderInfo& sliderInfo = mapping.deviceInfo.sliders_info[i];
            u32 mapIdx = sliderInfo.usage - kHIDUsage_GD_X;
            s32 sliderIdx = mapping.sliders_map[mapIdx];
            if (sliderIdx < 0) continue;
            IOHIDValueRef valueRef;
            if (IOHIDDeviceGetValue(device, sliderInfo.native, &valueRef) == kIOReturnSuccess) {
                u64 value = IOHIDValueGetIntegerValue(valueRef);
                pad.sliders[sliderIdx] = TranslateRawSliderValue(sliderInfo, value, (Sliders::Enum)sliderIdx);
            }
        }
        pad.curr_keys = keys;
    }

    // If the pad is new, and we got button input from it, finish creation
    if (padToUpdate == input.padCount) {
        bool validpad = hasValidInput(pad);
        if (validpad) {
            input.padCount++; // acknowledge the current pad
            pad.deviceHandle = (u64)device;
            __DEBUGEXP(platform::strncpy(pad.name, names[pad.type], sizeof(pad.name)));

            #if __DEBUG
            u64 vendorID = [(__bridge NSNumber*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
            u64 productID = [(__bridge NSNumber*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];
            platform::debuglog("Registered new pad 0x%lx 0x%lx\n", vendorID, productID);
            #endif
        }
    }
}
void init_hid_pads_mac(platform::State& platform) {
    @autoreleasepool{
        // todo: custom allocators?
        IOHIDManagerRef HIDManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        IOHIDManagerRegisterInputReportCallback(HIDManager, &process_hid_pads_mac, &platform);
        IOHIDManagerOpen(HIDManager, kIOHIDOptionsTypeNone);
        IOHIDManagerSetDeviceMatchingMultiple(HIDManager, (__bridge CFArrayRef)@[
            @{@(kIOHIDDeviceUsagePageKey) : @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey) : @(kHIDUsage_GD_GamePad)},
            @{@(kIOHIDDeviceUsagePageKey) : @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey) : @(kHIDUsage_GD_Joystick)},
            @{@(kIOHIDDeviceUsagePageKey) : @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey) : @(kHIDUsage_GD_MultiAxisController)},
        ]);
        IOHIDManagerScheduleWithRunLoop(HIDManager,
                                        CFRunLoopGetMain(),
                                        kCFRunLoopDefaultMode);
    }
}
#endif

}
}

#endif // __WASTELADNS_INPUT_HID_GAMEPAD_H__
