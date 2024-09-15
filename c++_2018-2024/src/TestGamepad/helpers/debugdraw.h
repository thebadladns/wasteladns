#ifndef __WASTELADNS_DEBUGDRAW_H__
#define __WASTELADNS_DEBUGDRAW_H__

#ifndef __WASTELADNS_GLFW3_H__
#include "GLFW/glfw3.h"
#define __WASTELADNS_GLFW3_H__
#endif

#ifndef __WASTELADNS_TYPES_H__
#include "types.h"
#endif

#ifndef __WASTELADNS_VEC_H__
#include "vec.h"
#endif

#ifndef __WASTELADNS_COLOR_H__
#include "color.h"
#endif

namespace DebugDraw
{
    void segment(const Vec3& vStart, const Vec3& vEnd, const Col color);
    void ray(const Vec3& vStart, const Vec3& vDir, Col color);
    void openSegment(const Vec3& vStart, const Vec3& vDir, const Col color);
    void line(const Vec3& vPos, const Vec3& vDir, const Col color);
    
    void poly(const Vec3* aPos, const u8 count, Col color);
    void circle(const Vec3& vPos, const Vec3& vAxis, const f32 radius, const Col color);
    void sphere(const Vec3& vPos, const f32 radius, const Col color);
    
#ifdef __WASTELADNS_DEBUGDRAW_TEXT__
    struct TextParams {
        TextParams();
        
        Vec3 pos;
        const char* text;
        f32 scale;
        Col color;
    };
    void text(const Vec3& pos, const char* data);
    void text(const TextParams& params);
#endif // __WASTELADNS_DEBUGDRAW_TEXT__
    
    struct GraphParams {
        GraphParams();
        
        Vec3 topLeft;
        f32* values;
        f32 min, max;
        f32 w, h;
        u32 count;
        Col color;
    };
    void graph(const GraphParams& params);
};

#endif // __WASTELADNS_DEBUGDRAW_H__

#ifdef __WASTELADNS_DEBUGDRAW_IMPL__

#ifndef __WASTELADNS_MATH_H__
#include "Math.h"
#endif

#ifndef __WASTELADNS_ANGLE_H__
#include "angle.h"
#endif

#ifdef __WASTELADNS_DEBUGDRAW_TEXT__
#include "../lib/stb/stb_easy_font.h"
#endif

namespace DebugDraw {
    namespace Private
    {
        const u8 kDebugCircleVertexCount = 98;
        const u8 kDebugSphereSectionCount = 98;
    }
}

void DebugDraw::segment(const Vec3& vStart, const Vec3& vEnd, Col color) {
    glBegin(GL_LINES);
    glColor4f(RGBA_PARAMS(color));
    glVertex3f(vStart.x, vStart.y, vStart.z);
    glVertex3f(vEnd.x, vEnd.y, vEnd.z);
    glEnd();
}

void DebugDraw::ray(const Vec3& vStart, const Vec3& vDir, Col color) {
    openSegment(vStart, vDir, color);
}

void DebugDraw::openSegment(const Vec3& vStart, const Vec3& vDir, Col color) {
    const f32 fLargeValue = 10000.f;
    const Vec3 vEnd = Vec::add(vStart, Vec::scale(vDir, fLargeValue));
    segment(vStart, vEnd, color);
}

void DebugDraw::line(const Vec3& vPos, const Vec3& vDir, Col color) {
    const f32 fLargeValue = 10000.f;
    const Vec3 vStart = Vec::subtract(vPos, Vec::scale(vDir, fLargeValue));
    const Vec3 vEnd = Vec::add(vPos, Vec::scale(vDir, fLargeValue));
    segment(vStart, vEnd, color);
}

void DebugDraw::poly(const Vec3* aPos, const u8 count, Col color) {
    for (s8 i = 0; i < count; i++) {
        const Vec3 vPrev = aPos[i];
        const Vec3 vNext = (i < count - 1) ? aPos[i + 1] : aPos[0];
        segment(vPrev, vNext, color);
    }
}

void DebugDraw::circle(const Vec3& vPos, const Vec3& vAxis, const f32 radius, const Col color) {
    
    const Vec3 expectedForward = Vec3(0.f, 0.f, -1.f);
    const Vec3 up = Vec::normalize(vAxis);
    Vec3 right = Vec::cross(up, expectedForward);
    if (!Vec::normalizeSafe(right))
    {
        right = Vec3(1.f, 0.f, 0.f);
    }
    const Vec3 forward = Vec::normalize(Vec::cross(up, right));
    
    Vec3 aPos[Private::kDebugCircleVertexCount];
    for (u8 i = 0; i < Private::kDebugCircleVertexCount; i++) {
        
        const f32 angle = i * 2.f * Angle::pi<f32> / (f32) Private::kDebugCircleVertexCount;
        const Vec3 normalizedOffset = Vec::add(Vec::scale(right, Angle::cos(angle)), Vec::scale(forward, Angle::sin(angle)));
        
        aPos[i] = Vec::add(vPos, Vec::scale(normalizedOffset, radius));
    }
    
    poly(aPos, Private::kDebugCircleVertexCount, color);
}

