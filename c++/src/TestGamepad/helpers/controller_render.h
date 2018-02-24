#ifndef __WASTELADNS_CONTROLLER_RENDER_H__
#define __WASTELADNS_CONTROLLER_RENDER_H__

namespace Hash {
    
    // TODO: understand this
    u32 fnv(const char* name) {
        const u8* data = (const u8*)name;
        u32 val = 3759247821;
        while(*data){
            val ^= *data++;
            val *= 0x01000193;
        }
        val &= 0x7fffffff;
        val |= val==0;
        return val;
    }
}

namespace ControllerVertex {

const u32 vertexMemMaxCount = 1000;
f32 vertexMem[vertexMemMaxCount];
u32 vertexMemCount = 0;
    
const char* svg = R"(

<path id="base" class="cls-1" d="M89,155L59,199l-29,1L19,178,33,78,49,43H75l8,15H212l8-15h26l16,35,14,100-11,22-29-1-30-44H89Z"/>
<g id="buttons">
<path id="t" class="cls-1" d="M241.44,102.635l0.47-8.239-5-5.4h-8.491l-5.268,5.4,0.058,8.239,5.012,4.917h7.963Z"/>
<path id="x" class="cls-1" d="M239.354,139.259l0.355-6.231-4.365-4.056h-7.352l-4.567,4.056,0.044,6.231L227.841,143H234.8Z"/>
<path id="o" class="cls-1" d="M261.075,122.206l0.965-7.13-4.3-4.654h-7.881l-5.257,4.654-0.508,7.13,4.336,4.27h7.424Z"/>
<path id="s" class="cls-1" d="M219.575,122.206l-0.152-7.13-5.025-4.654h-7.881l-4.528,4.654,0.609,7.13,5,4.27h7.424Z"/>
</g>
<g id="axis-r">
<path id="axis-r-base" class="cls-2" d="M175.009,155.626l-2-10.876L178.877,134H196.12l5.865,10.75-3.378,12.028L193.428,162H181.569Z"/>
<path id="axis-r-mov" class="cls-1" d="M177.965,161.76l-1.924-7.646,4.4-8.114H194.6l4.4,8.114-3.01,8.425L191.96,166h-8.889Z"/>
</g>
<g id="axis-l">
<path id="axis-l-base" class="cls-2" d="M96.009,155.626l-2-10.876L99.877,134H117.12l5.865,10.75-3.378,12.028L114.428,162H102.569Z"/>
<path id="axis-l-mov" class="cls-1" d="M98.965,161.76l-1.923-7.646L101.436,146H115.6l4.395,8.114-3.01,8.425L112.96,166h-8.889Z"/>
</g>
<g id="dpad">
<path id="u" class="cls-1" d="M52.532,95l10.407,16.661L72.371,95H52.532Z"/>
<path id="r" class="cls-1" d="M87.152,112.677l-15.278,8.672,13.636,7.91Z"/>
<path id="d" class="cls-1" d="M71.5,141l-8.05-11.741L56.093,141H71.5Z"/>
<path id="l" class="cls-1" d="M41.4,129.259l13.172-7.91-15.786-8.672Z"/>
</g>
<path id="select" class="cls-1" d="M104.077,84.1H95l2.441,12.8h4.923Z"/>
<path id="start" class="cls-1" d="M190.986,84H200L197.576,97h-4.89Z"/>
<path id="r1" class="cls-1" d="M250,69H221l4-19h19Z"/>
<path id="r2" class="cls-1" d="M247,43H219l7-14h14Z"/>
<path id="l1" class="cls-1" d="M45,69H74L70,50H51Z"/>
<path id="l2" class="cls-1" d="M48,43H76L70,29H55Z"/>
<path id="tpad" class="cls-1" d="M112,65l-4,22,11,38h57l11-38-4-22H112Z"/>

)";
    
