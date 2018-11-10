#ifndef __WASTELADNS_DEBUGDRAW_H__
#define __WASTELADNS_DEBUGDRAW_H__

#ifndef UNITYBUILD
#include "color.h"
#include "angle.h"
#include "transform.h"
#include <assert.h>
#endif

#ifdef __WASTELADNS_DEBUG_TEXT__
#include "../lib/stb/stb_easy_font.h"
#endif

namespace Renderer
{
namespace Immediate
{
    struct Vertex {
        u32 color;
        Vec3 pos;
    };
    
    struct Buffer {
        static constexpr u32 kMaxVertexCount = 1 << 14;
        Vertex vertexMemory[kMaxVertexCount];
        
        // 16 byte stride: equivalent to struct { Vec3 pos; u32 color };
        // z is ignored
        static constexpr u32 kMaxCharVertexCount = 1 << 14;
        u8 charVertexMemory[kMaxCharVertexCount];
        
        u32 vertexIndex;
		u32 charVertexIndex;
    };
    
    constexpr u8 kTextVertexSize = 2;
    struct TextParams {
        TextParams()
        : color(1.f, 1.f, 1.f, 1.f)
        , scale(1)
        {}
        
        Vec3 pos;
        const char* text;
        Col color;
        u8 scale;
    };
    
    // Platform dependent function
    void present(Buffer& buffer);
    
    void clear(Buffer& buffer) {
        buffer.vertexIndex = 0;
        buffer.charVertexIndex = 0;
    }
    
    void segment(Buffer& buffer, const Vec3& v1, const Vec3& v2, const Col color) {

        // Too many vertex pushed during immediate mode
        // Either bump Buffer::kMaxVertexCount or re-implement
        // complex primitives to avoid vertex usage
        assert(buffer.vertexIndex < Buffer::kMaxVertexCount);
        
        Vertex& vertexStart = buffer.vertexMemory[buffer.vertexIndex];
        Vertex& vertexEnd = buffer.vertexMemory[buffer.vertexIndex + 1];
        buffer.vertexIndex = buffer.vertexIndex + 2;
        
        vertexStart.pos = v1;
        vertexStart.color = color.ABGR();
        vertexEnd.pos = v2;
        vertexEnd.color = color.ABGR();
    }
    
    void openSegment(Buffer& buffer, const Vec3& start, const Vec3& dir, Col color) {
        const f32 segmentLength = 10000.f;
        const Vec3 end = Math::add(start, Math::scale(dir, segmentLength));
        segment(buffer, start, end, color);
    }
    
    void ray(Buffer& buffer, const Vec3& start, const Vec3& dir, Col color) {
        openSegment(buffer, start, dir, color);
    }
    
    void line(Buffer& buffer, const Vec3& pos, const Vec3& dir, Col color) {
        const f32 extents = 10000.f;
        const Vec3 start = Math::subtract(pos, Math::scale(dir, extents));
        const Vec3 end = Math::add(pos, Math::scale(dir, extents));
        segment(buffer, start, end, color);
    }
    
    void poly(Buffer& buffer, const Vec3* vertices, const u8 count, Col color) {
        for (u8 i = 0; i < count; i++) {
            const Vec3 prev = vertices[i];
            const Vec3 next = vertices[(i + 1) % count];
            segment(buffer, prev, next, color);
        }
    }
    
    void circle(Buffer& buffer, const Vec3& center, const Vec3& normal, const f32 radius, const Col color) {
        Transform33 m = Math::fromUp(normal);
        
        static constexpr u32 kVertexCount = 24;
        Vec3 vertices[kVertexCount];
        for (u8 i = 0; i < kVertexCount; i++) {
            
            const f32 angle = i * 2.f * Math::pi<f32> / (f32) kVertexCount;
            const Vec3 vertexDirLocal = Math::add(Math::scale(m.right, Math::cos(angle)), Math::scale(m.front, Math::sin(angle)));
            
            vertices[i] = Math::add(center, Math::scale(vertexDirLocal, radius));
        }
        poly(buffer, vertices, kVertexCount, color);
    }
    
    void sphere(Buffer& buffer, const Vec3& center, const f32 radius, const Col color) {
        
        static constexpr u32 kSectionCount = 12;
        const Col colorVariation(0.25f, 0.25f, 0.25f, 0.f);
        const Vec3 up = Math::upAxis();
        const Vec3 front = Math::frontAxis();
        const Vec3 right = Math::rightAxis();
        for (u8 i = 0; i < kSectionCount; i++) {
            
            const f32 angle = i * Math::pi<f32> / (f32) kSectionCount;
            const f32 s = Math::sin(angle);
            const f32 c = Math::cos(angle);
            
            const Vec3 rotatedFront = Math::add(Math::scale(right, c), Math::scale(front, s));
            circle(buffer, center, rotatedFront, radius, color);
            
            const f32 sectionRadius = radius * s;
            const f32 sectionOffset = radius * c;
            const Vec3 sectionPos = Math::add(center, Math::scale(up, sectionOffset));
            circle(buffer, sectionPos, up, sectionRadius, color);
        }
    }
    
#ifdef __WASTELADNS_DEBUG_TEXT__
    void text2d(Buffer& buffer, const TextParams& params) {
        
        // Too many chars pushed during immediate mode
        // Bump Buffer::kMaxCharVertexCount
        assert(buffer.charVertexIndex < Buffer::kMaxCharVertexCount);
        
        unsigned char color[4];
        color[0] = params.color.getRu();
        color[1] = params.color.getGu();
        color[2] = params.color.getBu();
        color[3] = params.color.getAu();
        u8* vertexBuffer = &buffer.charVertexMemory[buffer.charVertexIndex];
        u32 quadCount = stb_easy_font_print(params.pos.x, -params.pos.y, params.scale, const_cast<char*>(params.text), color, vertexBuffer, Buffer::kMaxCharVertexCount - buffer.charVertexIndex);
        buffer.charVertexIndex += quadCount * 4 * kTextVertexSize;
    }
    
    void text2d(Buffer& buffer, const Vec3& pos, const char* data) {
        TextParams params;
        params.pos = pos;
        params.text = data;
        params.scale = 1;
        text2d(buffer, params);
    }
#endif // __WASTELADNS_DEBUG_TEXT__
}
}

#if PLATFORM_GLFW
#include "glfw/renderer_debug.h"
#endif

#endif // __WASTELADNS_DEBUGDRAW_H__
