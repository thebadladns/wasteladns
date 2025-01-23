#ifndef __WASTELADNS_BVH_H__
#define __WASTELADNS_BVH_H__

namespace bvh {

struct Node {
    float3 min;
    float3 max;
    u16 lchildId;
    u16 firstIndexId;
    u16 sourceId;
    bool isLeaf;
};
    
struct Triangle {
    float3 min;
    float3 max;
    float3 center;
    u32 firstVertexIndex;
    u32 sourceId;
};
struct BuildTreeContext {
    allocator::Buffer<Node>& nodes;
    allocator::Arena& persistentArena;
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
        // node: this has already been set by the parent node
        //ctx.nodes.data[nodeId].min = tri.min;
        //ctx.nodes.data[nodeId].max = tri.max;
        ctx.nodes.data[nodeId].firstIndexId = triangleIds[0] * 3;
        ctx.nodes.data[nodeId].sourceId = tri.sourceId;
    } else {
        ctx.nodes.data[nodeId].lchildId = (u32)ctx.nodes.len;
        Node lchild;
        Node rchild;
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
allocator::Arena& persistentArena, allocator::Arena scratchArena,
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
    Node root;
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
    bvh.nodes = nodes.data;
    bvh.nodeCount = (u32)nodes.len;
}

struct FrustumStatus { enum Enum { In, Intersecting, Out }; };
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
        out += (math::dot(planes[p], boxPoints[0]) < 0.f) ? 1 : 0;
        out += (math::dot(planes[p], boxPoints[1]) < 0.f) ? 1 : 0;
        out += (math::dot(planes[p], boxPoints[2]) < 0.f) ? 1 : 0;
        out += (math::dot(planes[p], boxPoints[3]) < 0.f) ? 1 : 0;
        out += (math::dot(planes[p], boxPoints[4]) < 0.f) ? 1 : 0;
        out += (math::dot(planes[p], boxPoints[5]) < 0.f) ? 1 : 0;
        out += (math::dot(planes[p], boxPoints[6]) < 0.f) ? 1 : 0;
        out += (math::dot(planes[p], boxPoints[7]) < 0.f) ? 1 : 0;
        if (out == 8) { status = FrustumStatus::Out; break; }
        else if (out > 0) { status = FrustumStatus::Intersecting; };
    }
    return status;
}
void findTrianglesIntersectingFrustum(
    allocator::Arena scratchArena,
    bool* sourceVisibility, const Tree& bvh,
    const float4* planes, const u32 numPlanes) {
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
                nodeStack[stackCount++] = { bvh.nodes[n.nodeId].lchildId + 1u, rStatus };
            } else {
                // test each child against the frustum, cull if fully not visible
                lStatus = queryIsBoxVisibleInFrustum(planes, numPlanes, lchild.min, lchild.max);
                rStatus = queryIsBoxVisibleInFrustum(planes, numPlanes, rchild.min, rchild.max);
                if (lStatus != FrustumStatus::Out) {
                    nodeStack[stackCount++] = { bvh.nodes[n.nodeId].lchildId, lStatus };
                }
                if (rStatus != FrustumStatus::Out) {
                    nodeStack[stackCount++] = { bvh.nodes[n.nodeId].lchildId + 1u, rStatus };
                }
            }
        }
    }
}

}; // namespace BVH

#endif // __WASTELADNS_BVH_H__
