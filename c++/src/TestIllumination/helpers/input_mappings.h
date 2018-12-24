#ifndef __WASTELADNS_INPUT_MAPPINGS_H__
#define __WASTELADNS_INPUT_MAPPINGS_H__

namespace Input {
    
    namespace Gamepad {
        
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
                return true;
            } else if (hash == MappingPresets::mapping_8bitdoName) {
                mapping.b_mapping = MappingPresets::b_mapping_8bitdo;
                mapping.b_mappingCount = MappingPresets::b_mapping_8bitdoCount;
                mapping.a_mapping = MappingPresets::a_mapping_8bitdo;
                mapping.a_mappingCount = MappingPresets::a_mapping_8bitdoCount;
                return true;
            } else if (hash == MappingPresets::mapping_winbluetoothwirelessName) {
                mapping.b_mapping = MappingPresets::b_mapping_winbluetoothwireless;
                mapping.b_mappingCount = MappingPresets::b_mapping_winbluetoothwirelessCount;
                mapping.a_mapping = MappingPresets::a_mapping_winbluetoothwireless;
                mapping.a_mappingCount = MappingPresets::a_mapping_winbluetoothwirelessCount;
                return true;
            } else if (hash == MappingPresets::mapping_xboxName) {
                mapping.b_mapping = MappingPresets::b_mapping_xbox;
                mapping.b_mappingCount = MappingPresets::b_mapping_xboxCount;
                mapping.a_mapping = MappingPresets::a_mapping_xbox;
                mapping.a_mappingCount = MappingPresets::a_mapping_xboxCount;
                return true;
            }
            
            return false;
        };
    }
};

#endif // __WASTELADNS_INPUT_MAPPINGS_H__
