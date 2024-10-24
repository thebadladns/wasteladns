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
    struct Vertex2D {
        float2 pos;
        u32 color;
    };
    struct Vertex3D {
        float3 pos;
        u32 color;
    };

    const u32 max_3d_vertices = 1 << 14;
    const u32 max_2d_vertices = 1 << 14;
    // 2d vertices are stored in quads: per 4 vertex quad, we store 6 indexes (2 tris) = 6 / 4 = 3 / 2
    inline u32 vertexSizeToIndexCount(const u32 count) { return 3 * count / 2; }
    const size_t arena_size =
          max_3d_vertices * sizeof(Vertex3D)
        + max_2d_vertices * sizeof(Vertex2D)
        + vertexSizeToIndexCount(max_2d_vertices) * sizeof(u32);
    
    struct Buffer {

        Vertex3D* vertices_3d;
        Vertex2D* vertices_2d;
        u32* indices_2d;
        
        Driver::RscVertexBuffer buffer_3d;
        Driver::RscIndexedVertexBuffer buffer_2d;
        Driver::RscShaderSet shader_3d;
        Driver::RscShaderSet shader_2d;
        Driver::RscCBuffer cbuffer;
        Driver::RscRasterizerState rasterizerState;
        Driver::RscDepthStencilState orthoDepthState, perspDepthState;

        u32 vertices_3d_head;
        u32 vertices_2d_head;

        u32 vertexBufferIndex;
        u32 layoutBufferIndex;
        u32 shader;
    };
    
    struct TextParams {
        TextParams()
        : color(1.f, 1.f, 1.f, 1.f)
        , scale(1)
        {}
        
        float3 pos;
        Color32 color;
        u8 scale;
    };
    
    void clear3d(Buffer& buffer) {
        buffer.vertices_3d_head = 0;
    }
    void clear2d(Buffer& buffer) {
        buffer.vertices_2d_head = 0;
    }

    void segment(Buffer& buffer, const float3& v1, const float3& v2, const Color32 color) {

        // Too many vertex pushed during immediate mode
        // Either bump Buffer::vertices_3d_head or re-implement
        // complex primitives to avoid vertex usage
        assert(buffer.vertices_3d_head + 2 < max_3d_vertices);
        
        Vertex3D& vertexStart = buffer.vertices_3d[buffer.vertices_3d_head];
        Vertex3D& vertexEnd = buffer.vertices_3d[buffer.vertices_3d_head + 1];
        buffer.vertices_3d_head = buffer.vertices_3d_head + 2;

        vertexStart.pos = v1;
        vertexStart.color = color.ABGR();
        vertexEnd.pos = v2;
        vertexEnd.color = color.ABGR();
    }
    void openSegment(Buffer& buffer, const float3& start, const float3& dir, Color32 color) {
        const f32 segmentLength = 10000.f;
        const float3 end = Math::add(start, Math::scale(dir, segmentLength));
        segment(buffer, start, end, color);
    }
    void ray(Buffer& buffer, const float3& start, const float3& dir, Color32 color) {
        openSegment(buffer, start, dir, color);
    }
    void line(Buffer& buffer, const float3& pos, const float3& dir, Color32 color) {
        const f32 extents = 10000.f;
        const float3 start = Math::subtract(pos, Math::scale(dir, extents));
        const float3 end = Math::add(pos, Math::scale(dir, extents));
        segment(buffer, start, end, color);
    }
    void poly(Buffer& buffer, const float3* vertices, const u8 count, Color32 color) {
        for (u8 i = 0; i < count; i++) {
            const float3 prev = vertices[i];
            const float3 next = vertices[(i + 1) % count];
            segment(buffer, prev, next, color);
        }
    }
    void aabb(Buffer& buffer, const float3 min, const float3 max, Color32 color) {

        float3 leftNearBottom(min.x, min.y, min.z);
        float3 leftFarBottom(min.x, max.y, min.z);
        float3 leftFarTop(min.x, max.y, max.z);
        float3 leftNearTop(min.x, min.y, max.z);
        float3 rightNearBottom(max.x, min.y, min.z);
        float3 rightFarBottom(max.x, max.y, min.z);
        float3 rightFarTop(max.x, max.y, max.z);
        float3 rightNearTop(max.x, min.y, max.z);
        
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
    void obb(Buffer& buffer, const float4x4& mat, const float3 aabb_min, const float3 aabb_max, Color32 color) {

        float3 leftNearBottom(aabb_min.x, aabb_min.y, aabb_min.z);
        float3 leftFarBottom(aabb_min.x, aabb_max.y, aabb_min.z);
        float3 leftFarTop(aabb_min.x, aabb_max.y, aabb_max.z);
        float3 leftNearTop(aabb_min.x, aabb_min.y, aabb_max.z);
        float3 rightNearBottom(aabb_max.x, aabb_min.y, aabb_min.z);
        float3 rightFarBottom(aabb_max.x, aabb_max.y, aabb_min.z);
        float3 rightFarTop(aabb_max.x, aabb_max.y, aabb_max.z);
        float3 rightNearTop(aabb_max.x, aabb_min.y, aabb_max.z);

        float4 leftNearBottom_WS = Math::mult(mat, float4(leftNearBottom, 1.f));
        float4 leftFarBottom_WS = Math::mult(mat, float4(leftFarBottom, 1.f));
        float4 leftFarTop_WS = Math::mult(mat, float4(leftFarTop, 1.f));
        float4 leftNearTop_WS = Math::mult(mat, float4(leftNearTop, 1.f));
        float4 rightNearBottom_WS = Math::mult(mat, float4(rightNearBottom, 1.f));
        float4 rightFarBottom_WS = Math::mult(mat, float4(rightFarBottom, 1.f));
        float4 rightFarTop_WS = Math::mult(mat, float4(rightFarTop, 1.f));
        float4 rightNearTop_WS = Math::mult(mat, float4(rightNearTop, 1.f));
        float3 leftNearBottom_NDC = Math::invScale(leftNearBottom_WS.xyz, leftNearBottom_WS.w);
        float3 leftFarBottom_NDC = Math::invScale(leftFarBottom_WS.xyz, leftFarBottom_WS.w);
        float3 leftFarTop_NDC = Math::invScale(leftFarTop_WS.xyz, leftFarTop_WS.w);
        float3 leftNearTop_NDC = Math::invScale(leftNearTop_WS.xyz, leftNearTop_WS.w);
        float3 rightNearBottom_NDC = Math::invScale(rightNearBottom_WS.xyz, rightNearBottom_WS.w);
        float3 rightFarBottom_NDC = Math::invScale(rightFarBottom_WS.xyz, rightFarBottom_WS.w);
        float3 rightFarTop_NDC = Math::invScale(rightFarTop_WS.xyz, rightFarTop_WS.w);
        float3 rightNearTop_NDC = Math::invScale(rightNearTop_WS.xyz, rightNearTop_WS.w);

        // left plane
        segment(buffer, leftNearBottom_NDC, leftFarBottom_NDC, color);
        segment(buffer, leftFarBottom_NDC, leftFarTop_NDC, color);
        segment(buffer, leftFarTop_NDC, leftNearTop_NDC, color);
        segment(buffer, leftNearTop_NDC, leftNearBottom_NDC, color);

        // right plane
        segment(buffer, rightNearBottom_NDC, rightFarBottom_NDC, color);
        segment(buffer, rightFarBottom_NDC, rightFarTop_NDC, color);
        segment(buffer, rightFarTop_NDC, rightNearTop_NDC, color);
        segment(buffer, rightNearTop_NDC, rightNearBottom_NDC, color);

        // remaining top
        segment(buffer, leftNearTop_NDC, rightNearTop_NDC, color);
        segment(buffer, leftFarTop_NDC, rightFarTop_NDC, color);
        // remaining bottom
        segment(buffer, leftNearBottom_NDC, rightNearBottom_NDC, color);
        segment(buffer, leftFarBottom_NDC, rightFarBottom_NDC, color);
    }
    void circle(Buffer& buffer, const float3& center, const float3& normal, const f32 radius, const Color32 color) {
        Transform33 m = Math::fromUp(normal);
        
        static constexpr u32 kVertexCount = 8;
        float3 vertices[kVertexCount];
        for (u8 i = 0; i < kVertexCount; i++) {
            
            const f32 angle = i * 2.f * Math::pi_f / (f32) kVertexCount;
            const float3 vertexDirLocal = Math::add(Math::scale(m.right, Math::cos(angle)), Math::scale(m.front, Math::sin(angle)));
            
            vertices[i] = Math::add(center, Math::scale(vertexDirLocal, radius));
        }
        poly(buffer, vertices, kVertexCount, color);
    }
    void sphere(Buffer& buffer, const float3& center, const f32 radius, const Color32 color) {
        
        static constexpr u32 kSectionCount = 2;
        const Color32 colorVariation(0.25f, 0.25f, 0.25f, 0.f);
        const float3 up = Math::upAxis();
        const float3 front = Math::frontAxis();
        const float3 right = Math::rightAxis();
        for (u8 i = 0; i < kSectionCount; i++) {
            
            const f32 angle = i * Math::pi_f / (f32) kSectionCount;
            const f32 s = Math::sin(angle);
            const f32 c = Math::cos(angle);
            
            const float3 rotatedFront = Math::add(Math::scale(right, c), Math::scale(front, s));
            circle(buffer, center, rotatedFront, radius, color);
            
            const f32 sectionRadius = radius * s;
            const f32 sectionOffset = radius * c;
            const float3 sectionPos = Math::add(center, Math::scale(up, sectionOffset));
            circle(buffer, sectionPos, up, sectionRadius, color);
        }
    }
    
    void box_2d(Buffer& buffer, const float2 min, const float2 max, Color32 color) {

        u32 vertexStart = buffer.vertices_2d_head;
        Vertex2D& bottomLeft = buffer.vertices_2d[vertexStart];
        bottomLeft.pos = { min.x, max.y };
        Vertex2D& topLeft = buffer.vertices_2d[vertexStart + 1];
        topLeft.pos = { min.x, min.y };
        Vertex2D& topRight = buffer.vertices_2d[vertexStart + 2];
        topRight.pos = { max.x, min.y };
        Vertex2D& bottomRigth = buffer.vertices_2d[vertexStart + 3];
        bottomRigth.pos = { max.x, max.y };
        
        u32 indexStart = vertexSizeToIndexCount(vertexStart);
        buffer.indices_2d[indexStart + 0] = vertexStart + 2;
        buffer.indices_2d[indexStart + 1] = vertexStart + 1;
        buffer.indices_2d[indexStart + 2] = vertexStart + 0;
        buffer.indices_2d[indexStart + 3] = vertexStart + 0;
        buffer.indices_2d[indexStart + 4] = vertexStart + 3;
        buffer.indices_2d[indexStart + 5] = vertexStart + 2;
        
        buffer.vertices_2d_head += 4;
        
        bottomLeft.color = color.ABGR();
        topLeft.color = color.ABGR();
        topRight.color = color.ABGR();
        bottomRigth.color = color.ABGR();
    }
#ifdef __WASTELADNS_DEBUG_TEXT__
    void text2d(Buffer& buffer, const TextParams& params, const char* format, va_list argList) {
        
        char text[256];
        Platform::format_va(text, sizeof(text), format, argList);
        
        u32 vertexCount = buffer.vertices_2d_head;
        u32 indexCount = vertexSizeToIndexCount(buffer.vertices_2d_head);
        
        unsigned char color[4];
        color[0] = params.color.getRu();
        color[1] = params.color.getGu();
        color[2] = params.color.getBu();
        color[3] = params.color.getAu();
        u8* vertexBuffer = (u8*) &buffer.vertices_2d[buffer.vertices_2d_head];
        s32 vertexBufferSize = (max_2d_vertices - buffer.vertices_2d_head) * sizeof(Vertex2D);
        // negate y, since our (0,0) is the center of the screen, stb's is bottom left
        u32 quadCount = stb_easy_font_print(params.pos.x, -params.pos.y, params.scale, text, color, vertexBuffer, vertexBufferSize);
        buffer.vertices_2d_head += quadCount * 4;

        assert(buffer.vertices_2d_head < max_2d_vertices); // increase max_2d_vertices

        // stb uses ccw winding, but we are negating the y, so it matches our cw winding
        for(u32 i = 0; i < quadCount; i++) {
            const u32 vertexIndex = vertexCount + i * 4;
            const u32 indexIndex = indexCount + i * 6;
            buffer.indices_2d[indexIndex] = vertexIndex + 1;
            buffer.indices_2d[indexIndex+1] = vertexIndex + 2;
            buffer.indices_2d[indexIndex+2] = vertexIndex + 3;

            buffer.indices_2d[indexIndex+3] = vertexIndex + 3;
            buffer.indices_2d[indexIndex+4] = vertexIndex + 0;
            buffer.indices_2d[indexIndex+5] = vertexIndex + 1;
        }
    }

    void text2d(Buffer& buffer, const TextParams& params, const char* format, ...) {
        va_list va;
        va_start(va, format);
        text2d(buffer, params, format, va);
        va_end(va);
    }
    
    void text2d(Buffer& buffer, const float3& pos, const char* format, ...) {
        TextParams params;
        params.pos = pos;
        params.scale = 1;

        va_list va;
        va_start(va, format);
        text2d(buffer, params, format, va);
        va_end(va);
    }
#endif // __WASTELADNS_DEBUG_TEXT__
    
    void init(Buffer& buffer, Allocator::Arena& arena) {

        buffer = {};

        {
            // reserve memory for buffers
            u32 vertices_3d_size = max_3d_vertices * sizeof(Vertex3D);
            u32 vertices_2d_size = max_2d_vertices * sizeof(Vertex2D);
            u32 indices_2d_size = vertexSizeToIndexCount(max_2d_vertices) * sizeof(u32);
            buffer.vertices_3d = (Vertex3D*)Allocator::alloc_arena(arena, vertices_3d_size, alignof(Vertex3D));
            buffer.vertices_2d = (Vertex2D*)Allocator::alloc_arena(arena, vertices_2d_size, alignof(Vertex2D));
            buffer.indices_2d = (u32*)Allocator::alloc_arena(arena, indices_2d_size, alignof(u32));

            Driver::create_cbuffer(buffer.cbuffer, { sizeof(float4x4), 4 });
            const Renderer::Driver::CBufferBindingDesc bufferBindings_MVP[] = {{ "type_PerGroup", Driver::CBufferStageMask::VS }};

            // 2d
            {
                const Driver::VertexAttribDesc attribs_color2d[] = {
                    Driver::make_vertexAttribDesc("POSITION", OFFSET_OF(Vertex2D, pos), sizeof(Vertex2D), Driver::BufferAttributeFormat::R32G32_FLOAT),
                    Driver::make_vertexAttribDesc("COLOR", OFFSET_OF(Vertex2D, color), sizeof(Vertex2D), Driver::BufferAttributeFormat::R8G8B8A8_UNORM)
                };
                Renderer::ShaderDesc desc = {};
                desc.vertexAttrs = attribs_color2d;
                desc.vertexAttr_count = COUNT_OF(attribs_color2d);
                desc.textureBindings = nullptr;
                desc.textureBinding_count = 0;
                desc.bufferBindings = bufferBindings_MVP;
                desc.bufferBinding_count = COUNT_OF(bufferBindings_MVP);
                // reuse 3d shaders
                desc.vs_name = Shaders::vs_color3d_unlit.name;
                desc.vs_src = Shaders::vs_color3d_unlit.src;
                desc.ps_name = Shaders::ps_color3d_unlit.name;
                desc.ps_src = Shaders::ps_color3d_unlit.src;
                Renderer::compile_shader(buffer.shader_2d, desc);

                Renderer::Driver::IndexedVertexBufferDesc bufferParams;
                bufferParams.vertexData = nullptr;
                bufferParams.indexData = nullptr;
                bufferParams.vertexSize = vertices_2d_size;
                bufferParams.vertexCount = max_2d_vertices;
                bufferParams.indexSize = indices_2d_size;
                bufferParams.memoryUsage = Renderer::Driver::BufferMemoryUsage::CPU;
                bufferParams.accessType = Renderer::Driver::BufferAccessType::CPU;
                bufferParams.indexType = Renderer::Driver::BufferItemType::U32;
                bufferParams.type = Renderer::Driver::BufferTopologyType::Triangles;
                bufferParams.indexCount = 0;
                Driver::create_indexed_vertex_buffer(buffer.buffer_2d, bufferParams, attribs_color2d, COUNT_OF(attribs_color2d));
            }
            // 3d
            {
                const Driver::VertexAttribDesc attribs_color3d[] = {
                    Driver::make_vertexAttribDesc("POSITION", OFFSET_OF(Vertex3D, pos), sizeof(Vertex3D), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
                    Driver::make_vertexAttribDesc("COLOR", OFFSET_OF(Vertex3D, color), sizeof(Vertex3D), Driver::BufferAttributeFormat::R8G8B8A8_UNORM)
                };
                Renderer::ShaderDesc desc = {};
                desc.vertexAttrs = attribs_color3d;
                desc.vertexAttr_count = COUNT_OF(attribs_color3d);
                desc.textureBindings = nullptr;
                desc.textureBinding_count = 0;
                desc.bufferBindings = bufferBindings_MVP;
                desc.bufferBinding_count = COUNT_OF(bufferBindings_MVP);
                desc.vs_name = Shaders::vs_color3d_unlit.name;
                desc.vs_src = Shaders::vs_color3d_unlit.src;
                desc.ps_name = Shaders::ps_color3d_unlit.name;
                desc.ps_src = Shaders::ps_color3d_unlit.src;
                Renderer::compile_shader(buffer.shader_3d, desc);

                Renderer::Driver::VertexBufferDesc bufferParams;
                bufferParams.vertexData = nullptr;
                bufferParams.vertexSize = vertices_3d_size;
                bufferParams.vertexCount = max_3d_vertices;
                bufferParams.memoryUsage = Renderer::Driver::BufferMemoryUsage::CPU;
                bufferParams.accessType = Renderer::Driver::BufferAccessType::CPU;
                bufferParams.type = Renderer::Driver::BufferTopologyType::Lines;
                Driver::create_vertex_buffer(buffer.buffer_3d, bufferParams, attribs_color3d, COUNT_OF(attribs_color3d));
            }
        }
        
        Renderer::Driver::create_RS(buffer.rasterizerState, { Renderer::Driver::RasterizerFillMode::Fill, Renderer::Driver::RasterizerCullMode::CullBack });
        {
            Renderer::Driver::DepthStencilStateParams dsParams;
            dsParams = {};
            dsParams.depth_enable = false;
            Renderer::Driver::create_DS(buffer.orthoDepthState, dsParams);
            dsParams = {};
            dsParams.depth_enable = true;
            dsParams.depth_func = Renderer::Driver::CompFunc::Less;
            dsParams.depth_writemask = Renderer::Driver::DepthWriteMask::All;
            Renderer::Driver::create_DS(buffer.perspDepthState, dsParams);
        }
    }
    
    void present3d(Buffer& buffer, const float4x4& projMatrix, const float4x4& viewMatrix) {
        
        Driver::Marker_t marker;
        Driver::set_marker_name(marker, "DEBUG 3D");
        Driver::start_event(marker);
        {
            Renderer::Driver::bind_RS(buffer.rasterizerState);
            Renderer::Driver::bind_DS(buffer.perspDepthState);

            Driver::BufferUpdateParams bufferUpdateParams;
            bufferUpdateParams.vertexData = buffer.vertices_3d;
            bufferUpdateParams.vertexSize = sizeof(Vertex3D) * buffer.vertices_3d_head;
            bufferUpdateParams.vertexCount = buffer.vertices_3d_head;
            Driver::update_vertex_buffer(buffer.buffer_3d, bufferUpdateParams);

            float4x4 mvp = Math::mult(projMatrix, viewMatrix);
            Driver::update_cbuffer(buffer.cbuffer, &mvp);

            Driver::bind_shader(buffer.shader_3d);
            Driver::RscCBuffer cbuffers[] = { buffer.cbuffer };
            Driver::bind_cbuffers(buffer.shader_3d, cbuffers, 1);
            Driver::bind_vertex_buffer(buffer.buffer_3d);
            Driver::draw_vertex_buffer(buffer.buffer_3d);
        }
        Driver::end_event();
    }
    
    void present2d(Buffer& buffer, const float4x4& projMatrix) {

        Driver::Marker_t marker;
        Driver::set_marker_name(marker, "DEBUG 2D");
        Driver::start_event(marker);
        {
            Renderer::Driver::bind_RS(buffer.rasterizerState);
            Renderer::Driver::bind_DS(buffer.orthoDepthState);

            u32 indexCount = vertexSizeToIndexCount(buffer.vertices_2d_head);
            Driver::IndexedBufferUpdateParams bufferUpdateParams;
            bufferUpdateParams.vertexData = buffer.vertices_2d;
            bufferUpdateParams.vertexSize = sizeof(Vertex2D) * buffer.vertices_2d_head;
            bufferUpdateParams.indexData = buffer.indices_2d;
            bufferUpdateParams.indexSize = indexCount * sizeof(u32);
            bufferUpdateParams.indexCount = indexCount;
            Driver::update_indexed_vertex_buffer(buffer.buffer_2d, bufferUpdateParams);

            Driver::update_cbuffer(buffer.cbuffer, &projMatrix);

            Driver::bind_shader(buffer.shader_2d);
            Driver::RscCBuffer cbuffers[] = { buffer.cbuffer };
            Driver::bind_cbuffers(buffer.shader_2d, cbuffers, 1);
            Driver::bind_indexed_vertex_buffer(buffer.buffer_2d);
            Driver::draw_indexed_vertex_buffer(buffer.buffer_2d);
        }
        Driver::end_event();
    }
    
}
}

#endif // __WASTELADNS_DEBUGDRAW_H__
