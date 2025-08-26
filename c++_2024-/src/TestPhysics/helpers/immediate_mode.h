#ifndef __WASTELADNS_IM_H__
#define __WASTELADNS_IM_H__

namespace im {

struct Vertex2D {
    float2 pos;
    u32 color;
};
struct Vertex3D {
    float3 pos;
    u32 color;
};

const u32 max_3d_vertices = 1 << 17;
const u32 max_2d_vertices = 1 << 16;
const size_t arena_size =                       // 3.125MB
      max_3d_vertices * sizeof(Vertex3D)        // (1 << 17) * 16 = 2MB
    + max_2d_vertices * sizeof(Vertex2D)        // 2^16 * 12 = 768KB
    + (max_2d_vertices * 3 / 2) * sizeof(u32);  // ((2^16 * 3) / 2) * 4 = 384KB (at worst we use 6 indices per quad)

struct GraphicsContext {

    Vertex3D* vertices_3d;
    Vertex2D* vertices_2d;
    u32* indices_2d;

    gfx::rhi::RscVertexBuffer buffer_3d;
    gfx::rhi::RscIndexedVertexBuffer buffer_2d;
    gfx::rhi::RscShaderSet shader_3d;
    gfx::rhi::RscShaderSet shader_2d;
    gfx::rhi::RscCBuffer cbuffer;
    gfx::rhi::RscRasterizerState rasterizerState;
    gfx::rhi::RscDepthStencilState orthoDepthState;

    float2 clip_bb_min;
    float2 clip_bb_max;

    u32 vertices_3d_head;
    u32 vertices_2d_head;
    u32 indices_2d_head;

    u32 vertexBufferIndex;
    u32 layoutBufferIndex;
    u32 shader;
};

} // im

namespace debug {

bool frustum_planes_off[6] = {};
bool force_cut_frustum = false;
bool pauseSceneRender = false;
const char* frustum_planes_names[] = { "near", "far", "left", "right", "bottom", "top" };
u32 vertices_3d_head_last_frame = 0;
u32 vertices_2d_head_last_frame = 0;

}

