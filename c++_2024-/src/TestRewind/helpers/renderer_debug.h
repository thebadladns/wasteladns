#ifndef __WASTELADNS_DEBUGDRAW_H__
#define __WASTELADNS_DEBUGDRAW_H__

#ifdef __WASTELADNS_DEBUG_TEXT__
#include "../lib/stb/stb_easy_font.h"
#endif

namespace debug {
    bool frustum_planes_off[6] = {};
    bool force_cut_frustum = false;
    const char* frustum_planes_names[] = { "near", "far", "left", "right", "bottom", "top" };
    u32 vertices_3d_head_last_frame = 0;
    u32 vertices_2d_head_last_frame = 0;
}

namespace renderer
{
namespace im
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
    
    struct Context {

        Vertex3D* vertices_3d;
        Vertex2D* vertices_2d;
        u32* indices_2d;
        
        driver::RscVertexBuffer buffer_3d;
        driver::RscIndexedVertexBuffer buffer_2d;
        driver::RscShaderSet shader_3d;
        driver::RscShaderSet shader_2d;
        driver::RscCBuffer cbuffer;
        driver::RscRasterizerState rasterizerState;
        driver::RscDepthStencilState orthoDepthState;

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
    
    void segment(Context& buffer, const float3& v1, const float3& v2, const Color32 color) {

        // Too many vertex pushed during immediate mode
        // Either bump Context::vertices_3d_head or re-implement
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
    void openSegment(Context& buffer, const float3& start, const float3& dir, Color32 color) {
        const f32 segmentLength = 10000.f;
        const float3 end = math::add(start, math::scale(dir, segmentLength));
        segment(buffer, start, end, color);
    }
    void ray(Context& buffer, const float3& start, const float3& dir, Color32 color) {
        openSegment(buffer, start, dir, color);
    }
    void line(Context& buffer, const float3& pos, const float3& dir, Color32 color) {
        const f32 extents = 10000.f;
        const float3 start = math::subtract(pos, math::scale(dir, extents));
        const float3 end = math::add(pos, math::scale(dir, extents));
        segment(buffer, start, end, color);
    }
    void poly(Context& buffer, const float3* vertices, const u8 count, Color32 color) {
        for (u8 i = 0; i < count; i++) {
            const float3 prev = vertices[i];
            const float3 next = vertices[(i + 1) % count];
            segment(buffer, prev, next, color);
        }
    }
    void aabb(Context& buffer, const float3 min, const float3 max, Color32 color) {

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
    void obb(Context& buffer, const float4x4& mat, const float3 aabb_min, const float3 aabb_max, Color32 color) {

        float3 leftNearBottom(aabb_min.x, aabb_min.y, aabb_min.z);
        float3 leftFarBottom(aabb_min.x, aabb_max.y, aabb_min.z);
        float3 leftFarTop(aabb_min.x, aabb_max.y, aabb_max.z);
        float3 leftNearTop(aabb_min.x, aabb_min.y, aabb_max.z);
        float3 rightNearBottom(aabb_max.x, aabb_min.y, aabb_min.z);
        float3 rightFarBottom(aabb_max.x, aabb_max.y, aabb_min.z);
        float3 rightFarTop(aabb_max.x, aabb_max.y, aabb_max.z);
        float3 rightNearTop(aabb_max.x, aabb_min.y, aabb_max.z);

        float4 leftNearBottom_WS = math::mult(mat, float4(leftNearBottom, 1.f));
        float4 leftFarBottom_WS = math::mult(mat, float4(leftFarBottom, 1.f));
        float4 leftFarTop_WS = math::mult(mat, float4(leftFarTop, 1.f));
        float4 leftNearTop_WS = math::mult(mat, float4(leftNearTop, 1.f));
        float4 rightNearBottom_WS = math::mult(mat, float4(rightNearBottom, 1.f));
        float4 rightFarBottom_WS = math::mult(mat, float4(rightFarBottom, 1.f));
        float4 rightFarTop_WS = math::mult(mat, float4(rightFarTop, 1.f));
        float4 rightNearTop_WS = math::mult(mat, float4(rightNearTop, 1.f));
        float3 leftNearBottom_NDC = math::invScale(leftNearBottom_WS.xyz, leftNearBottom_WS.w);
        float3 leftFarBottom_NDC = math::invScale(leftFarBottom_WS.xyz, leftFarBottom_WS.w);
        float3 leftFarTop_NDC = math::invScale(leftFarTop_WS.xyz, leftFarTop_WS.w);
        float3 leftNearTop_NDC = math::invScale(leftNearTop_WS.xyz, leftNearTop_WS.w);
        float3 rightNearBottom_NDC = math::invScale(rightNearBottom_WS.xyz, rightNearBottom_WS.w);
        float3 rightFarBottom_NDC = math::invScale(rightFarBottom_WS.xyz, rightFarBottom_WS.w);
        float3 rightFarTop_NDC = math::invScale(rightFarTop_WS.xyz, rightFarTop_WS.w);
        float3 rightNearTop_NDC = math::invScale(rightNearTop_WS.xyz, rightNearTop_WS.w);

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
    void frustum(Context& buffer, const float4x4& projectionMat, Color32 color) {

        // extract clipping planes (note that near plane depends on API's min z)
        float4x4 transpose = math::transpose(projectionMat);
        float4 planes[6] = {
            math::add(math::scale(transpose.col3, -renderer::min_z), transpose.col2), // near
            math::subtract(transpose.col3, transpose.col2),   // far
            math::add(transpose.col3, transpose.col0),        // left
            math::subtract(transpose.col3, transpose.col0),   // right
            math::add(transpose.col3, transpose.col1),        // bottom
            math::subtract(transpose.col3, transpose.col1),   // top
        };

        for (float4& p : planes) { p = math::invScale(p, math::mag(p.xyz)); } // normalize so we can do thickness adjustments

        // If the near and far planes are parallel, project the intersection points in clipspace back into world view
        if (!debug::force_cut_frustum && math::abs(math::dot(planes[0].xyz, planes[1].xyz) + 1.f) < math::eps32) {
            float4x4 inverse = projectionMat;
            math::inverse(inverse);
            const float3 aabb_min(-1.f, -1.f, renderer::min_z);
            const float3 aabb_max(1.f, 1.f, 1.f);
            obb(buffer, inverse, aabb_min, aabb_max, color);
            return;
        }

        // Otherwise, we'll generate a large cube and clip it via Sutherland–Hodgman
        // (this code uses a variation where we add a new face for each clip plane,
        // to prevent holes in our polyhedron
        // note that if we were to store vertices in a shared array, and edges / faces referenced them,
        // we'd only have to clip edges once (as opposed to once per face), and sorting faces would probably be easier,
        // since we'd already have the vertex as pairs inside edges, and we'd just have to sort them by their common points
        
        // Build large cube around near plane (not too big, or we'll get floating point errors)
        const f32 size = 100000.f;
        float3 pts_in[8];
        pts_in[0] = float3(-size, -size, -size);    // left near bottom
        pts_in[1] = float3(-size, size, -size);     // left near top
        pts_in[2] = float3(size, -size, -size);     // right near bottom
        pts_in[3] = float3(size, size, -size);      // right near top
        pts_in[4] = float3(-size, -size, size);     // left far bottom
        pts_in[5] = float3(-size, size, size);      // left far top
        pts_in[6] = float3(size, -size, size);      // right far bottom
        pts_in[7] = float3(size, size, size);       // right far top
        float3 pointOnNearPlane = math::scale(planes[0].xyz, -planes[0].w);
        for (float3& p : pts_in) {
            p = math::add(p, pointOnNearPlane);
        }
        struct Face { float3 pts[32]; u32 count; };
        Face pts_out[12] = {};
        pts_out[0] = { { pts_in[1], pts_in[0], pts_in[4], pts_in[5] }, 4 }; // left
        pts_out[1] = { { pts_in[3], pts_in[2], pts_in[6], pts_in[7] }, 4 }; // right
        pts_out[2] = { { pts_in[1], pts_in[5], pts_in[7], pts_in[3] }, 4 }; // top
        pts_out[3] = { { pts_in[0], pts_in[2], pts_in[6], pts_in[4] }, 4 }; // bottom
        pts_out[4] = { { pts_in[1], pts_in[3], pts_in[2], pts_in[0] }, 4 }; // near
        pts_out[5] = { { pts_in[4], pts_in[6], pts_in[7], pts_in[5] }, 4 }; // far
        u32 num_faces = 6;

        // Cut our cube by each culling plane
        for (u32 plane_id = 0; plane_id < 6; plane_id++) {
            if (debug::frustum_planes_off[plane_id]) continue;

            float4 plane = planes[plane_id];

            Face cutface = {}; // each plane cut may generate one new face on our shape
            for (u32 face_id = 0; face_id < num_faces; face_id++) {
                Face& face_out = pts_out[face_id];
                if (face_out.count == 0) continue;

                const Face face_in = pts_out[face_id]; // explicit copy of inputs, so we can write directly on our input array
                face_out.count = 0;
                float3 va = face_in.pts[face_in.count - 1];
                f32 va_d = math::dot(va, plane.xyz) + plane.w;
                for (u32 currvertex_in = 0; currvertex_in < face_in.count; currvertex_in++) {

                    float3 vb = face_in.pts[currvertex_in];
                    f32 vb_d = math::dot(vb, plane.xyz) + plane.w;
                    bool addToCutface = false;
                    float3 intersection;

                    const f32 eps = 0.001f;
                    if (vb_d > eps) { // current vertex in positive zone, add to poly
                        if (va_d < -eps) { // edge entering the positive zone, add intersection point first
                            float3 ab = math::subtract(vb, va);
                            f32 t = (-va_d) / math::dot(plane.xyz, ab);
                            intersection = math::add(va, math::scale(ab, t));
                            face_out.pts[face_out.count++] = intersection;
                            addToCutface = true;
                        }
                        face_out.pts[face_out.count++] = vb;
                    }
                    else if (vb_d < -eps) { // current vertex in negative zone, don't add to poly
                        if (va_d > eps) { // edge entering the negative zone, add intersection to face
                            float3 ab = math::subtract(vb, va);
                            f32 t = (-va_d) / math::dot(plane.xyz, ab);
                            intersection = math::add(va, math::scale(ab, t));
                            face_out.pts[face_out.count++] = intersection;
                            addToCutface = true;
                        }
                    }
                    else {
                        face_out.pts[face_out.count++] = vb;
                        addToCutface = true;
                        intersection = vb;
                    }

                    // Skip repeated points on the new face
                    for (u32 i = 0; i < cutface.count && addToCutface; i++) {
                        if (math::isCloseAll(cutface.pts[i], intersection, 0.01f)) { addToCutface = false; }
                    }
                    if (addToCutface) { cutface.pts[cutface.count++] = intersection; }

                    va = vb;
                    va_d = vb_d;
                }
            }

            // If we have cut points with this plane, we need to compute a convex hull of the points, and add it as a new face to our shape
            // We'll compute the hull using Andrew's monotone chain algorithm
            if (cutface.count > 2) {
                // Project onto xz, xy or yz depending on which will yield a larger area (based on largest component of normal)
                f32 x = math::abs(plane.x), y = math::abs(plane.y), z = math::abs(plane.z);
                u32 ax = 0, ay = 1; // project on xy plane
                if (x > y && x > z) { ax = 1; ay = 2; } // project on yz plane
                else if (y > x && y > z) { ax = 0; ay = 2; } // project on xz plane

                // Sort points on the x axis, then on the y axis, using insertion since we have a few amount of points
                auto comp = [](const u32 ax, const u32 ay, const float3 a, const float3 b) -> bool {
                    const f32 eps = 0.0001f;
                    const f32 dx = a.v[ax] - b.v[ax];
                    return dx > eps || (dx > -eps && a.v[ay] > b.v[ay]);
                    };
                for (s32 i = 1; i < (s32)cutface.count; i++) {
                    float3 key = cutface.pts[i];
                    s32 j = i - 1;
                    while (j >= 0 && comp(ax, ay, cutface.pts[j], key)) {
                        cutface.pts[j + 1] = cutface.pts[j];
                        j--;
                    }
                    cutface.pts[j + 1] = key;
                }
                // Cross product in 2d will tell us wether the sequence O->A->B makes a counter clockwise turn
                auto a_rightof_b = [](const u32 ax, const u32 ay, const float3 o, const float3 a, const float3 b) -> f32 {
                    return (a.v[ax] - o.v[ax]) * (b.v[ay] - o.v[ay]) - (a.v[ay] - o.v[ay]) * (b.v[ax] - o.v[ax]);
                    };
                Face sortedcutface = {};
                // Compute lower hull
                for (u32 i = 0; i < cutface.count; i++) {
                    while (sortedcutface.count >= 2 &&
                        a_rightof_b(ax, ay, sortedcutface.pts[sortedcutface.count - 2], sortedcutface.pts[sortedcutface.count - 1], cutface.pts[i]) > 0.f) {
                        sortedcutface.count--;
                    }
                    sortedcutface.pts[sortedcutface.count++] = cutface.pts[i];
                }
                // Compute upper hull
                for (u32 i = cutface.count - 1, t = sortedcutface.count + 1; i > 0; i--) {
                    while (sortedcutface.count >= t &&
                        a_rightof_b(ax, ay, sortedcutface.pts[sortedcutface.count - 2], sortedcutface.pts[sortedcutface.count - 1], cutface.pts[i - 1]) > 0.f) {
                        sortedcutface.count--;
                    }
                    sortedcutface.pts[sortedcutface.count++] = cutface.pts[i - 1];
                }
                sortedcutface.count--;
                pts_out[num_faces++] = sortedcutface;
            }
        }

        for (u32 face_id = 0; face_id < num_faces; face_id++) {
            im::poly(buffer, pts_out[face_id].pts, pts_out[face_id].count, color);
        }
    }
    void circle(Context& buffer, const float3& center, const float3& normal, const f32 radius, const Color32 color) {
        Transform33 m = math::fromUp(normal);
        
        static constexpr u32 kVertexCount = 8;
        float3 vertices[kVertexCount];
        for (u8 i = 0; i < kVertexCount; i++) {
            
            const f32 angle = i * 2.f * math::pi32 / (f32) kVertexCount;
            const float3 vertexDirLocal = math::add(math::scale(m.right, math::cos(angle)), math::scale(m.front, math::sin(angle)));
            
            vertices[i] = math::add(center, math::scale(vertexDirLocal, radius));
        }
        poly(buffer, vertices, kVertexCount, color);
    }
    void sphere(Context& buffer, const float3& center, const f32 radius, const Color32 color) {
        
        static constexpr u32 kSectionCount = 2;
        const Color32 colorVariation(0.25f, 0.25f, 0.25f, 0.f);
        const float3 up = math::upAxis();
        const float3 front = math::frontAxis();
        const float3 right = math::rightAxis();
        for (u8 i = 0; i < kSectionCount; i++) {
            
            const f32 angle = i * math::pi32 / (f32) kSectionCount;
            const f32 s = math::sin(angle);
            const f32 c = math::cos(angle);
            
            const float3 rotatedFront = math::add(math::scale(right, c), math::scale(front, s));
            circle(buffer, center, rotatedFront, radius, color);
            
            const f32 sectionRadius = radius * s;
            const f32 sectionOffset = radius * c;
            const float3 sectionPos = math::add(center, math::scale(up, sectionOffset));
            circle(buffer, sectionPos, up, sectionRadius, color);
        }
    }
    
    void box_2d(Context& buffer, const float2 min, const float2 max, Color32 color) {

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
    void text2d_va(Context& buffer, const TextParams& params, const char* format, va_list argList) {
        
        char text[256];
        platform::format_va(text, sizeof(text), format, argList);
        
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

    void text2d(Context& buffer, const TextParams& params, const char* format, ...) {
        va_list va;
        va_start(va, format);
        text2d_va(buffer, params, format, va);
        va_end(va);
    }
    
    void text2d(Context& buffer, const float3& pos, const char* format, ...) {
        TextParams params;
        params.pos = pos;
        params.scale = 1;

        va_list va;
        va_start(va, format);
        text2d_va(buffer, params, format, va);
        va_end(va);
    }
#endif // __WASTELADNS_DEBUG_TEXT__
    
    void init(Context& buffer, allocator::Arena& arena) {

        buffer = {};

        {
            // reserve memory for buffers
            u32 vertices_3d_size = max_3d_vertices * sizeof(Vertex3D);
            u32 vertices_2d_size = max_2d_vertices * sizeof(Vertex2D);
            u32 indices_2d_size = vertexSizeToIndexCount(max_2d_vertices) * sizeof(u32);
            buffer.vertices_3d = (Vertex3D*)allocator::alloc_arena(arena, vertices_3d_size, alignof(Vertex3D));
            buffer.vertices_2d = (Vertex2D*)allocator::alloc_arena(arena, vertices_2d_size, alignof(Vertex2D));
            buffer.indices_2d = (u32*)allocator::alloc_arena(arena, indices_2d_size, alignof(u32));

            driver::create_cbuffer(buffer.cbuffer, { sizeof(float4x4), 4 });
            const renderer::driver::CBufferBindingDesc bufferBindings_MVP[] = {{ "type_PerGroup", driver::CBufferStageMask::VS }};

            // 2d
            {
                const driver::VertexAttribDesc attribs_color2d[] = {
                    driver::make_vertexAttribDesc("POSITION", offsetof(Vertex2D, pos), sizeof(Vertex2D), driver::BufferAttributeFormat::R32G32_FLOAT),
                    driver::make_vertexAttribDesc("COLOR", offsetof(Vertex2D, color), sizeof(Vertex2D), driver::BufferAttributeFormat::R8G8B8A8_UNORM)
                };
                renderer::ShaderDesc desc = {};
                desc.vertexAttrs = attribs_color2d;
                desc.vertexAttr_count = countof(attribs_color2d);
                desc.textureBindings = nullptr;
                desc.textureBinding_count = 0;
                desc.bufferBindings = bufferBindings_MVP;
                desc.bufferBinding_count = countof(bufferBindings_MVP);
                // reuse 3d shaders
                desc.vs_name = shaders::vs_color3d_unlit.name;
                desc.vs_src = shaders::vs_color3d_unlit.src;
                desc.ps_name = shaders::ps_color3d_unlit.name;
                desc.ps_src = shaders::ps_color3d_unlit.src;
                renderer::compile_shader(buffer.shader_2d, desc);

                renderer::driver::IndexedVertexBufferDesc bufferParams;
                bufferParams.vertexData = nullptr;
                bufferParams.indexData = nullptr;
                bufferParams.vertexSize = vertices_2d_size;
                bufferParams.vertexCount = max_2d_vertices;
                bufferParams.indexSize = indices_2d_size;
                bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::CPU;
                bufferParams.accessType = renderer::driver::BufferAccessType::CPU;
                bufferParams.indexType = renderer::driver::BufferItemType::U32;
                bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
                bufferParams.indexCount = 0;
                driver::create_indexed_vertex_buffer(buffer.buffer_2d, bufferParams, attribs_color2d, countof(attribs_color2d));
            }
            // 3d
            {
                const driver::VertexAttribDesc attribs_color3d[] = {
                    driver::make_vertexAttribDesc("POSITION", offsetof(Vertex3D, pos), sizeof(Vertex3D), driver::BufferAttributeFormat::R32G32B32_FLOAT),
                    driver::make_vertexAttribDesc("COLOR", offsetof(Vertex3D, color), sizeof(Vertex3D), driver::BufferAttributeFormat::R8G8B8A8_UNORM)
                };
                renderer::ShaderDesc desc = {};
                desc.vertexAttrs = attribs_color3d;
                desc.vertexAttr_count = countof(attribs_color3d);
                desc.textureBindings = nullptr;
                desc.textureBinding_count = 0;
                desc.bufferBindings = bufferBindings_MVP;
                desc.bufferBinding_count = countof(bufferBindings_MVP);
                desc.vs_name = shaders::vs_color3d_unlit.name;
                desc.vs_src = shaders::vs_color3d_unlit.src;
                desc.ps_name = shaders::ps_color3d_unlit.name;
                desc.ps_src = shaders::ps_color3d_unlit.src;
                renderer::compile_shader(buffer.shader_3d, desc);

                renderer::driver::VertexBufferDesc bufferParams;
                bufferParams.vertexData = nullptr;
                bufferParams.vertexSize = vertices_3d_size;
                bufferParams.vertexCount = max_3d_vertices;
                bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::CPU;
                bufferParams.accessType = renderer::driver::BufferAccessType::CPU;
                bufferParams.type = renderer::driver::BufferTopologyType::Lines;
                driver::create_vertex_buffer(buffer.buffer_3d, bufferParams, attribs_color3d, countof(attribs_color3d));
            }
        }
        
        renderer::driver::create_RS(buffer.rasterizerState, { renderer::driver::RasterizerFillMode::Fill, renderer::driver::RasterizerCullMode::CullBack });
        {
            renderer::driver::DepthStencilStateParams dsParams;
            dsParams = {};
            dsParams.depth_enable = false;
            renderer::driver::create_DS(buffer.orthoDepthState, dsParams);
        }
    }
    void commit3d(Context& buffer) {
        driver::BufferUpdateParams bufferUpdateParams;
        bufferUpdateParams.vertexData = buffer.vertices_3d;
        bufferUpdateParams.vertexSize = sizeof(Vertex3D) * buffer.vertices_3d_head;
        bufferUpdateParams.vertexCount = buffer.vertices_3d_head;
        driver::update_vertex_buffer(buffer.buffer_3d, bufferUpdateParams);
        debug::vertices_3d_head_last_frame = buffer.vertices_3d_head;
        buffer.vertices_3d_head = 0;
    }
    void present3d(Context& buffer, const float4x4& projMatrix, const float4x4& viewMatrix) {
        driver::Marker_t marker;
        driver::set_marker_name(marker, "DEBUG 3D");
        driver::start_event(marker);
        {
            // depth state needs to be set by the caller
            renderer::driver::bind_RS(buffer.rasterizerState);

            float4x4 mvp = math::mult(projMatrix, viewMatrix);
            driver::update_cbuffer(buffer.cbuffer, &mvp);

            driver::bind_shader(buffer.shader_3d);
            driver::RscCBuffer cbuffers[] = { buffer.cbuffer };
            driver::bind_cbuffers(buffer.shader_3d, cbuffers, 1);
            driver::bind_vertex_buffer(buffer.buffer_3d);
            driver::draw_vertex_buffer(buffer.buffer_3d);
        }
        driver::end_event();
    }

    void commit2d(Context& buffer) {
        u32 indexCount = vertexSizeToIndexCount(buffer.vertices_2d_head);
        driver::IndexedBufferUpdateParams bufferUpdateParams;
        bufferUpdateParams.vertexData = buffer.vertices_2d;
        bufferUpdateParams.vertexSize = sizeof(Vertex2D) * buffer.vertices_2d_head;
        bufferUpdateParams.indexData = buffer.indices_2d;
        bufferUpdateParams.indexSize = indexCount * sizeof(u32);
        bufferUpdateParams.indexCount = indexCount;
        driver::update_indexed_vertex_buffer(buffer.buffer_2d, bufferUpdateParams);
        debug::vertices_2d_head_last_frame = buffer.vertices_2d_head;
        buffer.vertices_2d_head = 0;
    }
    void present2d(Context& buffer, const float4x4& projMatrix) {

        driver::Marker_t marker;
        driver::set_marker_name(marker, "DEBUG 2D");
        driver::start_event(marker);
        {
            renderer::driver::bind_RS(buffer.rasterizerState);
            renderer::driver::bind_DS(buffer.orthoDepthState);

            driver::update_cbuffer(buffer.cbuffer, &projMatrix);

            driver::bind_shader(buffer.shader_2d);
            driver::RscCBuffer cbuffers[] = { buffer.cbuffer };
            driver::bind_cbuffers(buffer.shader_2d, cbuffers, 1);
            driver::bind_indexed_vertex_buffer(buffer.buffer_2d);
            driver::draw_indexed_vertex_buffer(buffer.buffer_2d);
        }
        driver::end_event();
        buffer.vertices_2d_head = 0;
    }
    
}
}

#endif // __WASTELADNS_DEBUGDRAW_H__
