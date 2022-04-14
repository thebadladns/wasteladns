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
    constexpr u32 kMaxVertexCount = 1 << 12;
    constexpr u32 kMaxCharVertexCount = 1 << 16;
    // Chars vertex buffer stores quad: per 4 vertex quad, we store 6 indexes (2 tris) = 6 / 4 = 3 / 2
    constexpr u32 vertexSizeToIndexCount(const u32 count) { return 3 * count / (2 * sizeof(Layout_Vec2Color4B)); }
    
    struct Buffer {
        Layout_Vec3Color4B vertexMemory[kMaxVertexCount];
        
        u8 charVertexMemory[kMaxCharVertexCount];
        u32 charIndexVertexMemory[ vertexSizeToIndexCount(kMaxCharVertexCount) ];
        
        Driver::RscBuffer<Layout_Vec3Color4B> perspVertex;
        Driver::RscIndexedBuffer<Layout_Vec2Color4B> orthoVertex;
        Driver::RscShaderSet<Layout_Vec3Color4B, Layout_CBuffer_DebugScene::Buffers> shaderSetPersp;
        Driver::RscShaderSet<Layout_Vec2Color4B, Layout_CBuffer_DebugScene::Buffers> shaderSetOrtho;
        Driver::RscCBuffer cbuffers[Layout_CBuffer_DebugScene::Buffers::Count];
        Driver::RscRasterizerState orthoRasterizerState;
        
        u32 vertexIndex;
        u32 vertexSize;
        u32 charVertexIndex;

        u32 vertexBufferIndex;
        u32 layoutBufferIndex;
        u32 shader;
    };
    
    struct TextParams {
        TextParams()
        : color(1.f, 1.f, 1.f, 1.f)
        , scale(1)
        {}
        
        Vec3 pos;
        Col color;
        u8 scale;
    };
    
    void clear(Buffer& buffer) {
        buffer.vertexIndex = 0;
        buffer.vertexSize = 0;
        buffer.charVertexIndex = 0;
    }
    
    void segment(Buffer& buffer, const Vec3& v1, const Vec3& v2, const Col color) {

        // Too many vertex pushed during immediate mode
        // Either bump Buffer::kMaxVertexCount or re-implement
        // complex primitives to avoid vertex usage
        assert(buffer.vertexIndex < kMaxVertexCount);
        if (buffer.vertexIndex + 2 >= kMaxVertexCount) { buffer.vertexIndex = 0; }
        //if (buffer.vertexIndex+2 >= kMaxVertexCount) {

        //}
        
        Layout_Vec3Color4B& vertexStart = buffer.vertexMemory[buffer.vertexIndex];
        Layout_Vec3Color4B& vertexEnd = buffer.vertexMemory[buffer.vertexIndex + 1];
        buffer.vertexIndex = buffer.vertexIndex + 2;
        buffer.vertexSize = Math::min(buffer.vertexSize + 2, kMaxVertexCount);

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

    void box(Buffer& buffer, const Vec3 min, const Vec3 max, Col color) {

        Vec3 leftNearBottom(min.x, min.y, min.z);
        Vec3 leftFarBottom(min.x, max.y, min.z);
        Vec3 leftFarTop(min.x, max.y, max.z);
        Vec3 leftNearTop(min.x, min.y, max.z);
        Vec3 rightNearBottom(max.x, min.y, min.z);
        Vec3 rightFarBottom(max.x, max.y, min.z);
        Vec3 rightFarTop(max.x, max.y, max.z);
        Vec3 rightNearTop(max.x, min.y, max.z);
        
        // left plane
        segment(buffer, leftNearBottom, leftFarBottom, color);
        segment(buffer, leftFarBottom, leftFarTop, color);
        segment(buffer, leftFarTop, leftNearTop, color);
        segment(buffer, leftNearTop, leftNearBottom, color);

        // right plane
        segment(buffer, rightNearBottom, rightFarBottom, color);
        segment(buffer, rightFarBottom, rightFarTop, color);
        segment(buffer, rightFarTop, rightNearTop, color);
        segment(buffer, rightNearTop, rightNearBottom, color);

        // remaining top
        segment(buffer, leftNearTop, rightNearTop, color);
        segment(buffer, leftFarTop, rightFarTop, color);
        // remaining bottom
        segment(buffer, leftNearBottom, rightNearBottom, color);
        segment(buffer, leftFarBottom, rightFarBottom, color);
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
    void text2d(Buffer& buffer, const TextParams& params, const char* format, va_list argList) {
        
        char text[256];
        vsnprintf(text, 256, format, argList);

        // Too many chars pushed during immediate mode
        // Bump Buffer::kMaxCharVertexCount
        // stb prevents array overflow so this assert is likely to never hit
//        assert(buffer.charVertexIndex < kMaxCharVertexCount);
        
        u32 vertexCount = buffer.charVertexIndex / sizeof(Layout_Vec2Color4B);
        u32 indexCount = vertexSizeToIndexCount(buffer.charVertexIndex);
        
        unsigned char color[4];
        color[0] = params.color.getRu();
        color[1] = params.color.getGu();
        color[2] = params.color.getBu();
        color[3] = params.color.getAu();
        u8* vertexBuffer = &buffer.charVertexMemory[buffer.charVertexIndex];
        u32 quadCount = stb_easy_font_print(params.pos.x, -params.pos.y, params.scale, text, color, vertexBuffer, kMaxCharVertexCount - buffer.charVertexIndex);
        buffer.charVertexIndex += quadCount * 4 * sizeof(Layout_Vec2Color4B);

        for(u32 i = 0; i < quadCount; i++) {
            const u32 vertexIndex = vertexCount + i * 4;
            const u32 indexIndex = indexCount + i * 6;
            buffer.charIndexVertexMemory[indexIndex] = vertexIndex+3;
            buffer.charIndexVertexMemory[indexIndex+1] = vertexIndex+2;
            buffer.charIndexVertexMemory[indexIndex+2] = vertexIndex+1;

            buffer.charIndexVertexMemory[indexIndex+3] = vertexIndex+1;
            buffer.charIndexVertexMemory[indexIndex+4] = vertexIndex+0;
            buffer.charIndexVertexMemory[indexIndex+5] = vertexIndex+3;
        }
    }

    void text2d(Buffer& buffer, const TextParams& params, const char* format, ...) {
        va_list va;
        va_start(va, format);
        text2d(buffer, params, format, va);
        va_end(va);
    }
    
    void text2d(Buffer& buffer, const Vec3& pos, const char* format, ...) {
        TextParams params;
        params.pos = pos;
        params.scale = 1;

        va_list va;
        va_start(va, format);
        text2d(buffer, params, format, va);
        va_end(va);
    }
#endif // __WASTELADNS_DEBUG_TEXT__
    
    void load(Buffer& buffer) {
        
        Driver::create<Layout_CBuffer_DebugScene::GroupData>(buffer.cbuffers[Layout_CBuffer_DebugScene::Buffers::GroupData], {});
        
        // 3d
        {
            Renderer::Driver::BufferParams bufferParams;
            bufferParams.vertexData = nullptr;
            bufferParams.vertexSize = sizeof(Buffer::vertexMemory);
            bufferParams.memoryMode = Renderer::BufferMemoryMode::CPU;
            bufferParams.type = Renderer::BufferTopologyType::Lines;
            bufferParams.vertexCount = 0;
            Driver::create(buffer.perspVertex, bufferParams);
        }
        // 2d
        {
            Renderer::Driver::IndexedBufferParams bufferParams;
            bufferParams.vertexData = nullptr;
            bufferParams.indexData = nullptr;
            bufferParams.vertexSize = sizeof(Buffer::charVertexMemory);
            bufferParams.indexSize = sizeof(Buffer::charIndexVertexMemory);
            bufferParams.memoryMode = Renderer::BufferMemoryMode::CPU;
            bufferParams.indexType = Renderer::BufferItemType::U32;
            bufferParams.type = Renderer::BufferTopologyType::Triangles;
            bufferParams.indexCount = 0;
            Driver::create(buffer.orthoVertex, bufferParams);
        }
        
        {
            Renderer::Driver::RscVertexShader<Renderer::Layout_Vec3Color4B, Renderer::Layout_CBuffer_DebugScene::Buffers> rscVS;
            Renderer::Driver::RscPixelShader rscPS;
            Renderer::Driver::RscShaderSet<Renderer::Layout_Vec3Color4B, Renderer::Layout_CBuffer_DebugScene::Buffers> shaderSet;
            Renderer::Driver::ShaderResult result = Renderer::Driver::create(rscVS, { coloredVertexShaderStr, (u32)strlen(coloredVertexShaderStr) });
            if (result.compiled) {
                result = Renderer::Driver::create(rscPS, { defaultPixelShaderStr, (u32)strlen(defaultPixelShaderStr) });
                if (result.compiled) {
                    result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, buffer.cbuffers });
                    if (result.compiled) {
                        buffer.shaderSetPersp = shaderSet;
                    }
                    else {
                        Platform::printf("link: %s", result.error);
                    }
                }
                else {
                    Platform::printf("PS: %s", result.error);
                }
            }
            else {
                Platform::printf("VS: %s", result.error);
            }
        }
        {
            Renderer::Driver::RscVertexShader<Renderer::Layout_Vec2Color4B, Renderer::Layout_CBuffer_DebugScene::Buffers> rscVS;
            Renderer::Driver::RscPixelShader rscPS;
            Renderer::Driver::RscShaderSet<Renderer::Layout_Vec2Color4B, Renderer::Layout_CBuffer_DebugScene::Buffers> shaderSet;
            Renderer::Driver::ShaderResult result = Renderer::Driver::create(rscVS, { coloredVertexShaderStr, (u32)strlen(coloredVertexShaderStr) });
            if (result.compiled) {
                result = Renderer::Driver::create(rscPS, { defaultPixelShaderStr, (u32)strlen(defaultPixelShaderStr) });
                if (result.compiled) {
                    result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, buffer.cbuffers });
                    if (result.compiled) {
                        buffer.shaderSetOrtho = shaderSet;
                    }
                    else {
                        Platform::printf("link: %s", result.error);
                    }
                }
                else {
                    Platform::printf("PS: %s", result.error);
                }
            }
            else {
                Platform::printf("VS: %s", result.error);
            }
        }
        
        Renderer::Driver::create(buffer.orthoRasterizerState, { Renderer::RasterizerFillMode::Fill, true });
    }
    
    void present3d(Buffer& buffer, const Mat4& projMatrix, const Mat4& viewMatrix) {

        Driver::BufferUpdateParams bufferUpdateParams;
        bufferUpdateParams.vertexData = &buffer.vertexMemory;
        bufferUpdateParams.vertexSize = sizeof(Layout_Vec3Color4B) * buffer.vertexSize;
        bufferUpdateParams.vertexCount = buffer.vertexSize;
        Driver::update(buffer.perspVertex, bufferUpdateParams);

        Mat4 mvp = Math::mult(projMatrix, viewMatrix);
        Driver::update(buffer.cbuffers[Layout_CBuffer_DebugScene::Buffers::GroupData], mvp);

        Driver::bind(buffer.shaderSetPersp);
        Driver::bind(buffer.perspVertex);
        Driver::bind(buffer.cbuffers, Layout_CBuffer_DebugScene::Buffers::Count, { true, false });
        Driver::draw(buffer.perspVertex);
    }
    
    void present2d(Buffer& buffer, const Mat4& projMatrix) {
        
        Renderer::Driver::bind(buffer.orthoRasterizerState);

        u32 indexCount = vertexSizeToIndexCount(buffer.charVertexIndex);
        Driver::IndexedBufferUpdateParams bufferUpdateParams;
        bufferUpdateParams.vertexData = &buffer.charVertexMemory;
        bufferUpdateParams.vertexSize = buffer.charVertexIndex;
        bufferUpdateParams.indexData = &buffer.charIndexVertexMemory;
        bufferUpdateParams.indexSize = indexCount * sizeof(u32);
        bufferUpdateParams.indexCount = indexCount;
        Driver::update(buffer.orthoVertex, bufferUpdateParams);

        Driver::update(buffer.cbuffers[Layout_CBuffer_DebugScene::Buffers::GroupData], projMatrix);
        
        Driver::bind(buffer.shaderSetOrtho);
        Driver::bind(buffer.orthoVertex);
        Driver::bind(buffer.cbuffers, Layout_CBuffer_DebugScene::Buffers::Count, { true, false });
        Driver::draw(buffer.orthoVertex);
    }
    
}
}

#endif // __WASTELADNS_DEBUGDRAW_H__