enum class StaticShape { Base, AxisBase_L, AxisBase_R, Count };
enum class DynamicShape { Tpad, Button_D, Button_L, Button_R, Button_U, Dpad_D, Dpad_L, Dpad_R, Dpad_U, Start, Select, L1, R1, Axis_L, Axis_R, L2, R2, Count, ButtonEnd = (s32)R1 + 1, AxisStart = Axis_L, AxisEnd = (s32)Axis_R + 1, TriggerStart = L2, TriggerEnd = (s32)R2 + 1 };

enum class Digital { Tpad, Button_D, Button_L, Button_R, Button_U, Dpad_D, Dpad_L, Dpad_R, Dpad_U, Start, Select, L1, R1, Axis_L, Axis_R, L2, R2, Count, Invalid = -1 };
enum class Analog { Axis_LH, Axis_LV, Axis_RH, Axis_RV, L2, R2, Count, Invalid = -1 };

const ControllerVertex::Analog axis2analog_mapping[] = {
      ControllerVertex::Analog::Axis_LH
    , ControllerVertex::Analog::Axis_RH
};

const ControllerVertex::Analog trigger2analog_mapping[] = {
      ControllerVertex::Analog::L2
    , ControllerVertex::Analog::R2
};
    
const ControllerVertex::Digital shape2button_mapping[] = {
      ControllerVertex::Digital::Tpad
    , ControllerVertex::Digital::Button_D
    , ControllerVertex::Digital::Button_L
    , ControllerVertex::Digital::Button_R
    , ControllerVertex::Digital::Button_U
    , ControllerVertex::Digital::Dpad_D
    , ControllerVertex::Digital::Dpad_L
    , ControllerVertex::Digital::Dpad_R
    , ControllerVertex::Digital::Dpad_U
    , ControllerVertex::Digital::Start
    , ControllerVertex::Digital::Select
    , ControllerVertex::Digital::L1
    , ControllerVertex::Digital::R1
    , ControllerVertex::Digital::Axis_L
    , ControllerVertex::Digital::Axis_R
    , ControllerVertex::Digital::L2
    , ControllerVertex::Digital::R2
};
    
const ControllerVertex::Analog a_mapping_default[] = {
      ControllerVertex::Analog::Axis_LH
    , ControllerVertex::Analog::Axis_LV
    , ControllerVertex::Analog::Axis_RH
    , ControllerVertex::Analog::Axis_RV
    , ControllerVertex::Analog::L2
    , ControllerVertex::Analog::R2
};
const u32 a_mapping_defaultCount = sizeof(a_mapping_default) / sizeof(a_mapping_default[0]);
const ControllerVertex::Digital b_mapping_default[] = {
    ControllerVertex::Digital::Button_R,
    ControllerVertex::Digital::Button_D,
    ControllerVertex::Digital::Button_U,
    ControllerVertex::Digital::Button_L,
    ControllerVertex::Digital::L2,
    ControllerVertex::Digital::R2,
    ControllerVertex::Digital::L1,
    ControllerVertex::Digital::R1,
    ControllerVertex::Digital::Select,
    ControllerVertex::Digital::Start,
    ControllerVertex::Digital::Axis_L,
    ControllerVertex::Digital::Axis_R,
    ControllerVertex::Digital::Dpad_U,
    ControllerVertex::Digital::Dpad_R,
    ControllerVertex::Digital::Dpad_D,
    ControllerVertex::Digital::Dpad_L
};
const u32 b_mapping_defaultCount = sizeof(b_mapping_default) / sizeof(b_mapping_default[0]);
    
