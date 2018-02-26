#ifndef __WASTELADNS_INPUT_H__
#define __WASTELADNS_INPUT_H__

namespace ControllerVertex {
    struct RenderBuffers;
    extern RenderBuffers* currentBuffer;
    extern RenderBuffers ps4_buffers;
}

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
		extern const Mapping mapping = {
			GLFW_KEY_ESCAPE
		};
	};

	template<typename _Set, typename _Buttons>
	struct DigitalState {
		bool down(_Set key) const {
			return (current & (1 << key)) != 0;
		}
		bool up(_Set key) const {
			return (current & (1 << key)) == 0;
		}
		bool released(_Set key) const {
            const _Buttons mask = (1 << key);
			return (current & mask) == 0 && (last & mask) != 0;
		}
		bool pressed(_Set key) const {
            const _Buttons mask = (1 << key);
			return (current & mask) != 0 && (last & mask) == 0;
		}

		_Buttons last;
		_Buttons current;
	};
    typedef DigitalState<DebugSet::Keys::Enum, u8> DebugState;
    
    template<typename _Set>
    struct AnalogState {
        f32 values[_Set::COUNT];
    };
    
    template <typename _Set, typename _Buttons>
    void pollState(DigitalState<_Set, _Buttons>& set, const s32* mapping, GLFWwindow* window) {
        set.last = set.current;
        set.current = (_Set) 0;
        for (int i = 0; i < _Set::COUNT; i++) {
            s32 keyState = glfwGetKey(window, mapping[i]);
            s32 keyOn = (keyState == GLFW_PRESS) || (keyState == GLFW_REPEAT);
            set.current = (_Set)(set.current | (keyOn << i));
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
            const f32 pressedThreshold = 0.5f;
        };
        
        namespace MappingPresets {
            
            const Analog::Enum a_mapping_default[] = {
                  Analog::AxisLH
                , Analog::AxisLV
                , Analog::AxisRH
                , Analog::AxisRV
                , Analog::Trigger_L
                , Analog::Trigger_R
            };
            const u32 a_mapping_defaultCount = sizeof(a_mapping_default) / sizeof(a_mapping_default[0]);
            const Digital::Enum b_mapping_default[] = {
                Digital::B_R,
                Digital::B_D,
                Digital::B_U,
                Digital::B_L,
                Digital::L2,
                Digital::R2,
                Digital::L1,
                Digital::R1,
                Digital::SELECT,
                Digital::START,
                Digital::A_L,
                Digital::A_R,
                Digital::D_U,
                Digital::D_R,
                Digital::D_D,
                Digital::D_L
            };
            const u32 b_mapping_defaultCount = sizeof(b_mapping_default) / sizeof(b_mapping_default[0]);
            
            const Analog::Enum a_mapping_8bitdo[] = {
                  Analog::AxisLH
                , Analog::AxisLV
                , Analog::AxisRH
                , Analog::AxisRV
            };
            const u32 a_mapping_8bitdoCount = sizeof(a_mapping_8bitdo) / sizeof(a_mapping_8bitdo[0]);
            const Digital::Enum b_mapping_8bitdo[] = {
                Digital::B_R,
                Digital::B_D,
                Digital::INVALID,
                Digital::B_U,
                Digital::B_L,
                Digital::INVALID,
                Digital::L1,
                Digital::R1,
                Digital::L2,
                Digital::R2,
                Digital::SELECT,
                Digital::START,
                Digital::INVALID,
                Digital::A_L,
                Digital::A_R,
                Digital::D_U,
                Digital::D_R,
                Digital::D_D,
                Digital::D_L
            };
            const u32 b_mapping_8bitdoCount = sizeof(b_mapping_8bitdo) / sizeof(b_mapping_8bitdo[0]);
            const u32 mapping_8bitdoName = Hash::fnv("8Bitdo NES30 Pro");
            
            const Analog::Enum a_mapping_ps4[] = {
                  Analog::AxisLH
                , Analog::AxisLV
                , Analog::AxisRH
                , Analog::Trigger_L
                , Analog::Trigger_R
                , Analog::AxisRV
            };
            const u32 a_mapping_ps4Count = sizeof(a_mapping_ps4) / sizeof(a_mapping_ps4[0]);
            const Digital::Enum b_mapping_ps4[] = {
                Digital::B_L,
                Digital::B_D,
                Digital::B_R,
                Digital::B_U,
                Digital::L1,
                Digital::R1,
                Digital::L2,
                Digital::R2,
                Digital::SELECT,
                Digital::START,
                Digital::A_L,
                Digital::A_R,
                Digital::INVALID,
                Digital::T,
                Digital::D_U,
                Digital::D_R,
                Digital::D_D,
                Digital::D_L
            };
            const u32 b_mapping_ps4Count = sizeof(b_mapping_ps4) / sizeof(b_mapping_ps4[0]);
            const u32 mapping_ps4Name = Hash::fnv("Wireless Controller");
            
            const Analog::Enum a_mapping_winbluetoothwireless[] = {
                  Analog::AxisLH
                , Analog::AxisLV
                , Analog::AxisLV
                , Analog::AxisRH
                , Analog::AxisRV
                , Analog::AxisRV
            };
            const u32 a_mapping_winbluetoothwirelessCount = sizeof(a_mapping_winbluetoothwireless) / sizeof(a_mapping_winbluetoothwireless[0]);
            const Digital::Enum b_mapping_winbluetoothwireless[] = {
                Digital::B_R,
                Digital::B_D,
                Digital::INVALID,
                Digital::B_U,
                Digital::B_L,
                Digital::INVALID,
                Digital::L2,
                Digital::R2,
                Digital::L1,
                Digital::R1,
                Digital::SELECT,
                Digital::START,
                Digital::INVALID,
                Digital::A_L,
                Digital::A_R,
                Digital::INVALID,
                Digital::D_U,
                Digital::D_R,
                Digital::D_D,
                Digital::D_L
            };
            const u32 b_mapping_winbluetoothwirelessCount = sizeof(b_mapping_winbluetoothwireless) / sizeof(b_mapping_winbluetoothwireless[0]);
            const u32 mapping_winbluetoothwirelessName = Hash::fnv("Bluetooth Wireless Controller   "); // TODO: handle spaces
            
            const Analog::Enum a_mapping_xbox[] = {
                Analog::AxisLH
                , Analog::AxisLV
                , Analog::AxisRH
                , Analog::AxisRV
                , Analog::Trigger_L
                , Analog::Trigger_R
            };
            const u32 a_mapping_xboxCount = sizeof(a_mapping_xbox) / sizeof(a_mapping_xbox[0]);
            const Digital::Enum b_mapping_xbox[] = {
                Digital::B_D,
                Digital::B_R,
                Digital::B_L,
                Digital::B_U,
                Digital::L1,
                Digital::R1,
                Digital::SELECT,
                Digital::START,
                Digital::A_L,
                Digital::A_R,
                Digital::D_U,
                Digital::D_R,
                Digital::D_D,
                Digital::D_L
            };
            const u32 b_mapping_xboxCount = sizeof(b_mapping_xbox) / sizeof(b_mapping_xbox[0]);
            const u32 mapping_xboxName = Hash::fnv("Xbox Controller");
        }
        
        struct Mapping {
            const Digital::Enum* b_mapping;
            s32 b_mappingCount;
            const Analog::Enum* a_mapping;
            s32 a_mappingCount;
            u32 name;
            bool set = false;
        };
        
        bool loadPreset(Gamepad::Mapping& mapping, const char* name) {
            u32 hash = Hash::fnv(name);
            mapping.b_mapping = MappingPresets::b_mapping_default;
            mapping.b_mappingCount = MappingPresets::b_mapping_defaultCount;
            mapping.a_mapping = MappingPresets::a_mapping_default;
            mapping.a_mappingCount = MappingPresets::a_mapping_defaultCount;
            
            if (hash == MappingPresets::mapping_ps4Name) {
                mapping.b_mapping = MappingPresets::b_mapping_ps4;
                mapping.b_mappingCount = MappingPresets::b_mapping_ps4Count;
                mapping.a_mapping = MappingPresets::a_mapping_ps4;
                mapping.a_mappingCount = MappingPresets::a_mapping_ps4Count;
#if CONTROLLER_LOAD_SVG_DATA == 0
                ControllerVertex::currentBuffer = &ControllerVertex::ps4_buffers;
#endif
                return true;
            } else if (hash == MappingPresets::mapping_8bitdoName) {
                mapping.b_mapping = MappingPresets::b_mapping_8bitdo;
                mapping.b_mappingCount = MappingPresets::b_mapping_8bitdoCount;
                mapping.a_mapping = MappingPresets::a_mapping_8bitdo;
                mapping.a_mappingCount = MappingPresets::a_mapping_8bitdoCount;
#if CONTROLLER_LOAD_SVG_DATA == 0
                ControllerVertex::currentBuffer = &ControllerVertex::ps4_buffers;
#endif
                return true;
            } else if (hash == MappingPresets::mapping_winbluetoothwirelessName) {
                mapping.b_mapping = MappingPresets::b_mapping_winbluetoothwireless;
                mapping.b_mappingCount = MappingPresets::b_mapping_winbluetoothwirelessCount;
                mapping.a_mapping = MappingPresets::a_mapping_winbluetoothwireless;
                mapping.a_mappingCount = MappingPresets::a_mapping_winbluetoothwirelessCount;
#if CONTROLLER_LOAD_SVG_DATA == 0
                ControllerVertex::currentBuffer = &ControllerVertex::ps4_buffers;
#endif
                return true;
            } else if (hash == MappingPresets::mapping_xboxName) {
                mapping.b_mapping = MappingPresets::b_mapping_xbox;
                mapping.b_mappingCount = MappingPresets::b_mapping_xboxCount;
                mapping.a_mapping = MappingPresets::a_mapping_xbox;
                mapping.a_mappingCount = MappingPresets::a_mapping_xboxCount;
#if CONTROLLER_LOAD_SVG_DATA == 0
                ControllerVertex::currentBuffer = &ControllerVertex::ps4_buffers;
#endif
                return true;
            }
            
            return false;
        };
        
        struct State {
            DigitalState<Digital::Enum, u32> buttons;
            AnalogState<Analog::Enum> analogs;
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
                        pad.analogs.values[axisIndex] = axes[i];
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
                    bool triggerPressed = trigger_l > Gamepad::Analog::pressedThreshold;
                    pad.buttons.current = (Gamepad::Digital::Enum) (pad.buttons.current | (triggerPressed << Gamepad::Digital::L2));
                    digitalTrigger_L = true;
                }
            }
            if (!digitalTrigger_R) {
                const f32 trigger_r = pad.analogs.values[Gamepad::Analog::Trigger_R];
                if (trigger_r != Gamepad::Analog::novalue) {
                    bool triggerPressed = trigger_r > Gamepad::Analog::pressedThreshold;
                    pad.buttons.current = (Gamepad::Digital::Enum) (pad.buttons.current | (triggerPressed << Gamepad::Digital::R2));
                    digitalTrigger_R = true;
                }
            }
        }
    }
};

#endif // __WASTELADNS_INPUT_H__
