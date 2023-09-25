#ifndef __WASTELADNS_INPUT_H__
#define __WASTELADNS_INPUT_H__

#ifndef UNITYBUILD
#include "hash.h"
#endif

namespace Input {
    
    namespace Keyboard {
        
        struct State {
            bool down(Keys::Enum key) const {
                return current[key] != 0;
            }
            bool up(Keys::Enum key) const {
                return current[key] == 0;
            }
            bool released(Keys::Enum key) const {
                return current[key] == 0 && last[key] != 0;
            }
            bool pressed(Keys::Enum key) const {
                return current[key] != 0 && last[key] == 0;
            }

            u8 last[Keys::COUNT];
            u8 current[Keys::COUNT];
        };
    };

    namespace Gamepad {
        
        namespace Keys { enum Enum : s32 {
              B_U = 0
            , B_D = 1
            , B_L = 2
            , B_R = 3
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
            , TOUCH = 16
            , COUNT
            , INVALID = -1
        }; };

        namespace Analog { enum Enum : s8 {
              AxisLH = 0
            , AxisLV
            , AxisRH
            , AxisRV
            , Trigger_L
            , Trigger_R
            , COUNT
            , INVALID = -1
        }; };

//        struct Mapping {
//            const Digital::Enum* b_mapping;
//            const Analog::Enum* a_mapping;
//            s32 b_mappingCount;
//            s32 a_mappingCount;
//            u32 name;
//            bool set = false;
//        };

//        bool loadPreset(Gamepad::Mapping& mapping, const char* name);

        struct State {
            
            struct KeyState {
                u32 last;
                u32 current;
            };
            
            bool down(Keys::Enum key) const {
                return (keys.current & (1 << key)) != 0;
            }
            bool up(Keys::Enum key) const {
                return (keys.current & (1 << key)) == 0;
            }
            bool released(Keys::Enum key) const {
                const u32 mask = (1 << key);
                return (keys.current & mask) == 0 && (keys.last & mask) != 0;
            }
            bool pressed(Keys::Enum key) const {
                const u32 mask = (1 << key);
                return (keys.current & mask) != 0 && (keys.last & mask) == 0;
            }
            
            KeyState keys;
            f32 analogs[Analog::COUNT];
            bool active = false;
        };
    };

	namespace Mouse {
    
		struct State {
			bool down(Keys::Enum key) const {
				return current[key] != 0;
			}
			bool up(Keys::Enum key) const {
				return current[key] == 0;
			}
			bool released(Keys::Enum key) const {
				return current[key] == 0 && last[key] != 0;
			}
			bool pressed(Keys::Enum key) const {
				return current[key] != 0 && last[key] == 0;
			}

			u8 last[Keys::COUNT];
			u8 current[Keys::COUNT];
			f32 dx;
			f32 dy;
			f32 x;
			f32 y;
			f32 scrolldx;
			f32 scrolldy;
		};
	};
};

//#include "input_mappings.h"

#endif // __WASTELADNS_INPUT_H__
