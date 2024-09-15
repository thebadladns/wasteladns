#include "MathPrimitives.h"

namespace GrcPrimitives {
    namespace Private
    {
        const u8 kDebugCircleVertexCount = 98;
        const u8 kDebugSphereSectionCount = 98;
    }
}

void GrcPrimitives::segment(const Vec3& vStart, const Vec3& vEnd, Col color) {
    glBegin(GL_LINES);
    glColor4f(RGBA_PARAMS(color));
    glVertex3f(vStart.x, vStart.y, vStart.z);
    glVertex3f(vEnd.x, vEnd.y, vEnd.z);
    glEnd();
}

void GrcPrimitives::ray(const Vec3& vStart, const Vec3& vDir, Col color) {
    openSegment(vStart, vDir, color);
}

void GrcPrimitives::openSegment(const Vec3& vStart, const Vec3& vDir, Col color) {
    const f32 fLargeValue = 10000.f;
    const Vec3 vEnd = vStart + fLargeValue * vDir;
    segment(vStart, vEnd, color);
}

void GrcPrimitives::line(const Vec3& vPos, const Vec3& vDir, Col color) {
    const f32 fLargeValue = 10000.f;
    const Vec3 vStart = vPos - fLargeValue * vDir;
    const Vec3 vEnd = vPos + fLargeValue * vDir;
    segment(vStart, vEnd, color);
}

void GrcPrimitives::poly(const Vec3* aPos, const u8 count, Col color) {
    for (s8 i = 0; i < count; i++) {
        const Vec3 vPrev = aPos[i];
        const Vec3 vNext = (i < count - 1) ? aPos[i + 1] : aPos[0];
        segment(vPrev, vNext, color);
    }
}

void GrcPrimitives::circle(const Vec3& vPos, const Vec3 vAxis, const f32 radius, const Col color) {

    const Vec3 zero(0.f, 0.f, 0.f);
    const Vec3 expectedForward = Vec3(0.f, 0.f, -1.f);
    const Vec3 up = normalize(vAxis);
    Vec3 right = normalize(crossProduct(up, expectedForward));
    if (right == zero)
    {
        right = Vec3(1.f, 0.f, 0.f);
    }
    const Vec3 forward = normalize(crossProduct(up, right));
    
    Vec3 aPos[Private::kDebugCircleVertexCount];
    for (u8 i = 0; i < Private::kDebugCircleVertexCount; i++) {
        
        const f32 angle = i * 2.f * M_PI / (f32) Private::kDebugCircleVertexCount;
        const Vec3 normalizedOffset = ((f32) cos(angle)) * right + ((f32) sin(angle)) * forward;
        
        aPos[i] = vPos + radius * normalizedOffset;
    }
    
    poly(aPos, Private::kDebugCircleVertexCount, color);
}

void GrcPrimitives::sphere(const Vec3& vPos, const f32 radius, const Col color) {
    
    const Col colorVariation(0.25f, 0.25f, 0.25f, 0.f);
    const Vec3 sectionXAxis(1.f, 0.f, 0.f);
    const Vec3 sectionYAxis(0.f, 1.f, 0.f);
    for (u8 i = 0; i < Private::kDebugSphereSectionCount; i++) {

        const f32 angle = i * M_PI / (f32) Private::kDebugSphereSectionCount;
        const f32 sinAngle = sin(angle);
        const f32 cosAngle = cos(angle);
        const f32 sectionRadius = radius * sinAngle;
        const f32 sectionOffset = radius * cosAngle;
        const Vec3 sectionXPos = vPos + sectionOffset * sectionXAxis;
        const Vec3 sectionYPos = vPos + sectionOffset * sectionYAxis;
        
        const Col colorOffset = (1.f - sinAngle) * colorVariation;
        const Col sectionColor = color + colorOffset;
        
        circle(sectionXPos, sectionXAxis, sectionRadius, sectionColor);
        circle(sectionYPos, sectionYAxis, sectionRadius, sectionColor);
    }
}

void GrcPrimitives::matrix(const Mat44& matrix) {
    
    const Col axisX(0.8f, 0.15f, 0.25f, 0.7f);
    const Col axisY(0.25f, 0.8f, 0.15f, 0.7f);
    const Col axisZ(0.15f, 0.25f, 0.8f, 0.7f);
    
    const Vec3 camPos = matrix.getCol(3).xyz();
    const Vec3 camForward = matrix.getCol(2).xyz();
    const Vec3 camRight = matrix.getCol(0).xyz();
    const Vec3 camUp = matrix.getCol(1).xyz();
    GrcPrimitives::segment(camPos, camPos + camRight, axisX);
    GrcPrimitives::segment(camPos, camPos + camUp, axisY);
    GrcPrimitives::segment(camPos, camPos + camForward, axisZ);
}

#ifdef GRC_PRIMITIVES_ENABLE_TEXT

void GrcPrimitives::text(const Vec3& pos, const char* data) {
    TextParams params;
    params.pos = pos;
    params.text = data;
    params.scale = 1.f;
    text(params);
}
void GrcPrimitives::text(const GrcPrimitives::TextParams& params) {
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

#endif

void GrcPrimitives::graph(const GrcPrimitives::GraphParams& params) {
    
    Col axisColor(1.f, 1.f, 1.f, 0.75f);
    
    Vec3 vAxis(0.f, 1.f, 0.f);
    Vec3 hAxis(1.f, 0.f, 0.f);
    
    if (params.count > 0) {
        
        f32 max = params.max;
        f32 min = params.min;
        if (min == max) {
            min = std::numeric_limits<float>::max();
            max = std::numeric_limits<float>::lowest();
            for (int i = 0; i < params.count; i++) {
                min = fminf(params.values[i], min);
                max = fmaxf(params.values[i], max);
            }
        }
        
        Vec3 prevNode;
        f32 stepW = params.w / params.count;
        f32 dataWidth = max - min;
        for (int i = 0; i < params.count; i++) {
            
            f32 valueLS = fabsf(dataWidth) > 0.f ? params.h * (params.values[i] - min) / dataWidth : max;
            Vec3 currNode = params.topLeft + hAxis * stepW * i - vAxis * (params.h - valueLS);
            if (i > 0) {
                segment(prevNode, currNode, params.color);
            }
            prevNode = currNode;
        }
    }
    
    // Axis
    segment(params.topLeft, params.topLeft - vAxis * params.h, axisColor);
}