void DebugDraw::sphere(const Vec3& vPos, const f32 radius, const Col color) {
    
    const Col colorVariation(0.25f, 0.25f, 0.25f, 0.f);
    const Vec3 sectionXAxis(1.f, 0.f, 0.f);
    const Vec3 sectionYAxis(0.f, 1.f, 0.f);
    for (u8 i = 0; i < Private::kDebugSphereSectionCount; i++) {
        
        const f32 angle = i * Angle::pi<f32> / (f32) Private::kDebugSphereSectionCount;
        const f32 sinAngle = sin(angle);
        const f32 cosAngle = cos(angle);
        const f32 sectionRadius = radius * sinAngle;
        const f32 sectionOffset = radius * cosAngle;
        const Vec3 sectionXPos = Vec::add(vPos, Vec::scale(sectionXAxis, sectionOffset));
        const Vec3 sectionYPos = Vec::add(vPos, Vec::scale(sectionYAxis, sectionOffset));
        
        const Col colorOffset = Col::scale(colorVariation, (1.f - sinAngle));
        const Col sectionColor = Col::add(color, colorOffset);
        
        circle(sectionXPos, sectionXAxis, sectionRadius, sectionColor);
        circle(sectionYPos, sectionYAxis, sectionRadius, sectionColor);
    }
}

#ifdef __WASTELADNS_DEBUGDRAW_TEXT__

DebugDraw::TextParams::TextParams()
: scale(1.f)
, color(1.f, 1.f, 1.f, 1.f)
{}

void DebugDraw::text(const Vec3& pos, const char* data) {
    TextParams params;
    params.pos = pos;
    params.text = data;
    params.scale = 1.f;
    text(params);
}
void DebugDraw::text(const DebugDraw::TextParams& params) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        glColor4f(RGBA_PARAMS(params.color));
        
        // Reverse stb_easy_font y axis
        glScalef(params.scale, -params.scale, 1.f);
        static char buffer[99999]; // ~500 chars
        s32 num_quads;
        
        num_quads = stb_easy_font_print(params.pos.x / params.scale, -params.pos.y / params.scale, const_cast<char*>(params.text), NULL, buffer, sizeof(buffer));
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 16, buffer);
        glDrawArrays(GL_QUADS, 0, num_quads*4);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    glPopMatrix();
}

#endif // __WASTELADNS_DEBUGDRAW_TEXT__

DebugDraw::GraphParams::GraphParams()
: w(300.f)
, h(60.f)
, count(0)
, color(1.f, 1.f, 1.f, 1.f)
{}

void DebugDraw::graph(const DebugDraw::GraphParams& params) {
    
    Col axisColor(1.f, 1.f, 1.f, 0.75f);
    
    Vec3 vAxis(0.f, 1.f, 0.f);
    Vec3 hAxis(1.f, 0.f, 0.f);
    
    if (params.count > 0) {
        
        f32 max = params.max;
        f32 min = params.min;
        if (min == max) {
            min = std::numeric_limits<float>::max();
            max = std::numeric_limits<float>::lowest();
            for (u32 i = 0; i < params.count; i++) {
                min = fminf(params.values[i], min);
                max = fmaxf(params.values[i], max);
            }
        }
        
        Vec3 prevNode;
        f32 stepW = params.w / params.count;
        f32 dataWidth = max - min;
        for (u32 i = 0; i < params.count; i++) {
            
            f32 valueLS = fabsf(dataWidth) > 0.f ? params.h * (params.values[i] - min) / dataWidth : max;
            Vec3 currNode = Vec::add(Vec::add(params.topLeft, Vec::scale(hAxis, stepW * i)), Vec::negate(Vec::scale(vAxis, (params.h - valueLS))));
            if (i > 0) {
                segment(prevNode, currNode, params.color);
            }
            prevNode = currNode;
        }
    }
    
    // Axis
    segment(params.topLeft, Vec::subtract(params.topLeft, Vec::scale(vAxis, params.h)), axisColor);
}

#endif // __WASTELADNS_DEBUGDRAW_IMPL__
