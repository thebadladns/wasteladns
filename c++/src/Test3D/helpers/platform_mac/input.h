#ifndef __WASTELADNS_INPUT_MACOS_H__
#define __WASTELADNS_INPUT_MACOS_H__

#ifndef UNITYBUILD
#include <cstring>
#include "../input.h"
#include "core.h"
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

    void processPads(void *context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength) {
        namespace Pad = ::Input::Gamepad;

        ::Platform::State& ctx = *(::Platform::State*)context;
        ::Platform::Input& input = ctx.input;
        // explicit copy, scratch header will be cleared on function exit
//        Allocator::Arena scratchArena = ctx.memory.scratchArenaRoot;
//        ptrdiff_t align = 16;
        
        IOHIDDeviceRef device = (IOHIDDeviceRef)sender;
        
        u32 padToUpdate = input.padCount;
        for (u32 padidx = 0; padidx < input.padCount; padidx++) {
            if (input.pads[padidx].deviceHandle == device) {
                padToUpdate = padidx;
                break;
            }
        }
        
        if (padToUpdate >= COUNT_OF(input.pads)) return;
        Pad::State& pad = input.pads[padToUpdate];
        
        // this will be a new pad, check the type
        if (padToUpdate == input.padCount) {
            pad = {};

            u64 vendorID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
            u64 productID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];
            
            pad.type = ComputeControllerType(vendorID, productID);
        }
        
        // handle Dualshock4 separately, since it doesn't specify HID gamepad usages in the report
        if (pad.type == ::Input::Gamepad::Type::DUALSHOCK4) {
            Dualshock4::parseDualshock4(pad, report[0], &(report[1]));
        }
        else {
            Pad::Mapping& mapping = Pad::mappings[pad.type];

            if (!mapping.deviceInfo.loaded) {
                CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
                for (CFIndex i = 0;  i < CFArrayGetCount(elements);  i++) {
                    IOHIDElementRef native = (IOHIDElementRef) CFArrayGetValueAtIndex(elements, i);
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
                                if (mapping.deviceInfo.sliders_count >= COUNT_OF(mapping.deviceInfo.sliders_info)) break;
                                Pad::SliderInfo& sliderInfo = mapping.deviceInfo.sliders_info[mapping.deviceInfo.sliders_count++];
                                sliderInfo.native = native;
                                sliderInfo.usage = usage;
                                
                                sliderInfo.min = IOHIDElementGetLogicalMin(native);
                                sliderInfo.max = IOHIDElementGetLogicalMax(native);
                            }
                            break;
                            case kHIDUsage_GD_Hatswitch: {
                                Pad::HatInfo& hatInfo = mapping.deviceInfo.hat_info;
                                hatInfo.native = native;
                                hatInfo.usage = usage;
                                hatInfo.min = IOHIDElementGetLogicalMin(native);
                                hatInfo.max = IOHIDElementGetLogicalMax(native);
                            }
                            break;
                            case kHIDUsage_GD_DPadUp:
                            case kHIDUsage_GD_DPadRight:
                            case kHIDUsage_GD_DPadDown:
                            case kHIDUsage_GD_DPadLeft:
                            case kHIDUsage_GD_SystemMainMenu:
                            case kHIDUsage_GD_Select:
                            case kHIDUsage_GD_Start: {
                                if (mapping.deviceInfo.keys_count >= COUNT_OF(mapping.deviceInfo.keys_info)) break;
                                Pad::KeyInfo& keyInfo = mapping.deviceInfo.keys_info[mapping.deviceInfo.keys_count++];
                                keyInfo.native = native;
                                keyInfo.usage = usage;
                                keyInfo.min = IOHIDElementGetLogicalMin(native);
                                keyInfo.max = IOHIDElementGetLogicalMax(native);
                            }
                            break;
                        }
                    } else if (page == kHIDPage_Button || page == kHIDPage_Consumer) { // see https://github.com/libsdl-org/SDL/issues/1973
                        if (mapping.deviceInfo.keys_count >= COUNT_OF(mapping.deviceInfo.keys_info)) break;
                        Pad::KeyInfo& keyInfo = mapping.deviceInfo.keys_info[mapping.deviceInfo.keys_count++];
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
            const u32 mappedKeysCount = (u32)COUNT_OF(mapping.keys_map);
            for (u32 i = 0; i < mapping.deviceInfo.keys_count; i++) {
                Pad::KeyInfo& keysInfo = mapping.deviceInfo.keys_info[i];
                IOHIDValueRef valueRef;
                if (IOHIDDeviceGetValue(device, keysInfo.native, &valueRef) == kIOReturnSuccess) {
                    u8 buttonIdx = i;//IOHIDValueGetIntegerValue(valueRef) - keysInfo.min;
                    if (buttonIdx >= mappedKeysCount) continue;
                    if (IOHIDValueGetIntegerValue(valueRef) == 0) continue;
                    s32 keyMask = mapping.keys_map[buttonIdx];
                    if (keyMask == Pad::KeyMask::L2) pad.sliders[Pad::Sliders::TRIGGER_LEFT] = 1.f;
                    else if (keyMask == Pad::KeyMask::R2) pad.sliders[Pad::Sliders::TRIGGER_RIGHT] = 1.f;
                    keys |= keyMask;
                }
            }
            // Dpad
            IOHIDValueRef valueRef;
            if (IOHIDDeviceGetValue(device, mapping.deviceInfo.hat_info.native, &valueRef) == kIOReturnSuccess) {
                u64 value = IOHIDValueGetIntegerValue(valueRef) - mapping.deviceInfo.hat_info.min;
                if (value < COUNT_OF(mapping.dpad_map)) {
                    keys |= mapping.dpad_map[value];
                }
            }
            // Axes
            for (u32 i = 0; i < mapping.deviceInfo.sliders_count; i++) {
                Pad::SliderInfo& sliderInfo = mapping.deviceInfo.sliders_info[i];
                u32 mapIdx = sliderInfo.usage - kHIDUsage_GD_X;
                s32 sliderIdx = mapping.sliders_map[mapIdx];
                if (sliderIdx < 0) continue;
                IOHIDValueRef valueRef;
                if (IOHIDDeviceGetValue(device, sliderInfo.native, &valueRef) == kIOReturnSuccess) {
                    u64 value = IOHIDValueGetIntegerValue(valueRef);
                    pad.sliders[sliderIdx] = TranslateRawSliderValue(sliderInfo, value, (Sliders::Enum)sliderIdx);
                }
            }
            pad.last_keys = pad.curr_keys;
            pad.curr_keys = keys;
        }
        
        // If the pad is new, and we got button input from it, finish creation
        if (padToUpdate == input.padCount) {
            bool validpad = hasValidInput(pad);
            if (validpad) {
                input.padCount++; // acknowledge the current pad
                pad.deviceHandle = device;
                __DEBUGEXP(strncpy(pad.name, Pad::names[pad.type], sizeof(pad.name)));

    #if __DEBUG
                u64 vendorID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
                u64 productID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];
                Platform::debuglog("Registered new pad 0x%lx 0x%lx\n", vendorID, productID);
    #endif
            }
        }
    }
};

};

#endif // __WASTELADNS_INPUT_MACOS_H__
