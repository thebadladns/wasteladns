#ifndef __WASTELADNS_BVH_H__
#define __WASTELADNS_BVH_H__

namespace bvh {

struct Node {
#if DISABLE_INTRINSICS
    float3 min;
    float3 max;
#else
    union {
        struct {
            float3 min;
            float3 max;
        };
        struct {
            __m256 xcoords_256;
            __m256 ycoords_256;
            __m256 zcoords_256;
        };
    };
#endif
    u16 lchildId;
    u16 firstIndexId;
    u16 sourceId;
    bool isLeaf;
};

#if !DISABLE_INTRINSICS
// todo: having to do these because of the union is atrocious
force_inline void ymm_to_minmax(
    float3& min, float3& max, const __m256 xcoords, const __m256 ycoords, const __m256 zcoords) {
    f32* vx = (f32*)&(xcoords);
    f32* vy = (f32*)&(ycoords);
    f32* vz = (f32*)&(zcoords);
    min = float3(vx[7], vy[7], vz[7]);
    max = float3(vx[0], vy[0], vz[0]);
}
force_inline void minmax_to_ymm(
    __m256& xcoords, __m256& ycoords, __m256& zcoords, const float3 min, const float3 max) {
    xcoords = _mm256_set_ps(
        min.x, max.x, min.x, max.x, min.x, max.x, min.x, max.x);
    ycoords = _mm256_set_ps(
        min.y, min.y, max.y, max.y, min.y, min.y, max.y, max.y);
    zcoords = _mm256_set_ps(
        min.z, min.z, min.z, min.z, max.z, max.z, max.z, max.z);

}
#endif
    
struct Triangle {
    float3 min;
    float3 max;
    float3 center;
    u32 firstVertexIndex;
    u32 sourceId;
};
struct BuildTreeContext {
    allocator::Buffer<Node>& nodes;
    allocator::PagedArena& persistentArena;
    const Triangle* trianglePool;
    const f32* vertexPool;
    const u16* indexPool;
    u32 indexCount;
};
struct Tree {
    Node* nodes;
    u32 nodeCount;
};
force_inline void emptyNode(Node& n) {
    n.min = float3( FLT_MAX,  FLT_MAX,  FLT_MAX);
    n.max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}
force_inline void expandNodeBounds(Node& n, const Triangle& tri) {
    n.min = math::min(n.min, tri.min);
    n.max = math::max(n.max, tri.max);
}
void buildTreeRecursive(
BuildTreeContext& ctx, u16* triangleIds, u32 triangleId_count, const u32 nodeId) {
    ctx.nodes.data[nodeId].isLeaf = triangleId_count == 1;
    if (ctx.nodes.data[nodeId].isLeaf) {
        const Triangle& tri = ctx.trianglePool[triangleIds[0]];
        // note: this has already been set by the parent node
        //ctx.nodes.data[nodeId].min = tri.min;
        //ctx.nodes.data[nodeId].max = tri.max;
        ctx.nodes.data[nodeId].firstIndexId = triangleIds[0] * 3;
        ctx.nodes.data[nodeId].sourceId = tri.sourceId;
    } else {
        ctx.nodes.data[nodeId].lchildId = (u32)ctx.nodes.len;
        Node lchild = {};
        Node rchild = {};
        emptyNode(lchild);
        emptyNode(rchild);

        // Choose the largest axis of the bounding box to split the triangles
        float3 currExtents = math::subtract(ctx.nodes.data[nodeId].max, ctx.nodes.data[nodeId].min);
        u8 widestCoord = 0;
        if (currExtents.y > math::max(currExtents.x, currExtents.z)) {
            widestCoord = 1;
        } else if (currExtents.z > math::max(currExtents.x, currExtents.y)) {
            widestCoord = 2;
        }
        f32 widestCoordCenter = 0.5f * 
            (ctx.nodes.data[nodeId].max.v[widestCoord] + ctx.nodes.data[nodeId].min.v[widestCoord]);

        // Partition triangles in place, on each side of the widest axis
        u32 lTriangleIdIdx = 0;
        u32 rTriangleIdIdx = triangleId_count - 1;
        while (lTriangleIdIdx < rTriangleIdIdx) {
            while (ctx.trianglePool[triangleIds[lTriangleIdIdx]].center.v[widestCoord] < widestCoordCenter
                && lTriangleIdIdx <= triangleId_count - 2) { lTriangleIdIdx++; }
            while (ctx.trianglePool[triangleIds[rTriangleIdIdx]].center.v[widestCoord] >= widestCoordCenter
                && rTriangleIdIdx >= 1) { rTriangleIdIdx--; }
            if (lTriangleIdIdx < rTriangleIdIdx) {
                u32 temp = triangleIds[lTriangleIdIdx];
                triangleIds[lTriangleIdIdx] = triangleIds[rTriangleIdIdx];
                triangleIds[rTriangleIdIdx] = temp;
            }
        }
        u32 lTriangleCount = lTriangleIdIdx;
        u32 rTriangleCount = triangleId_count - lTriangleCount;

        // One of the sides is empty (can happen, since the widest axis
        // is determined from bounding boxes, but the center of every
        // triangle may lie on one side)
        if (lTriangleCount == 0 || rTriangleCount == 0) {
            lTriangleIdIdx = 0;
            rTriangleIdIdx = triangleId_count - 1;
            while (lTriangleIdIdx < rTriangleIdIdx) {
                if ((lTriangleIdIdx & 1) != 0) {
                    u16 rTriangle = triangleIds[rTriangleIdIdx];
                    triangleIds[rTriangleIdIdx] = triangleIds[lTriangleIdIdx];
                    triangleIds[lTriangleIdIdx] = rTriangle;
                }
                lTriangleIdIdx++;
                rTriangleIdIdx--;
            }
            lTriangleCount = lTriangleIdIdx;
            rTriangleCount = triangleId_count - lTriangleCount;
        }

        // ensure the children nodes have the right bounds, so they can split triangles inside
        // of the, appropriately
        for (u32 i = 0; i < lTriangleCount; i++) {
            const Triangle& tri = ctx.trianglePool[triangleIds[i]];
            expandNodeBounds(lchild, tri);
        }
        for (u32 i = lTriangleCount; i < triangleId_count; i++) {
            const Triangle& tri = ctx.trianglePool[triangleIds[i]];
            expandNodeBounds(rchild, tri);
        }

        // Note that it's not useful to keep a reference to nodes.data[nodeId] as a variable
        // inside this function, since the array may move the location of the node during
        // this reallocate-emplace
        allocator::push(ctx.nodes, ctx.persistentArena) = lchild;
        allocator::push(ctx.nodes, ctx.persistentArena) = rchild;
        buildTreeRecursive(
            ctx, triangleIds, lTriangleCount,
            ctx.nodes.data[nodeId].lchildId);
        buildTreeRecursive(
            ctx, triangleIds + lTriangleCount, rTriangleCount,
            ctx.nodes.data[nodeId].lchildId + 1);
    }
}
void buildTree(
allocator::PagedArena& persistentArena, allocator::PagedArena scratchArena,
Tree& bvh, f32* vertexPool, const u16* indexPool, const u32 indexCount,
const u32* sourceIds) {

    u32 triangleCount = indexCount / 3;
    u16* triangleIds =
        (u16*) allocator::alloc_arena(
            scratchArena, triangleCount * sizeof(u16), alignof(u16));
    Triangle* trianglePool =
        (Triangle*)allocator::alloc_arena(
            scratchArena, triangleCount * sizeof(Triangle), alignof(Triangle));

    allocator::Buffer<Node> nodes = {};
    Node root = {};
    emptyNode(root);
    for (u32 triangleId = 0; triangleId < triangleCount; triangleId++) {
        Triangle tri;
        float3 a(
            vertexPool[indexPool[triangleId * 3] * 3],
            vertexPool[indexPool[triangleId * 3] * 3 + 1],
            vertexPool[indexPool[triangleId * 3] * 3 + 2]);
        float3 b(
            vertexPool[indexPool[triangleId * 3 + 1] * 3],
            vertexPool[indexPool[triangleId * 3 + 1] * 3 + 1],
            vertexPool[indexPool[triangleId * 3 + 1] * 3 + 2]);
        float3 c(
            vertexPool[indexPool[triangleId * 3 + 2] * 3],
            vertexPool[indexPool[triangleId * 3 + 2] * 3 + 1],
            vertexPool[indexPool[triangleId * 3 + 2] * 3 + 2]);
        tri.min = math::min(math::min(a, b), c);
        tri.max = math::max(math::max(a, b), c);
        tri.center = math::scale(math::add(tri.max, tri.min), 0.5f);
        tri.sourceId = sourceIds[triangleId];
        tri.firstVertexIndex = triangleId * 3;
        expandNodeBounds(root, tri);
        trianglePool[triangleId] = tri;
        triangleIds[triangleId] = triangleId;
    }
    allocator::push(nodes, persistentArena) = root;

    BuildTreeContext context = {
        nodes,
        persistentArena,
        trianglePool,
        vertexPool,
        indexPool,
        indexCount
    };
    buildTreeRecursive(context, triangleIds, triangleCount, 0);

#if !DISABLE_INTRINSICS
    for (u32 n = 0; n < (u32)nodes.len; n++) {
        Node& node = nodes.data[n];
        minmax_to_ymm(
            node.xcoords_256, node.ycoords_256, node.zcoords_256,
            node.min, node.max);
    };
#endif

    bvh.nodes = nodes.data;
    bvh.nodeCount = (u32)nodes.len;
}

struct FrustumStatus { enum Enum { In, Intersecting, Out }; };
#if DISABLE_INTRINSICS
FrustumStatus::Enum queryIsBoxVisibleInFrustum(
    const float4* planes, const u32 numPlanes, const float3& min, const float3& max) {
    const float4 boxPoints[8] = {
        { min.x, min.y, min.z, 1.f },
        { max.x, min.y, min.z, 1.f },
        { min.x, max.y, min.z, 1.f },
        { max.x, max.y, min.z, 1.f },
        { min.x, min.y, max.z, 1.f },
        { max.x, min.y, max.z, 1.f },
        { min.x, max.y, max.z, 1.f },
        { max.x, max.y, max.z, 1.f }
    };

    FrustumStatus::Enum status = FrustumStatus::In;
    for (u32 p = 0; p < numPlanes; p++) {
        int out = 0;
        // explicitly doing the dot product instead of calling math::dot,
        // and looping over all 8 points, instead of manually unrolling,
        // produces better auto-vectorization in MSVC
        for (u32 i = 0; i < 8; i++) {
            out += ((planes[p].x * boxPoints[i].x
                   + planes[p].y * boxPoints[i].y
                   + planes[p].z * boxPoints[i].z
                   + planes[p].w) < 0.f) ? 1 : 0;
        }
        if (out == 8) { status = FrustumStatus::Out; break; }
        else if (out > 0) { status = FrustumStatus::Intersecting; };
    }
    return status;
}
#else
FrustumStatus::Enum queryIsBoxVisibleInFrustum_256(
    const m256_4* planes, const u32 numPlanes, const __m256 vx, const __m256 vy, const __m256 vz) {

    FrustumStatus::Enum status = FrustumStatus::In;
    for (u32 p = 0; p < numPlanes; p++) {
        int out = 0;
        const __m256 vdotx = _mm256_fmadd_ps(vx, planes[p].vx, planes[p].vw);
        const __m256 vdotxy = _mm256_fmadd_ps(vy, planes[p].vy, vdotx);
        const __m256 vdot = _mm256_fmadd_ps(vz, planes[p].vz, vdotxy);
        const s32 mask = _mm256_movemask_ps(vdot);
        out += __popcnt(mask);
        if (out == 8) { status = FrustumStatus::Out; break; }
        else if (out > 0) { status = FrustumStatus::Intersecting; };
    }
    return status;
}
#if 0 // for potential future reference
FrustumStatus::Enum queryIsBoxVisibleInFrustum_FMA(  // this is the second best one
    const m128_4* planes, const u32 numPlanes,
    const __m128 xxxx, const __m128 yyyy, const __m128 z0z1z2z3, const __m128 z4z5z6z7) {

    FrustumStatus::Enum status = FrustumStatus::In;
    for (u32 p = 0; p < numPlanes; p++) {
        int out = 0;
        int mask;
        // compute p[i].x * plane.x + p[i].y * plane.y + plane.w
        __m128 dotxxxx = _mm_fmadd_ps(xxxx, planes[p].xxxx, planes[p].wwww);
        // compute p[i] * plane.z, for the lower 4 points of the bounding box:
        // { min.x, min.y, min.z}, { max.x, min.y, min.z },
        // { min.x, max.y, min.z } and { max.x, max.y, min.z }
        __m128 dotxyxyxyxy = _mm_fmadd_ps(yyyy, planes[p].yyyy, dotxxxx);
        // dot(p[i].xyz, plane.xyz) + plane.w for the lower 4 points of the bounding box
        __m128 dotp0p1p2p3 = _mm_fmadd_ps(z0z1z2z3, planes[p].zzzz, dotxyxyxyxy);
        // dot(p[i].xyz, plane.xyz) + plane.w < 0.f for the lower 4 points of the bounding box
        mask = _mm_movemask_ps(dotp0p1p2p3);
        // count how many points in the bounding box passed the comparison
        out += __popcnt(mask);
        // same thing, for the upper 4 points of the bounding box:
        // { min.x, min.y, max.z }, { max.x, min.y, max.z },
        // { min.x, max.y, max.z } and { max.x, max.y, max.z }
        // note that the x and y components are the same as the upper points, so we'll reuse those
        __m128 dotz4z5z6z7 = _mm_fmadd_ps(z4z5z6z7, planes[p].zzzz, dotxyxyxyxy);
        mask = _mm_movemask_ps(dotz4z5z6z7);
        out += __popcnt(mask);
        if (out == 8) { status = FrustumStatus::Out; break; }
        else if (out > 0) { status = FrustumStatus::Intersecting; };
    }
    return status;
}
FrustumStatus::Enum queryIsBoxVisibleInFrustum_SSE2(// this is the best one if we are not allowed any FMAs (SSE2 only)
const m128_4* planes, const u32 numPlanes,
const __m128 xxxx, const __m128 yyyy, const __m128 z0z1z2z3, const __m128 z4z5z6z7) {

    FrustumStatus::Enum status = FrustumStatus::In;
    for (u32 p = 0; p < numPlanes; p++) {
        int out = 0;
        int mask;
        // compute p[i].x * plane.x + p[i].y * plane.y and p[i] * plane.z, for the
        // lower 4 points of the bounding box:
        // { min.x, min.y, min.z}, { max.x, min.y, min.z },
        // { min.x, max.y, min.z } and { max.x, max.y, min.z }
        __m128 dotxxxx = _mm_mul_ps(xxxx, planes[p].xxxx);
        __m128 dotyyyy = _mm_mul_ps(yyyy, planes[p].yyyy);
        __m128 dotxyxyxyxy = _mm_add_ps(dotxxxx, dotyyyy);
        __m128 dotz0z1z2z3 = _mm_mul_ps(z0z1z2z3, planes[p].zzzz);
        // dot(p[i].xyz, plane.xyz) for the lower 4 points of the bounding box
        __m128 dotp0p1p2p3 = _mm_add_ps(dotxyxyxyxy, dotz0z1z2z3);
        // dot(p[i].xyz, plane.xyz) < plane.w for the lower 4 points of the bounding box
        // note that plane.w has to have the right sign for this
        // plane is given in ax + by + cz - d = 0 form, so we can skip a negation in the comparison
        __m128 maskp0p1p2p3 = _mm_cmplt_ps(dotp0p1p2p3, planes[p].wwww);
        // count how many points in the bounding box passed the comparison
        mask = _mm_movemask_ps(maskp0p1p2p3);
        out += __popcnt(mask);
        // same thing, for the upper 4 points of the bounding box:
        // { min.x, min.y, max.z }, { max.x, min.y, max.z },
        // { min.x, max.y, max.z } and { max.x, max.y, max.z }
        // note that the x and y components are the same as the upper points, so we'll reuse those
        __m128 dotz4z5z6z7 = _mm_mul_ps(z4z5z6z7, planes[p].zzzz);
        __m128 dotp4p5p6p7 = _mm_add_ps(dotxyxyxyxy, dotz4z5z6z7);
        __m128 maskp4p5p6p7 = _mm_cmplt_ps(dotp4p5p6p7, planes[p].wwww);
        mask = _mm_movemask_ps(maskp4p5p6p7);
        out += __popcnt(mask);
        if (out == 8) { status = FrustumStatus::Out; break; }
        else if (out > 0) { status = FrustumStatus::Intersecting; };
    }
    return status;
}
#endif
#endif
#if DISABLE_INTRINSICS
void findTrianglesIntersectingFrustum(
    allocator::PagedArena scratchArena,
    bool* sourceVisibility, const Tree& bvh,
    const float4* planes, const u32 numPlanes) {
#else
void findTrianglesIntersectingFrustum(
    allocator::PagedArena scratchArena,
    bool* sourceVisibility, const Tree & bvh,
    const m256_4* planes_256, const u32 numPlanes) {
#endif
    struct FrustumQueryNode { u16 nodeId; FrustumStatus::Enum frustumStatus; };

    FrustumQueryNode* nodeStack = (FrustumQueryNode*)allocator::alloc_arena(
        scratchArena, bvh.nodeCount * sizeof(FrustumQueryNode), alignof(FrustumQueryNode));
    u32 stackCount = 0;
    nodeStack[stackCount++] = { 0, FrustumStatus::Intersecting };

    while (stackCount > 0) {
        FrustumQueryNode n = nodeStack[--stackCount];

        // Add source id to the result array, if the triangle leaf is visible by the frustum
        if (bvh.nodes[n.nodeId].isLeaf) {
            u32 sourceId = bvh.nodes[n.nodeId].sourceId;
            sourceVisibility[sourceId] = true;
        } else {
            const Node& lchild = bvh.nodes[bvh.nodes[n.nodeId].lchildId];
            const Node& rchild = bvh.nodes[bvh.nodes[n.nodeId].lchildId + 1];
            FrustumStatus::Enum lStatus, rStatus;
            // if the parent node was fully visible in the frustum,
            // add the children without testing, and propagate the frustum result
            if (n.frustumStatus == FrustumStatus::In) {
                lStatus = rStatus = n.frustumStatus;
                nodeStack[stackCount++] = { bvh.nodes[n.nodeId].lchildId, lStatus };
                nodeStack[stackCount++] = { u16(bvh.nodes[n.nodeId].lchildId + 1u), rStatus };
            } else {
                // test each child against the frustum, cull if fully not visible
                #if DISABLE_INTRINSICS
                    lStatus = queryIsBoxVisibleInFrustum(planes, numPlanes, lchild.min, lchild.max);
                    rStatus = queryIsBoxVisibleInFrustum(planes, numPlanes, rchild.min, rchild.max);
                #else
                    lStatus = queryIsBoxVisibleInFrustum_256(
                        planes_256, numPlanes, lchild.xcoords_256, lchild.ycoords_256, lchild.zcoords_256);
                    rStatus = queryIsBoxVisibleInFrustum_256(
                        planes_256, numPlanes, rchild.xcoords_256, rchild.ycoords_256, rchild.zcoords_256);
                #endif
                if (lStatus != FrustumStatus::Out) {
                    nodeStack[stackCount++] = { bvh.nodes[n.nodeId].lchildId, lStatus };
                }
                if (rStatus != FrustumStatus::Out) {
                    nodeStack[stackCount++] = { u16(bvh.nodes[n.nodeId].lchildId + 1u), rStatus };
                }
            }
        }
    }
}

}; // namespace BVH

#endif // __WASTELADNS_BVH_H__
