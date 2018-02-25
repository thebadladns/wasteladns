#ifndef __WASTELADNS_CONTROLLER_RENDER_H__
#define __WASTELADNS_CONTROLLER_RENDER_H__

namespace ControllerVertex {
    
enum class StaticShape { Base, AxisBase_L, AxisBase_R, Count };
enum class DynamicShape { Tpad, Button_D, Button_L, Button_R, Button_U, Dpad_D, Dpad_L, Dpad_R, Dpad_U, Start, Select, L1, R1, Axis_L, Axis_R, L2, R2, Count, ButtonEnd = (s32)R1 + 1, AxisStart = Axis_L, AxisEnd = (s32)Axis_R + 1, TriggerStart = L2, TriggerEnd = (s32)R2 + 1 };
    
const Input::Gamepad::Analog::Enum axis2analog_mapping[] = {
  Input::Gamepad::Analog::AxisLH
, Input::Gamepad::Analog::AxisRH
};

const Input::Gamepad::Digital::Enum shape2button_mapping[] = {
  Input::Gamepad::Digital::T
, Input::Gamepad::Digital::B_D
, Input::Gamepad::Digital::B_L
, Input::Gamepad::Digital::B_R
, Input::Gamepad::Digital::B_U
, Input::Gamepad::Digital::D_D
, Input::Gamepad::Digital::D_L
, Input::Gamepad::Digital::D_R
, Input::Gamepad::Digital::D_U
, Input::Gamepad::Digital::START
, Input::Gamepad::Digital::SELECT
, Input::Gamepad::Digital::L1
, Input::Gamepad::Digital::R1
, Input::Gamepad::Digital::A_L
, Input::Gamepad::Digital::A_R
, Input::Gamepad::Digital::L2
, Input::Gamepad::Digital::R2
};
    
const Input::Gamepad::Analog::Enum trigger2analog_mapping[] = {
  Input::Gamepad::Analog::Trigger_L
, Input::Gamepad::Analog::Trigger_R
};

struct RenderBuffer {
    Vec2* vertex;
    Vec2 max, min;
    u32 count;
};
    
struct RenderBuffers {

    RenderBuffers() {}
    RenderBuffers(
          const RenderBuffer& base
        , const RenderBuffer& axisbase_l
        , const RenderBuffer& axisbase_r
        , const RenderBuffer& tpad
        , const RenderBuffer& button_d
        , const RenderBuffer& button_l
        , const RenderBuffer& button_r
        , const RenderBuffer& button_u
        , const RenderBuffer& dpad_d
        , const RenderBuffer& dpad_l
        , const RenderBuffer& dpad_r
        , const RenderBuffer& dpad_u
        , const RenderBuffer& start
        , const RenderBuffer& select
        , const RenderBuffer& l1
        , const RenderBuffer& r1
        , const RenderBuffer& axis_l
        , const RenderBuffer& axis_r
        , const RenderBuffer& l2
        , const RenderBuffer& r2
        , const Vec2& max
        , const Vec2& min
        ) {
        this->base = base;
        this->axisbase_l = axisbase_l;
        this->axisbase_r = axisbase_r;
        this->tpad = tpad;
        this->button_d = button_d;
        this->button_l = button_l;
        this->button_r = button_r;
        this->button_u = button_u;
        this->dpad_d = dpad_d;
        this->dpad_l = dpad_l;
        this->dpad_r = dpad_r;
        this->dpad_u = dpad_u;
        this->start = start;
        this->select = select;
        this->l1 = l1;
        this->r1 = r1;
        this->axis_l = axis_l;
        this->axis_r = axis_r;
        this->l2 = l2;
        this->r2 = r2;
        this->max = max;
        this->min = min;
    }
    
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
    
    Vec2 max, min;
};
    
RenderBuffers* currentBuffer = nullptr;
    
}

#include "controller_renderdata.h"