namespace im {

GraphicsContext ctx;

force_inline bool valid_boundingbox_2d(const float2 min, const float2 max) {
    return (min.x < max.x) && (min.y < max.y);
}

void segment(const float3& v1, const float3& v2, const Color32 color) {

    // Too many vertex pushed during immediate mode
    // Either bump Context::vertices_3d_head or re-implement
    // complex primitives to avoid vertex usage
    assert(ctx.vertices_3d_head + 2 < max_3d_vertices);
        
    Vertex3D& vertexStart = ctx.vertices_3d[ctx.vertices_3d_head];
    Vertex3D& vertexEnd = ctx.vertices_3d[ctx.vertices_3d_head + 1];
    ctx.vertices_3d_head = ctx.vertices_3d_head + 2;

    vertexStart.pos = v1;
    vertexStart.color = color.ABGR();
    vertexEnd.pos = v2;
    vertexEnd.color = color.ABGR();
}
void openSegment(const float3& start, const float3& dir, Color32 color) {
    const f32 segmentLength = 10000.f;
    const float3 end = math::add(start, math::scale(dir, segmentLength));
    segment(start, end, color);
}
void ray(const float3& start, const float3& dir, Color32 color) {
    openSegment(start, dir, color);
}
void line(const float3& pos, const float3& dir, Color32 color) {
    const f32 extents = 10000.f;
    const float3 start = math::subtract(pos, math::scale(dir, extents));
    const float3 end = math::add(pos, math::scale(dir, extents));
    segment(start, end, color);
}
void poly(const float3* vertices, const u8 count, Color32 color) {
    for (u8 i = 0; i < count; i++) {
        const float3 prev = vertices[i];
        const float3 next = vertices[(i + 1) % count];
        segment(prev, next, color);
    }
}
void aabb(const float3 min, const float3 max, Color32 color) {

    float3 leftNearBottom(min.x, min.y, min.z);
    float3 leftFarBottom(min.x, max.y, min.z);
    float3 leftFarTop(min.x, max.y, max.z);
    float3 leftNearTop(min.x, min.y, max.z);
    float3 rightNearBottom(max.x, min.y, min.z);
    float3 rightFarBottom(max.x, max.y, min.z);
    float3 rightFarTop(max.x, max.y, max.z);
    float3 rightNearTop(max.x, min.y, max.z);
        
    // left plane
    segment(leftNearBottom, leftFarBottom, color);
    segment(leftFarBottom, leftFarTop, color);
    segment(leftFarTop, leftNearTop, color);
    segment(leftNearTop, leftNearBottom, color);

    // right plane
    segment(rightNearBottom, rightFarBottom, color);
    segment(rightFarBottom, rightFarTop, color);
    segment(rightFarTop, rightNearTop, color);
    segment(rightNearTop, rightNearBottom, color);

    // remaining top
    segment(leftNearTop, rightNearTop, color);
    segment(leftFarTop, rightFarTop, color);
    // remaining bottom
    segment(leftNearBottom, rightNearBottom, color);
    segment(leftFarBottom, rightFarBottom, color);
}
void obb(const float4x4& mat, const float3 aabb_min, const float3 aabb_max, Color32 color) {

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
    segment(leftNearBottom_NDC, leftFarBottom_NDC, color);
    segment(leftFarBottom_NDC, leftFarTop_NDC, color);
    segment(leftFarTop_NDC, leftNearTop_NDC, color);
    segment(leftNearTop_NDC, leftNearBottom_NDC, color);

    // right plane
    segment(rightNearBottom_NDC, rightFarBottom_NDC, color);
    segment(rightFarBottom_NDC, rightFarTop_NDC, color);
    segment(rightFarTop_NDC, rightNearTop_NDC, color);
    segment(rightNearTop_NDC, rightNearBottom_NDC, color);

    // remaining top
    segment(leftNearTop_NDC, rightNearTop_NDC, color);
    segment(leftFarTop_NDC, rightFarTop_NDC, color);
    // remaining bottom
    segment(leftNearBottom_NDC, rightNearBottom_NDC, color);
    segment(leftFarBottom_NDC, rightFarBottom_NDC, color);
}

void frustum(const float4* planes, const u32 plane_count, Color32 color) {
    // We'll generate a large cube and clip it via Sutherland–Hodgman
    // (this code uses a variation where we add a new face for each clip plane,
    // to prevent holes in our polyhedron
    // note that if we were to store vertices in a shared array, and edges / faces referenced them,
    // we'd only have to clip edges once (as opposed to once per face),
    // and sorting faces would probably be easier, since we'd already have the vertex as pairs
    // inside edges, and we'd just have to sort them by their common points
       
    // upper limit so we can allocate on the stack (good enough for now)
    enum { MAX_NUM_PLANES = 16 };
    assert(plane_count <= MAX_NUM_PLANES);
    // for degenerate cases, if applicable:
    //for (u32 i = 0; i < plane_count; i++) { assert(!math::isNanAny(planes[i])); if (math::isNanAny(planes[i])) return; }

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
    Face pts_out[MAX_NUM_PLANES + 6] = {};
    pts_out[0] = { { pts_in[1], pts_in[0], pts_in[4], pts_in[5] }, 4 }; // left
    pts_out[1] = { { pts_in[3], pts_in[2], pts_in[6], pts_in[7] }, 4 }; // right
    pts_out[2] = { { pts_in[1], pts_in[5], pts_in[7], pts_in[3] }, 4 }; // top
    pts_out[3] = { { pts_in[0], pts_in[2], pts_in[6], pts_in[4] }, 4 }; // bottom
    pts_out[4] = { { pts_in[1], pts_in[3], pts_in[2], pts_in[0] }, 4 }; // near
    pts_out[5] = { { pts_in[4], pts_in[6], pts_in[7], pts_in[5] }, 4 }; // far
    u32 num_faces = 6;

    // Cut our cube by each culling plane
    for (u32 plane_id = 0; plane_id < plane_count; plane_id++) {
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
                    if (math::isCloseAll(cutface.pts[i], intersection, 0.05f)) { addToCutface = false; }
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
                if (sortedcutface.count == countof(sortedcutface.pts)) break; // crash fix for degenerate cases
                sortedcutface.pts[sortedcutface.count++] = cutface.pts[i];
            }
            // Compute upper hull
            for (u32 i = cutface.count - 1, t = sortedcutface.count + 1; i > 0; i--) {
                while (sortedcutface.count >= t &&
                    a_rightof_b(ax, ay, sortedcutface.pts[sortedcutface.count - 2], sortedcutface.pts[sortedcutface.count - 1], cutface.pts[i - 1]) > 0.f) {
                    sortedcutface.count--;
                }
                if (sortedcutface.count == countof(sortedcutface.pts)) break; // crash fix for degenerate cases
                sortedcutface.pts[sortedcutface.count++] = cutface.pts[i - 1];
            }
            sortedcutface.count--;
            pts_out[num_faces++] = sortedcutface;
        }
    }

    for (u32 face_id = 0; face_id < num_faces; face_id++) {
        im::poly(pts_out[face_id].pts, pts_out[face_id].count, color);
    }
}
void frustum(const float4x4& projectionMat, Color32 color) {
    // extract clipping planes (note that near plane depends on API's min z)
    float4x4 transpose = math::transpose(projectionMat);
    float4 planes[6] = {
        math::add(math::scale(transpose.col3, -gfx::min_z), transpose.col2), // near
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
        const float3 aabb_min(-1.f, -1.f, gfx::min_z);
        const float3 aabb_max(1.f, 1.f, 1.f);
        obb(inverse, aabb_min, aabb_max, color);
        return;
    }
    else { frustum(planes, 6, color); }
}
void circle(const float3& center, const float3& normal, const f32 radius, const Color32 color) {
    Transform33 m = math::fromUp(normal);
        
    static constexpr u32 kVertexCount = 8;
    float3 vertices[kVertexCount];
    for (u8 i = 0; i < kVertexCount; i++) {
            
        const f32 angle = i * 2.f * math::pi32 / (f32) kVertexCount;
        const float3 vertexDirLocal = math::add(math::scale(m.right, math::cos(angle)), math::scale(m.front, math::sin(angle)));
            
        vertices[i] = math::add(center, math::scale(vertexDirLocal, radius));
    }
    poly(vertices, kVertexCount, color);
}
void sphere(const float3& center, const f32 radius, const Color32 color) {
        
    const u8 kSectionCount = 2;
    const Color32 colorVariation(0.25f, 0.25f, 0.25f, 0.f);
    const float3 up = math::upAxis();
    const float3 front = math::frontAxis();
    const float3 right = math::rightAxis();
    for (u8 i = 0; i < kSectionCount; i++) {
            
        const f32 angle = i * math::pi32 / (f32) kSectionCount;
        const f32 s = math::sin(angle);
        const f32 c = math::cos(angle);
            
        const float3 rotatedFront = math::add(math::scale(right, c), math::scale(front, s));
        circle(center, rotatedFront, radius, color);
            
        const f32 sectionRadius = radius * s;
        const f32 sectionOffset = radius * c;
        const float3 sectionPos = math::add(center, math::scale(up, sectionOffset));
        circle(sectionPos, up, sectionRadius, color);
    }
}
void plane(const float4& plane, Color32 color) { // xyz = normal, z = - distance plane to normal
    const f32 scale = 5.f;

    float3 quadCenter = math::scale(plane.xyz, -plane.w);
    float3 e2 = plane.xyz;
    float3 e0, e1;
    {
        float3 tmp = { 0.f, 0.f, 0.f };
        f32 e2x = math::abs(e2.x), e2y = math::abs(e2.y), e2z = math::abs(e2.z);
        if (e2x < e2y && e2x < e2z) tmp.x = 1.f; 
        else if (e2y < e2x && e2y < e2z) tmp.y = 1.f; 
        else tmp.z = 1.f;
        e0 = math::normalize(math::cross(tmp, e2));
        e1 = math::normalize(math::cross(e0, math::negate(e2)));
    }
    float3 quad[4] = {
        math::add(math::add(quadCenter, math::scale(e0, scale)), math::scale(e1, scale)),
        math::add(math::add(quadCenter, math::scale(e0, scale)), math::scale(e1, -scale)),
        math::add(math::add(quadCenter, math::scale(e0, -scale)), math::scale(e1, -scale)),
        math::add(math::add(quadCenter, math::scale(e0, -scale)), math::scale(e1, scale))
    };

    poly(quad, 4, color);
    segment(quadCenter, math::add(quadCenter, math::scale(e2, scale / 2.f)), color);
}