const ControllerVertex::Analog a_mapping_8bitdo[] = {
      ControllerVertex::Analog::Axis_LH
    , ControllerVertex::Analog::Axis_LV
    , ControllerVertex::Analog::Axis_RH
    , ControllerVertex::Analog::Axis_RV
};
const u32 a_mapping_8bitdoCount = sizeof(a_mapping_8bitdo) / sizeof(a_mapping_8bitdo[0]);
const ControllerVertex::Digital b_mapping_8bitdo[] = {
    ControllerVertex::Digital::Button_R,
    ControllerVertex::Digital::Button_D,
    ControllerVertex::Digital::Invalid,
    ControllerVertex::Digital::Button_U,
    ControllerVertex::Digital::Button_L,
    ControllerVertex::Digital::Invalid,
    ControllerVertex::Digital::L2,
    ControllerVertex::Digital::R2,
    ControllerVertex::Digital::L1,
    ControllerVertex::Digital::R1,
    ControllerVertex::Digital::Select,
    ControllerVertex::Digital::Start,
    ControllerVertex::Digital::Invalid,
    ControllerVertex::Digital::Axis_L,
    ControllerVertex::Digital::Axis_R,
    ControllerVertex::Digital::Dpad_U,
    ControllerVertex::Digital::Dpad_R,
    ControllerVertex::Digital::Dpad_D,
    ControllerVertex::Digital::Dpad_L
};
const u32 b_mapping_8bitdoCount = sizeof(b_mapping_8bitdo) / sizeof(b_mapping_8bitdo[0]);
const u32 mapping_8bitdoName = Hash::fnv("8Bitdo NES30 Pro");

const ControllerVertex::Analog a_mapping_ps4[] = {
      ControllerVertex::Analog::Axis_LH
    , ControllerVertex::Analog::Axis_LV
    , ControllerVertex::Analog::Axis_RH
    , ControllerVertex::Analog::L2
    , ControllerVertex::Analog::R2
    , ControllerVertex::Analog::Axis_RV
};
const u32 a_mapping_ps4Count = sizeof(a_mapping_ps4) / sizeof(a_mapping_ps4[0]);
const ControllerVertex::Digital b_mapping_ps4[] = {
    ControllerVertex::Digital::Button_L,
    ControllerVertex::Digital::Button_D,
    ControllerVertex::Digital::Button_R,
    ControllerVertex::Digital::Button_U,
    ControllerVertex::Digital::L1,
    ControllerVertex::Digital::R1,
    ControllerVertex::Digital::L2,
    ControllerVertex::Digital::R2,
    ControllerVertex::Digital::Select,
    ControllerVertex::Digital::Start,
    ControllerVertex::Digital::Axis_L,
    ControllerVertex::Digital::Axis_R,
    ControllerVertex::Digital::Invalid,
    ControllerVertex::Digital::Tpad,
    ControllerVertex::Digital::Dpad_U,
    ControllerVertex::Digital::Dpad_R,
    ControllerVertex::Digital::Dpad_D,
    ControllerVertex::Digital::Dpad_L
    
};
const u32 b_mapping_ps4Count = sizeof(b_mapping_ps4) / sizeof(b_mapping_ps4[0]);
const u32 mapping_ps4Name = Hash::fnv("Wireless Controller");
    
typedef std::map<u32, std::string> SVGPaths;
    
struct RenderBuffer {
    Vec2* vertex;
    Vec2 max, min;
    u32 count;
};

struct RenderBuffers {
    
    RenderBuffers() {}
    
    union {
        struct {
            RenderBuffer base, axisbase_l, axisbase_r;
            RenderBuffer tpad, button_d, button_l, button_r, button_u, dpad_d, dpad_l, dpad_r, dpad_u, start, select, l1, r1, axis_l, axis_r, l2, r2;
        };
        struct {
            RenderBuffer sbuffers[(s32)StaticShape::Count];
            RenderBuffer dbuffers[(s32)DynamicShape::Count];
        };
        RenderBuffer buffers[(s32)StaticShape::Count + (s32)DynamicShape::Count];
    };
    
};

