#ifndef __WASTELADNS_INPUT_H__
#define __WASTELADNS_INPUT_H__

namespace Input {

	namespace DebugSet {
        struct Keys {
            enum Enum : u8 {
                  ESC = 0
                , EXIT = ESC
                , COUNT
                , CLEAR = 0
            };
        };
		typedef s32 Mapping[Keys::COUNT];
		const Mapping mapping = {
			GLFW_KEY_ESCAPE
		};
	};

	template<s32 _ButtonCount>
	struct DigitalState {

        typedef typename std::conditional< _ButtonCount <= 8, u8
              , typename std::conditional< _ButtonCount <= 16, u16
              , typename std::conditional< _ButtonCount <= 32, u32
              , u64
              >::type >::type >::type _Buttons;
        static_assert(_ButtonCount <= 64, "Unsupported _ButtonCount pased to DigitalState");
        
		bool down(s32 key) const {
			return (current & (1 << key)) != 0;
		}
		bool up(s32 key) const {
			return (current & (1 << key)) == 0;
		}
		bool released(s32 key) const {
            const _Buttons mask = (1 << key);
			return (current & mask) == 0 && (last & mask) != 0;
		}
		bool pressed(s32 key) const {
            const _Buttons mask = (1 << key);
			return (current & mask) != 0 && (last & mask) == 0;
		}

		_Buttons last;
		_Buttons current;
	};
    typedef DigitalState<DebugSet::Keys::COUNT> DebugState;
    
    template<s32 _Count>
    struct AnalogState {
        f32 values[_Count];
    };
    
    template <s32 _ButtonCount>
    void pollState(DigitalState<_ButtonCount>& set, const s32* mapping, GLFWwindow* window) {
        set.last = set.current;
        set.current = 0;
        for (int i = 0; i < _ButtonCount; i++) {
            s32 keyState = glfwGetKey(window, mapping[i]);
            s32 keyOn = keyState != GLFW_RELEASE;
            set.current = set.current | (keyOn << i);
        }
    }

	struct KeyboardState {
		bool down(s32 key) {
			return current[key] != 0;
		}
		bool up(s32 key) {
			return current[key] == 0;
		}
		bool released(s32 key) {
			return current[key] == 0 && last[key] != 0;
		}
		bool pressed(s32 key) {
			return current[key] != 0 && last[key] == 0;
		}
		void pollState(GLFWwindow* window) {
			memcpy(last, current, GLFW_KEY_LAST);
			memset(current, 0, GLFW_KEY_LAST);
			for (int i = 0; i < GLFW_KEY_LAST; i++) {
				current[i] = glfwGetKey(window, i);
			}
		}

		u8 last[GLFW_KEY_LAST];
		u8 current[GLFW_KEY_LAST];
	};
    
    namespace Gamepad {
        
        namespace Digital {
            enum Enum : s32 {
                  B_U = 0
                , Sony_Triangle = 0
                , Nin_X = 0
                , Xbox_Y = 0
                , B_D = 1
                , Sony_X = 1
                , Nin_B = 1
                , Xbox_A = 1
                , B_L = 2
                , Sony_Square = 2
                , Nin_Y = 2
                , Xbox_X = 2
                , B_R = 3
                , Sony_Circle = 3
                , Nin_A = 3
                , Xbox_B = 3
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
                , T = 16
                , COUNT
                , INVALID = -1
            };
        }
        
        namespace Analog {
            enum Enum : s8 {
                  AxisLH = 0
                , AxisLV
                , AxisRH
                , AxisRV
                , Trigger_L
                , Trigger_R
                , COUNT
                , INVALID = -1
            };
            
            const f32 novalue = 404.f;
            const f32 digitalThreshold = 0.5f;
        };
        
        struct Mapping {
            const Digital::Enum* b_mapping;
            const Analog::Enum* a_mapping;
            s32 b_mappingCount;
            s32 a_mappingCount;
            u32 name;
            bool set = false;
        };
        
        bool loadPreset(Gamepad::Mapping& mapping, const char* name);
        
        struct State {
            DigitalState<Digital::COUNT> buttons;
            AnalogState<Analog::COUNT> analogs;
            Mapping mapping = {};
            bool active = false;
        };
    };
    
    void pollState(Gamepad::State& pad, s32 joyId, GLFWwindow* window) {
        
        pad.active = glfwJoystickPresent(joyId) != 0;
        if (pad.active) {
            
            if (!pad.mapping.set) {
                loadPreset(pad.mapping, glfwGetJoystickName(joyId));
                pad.mapping.set = true;
            }
            
            // Analogs (axis and triggers)
            for (s32 i = 0; i < Gamepad::Analog::COUNT; i++) {
                pad.analogs.values[i] = Gamepad::Analog::novalue;
            }
            s32 axesCount;
            const f32* axes = glfwGetJoystickAxes(joyId, &axesCount);
            for (s32 i = 0; i < axesCount; i++) {
                if (i < pad.mapping.a_mappingCount) {
                    const Gamepad::Analog::Enum axisIndex = pad.mapping.a_mapping[i];
                    if (axisIndex != Gamepad::Analog::INVALID) {
                        f32 v = axes[i];
                        const f32 deadzone = 0.025f;
                        if (Math::abs(v) < deadzone) {
                            v = 0.f;
                        }
                        pad.analogs.values[axisIndex] = v;
                    }
                }
            }
            
            bool digitalTrigger_L = false;
            bool digitalTrigger_R = false;
            
            pad.buttons.last = pad.buttons.current;
            pad.buttons.current = (Gamepad::Digital::Enum) 0;
            s32 buttonCount;
            const u8* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
            for (s32 i = 0; i < buttonCount; i++) {
                if (i < pad.mapping.b_mappingCount) {
                    const Gamepad::Digital::Enum buttonIndex = pad.mapping.b_mapping[i];
                    if (buttonIndex != Gamepad::Digital::INVALID) {
                        pad.buttons.current = pad.buttons.current | ((buttons[i] != 0) << buttonIndex);
                    }
                    if (buttonIndex == Gamepad::Digital::L2) {
                        digitalTrigger_L = true;
                    } else if (buttonIndex == Gamepad::Digital::R2) {
                        digitalTrigger_R = true;
                    }
                }
            }
            
            // Manually set digital trigger values, if needed
            if (!digitalTrigger_L) {
                const f32 trigger_l = pad.analogs.values[Gamepad::Analog::Trigger_L];
                if (trigger_l != Gamepad::Analog::novalue) {
                    bool triggerPressed = trigger_l > Gamepad::Analog::digitalThreshold;
                    pad.buttons.current = (Gamepad::Digital::Enum) (pad.buttons.current | (triggerPressed << Gamepad::Digital::L2));
                    digitalTrigger_L = true;
                }
            }
            if (!digitalTrigger_R) {
                const f32 trigger_r = pad.analogs.values[Gamepad::Analog::Trigger_R];
                if (trigger_r != Gamepad::Analog::novalue) {
                    bool triggerPressed = trigger_r > Gamepad::Analog::digitalThreshold;
                    pad.buttons.current = (Gamepad::Digital::Enum) (pad.buttons.current | (triggerPressed << Gamepad::Digital::R2));
                    digitalTrigger_R = true;
                }
            }
        }
    }
};

#include "input_mappings.h"
#include "input_helpers.h"

#endif // __WASTELADNS_INPUT_H__