void box_2d(const float2 min, const float2 max, Color32 color) {

    float2 clipped_min = min;
    float2 clipped_max = max;
    if (valid_boundingbox_2d(ctx.clip_bb_min, ctx.clip_bb_max)) {
        clipped_min = math::max(min, ctx.clip_bb_min);
        clipped_max = math::min(max, ctx.clip_bb_max);
    }
    if (!valid_boundingbox_2d(clipped_min, clipped_max)) {
        return;
    }

    u32 vertexStart = ctx.vertices_2d_head;
    Vertex2D& bottomLeft = ctx.vertices_2d[vertexStart];
    bottomLeft.pos = { clipped_min.x, clipped_max.y };
    Vertex2D& topLeft = ctx.vertices_2d[vertexStart + 1];
    topLeft.pos = { clipped_min.x, clipped_min.y };
    Vertex2D& topRight = ctx.vertices_2d[vertexStart + 2];
    topRight.pos = { clipped_max.x, clipped_min.y };
    Vertex2D& bottomRigth = ctx.vertices_2d[vertexStart + 3];
    bottomRigth.pos = { clipped_max.x, clipped_max.y };
        
    u32 indexStart = ctx.indices_2d_head;
    ctx.indices_2d[indexStart + 0] = vertexStart + 2;
    ctx.indices_2d[indexStart + 1] = vertexStart + 1;
    ctx.indices_2d[indexStart + 2] = vertexStart + 0;
    ctx.indices_2d[indexStart + 3] = vertexStart + 0;
    ctx.indices_2d[indexStart + 4] = vertexStart + 3;
    ctx.indices_2d[indexStart + 5] = vertexStart + 2;
        
    ctx.vertices_2d_head += 4;
    ctx.indices_2d_head += 6;
        
    bottomLeft.color = color.ABGR();
    topLeft.color = color.ABGR();
    topRight.color = color.ABGR();
    bottomRigth.color = color.ABGR();
}
void poly2d(const float2* vertices, const u8 count, Color32 color) {
    u32 colorv4 = color.ABGR();

    const float2* clipped_vertices = vertices;
    u32 clipped_vertices_count = count;

    float2 vertices_buffer[32];
    if (valid_boundingbox_2d(ctx.clip_bb_min, ctx.clip_bb_max)) {
        // temporary storage for clipped vertices in each iteration
        float2 tmp_vertices[countof(vertices_buffer)];
        assert(countof(vertices_buffer) > count + 4); // too many vertices in poly!
        memcpy(vertices_buffer, vertices, sizeof(float2) * count);
        // start off populating the output (we'll swap input and outputs every iteration)
        float2* input_vertices = tmp_vertices; u32 input_count = 0;
        float2* output_vertices = vertices_buffer; u32 output_count = count;
        // note that this uses the normalize plane equation ax + by + cz + w = 0
        // w as such represents -n*p, that is, the plane's negative distance along the normal
        // we do this so the distance check can be just a dot product (w * 1.f becomes -n*p)
        float3 planes[4] = {
            /* left */ float3(1.f, 0, -ctx.clip_bb_min.x), /* top */ float3(0, -1.f, ctx.clip_bb_max.y),
            /* right */ float3(-1.f, 0, ctx.clip_bb_max.x), /* bottom */ float3(0.f, 1.f, -ctx.clip_bb_min.y),
        };
        // 2D Sutherland-Hodgman for each 4 planes
        const float eps = 0.0001f;
        for (u32 plane_idx = 0; output_count > 2 && plane_idx < 4; plane_idx++) {
            // swap inputs and outputs
            float2* tmp = input_vertices; input_vertices = output_vertices; output_vertices = tmp;
            input_count = output_count; output_count = 0;
            // check which vertex is on which side of the plane
            u32 prev_v = input_count - 1;
            f32 prev_d = math::dot(float3(input_vertices[prev_v], 1.f), planes[plane_idx]);
            for (u32 curr_v = 0; curr_v < input_count; curr_v++) {
                f32 d = math::dot(float3(input_vertices[curr_v], 1.f), planes[plane_idx]);
                if (d > eps) { // current vertex on positive zone
                    if (prev_d < -eps) { // previous vertex on negative zone
                        // add intersection
                        float2 ab = math::subtract(input_vertices[curr_v], input_vertices[prev_v]);
                        f32 t = -prev_d / math::dot(planes[plane_idx].xy, ab);
                        float2 intersection = math::add(input_vertices[prev_v], math::scale(ab, t));
                        output_vertices[output_count++] = intersection;
                    }
                    output_vertices[output_count++] = input_vertices[curr_v];
                } else if (d < -eps) { // current vertex on negative zone
                    if (prev_d > eps) { // previous vertex on positive zone
                        // add intersection
                        float2 ab = math::subtract(input_vertices[curr_v], input_vertices[prev_v]);
                        f32 t = -prev_d / math::dot(planes[plane_idx].xy, ab);
                        float2 intersection = math::add(input_vertices[prev_v], math::scale(ab, t));
                        output_vertices[output_count++] = intersection;
                    }
                } else { // current vertex on plane
                    output_vertices[output_count++] = input_vertices[curr_v];
                }
                // update previous vertex data
                prev_v = curr_v; prev_d = d;
            }
        }
        // skip clipped polys
        if (output_count < 3) return;
        clipped_vertices = output_vertices;
        clipped_vertices_count = output_count;
    }

    // add vertices as a triangle fan from the first vertex
    assert(ctx.vertices_2d_head + clipped_vertices_count  < max_2d_vertices); // increase max_2d_vertices
    for (u32 i = 1; i < clipped_vertices_count; i++) {
        u32 vertexStart = ctx.vertices_2d_head;
        Vertex2D& v_out_0 = ctx.vertices_2d[vertexStart];
        Vertex2D& v_out_1 = ctx.vertices_2d[vertexStart + 1];
        Vertex2D& v_out_2 = ctx.vertices_2d[vertexStart + 2];
        v_out_0.pos = clipped_vertices[0];
        v_out_1.pos = clipped_vertices[i - 1];
        v_out_2.pos = clipped_vertices[i];
        v_out_0.color = colorv4;
        v_out_1.color = colorv4;
        v_out_2.color = colorv4;
        u32 indexStart = ctx.indices_2d_head;
        ctx.indices_2d[indexStart + 0] = vertexStart + 0;
        ctx.indices_2d[indexStart + 1] = vertexStart + 1;
        ctx.indices_2d[indexStart + 2] = vertexStart + 2;

        ctx.vertices_2d_head += 3;
        ctx.indices_2d_head += 3;
    }
}
struct Text2DParams {
    Text2DParams() : color(1.f, 1.f, 1.f, 1.f), scale(1) {}
    float2 pos;
    Color32 color;
    u8 scale;
};
void text2d_va(const Text2DParams& params, const char* format, va_list argList) {
        
    char text[256];
    io::format_va(text, sizeof(text), format, argList);
        
    // output to a temporary buffer, so we can clip before pushing to our vertex array
    float2 textpoly_buffer[2048];
        
    // negate y, since our (0,0) is the center of the screen, stb's is bottom left
    u32 quadCount =
        stb_easy_font_print(
            params.pos.x, -params.pos.y, params.scale,
            text, textpoly_buffer, sizeof(textpoly_buffer));

    // stb uses ccw winding, but we are negating the y, so it matches our cw winding
    float2* v = textpoly_buffer;
    for(u32 i = 0; i < quadCount; i++) {
        poly2d(v, 4, params.color);
        v += 4;
    }
}