void parsenode_svg(RenderBuffer& vbuffer, const char* svg_path) {
    
    Vec2& max = vbuffer.max;
    Vec2& min = vbuffer.min;
    max.x = max.y = -10000.f;
    min.x = min.y = 10000.f;
    vbuffer.count = 0;
    
    vbuffer.vertex = (Vec2*) &(vertexMem[vertexMemCount]);
    vbuffer.count = 0;
    const u32 vertexPerPoint = 2;
    const u32 maxVertexCount = (vertexMemMaxCount - vertexMemCount) / 3;
    
    enum class Mode { None, Move, Line, HLine, VLine };
    bool local = false;
    Mode mode = Mode::None;
    
    for (char* svgptr = const_cast<char*>(svg_path); svgptr && *svgptr != '\0' && vbuffer.count < maxVertexCount; svgptr++) {
        
        const char c = *svgptr;
        if (isspace(c)) {}
        else if (c == ',') {}
        else if (c == 'M') {
            mode = Mode::Move;
            local = false;
        } else if (c =='m') {
            mode = Mode::Move;
            local = true;
        } else if (c == 'L') {
            mode = Mode::Line;
            local = false;
        } else if (c =='l') {
            mode = Mode::Line;
            local = true;
        } else if (c == 'H') {
            mode = Mode::HLine;
            local = false;
        } else if (c =='h') {
            mode = Mode::HLine;
            local = true;
        } else if (c == 'V') {
            mode = Mode::VLine;
            local = false;
        } else if (c =='v') {
            mode = Mode::VLine;
            local = true;
        } else if (c == 'Z') {
            break;
        } else {
            
            Vec2* v = nullptr;
            switch(mode) {
                case Mode::Move:
                case Mode::Line:
                {
                    f32 x = strtof(svgptr, &svgptr);
                    if (*svgptr == ',') {
                        svgptr++;
                    }
                    f32 y = -strtof(svgptr, &svgptr);
                    
                    v = &vbuffer.vertex[vbuffer.count];
                    v->x = x; v->y = y;
                    if (local) {
                        Vec2& vprev = vbuffer.vertex[vbuffer.count-1];
                        *v = Vec::add(*v, vprev);
                    }
                }
                break;
    
                case Mode::VLine:
                {
                    f32 y = -strtof(svgptr, &svgptr);
                    
                    v = &vbuffer.vertex[vbuffer.count];
                    v->y = y;
                    Vec2& vprev = vbuffer.vertex[vbuffer.count-1];
                    if (local) {
                        v->x = 0.f;
                        *v = Vec::add(*v, vprev);
                    } else {
                        v->x = vprev.x;
                    }
                }
                break;
                case Mode::HLine:
                {
                    f32 x = strtof(svgptr, &svgptr);
                    
                    v = &vbuffer.vertex[vbuffer.count];
                    v->x = x;
                    Vec2& vprev = vbuffer.vertex[vbuffer.count-1];
                    if (local) {
                        v->y = 0.f;
                        *v = Vec::add(*v, vprev);
                    } else {
                        v->y = vprev.y;
                    }
                }
                break;
                default:
                    break;
            }
            
            if (v) {
                vbuffer.count++;
                max = Vec::max(max, *v);
                min = Vec::min(min, *v);
                
                // Read delimiter
                svgptr--;
            }
        }
    }
    
    vertexMemCount += vbuffer.count * vertexPerPoint;
}