#if CONTROLLER_LOAD_SVG_DATA

namespace ControllerVertex {

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

void parsetree_svg(RenderBuffers& vertexBuffers, const char* svg_tree, const char* name) {
    
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
    
    Vec2& max = vertexBuffers.max;
    Vec2& min = vertexBuffers.min;
    
	max.x = max.y = -10000.f;
	min.x = min.y = 10000.f;
    
    struct ShapeEntry {
        RenderBuffer* buffer;
        const char* name;
    };
    ShapeEntry shapes[] = {
          { &vertexBuffers.base, "base" }
        , { &vertexBuffers.axisbase_l, "axis_l_base" }
        , { &vertexBuffers.axisbase_r, "axis_r_base" }
        , { &vertexBuffers.tpad, "tpad" }
        , { &vertexBuffers.button_d, "x" }
        , { &vertexBuffers.button_l, "s" }
        , { &vertexBuffers.button_r, "o" }
        , { &vertexBuffers.button_u, "t" }
        , { &vertexBuffers.dpad_d, "d" }
        , { &vertexBuffers.dpad_l, "l" }
        , { &vertexBuffers.dpad_r, "r" }
        , { &vertexBuffers.dpad_u, "u" }
        , { &vertexBuffers.start, "start" }
        , { &vertexBuffers.select, "select" }
        , { &vertexBuffers.l1, "l1" }
        , { &vertexBuffers.r1, "r1" }
        , { &vertexBuffers.axis_l, "axis_l_mov" }
        , { &vertexBuffers.axis_r, "axis_r_mov" }
        , { &vertexBuffers.l2, "l2" }
        , { &vertexBuffers.r2, "r2" }
    };
    
    for (ShapeEntry& entry : shapes) {
        auto search = rawPaths.find(Hash::fnv(entry.name));
        if (search != rawPaths.end()) {
            parsenode_svg(*entry.buffer, search->second.c_str());
            max = Vec::max(max, entry.buffer->max);
            min = Vec::min(min, entry.buffer->min);
        }
    }
    
    // Center all nodes and recompute min-max
    const Vec2 center = Vec::scale(Vec::add(max, min), 0.5f);
    min.x = min.y = 10000.f;
    max.x = max.y = -10000.f;
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
        max = Vec::max(max, shapeMax);
        min = Vec::min(min, shapeMin);
    }
    
#if CONTROLLER_PRINT_LOADED_SVG
    const u32 shapeCount = sizeof(shapes) / sizeof(shapes[0]);
    printf("namespace %s_vertex {\n", name);
    for (ShapeEntry& entry : shapes) {
        printf("\tVec2 %s[] = {\n", entry.name);
        for (s32 i = 0; i< entry.buffer->count; i++) {
            Vec2& v = entry.buffer->vertex[i];
            printf("\t\t%s{ %f, %f }\n", (i > 0) ? ", " : "  ", v.x, v.y);
        }
        printf("\t};\n");
    }
    printf("}\n");
    
    printf("RenderBuffers %s_buffers = {\n", name);
    for (s32 i = 0; i < shapeCount; i++) {
        ShapeEntry& entry = shapes[i];
        printf("%s", (i > 0) ? "" : "\t{\n");
        printf("\t\t  %s_vertex::%s\n", name, entry.name);
        printf("\t\t, { %f, %f }, { %f, %f }\n", entry.buffer->max.x, entry.buffer->max.y, entry.buffer->min.x, entry.buffer->min.y);
        printf("\t\t, %d\n", entry.buffer->count);
        printf("\t}%s", (i < (shapeCount - 1)) ? ", {\n" : "\n");
    }
    printf("\t, { %f, %f }, { %f, %f }\n", max.x, max.y, min.x, min.y);
    printf("};\n");
#endif
}
    
}

#endif // CONTROLLER_LOAD_SVG_DATA

#endif // __WASTELADNS_CONTROLLER_RENDER_H__