void text2d(const Text2DParams& params, const char* format, ...) {
    va_list va;
    va_start(va, format);
    text2d_va(params, format, va);
    va_end(va);
}
    
void text2d(const float2& pos, const char* format, ...) {
    Text2DParams params;
    params.pos = pos;
    params.scale = 1;

    va_list va;
    va_start(va, format);
    text2d_va(params, format, va);
    va_end(va);
}
    
void init(allocator::PagedArena& arena) {

    ctx = {};

    {
        // reserve memory for buffers
        u32 vertices_3d_size = max_3d_vertices * sizeof(Vertex3D);
        u32 vertices_2d_size = max_2d_vertices * sizeof(Vertex2D);
        u32 indices_2d_size = (max_2d_vertices * 3) / 2; // at worst we have all quads, at 6 index per poly
        ctx.vertices_3d = ALLOC_ARRAY(arena, Vertex3D, vertices_3d_size);
        ctx.vertices_2d = ALLOC_ARRAY(arena, Vertex2D, vertices_2d_size);
        ctx.indices_2d = ALLOC_ARRAY(arena, u32, indices_2d_size);
        ctx.indices_2d_head = 0;

        gfx::rhi::create_cbuffer(ctx.cbuffer, { sizeof(float4x4) });
        const gfx::rhi::CBufferBindingDesc bufferBindings_MVP[] = {{ "type_PerGroup", gfx::rhi::CBufferStageMask::VS }};

        // 2d
        {
            const gfx::rhi::VertexAttribDesc attribs_color2d[] = {
                gfx::rhi::make_vertexAttribDesc("POSITION", offsetof(Vertex2D, pos), sizeof(Vertex2D), gfx::rhi::BufferAttributeFormat::R32G32_FLOAT),
                gfx::rhi::make_vertexAttribDesc("COLOR", offsetof(Vertex2D, color), sizeof(Vertex2D), gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM)
            };
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_color2d;
            desc.vertexAttr_count = countof(attribs_color2d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_MVP;
            desc.bufferBinding_count = countof(bufferBindings_MVP);
            // reuse 3d shaders
            desc.vs_name = gfx::shaders::vs_color3d_unlit.name;
            desc.vs_src = gfx::shaders::vs_color3d_unlit.src;
            desc.ps_name = gfx::shaders::ps_color3d_unlit.name;
            desc.ps_src = gfx::shaders::ps_color3d_unlit.src;
            gfx::compile_shader(ctx.shader_2d, desc);

            gfx::rhi::IndexedVertexBufferDesc bufferParams;
            bufferParams.vertexData = nullptr;
            bufferParams.indexData = nullptr;
            bufferParams.vertexSize = vertices_2d_size;
            bufferParams.vertexCount = max_2d_vertices;
            bufferParams.indexSize = indices_2d_size;
            bufferParams.memoryUsage = gfx::rhi::BufferMemoryUsage::CPU;
            bufferParams.accessType = gfx::rhi::BufferAccessType::CPU;
            bufferParams.indexType = gfx::rhi::BufferItemType::U32;
            bufferParams.type = gfx::rhi::BufferTopologyType::Triangles;
            bufferParams.indexCount = 0;
            gfx::rhi::create_indexed_vertex_buffer(ctx.buffer_2d, bufferParams, attribs_color2d, countof(attribs_color2d));
        }
        // 3d
        {
            const gfx::rhi::VertexAttribDesc attribs_color3d[] = {
                gfx::rhi::make_vertexAttribDesc("POSITION", offsetof(Vertex3D, pos), sizeof(Vertex3D), gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT),
                gfx::rhi::make_vertexAttribDesc("COLOR", offsetof(Vertex3D, color), sizeof(Vertex3D), gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM)
            };
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_color3d;
            desc.vertexAttr_count = countof(attribs_color3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_MVP;
            desc.bufferBinding_count = countof(bufferBindings_MVP);
            desc.vs_name = gfx::shaders::vs_color3d_unlit.name;
            desc.vs_src = gfx::shaders::vs_color3d_unlit.src;
            desc.ps_name = gfx::shaders::ps_color3d_unlit.name;
            desc.ps_src = gfx::shaders::ps_color3d_unlit.src;
            gfx::compile_shader(ctx.shader_3d, desc);

            gfx::rhi::VertexBufferDesc bufferParams;
            bufferParams.vertexData = nullptr;
            bufferParams.vertexSize = vertices_3d_size;
            bufferParams.vertexCount = max_3d_vertices;
            bufferParams.memoryUsage = gfx::rhi::BufferMemoryUsage::CPU;
            bufferParams.accessType = gfx::rhi::BufferAccessType::CPU;
            bufferParams.type = gfx::rhi::BufferTopologyType::Lines;
            gfx::rhi::create_vertex_buffer(ctx.buffer_3d, bufferParams, attribs_color3d, countof(attribs_color3d));
        }
    }
        
    gfx::rhi::create_RS(
        ctx.rasterizerState,
        { gfx::rhi::RasterizerFillMode::Fill,
          gfx::rhi::RasterizerCullMode::CullBack,
          false });
    {
        gfx::rhi::DepthStencilStateParams dsParams;
        dsParams = {};
        dsParams.depth_enable = false;
        gfx::rhi::create_DS(ctx.orthoDepthState, dsParams);
    }
}
void commit3d() {
    gfx::rhi::BufferUpdateParams bufferUpdateParams;
    bufferUpdateParams.vertexData = ctx.vertices_3d;
    bufferUpdateParams.vertexSize = sizeof(Vertex3D) * ctx.vertices_3d_head;
    bufferUpdateParams.vertexCount = ctx.vertices_3d_head;
    gfx::rhi::update_vertex_buffer(ctx.buffer_3d, bufferUpdateParams);
    debug::vertices_3d_head_last_frame = ctx.vertices_3d_head;
    ctx.vertices_3d_head = 0;
}
void present3d(const float4x4& projMatrix, const float4x4& viewMatrix) {
    gfx::rhi::start_event("DEBUG 3D");
    {
        // depth state needs to be set by the caller
        gfx::rhi::bind_RS(ctx.rasterizerState);

        float4x4 mvp = math::mult(projMatrix, viewMatrix);
        gfx::rhi::update_cbuffer(ctx.cbuffer, &mvp);

        gfx::rhi::bind_shader(ctx.shader_3d);
        gfx::rhi::RscCBuffer cbuffers[] = { ctx.cbuffer };
        gfx::rhi::bind_cbuffers(ctx.shader_3d, cbuffers, 1);
        gfx::rhi::bind_vertex_buffer(ctx.buffer_3d);
        gfx::rhi::draw_vertex_buffer(ctx.buffer_3d);
    }
    gfx::rhi::end_event();
}

void commit2d() {
    //u32 indexCount = vertexSizeToIndexCount(ctx.vertices_2d_head);
    gfx::rhi::IndexedBufferUpdateParams bufferUpdateParams;
    bufferUpdateParams.vertexData = ctx.vertices_2d;
    bufferUpdateParams.vertexSize = sizeof(Vertex2D) * ctx.vertices_2d_head;
    bufferUpdateParams.indexData = ctx.indices_2d;
    bufferUpdateParams.indexSize = ctx.indices_2d_head * sizeof(u32);
    bufferUpdateParams.indexCount = ctx.indices_2d_head;
    gfx::rhi::update_indexed_vertex_buffer(ctx.buffer_2d, bufferUpdateParams);
    debug::vertices_2d_head_last_frame = ctx.vertices_2d_head;
    ctx.vertices_2d_head = 0;
    ctx.indices_2d_head = 0;
}
void present2d(const float4x4& projMatrix) {

    gfx::rhi::start_event("DEBUG 2D");
    {
        gfx::rhi::bind_RS(ctx.rasterizerState);
        gfx::rhi::bind_DS(ctx.orthoDepthState);

        gfx::rhi::update_cbuffer(ctx.cbuffer, &projMatrix);

        gfx::rhi::bind_shader(ctx.shader_2d);
        gfx::rhi::RscCBuffer cbuffers[] = { ctx.cbuffer };
        gfx::rhi::bind_cbuffers(ctx.shader_2d, cbuffers, 1);
        gfx::rhi::bind_indexed_vertex_buffer(ctx.buffer_2d);
        gfx::rhi::draw_indexed_vertex_buffer(ctx.buffer_2d);
    }
    gfx::rhi::end_event();
}
    
} // im


namespace im {

typedef u32 uiid;
struct Layout { enum Enum { None = -1, VerticalStack = 0, HorizontalStack }; };
const Color32 color_highlight(0xff3319);
const Color32 color_bright(0xffffff);
const Color32 color_lighter(0x8e8ba6);
const Color32 color_light(0x535074);
const Color32 color_default(0x353464);
const Color32 color_dark(0x192020);
const Color32 color_middark(0x222229);

struct Pane {
    const char* text;
    // start coordinates for the next ui element in the pane
    float2 content_origin_WS;
    float2 content_cursor_WS;
    float2 content_extents;
    float2 content_offset;
};
struct WindowContext {
    // current parent pane
    Pane* parent_pane;
    // start coordinates for the next ui element, if there's no active pane
    float2 cursor_WS;
    // start coordinates for the next vertical ui element (works without an active pane too)
    float2 block_cursor_WS;
    // keep track of the currently pressed UI element
    uiid pressed_id;
    // keep track of the currently hovered UI element
    uiid hovered_id;
    // element to be considered hovered next frame
    uiid queued_hovered_id;
    // determines how the elements should be placed one after the other
    Layout::Enum layout;
    Layout::Enum prevlayout;
    // scale for the UI elements
    u8 scale = 1;
    // keeps track of how many frames the currently pressed UI element has been pressed
    u8 frames_active;
};
WindowContext ui = {};

void reset_frame() {
    ui.hovered_id = ui.queued_hovered_id;
    ui.queued_hovered_id = 0;
    ui.cursor_WS = {};
    ui.prevlayout = Layout::None;
    ui.layout = Layout::VerticalStack;
}

// mouse space (origin at top left, down is positive)
// to immediate window space (origin is center, down is negative)
force_inline float2 get_mouse_WS() {
    return float2(
          platform::state.input.mouse.x - platform::state.screen.window_width / 2.f,
        -(platform::state.input.mouse.y - platform::state.screen.window_height / 2.f)
    );
}
force_inline bool mousedown() {
    return platform::state.input.mouse.down(::input::mouse::Keys::BUTTON_LEFT);
}
force_inline bool mousepressed() {
    return platform::state.input.mouse.pressed(::input::mouse::Keys::BUTTON_LEFT);
}
force_inline bool mouseup() {
    return platform::state.input.mouse.released(::input::mouse::Keys::BUTTON_LEFT);
}
force_inline float2 mousescroll() {
    return float2(platform::state.input.mouse.scrolldx, platform::state.input.mouse.scrolldy);
}
force_inline bool is_mouse_captured() { return ui.hovered_id || ui.pressed_id; }

force_inline f32 element_padding() { return ui.scale * 5.f; }

namespace impl {

u32 fnv(const char* name) { // todo: understand this
    const u8* data = (const u8*)name;
    u32 val = 3759247821;
    while (*data) {
        val ^= *data++;
        val *= 0x01000193;
    }
    val &= 0x7fffffff;
    val |= val == 0;
    return val;
};

force_inline bool in_rect(const float2& p, const float2& min, const float2& max) {
    return ((p.x > min.x && p.x < max.x) && (p.y > min.y && p.y < max.y));
}

force_inline void adjust_bounds_in_parent(float2& origin_WS, float2& extents, bool match_parent_width) {

    const f32 padding = element_padding();

    float2* cursor_WS = &ui.cursor_WS;
    float2 offset(0.f);
    if (ui.parent_pane) {

        cursor_WS = &ui.parent_pane->content_cursor_WS;
        offset = ui.parent_pane->content_offset;

        // if the element's width needs to match the parents, do so now
        if (ui.layout == Layout::VerticalStack && match_parent_width) {
            extents.x = math::max(ui.parent_pane->content_extents.x, extents.x);
        }

        // update parent's pane for next frame
        float2 paneExtents(
            (ui.parent_pane->content_cursor_WS.x - ui.parent_pane->content_origin_WS.x) + extents.x,
            (ui.parent_pane->content_origin_WS.y - ui.parent_pane->content_cursor_WS.y) + extents.y);
        ui.parent_pane->content_extents = math::max(ui.parent_pane->content_extents, paneExtents);
    }

    // place element at the expected cursor
    origin_WS = *cursor_WS;
    origin_WS = math::subtract(origin_WS, offset);

    if (ui.layout == Layout::HorizontalStack) {
        // update cursor horizontally
        cursor_WS->x = cursor_WS->x + extents.x + padding;
        // update the block's vertical coordinate
        ui.block_cursor_WS.y =
            math::min(ui.block_cursor_WS.y, cursor_WS->y - extents.y - padding);
    } else {
        // update cursor vertically
        cursor_WS->y = cursor_WS->y - extents.y - padding;
    }
}

// compute maximum extents of a given label and its possible range of values
template<typename _type>
force_inline float2 get_label_dimensions(const char* text, const char* format, _type min, _type max) {
    char formatted[256];
    float2 extents;
    extents.y = ui.scale * (f32)stb_easy_font_height(text);

    io::format(formatted, sizeof(formatted), format, text, max);
    extents.x = ui.scale * (f32)stb_easy_font_width(formatted);
    io::format(formatted, sizeof(formatted), format, text, min);
    extents.x = ui.scale * math::max(extents.x, (f32)stb_easy_font_width(formatted));

    return extents;
}

force_inline u32 make_hash(const char* text) {
    char str[256];
    io::StrBuilder padStr{ str, sizeof(str) };
    if (ui.parent_pane) { io::append(padStr.curr, padStr.last, ui.parent_pane->text); }
    io::append(padStr.curr, padStr.last, text);
    uiid hash = fnv(padStr.str);
    return hash;
}

force_inline void set_active(u32 hash) { ui.pressed_id = hash; }
force_inline void keep_active(u32) { ui.frames_active++; }
force_inline void clear_active(u32) { ui.pressed_id = 0; ui.frames_active = 0; }

} // impl

void make_pane(Pane& pane, const char* text, float2 origin_WS) {
    pane = {};
    pane.text = text;
    pane.content_origin_WS = origin_WS;
}

void pane_start(Pane& pane) {
    ui.parent_pane = &pane;

    const f32 padding = ui.scale * 10.f;
    const f32 headerHeight = ui.scale * 15.f;
    const f32 headerPadding = ui.scale * 4.f;

    // mouse coords in window space
    float2 mouse_WS = get_mouse_WS();

    // text-based id for this pane
    uiid hash = impl::fnv(pane.text);

    // Drag when hovering and using the left click
    if (ui.pressed_id == 0) {
        if (ui.hovered_id == hash && mousepressed()) {
            impl::set_active(hash);
        }
    }
    // Finish dragging when releasing the left click
    if (ui.pressed_id == hash) {
        impl::keep_active(hash);
        pane.content_origin_WS =
            math::add(
                pane.content_origin_WS,
                float2(platform::state.input.mouse.dx, -platform::state.input.mouse.dy));
        if (mouseup()) { impl::clear_active(hash); }
    }

    // update dragged position and bounding boxes
    const f32 screenw = (f32)platform::state.screen.window_width;
    const f32 screenh = (f32)platform::state.screen.window_height;
    pane.content_cursor_WS = pane.content_origin_WS;
    float2 origin = float2(pane.content_origin_WS.x - padding, pane.content_origin_WS.y + padding);
    const float2 raw_extents = math::add(pane.content_extents, float2(2.f * padding));
    const float2 available_extents(
        (f32)screenw - (origin.x + screenw * 0.5f),
        (f32)screenh + (origin.y - screenh * 0.5f));
    float2 extents = math::min(raw_extents, available_extents);
    float2 bbox_min_WS(origin.x, origin.y - extents.y);
    float2 bbox_max_WS(origin.x + extents.x, origin.y);
    float2 header_bbox_min_WS(origin.x, origin.y);
    float2 header_bbox_max_WS(origin.x + extents.x, origin.y + headerHeight);

    // override the last hovered object with this one, including the header, if it's now hovered
    if (impl::in_rect(
            mouse_WS,
            math::min(bbox_min_WS, header_bbox_min_WS),
            math::max(bbox_max_WS, header_bbox_max_WS))) {
        ui.queued_hovered_id = hash;
    }

    // draw header before we set up the clip box
    im::box_2d(
        header_bbox_min_WS,
        header_bbox_max_WS,
        color_lighter);
    im::Text2DParams params;
    params.pos = float2(header_bbox_min_WS.x + headerPadding, header_bbox_max_WS.y - headerPadding);
    params.color = color_bright;
    params.scale = ui.scale;
    im::text2d(params, pane.text);

    // handle scrolling
    float2 scroll = mousescroll();
    if (ui.hovered_id == hash) {
        const float2 zero(0.f);
        float2 available_offset = math::max(math::subtract(raw_extents, available_extents), zero);
        pane.content_offset = math::add(pane.content_offset, math::scale(float2(5.f), scroll));
        pane.content_offset.x = math::clamp(pane.content_offset.x, 0.f, available_offset.x);
        pane.content_offset.y = math::clamp(pane.content_offset.y, -available_offset.y, 0.f);
    }
    ctx.clip_bb_min = bbox_min_WS;
    ctx.clip_bb_max = bbox_max_WS;

    // zero out extents, to be recomputed again this frame
    pane.content_extents = {};

    // draw background
    im::box_2d(
        bbox_min_WS,
        bbox_max_WS,
        color_middark);
}

void pane_end() {
    ui.parent_pane = nullptr;
    ctx.clip_bb_min = float2(FLT_MAX);
    ctx.clip_bb_max = float2(-FLT_MAX);
}

void horizontal_layout_start() {
    ui.prevlayout = ui.layout;
    ui.layout = Layout::HorizontalStack;
    if (ui.prevlayout != Layout::HorizontalStack) {
        if (ui.parent_pane) {
            ui.block_cursor_WS = ui.parent_pane->content_cursor_WS;
        } else {
            ui.block_cursor_WS = ui.cursor_WS;
        }
    }
}
void horizontal_layout_end() {
    ui.layout = ui.prevlayout;
    if (ui.prevlayout != Layout::HorizontalStack) {
        if (ui.parent_pane) {
            ui.parent_pane->content_cursor_WS = ui.block_cursor_WS;
        } else {
            ui.cursor_WS = ui.block_cursor_WS;
        }
    }
    ui.prevlayout = Layout::None;
}

void label(
    const char* text,
    Color32 textColor, float2 min_dimensions) {
    
    // calculate extents of the text to be displayed
    const f32 padding = ui.scale * 5.f;
    float2 text_extents;
    text_extents.x = math::max(ui.scale * (f32)stb_easy_font_width(text), min_dimensions.x);
    text_extents.y = math::max(ui.scale * (f32)stb_easy_font_height(text), min_dimensions.y);

    // calculate origin in window space, as well as extents
    float2 extents(text_extents.x + 2 * padding, text_extents.y + 2 * padding);
    float2 origin_WS;
    impl::adjust_bounds_in_parent(origin_WS, extents, /* match_parent_width */ false);

    // draw label
    im::Text2DParams params;
    params.pos = float2(origin_WS.x + padding, origin_WS.y - padding);
    params.color = textColor;
    params.scale = ui.scale;
    im::text2d(params, text);
}
void label_va(
    Color32 textColor, 
    float2 min_dimensions,
    const char* format, va_list argList) {
    char text[256];
    io::format_va(text, sizeof(text), format, argList);
    label(text, textColor, min_dimensions);
}
void label(const char* format, ...) {
    va_list va;
    va_start(va, format);
    label_va(color_bright, float2(0.f), format, va);
    va_end(va);
}
void label(Color32 textColor, const char* format, ...) {
    va_list va;
    va_start(va, format);
    label_va(textColor, float2(0.f), format, va);
    va_end(va);
}

bool button(
    const char* text,
    bool repeat_enabled = false, char* id = nullptr, float2 dimensions = float2(0.f)) {

    // text extents
    const f32 padding = ui.scale * 5.f;
    float2 text_extents;
    if (math::isZeroAll(dimensions)) {
        text_extents.x = ui.scale * (f32)stb_easy_font_width(text);
        text_extents.y = ui.scale * (f32)stb_easy_font_height(text);
    } else {
        text_extents = dimensions;
    }

    // calculate origin in window space, as well as element extents
    float2 extents(text_extents.x + 2 * padding, text_extents.y + 2 * padding);
    float2 origin_WS;
    impl::adjust_bounds_in_parent(origin_WS, extents, /* match_parent_width */ false);

    // bounding box in window space
    float2 bb_min_WS(origin_WS.x, origin_WS.y - extents.y);
    float2 bb_max_WS(origin_WS.x + extents.x, origin_WS.y);

    // mouse in window space
    float2 mouse_WS = get_mouse_WS();

    // make hash from hierarchical name
    uiid hash = impl::make_hash(id ? id : text);

    // process mouse click
    bool result = false;
    // mouse unpressed: press when hovered and left button is down
    if (ui.pressed_id == 0) {
        // override the last queued object with this one, if it's now hovered
        if (impl::in_rect(mouse_WS, bb_min_WS, bb_max_WS)) { ui.queued_hovered_id = hash; }

        if (ui.hovered_id == hash && mousedown()) { impl::set_active(hash); }
    }
    // mouse pressed: handle release
    if (ui.pressed_id == hash) {
        impl::keep_active(hash);

        // override the last queued object with this one, if it's now hovered
        if (impl::in_rect(mouse_WS, bb_min_WS, bb_max_WS)) { ui.queued_hovered_id = hash; }

        // mouse moved outside of box, release button
        if (ui.hovered_id != hash) { ui.pressed_id = 0; }
        // mouse released within button, button press has been completed
        else if (mouseup()) {
            result = true;
            impl::clear_active(hash);
        } else if (repeat_enabled && ui.frames_active > 16) { // todo: timeframe independent?
            result = true;
        }
    }

    // draw background
    Color32 bgColor = color_default;
    if (ui.pressed_id == hash) { bgColor = color_dark;
    } else if (ui.hovered_id == hash) { bgColor = color_light; }
    im::box_2d(bb_min_WS, bb_max_WS, bgColor);
    
    // draw text label
    im::Text2DParams params;
    params.pos = float2(origin_WS.x + padding, origin_WS.y - padding);
    params.color = color_bright;
    params.scale = ui.scale;
    im::text2d(params, text);

    return result;
};

void slider(const char* text, f32* v, const f32 min, const f32 max) {

    const float textPadding = ui.scale * 5.f;
    const float slider_width = ui.scale * 10.f;

    // compute extents of the text label
    const char* format = "%s: %.3f";
    float2 label_extents = impl::get_label_dimensions(text, format, min, max);

    // compute extents of the element, as well as the slider offset 
    float2 extents(label_extents.x + 2.f * textPadding, label_extents.y + 2.f * textPadding);
    float2 extents_slider(slider_width, extents.y);
    f32 sliderRatio = math::clamp(((*v - min) / (max - min)), 0.f, 1.f);
    float2 origin_WS;
    impl::adjust_bounds_in_parent(origin_WS, extents, /* match_parent_width */ true);
    f32 sliderOffset = sliderRatio * extents.x - slider_width / 2.f;
    sliderOffset = math::clamp(sliderOffset, 0.f, extents.x - extents_slider.x);

    // mouse coords in window space
    float2 mouse_WS = get_mouse_WS();

    // make hash from hierarchical name
    uiid hash = impl::make_hash(text);

    // Drag when hovering and using the left click
    if (ui.pressed_id == 0) {
        if (ui.hovered_id == hash && mousepressed()) { impl::set_active(hash); }
    }
    // Finish dragging when releasing the left click
    if (ui.pressed_id == hash) {
        impl::keep_active(hash);
        float mouse_relativex = mouse_WS.x - origin_WS.x;
        sliderRatio = mouse_relativex / extents.x;
        *v = math::clamp(sliderRatio * (max - min), min, max);
        sliderOffset = mouse_relativex - extents_slider.x * 0.5f;
        sliderOffset = math::clamp(sliderOffset, 0.f, extents.x - extents_slider.x);
        if (mouseup()) { impl::clear_active(hash); }
    }

    // now that we know the drag position, make the bounding boxes for this frame's element position
    float2 bb_min_WS(origin_WS.x, origin_WS.y - extents.y);
    float2 bb_max_WS(origin_WS.x + extents.x, origin_WS.y);
    float2 slider_bb_min_WS(origin_WS.x + sliderOffset, origin_WS.y - extents_slider.y);
    float2 slider_bb_max_WS(origin_WS.x + sliderOffset + extents_slider.x, origin_WS.y);

    // override the last hovered object with the slider picker, if it's now hovered
    if (impl::in_rect(
        mouse_WS,
        slider_bb_min_WS,
        slider_bb_max_WS)) {
        ui.queued_hovered_id = hash;
    }

    // draw background
    im::box_2d(bb_min_WS, bb_max_WS, color_dark);
    // draw slider picker
    im::box_2d(slider_bb_min_WS, slider_bb_max_WS, ui.pressed_id == hash ? color_light : color_default);
    // draw centered text
    char text_with_value[256];
    io::format(text_with_value, sizeof(text_with_value), "%s: %.3f", text, *v);
    im::Text2DParams params;
    params.pos = float2(
        origin_WS.x + extents.x * 0.5f - label_extents.x * 0.5f,
        origin_WS.y - extents.y * 0.5f + label_extents.y * 0.5f);
    params.color = Color32(0xFFFFFF);
    params.scale = ui.scale;
    im::text2d(params, text_with_value);
}

template<typename _type>
void input_step(const char* text, _type* v, const _type min, const _type max, bool wrap = false) {

    // compute max extents of the text label
    const char* format = "%s: %d";
    float2 label_extents = impl::get_label_dimensions(text, format, min, max);

    // draw element
    char text_with_value[256];
    io::format(text_with_value, sizeof(text_with_value), format, text, *v);
    horizontal_layout_start();
    {
        char text_id[256];
        io::format(text_id, sizeof(text_id), "inputstepminus_%s", text);
        if (button("-", /* repeat_enabled */ true, text_id)) {
            if (*v > min) { (*v)--; }
            else if (wrap) { (*v) = max; }
        }
        label(text_with_value, color_bright, label_extents);
        io::format(text_id, sizeof(text_id), "inputstepplus_%s", text);
        if (button("+", /* repeat_enabled */ true, text_id)) {
            if (*v < max) { (*v)++; }
            else if (wrap) { (*v) = min; }
        }
    }
    horizontal_layout_end();
}

bool checkbox(const char* text, bool* v) {

    bool result = false;

    horizontal_layout_start();
    {
        char text_id[256];
        io::format(text_id, sizeof(text_id), "checkbox_%s", text);
        float2 extents(ui.scale * 5.f, ui.scale * 8.f);
        if (button(*v ? "X" : " ", /* repeat_enabled */ false, text_id, extents)) {
            *v = !*v;
            result = true;
        }
        label(text);
    }
    horizontal_layout_end();

    return result;
}
} // im

#endif // __WASTELADNS_IM_H__