void parsetree_svg(RenderBuffers& vertexBuffers, const char* svg_tree) {
    
    SVGPaths rawPaths;
    
    {
        bool tag = false;
        bool id = false;
        bool d = false;;
        
        std::string idname;
        std::string direction;
        bool hasid = false;
        bool hasd = false;
        std::string* currentBuffer = nullptr;
        
        for (char* svgptr = const_cast<char*>(svg_tree); svgptr && *svgptr != '\0'; svgptr++) {
            
            const char c = *svgptr;
            if (isspace(c)) {}
            else if (c == '<') {
                tag = true;
            } else if (c == '>') {
                tag = false;
                id = hasid = d = hasd = false;
            } else if (tag) {
                
                if (c == '/') {
                    
                } else if (c == '\"') {
                    
                    svgptr++;
                    while (svgptr && *svgptr != '\0' && *svgptr != '\"') {
                        if (currentBuffer) {
                            *currentBuffer += *svgptr;
                        }
                        svgptr++;
                    }
                    currentBuffer = nullptr;
                    if (id) {
                        hasid = true;
                        id = false;
                    }
                    if (d) {
                        hasd = true;
                        d = false;
                    }
                    
                    if (hasid && hasd) {
                        hasid = hasd = false;
                        u32 hash = Hash::fnv(idname.c_str());
                        rawPaths[hash] = direction;
                    }
                    
                } else if (isalnum(c)) {
                    
                    char* advanceptr;
                    
                    advanceptr = svgptr;
                    bool matched_id = *(advanceptr++) == 'i' && advanceptr && *(advanceptr) == 'd';
                    if (matched_id && !hasid) {
                        id = true;
                        svgptr = advanceptr;
                        idname = "";
                        continue;
                    }
                    
                    advanceptr = svgptr;
                    bool matched_d = *advanceptr == 'd';
                    if (matched_d && !hasd) {
                        d = true;
                        svgptr = advanceptr;
                        direction = "";
                        continue;
                    }
                    
                    // consume
                    while (svgptr && *svgptr != '\0' && isalnum(*svgptr)) {
                        svgptr++;
                    }
                    
                } else if (c == '=') {
                    
                    if (id) {
                        currentBuffer = &idname;
                    } else if (d) {
                        currentBuffer = &direction;
                    }
                }
            }
        }
    }
    
    Vec2 max, min;
    
    struct ShapeEntry {
        RenderBuffer* buffer;
        u32 hash;
    };
    ShapeEntry shapes[] = {
          { &vertexBuffers.base, Hash::fnv("base") }
        , { &vertexBuffers.tpad, Hash::fnv("tpad") }
        , { &vertexBuffers.axisbase_l, Hash::fnv("axis-l-base") }
        , { &vertexBuffers.axisbase_r, Hash::fnv("axis-r-base") }
        , { &vertexBuffers.button_u, Hash::fnv("t") }
        , { &vertexBuffers.button_d, Hash::fnv("x") }
        , { &vertexBuffers.button_r, Hash::fnv("o") }
        , { &vertexBuffers.button_l, Hash::fnv("s") }
        , { &vertexBuffers.dpad_u, Hash::fnv("u") }
        , { &vertexBuffers.dpad_r, Hash::fnv("r") }
        , { &vertexBuffers.dpad_d, Hash::fnv("d") }
        , { &vertexBuffers.dpad_l, Hash::fnv("l") }
        , { &vertexBuffers.start, Hash::fnv("start") }
        , { &vertexBuffers.select, Hash::fnv("select") }
        , { &vertexBuffers.l1, Hash::fnv("l1") }
        , { &vertexBuffers.l2, Hash::fnv("l2") }
        , { &vertexBuffers.r1, Hash::fnv("r1") }
        , { &vertexBuffers.r2, Hash::fnv("r2") }
        , { &vertexBuffers.axis_l, Hash::fnv("axis-l-mov") }
        , { &vertexBuffers.axis_r, Hash::fnv("axis-r-mov") }
    };
    
    for (ShapeEntry& entry : shapes) {
        auto search = rawPaths.find(entry.hash);
        if (search != rawPaths.end()) {
            parsenode_svg(*entry.buffer, search->second.c_str());
            max = Vec::max(max, entry.buffer->max);
            min = Vec::min(min, entry.buffer->min);
        }
    }
    
    // Center all nodes and recompute min-max
    Vec2 center = Vec::scale(Vec::add(max, min), 0.5f);
    for (ShapeEntry& entry : shapes) {
        
        Vec2& shapeMax = entry.buffer->max;
        Vec2& shapeMin = entry.buffer->min;
        
        shapeMin.x = shapeMin.y = 10000.f;
        shapeMax.x = shapeMax.y = -10000.f;
        for (u32 i = 0; i < entry.buffer->count; i++) {
            Vec2& pos = entry.buffer->vertex[i];
            pos = Vec::subtract(pos, center);
            
            shapeMax = Vec::max(shapeMax, pos);
            shapeMin = Vec::min(shapeMin, pos);
        }
    }
}
    
}

#endif // __WASTELADNS_CONTROLLER_RENDER_H__
