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
                    } else if (page == kHIDPage_Button || page == kHIDPage_Consumer) {
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

//    void load(KeyboardMapping& mapping) {
//        memset(mapping.mapping, GLFW_KEY_UNKNOWN, sizeof(s32) * Keys::COUNT);
//        // Hardcoded for now
//        mapping.mapping[Keys::B_U] = GLFW_KEY_I;
//        mapping.mapping[Keys::B_D] = GLFW_KEY_K;
//        mapping.mapping[Keys::B_L] = GLFW_KEY_J;
//        mapping.mapping[Keys::B_R] = GLFW_KEY_L;
//        mapping.mapping[Keys::D_U] = GLFW_KEY_W;
//        mapping.mapping[Keys::D_D] = GLFW_KEY_S;
//        mapping.mapping[Keys::D_L] = GLFW_KEY_A;
//        mapping.mapping[Keys::D_R] = GLFW_KEY_D;
//    }
//
//    void pollState(State& pad, GLFWwindow* window, const KeyboardMapping& mapping) {
//        pad.active = true;
//        pad.keys.last = pad.keys.current;
//        pad.keys.current = 0;
//        for (int i = 0; i < Keys::COUNT; i++) {
//            s32 glfw_id = mapping.mapping[i];
//            if (glfw_id != GLFW_KEY_UNKNOWN) {
//                s32 keyState = glfwGetKey(window, glfw_id);
//                s32 keyOn = keyState != GLFW_RELEASE;
//                pad.keys.current = pad.keys.current | (keyOn << i);
//            }
//        }
//
//        // TODO: handle analog only if requested?
//    }
    
//    void pollState(Joypad& joypad, s32 joyId, GLFWwindow* window) {
//        joypad.active = glfwJoystickPresent(joyId) != 0;
//        if (joypad.active) {
//
//            if (!pad.mapping.set) {
//                loadPreset(pad.mapping, glfwGetJoystickName(joyId));
//                pad.mapping.set = true;
//            }
//
//            // Analogs (axis and triggers)
//            for (s32 i = 0; i < Gamepad::Analog::COUNT; i++) {
//                pad.analogs.values[i] = Gamepad::Analog::novalue;
//            }
//            s32 axesCount;
//            const f32* axes = glfwGetJoystickAxes(joyId, &axesCount);
//            for (s32 i = 0; i < axesCount; i++) {
//                if (i < pad.mapping.a_mappingCount) {
//                    const Gamepad::Analog::Enum axisIndex = pad.mapping.a_mapping[i];
//                    if (axisIndex != Gamepad::Analog::INVALID) {
//                        f32 v = axes[i];
//                        const f32 deadzone = 0.25f;
//                        if (Math::abs(v) < deadzone) {
//                            v = 0.f;
//                        }
//                        pad.analogs.values[axisIndex] = v;
//                    }
//                }
//            }
//
//            bool digitalTrigger_L = false;
//            bool digitalTrigger_R = false;
//
//            pad.buttons.last = pad.buttons.current;
//            pad.buttons.current = (Gamepad::Digital::Enum) 0;
//            s32 buttonCount;
//            const u8* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
//            for (s32 i = 0; i < buttonCount; i++) {
//                if (i < pad.mapping.b_mappingCount) {
//                    const Gamepad::Digital::Enum buttonIndex = pad.mapping.b_mapping[i];
//                    if (buttonIndex != Gamepad::Digital::INVALID) {
//                        pad.buttons.current = pad.buttons.current | ((buttons[i] != 0) << buttonIndex);
//                    }
//                    if (buttonIndex == Gamepad::Digital::L2) {
//                        digitalTrigger_L = true;
//                    } else if (buttonIndex == Gamepad::Digital::R2) {
//                        digitalTrigger_R = true;
//                    }
//                }
//            }
//
//            // Manually set digital trigger values, if needed
//            if (!digitalTrigger_L) {
//                const f32 trigger_l = pad.analogs.values[Gamepad::Analog::Trigger_L];
//                if (trigger_l != Gamepad::Analog::novalue) {
//                    bool triggerPressed = trigger_l > Gamepad::Analog::digitalThreshold;
//                    pad.buttons.current = (Gamepad::Digital::Enum) (pad.buttons.current | (triggerPressed << Gamepad::Digital::L2));
//                    digitalTrigger_L = true;
//                }
//            }
//            if (!digitalTrigger_R) {
//                const f32 trigger_r = pad.analogs.values[Gamepad::Analog::Trigger_R];
//                if (trigger_r != Gamepad::Analog::novalue) {
//                    bool triggerPressed = trigger_r > Gamepad::Analog::digitalThreshold;
//                    pad.buttons.current = (Gamepad::Digital::Enum) (pad.buttons.current | (triggerPressed << Gamepad::Digital::R2));
//                    digitalTrigger_R = true;
//                }
//            }
//        }
//    }
};
    
namespace Keyboard
{
    struct PollData {
        Queue queue;
    };

    void pollState(State& keyboard, const Queue& queue) {
        memcpy(keyboard.last, keyboard.current, sizeof(u8) * Keys::COUNT);
        memset(keyboard.current, 0, sizeof(u8) * Keys::COUNT);
        for (int i = 0; i < Keys::COUNT; i++) {
            keyboard.current[i] = queue.keyStates[i];
        }
    }
};

namespace Mouse
{
    struct PollData {
        f32 x, y;
        f32 scrollx, scrolly;
        u8 keyStates[Keys::COUNT];
    };
    void resetState(PollData& queue) {
        queue.scrollx = queue.scrolly = 0.0;
    }
    void pollState(State& mouse, const PollData& queue) {

        memcpy(mouse.last, mouse.current, sizeof(u8) * Keys::COUNT);
        memset(mouse.current, 0, sizeof(u8) * Keys::COUNT);
        for (int i = 0; i < Keys::COUNT; i++) {
            mouse.current[i] = queue.keyStates[i];
        }
        mouse.scrolldx = queue.scrollx;
        mouse.scrolldy = queue.scrolly;
        mouse.dx = queue.x - mouse.x;
        mouse.dy = queue.y - mouse.y;
        mouse.x = queue.x;
        mouse.y = queue.y;
    }
};

};

#endif // __WASTELADNS_INPUT_MACOS_H__
