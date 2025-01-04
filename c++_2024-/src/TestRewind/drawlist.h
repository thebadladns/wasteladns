#ifndef __WASTELADNS_DRAWLIST_H__
#define __WASTELADNS_DRAWLIST_H__

namespace renderer {

struct VertexLayout_Color_2D { float2 pos; u32 color; };
struct VertexLayout_Color_3D { float3 pos; u32 color; };
struct VertexLayout_Textured_3D { float3 pos; float2 uv; };
struct VertexLayout_Color_Skinned_3D {
    float3 pos;
    u32 color;
    u8 joint_indices[4];
    u8 joint_weights[4];
};
struct VertexLayout_Textured_Skinned_3D {
    float3 pos;
    float2 uv;
    u8 joint_indices[4];
    u8 joint_weights[4];
};

struct CBuffer_Binding { enum { Binding_0 = 0, Binding_1 = 1, Binding_2 = 2, Count }; };
struct SceneData {
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float3 viewPos;
    f32 padding1;
    float3 lightPos;
    f32 padding2;
};
struct NodeData {
    float4x4 worldMatrix;
    float4 groupColor;
};
struct Matrices32 {
    float4x4 data[32];
};
struct Matrices64 {
    float4x4 data[64];
};

struct Drawlist_Context {
    driver::RscCBuffer cbuffers[CBuffer_Binding::Count];
    driver::RscShaderSet* shader;
    driver::RscTexture* texture;
    driver::RscBlendState* blendState;
    driver::RscIndexedVertexBuffer* vertexBuffer;
};
struct Drawlist_Overrides {
    bool forced_shader;
    bool forced_texture;
    bool forced_blendState;
    bool forced_vertexBuffer;
    u32 forced_cbuffer_count;
};

struct DrawCall_Item {
    driver::RscShaderSet shader;
    driver::RscTexture texture;
    driver::RscBlendState blendState;
    driver::RscIndexedVertexBuffer vertexBuffer;
    driver::RscCBuffer cbuffers[2];
    const char* name;
    u32 cbuffer_count;
    u32 drawcount;
};

//#if USE_DEBUG_MEMORY
//typedef u64 SortKeyValue;
//typedef u32 DrawNodeHandle;
//struct DrawNodeMeta { enum Enum : u64 {
//    HandleBits = 64, HandleMask = 0xffffffffffffffff, MaxNodes = HandleMask - 1
//}; };
//#else 
typedef u64 SortKeyValue;
typedef u32 DrawNodeHandle;
struct DrawNodeMeta { enum Enum : u32 {
    HandleBits = 32, HandleMask = 0xffffffff, MaxNodes = HandleMask - 1
}; };
//#endif
typedef u32 InstancedNodeHandle;

struct SortKey { SortKeyValue v; s32 idx; };
struct DrawlistFilter { enum Enum { Alpha = 1, Mirror = 2 }; };
struct DrawlistBuckets { enum Enum { Base, Instanced, Count }; };
struct Drawlist {
    SortKey* keys;
    DrawCall_Item* items;
    u32 count[DrawlistBuckets::Count];
};
struct DrawlistStreams { enum Enum {
    Color3D, Color3DSkinned, Textured3D, Textured3DAlphaClip,
    Textured3DSkinned, Textured3DAlphaClipSkinned, Count }; };

typedef u32 MeshHandle;
struct ShaderTechniques { enum Enum {
        FullscreenBlitClearWhite, FullscreenBlitTextured,
        Instanced3D,
        Color3D, Color3DSkinned,
        Textured3D, Textured3DAlphaClip, Textured3DSkinned, Textured3DAlphaClipSkinned,
        Count, Bits = 3
}; };
const char* shaderNames[] = {
    "FullscreenBlitClearWhite", "FullscreenBlitTextured",
    "Instanced3D",
    "Color3D", "Color3DSkinned",
    "Textured3D", "Textured3DAlphaClip", "Textured3DSkinned", "Textured3DAlphaClipSkinned"
};
static_assert(countof(shaderNames) == ShaderTechniques::Count, 
    "Make sure there are enough shaderNames strings as there are ShaderTechniques::Enum values");

struct DrawMesh { // id of a piece of geometry loaded on the gpu
    ShaderTechniques::Enum shaderTechnique;
    driver::RscIndexedVertexBuffer vertexBuffer;
    driver::RscTexture texture; // should this be here?
};
struct DrawNode { // List of meshes (one of each type), and their render data in the scene
    void* ext_data;
    float3 min;
    float3 max;
    u32 cbuffer_node;
    u32 cbuffer_ext;
    MeshHandle meshHandles[DrawlistStreams::Count];
    NodeData nodeData;
};
struct DrawNodeInstanced {
    u32 cbuffer_node;
    u32 cbuffer_instances;
    u32 instanceCount;
    MeshHandle meshHandles[DrawlistStreams::Count];
    NodeData nodeData;
    Matrices64 instanceMatrices; // todo: dynamic per system?
};

void draw_drawlist(Drawlist& dl, Drawlist_Context& ctx, const Drawlist_Overrides& overrides) {
	u32 count = dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced];
    for (u32 i = 0; i < count; i++) {
        DrawCall_Item& item = dl.items[dl.keys[i].idx];
        driver::Marker_t marker;
        driver::set_marker_name(marker, item.name);
        renderer::driver::start_event(marker);
        if (!overrides.forced_shader && ctx.shader != &item.shader) {
            driver::bind_shader(item.shader);
            ctx.shader = &item.shader;
        }
		if (!overrides.forced_blendState && ctx.blendState != &item.blendState) {
			driver::bind_blend_state(item.blendState);
			ctx.blendState = &item.blendState;
		}
        if (!overrides.forced_texture && ctx.texture != &item.texture) {
            driver::bind_textures(&item.texture, 1);
            ctx.texture = &item.texture;
        }
        if (!overrides.forced_vertexBuffer && ctx.vertexBuffer != &item.vertexBuffer) {
            driver::bind_indexed_vertex_buffer(item.vertexBuffer);
            ctx.vertexBuffer = &item.vertexBuffer;
        }
        for (u32 i = 0; i < item.cbuffer_count; i++) {
            ctx.cbuffers[i + overrides.forced_cbuffer_count] = item.cbuffers[i];
        }
        driver::bind_cbuffers(
            *ctx.shader, ctx.cbuffers,item.cbuffer_count + overrides.forced_cbuffer_count);
		if (item.drawcount) {
            driver::draw_instances_indexed_vertex_buffer(item.vertexBuffer, item.drawcount);
        } else {
            driver::draw_indexed_vertex_buffer(item.vertexBuffer);
        }
        renderer::driver::end_event();
    }
}
struct Mirrors { // todo: figure out delete, unhack sizes
    Transform transforms[256]; // xy from 0.f to 1.f describe the mirror quad, z is the normal
    MeshHandle meshHandles[256];
    u32 count;
};
struct Scene {
    struct Sky {
        driver::RscCBuffer cbuffer;
        NodeData nodeData;
        driver::RscIndexedVertexBuffer buffers[6];
    };
    driver::RscShaderSet shaders[ShaderTechniques::Count];
	allocator::Arena persistentArena;
    allocator::Pool<DrawNode> drawNodes;
    allocator::Pool<DrawNodeInstanced> instancedDrawNodes;
    allocator::Pool<DrawMesh> meshes;
    allocator::Pool<driver::RscCBuffer> cbuffers;
    renderer::driver::RscRasterizerState rasterizerStateFillFrontfaces;
    renderer::driver::RscRasterizerState rasterizerStateFillBackfaces;
    renderer::driver::RscRasterizerState rasterizerStateFillCullNone;
    renderer::driver::RscRasterizerState rasterizerStateLine;
    renderer::driver::RscDepthStencilState depthStateOn;
    renderer::driver::RscDepthStencilState depthStateReadOnly;
    renderer::driver::RscDepthStencilState depthStateOff;
    renderer::driver::RscDepthStencilState depthStateMarkMirror;
    renderer::driver::RscDepthStencilState depthStateUnmarkMirror;
    renderer::driver::RscDepthStencilState depthStateClearDepthInMirror;
    renderer::driver::RscDepthStencilState depthStateMirrorReflections; 
    renderer::driver::RscDepthStencilState depthStateMirrorReflectionsDepthReadOnly;
    renderer::driver::RscDepthStencilState depthStateMirrorReflectionsDepthAlways;
    renderer::driver::RscBlendState blendStateOn;
    renderer::driver::RscBlendState blendStateBlendOff;
    renderer::driver::RscBlendState blendStateOff;
    renderer::driver::RscMainRenderTarget windowRT;
    renderer::driver::RscRenderTarget gameRT;
    Mirrors mirrors;
    camera::WindowProjection windowProjection;
    camera::PerspProjection perspProjection;
    Sky sky;
    u32 cbuffer_scene;
    u32 cbuffer_nodeIdentity;
};

DrawNodeHandle handle_from_node(Scene& scene, DrawNode& node) {
    return allocator::get_pool_index(scene.drawNodes, node) + 1;
}
DrawNode& node_from_handle(Scene& scene, const DrawNodeHandle handle) {
    return allocator::get_pool_slot(scene.drawNodes, handle - 1);
}
DrawNodeHandle handle_from_instanced_node(Scene& scene, DrawNodeInstanced& node) {
    return allocator::get_pool_index(scene.instancedDrawNodes, node) + 1;
}
void instanced_node_from_handle(Matrices64*& matrices, u32*& count, Scene& scene, const u32 handle) {
    DrawNodeInstanced& node = allocator::get_pool_slot(scene.instancedDrawNodes, handle - 1);
    matrices = &node.instanceMatrices;
	count = &node.instanceCount;
}
DrawMesh& drawMesh_from_handle(Scene& scene, const u32 handle) {
    return allocator::get_pool_slot(scene.meshes, handle - 1);
}
u32 handle_from_drawMesh(Scene& scene, DrawMesh& mesh) {
    return allocator::get_pool_index(scene.meshes, mesh) + 1;
}
driver::RscCBuffer& cbuffer_from_handle(Scene& scene, const u32 handle) {
    return allocator::get_pool_slot(scene.cbuffers, handle - 1);
}
u32 handle_from_cbuffer(Scene& scene, driver::RscCBuffer& cbuffer) {
    return allocator::get_pool_index(scene.cbuffers, cbuffer) + 1;
}

struct Frustum {
    float4 planes[10];
    u32 count;
};
struct CullEntry {
    float3 boxPointsWS[8]; // todo: surely we can do better?
    u32 poolId;
};
struct CullEntries {
    CullEntry* entries;
    u32 count;
};
void allocCullEntries(allocator::Arena scratchArena, CullEntries& cullEntries, const Scene& scene) {
    
    cullEntries.entries = 
        (CullEntry*)allocator::alloc_arena(
            scratchArena,
            scene.drawNodes.count * sizeof(CullEntry),
            alignof(CullEntry)
        );

    for (u32 n = 0, count = 0; n < scene.drawNodes.cap && count < scene.drawNodes.count; n++) {
        if (scene.drawNodes.data[n].alive == 0) { continue; }
        count++;

        CullEntry& entry = cullEntries.entries[cullEntries.count++];
        const DrawNode& node = scene.drawNodes.data[n].state.live;
        const float4 boxPointsLS[8] = {
            { node.min.x, node.min.y, node.min.z, 1.f },
            { node.max.x, node.min.y, node.min.z, 1.f },
            { node.min.x, node.max.y, node.min.z, 1.f },
            { node.max.x, node.max.y, node.min.z, 1.f },
            { node.min.x, node.min.y, node.max.z, 1.f },
            { node.max.x, node.min.y, node.max.z, 1.f },
            { node.min.x, node.max.y, node.max.z, 1.f },
            { node.max.x, node.max.y, node.max.z, 1.f }
        };
        entry.boxPointsWS[0] = math::mult(node.nodeData.worldMatrix, boxPointsLS[0]).xyz;
        entry.boxPointsWS[1] = math::mult(node.nodeData.worldMatrix, boxPointsLS[1]).xyz;
        entry.boxPointsWS[2] = math::mult(node.nodeData.worldMatrix, boxPointsLS[2]).xyz;
        entry.boxPointsWS[3] = math::mult(node.nodeData.worldMatrix, boxPointsLS[3]).xyz;
        entry.boxPointsWS[4] = math::mult(node.nodeData.worldMatrix, boxPointsLS[4]).xyz;
        entry.boxPointsWS[5] = math::mult(node.nodeData.worldMatrix, boxPointsLS[5]).xyz;
        entry.boxPointsWS[6] = math::mult(node.nodeData.worldMatrix, boxPointsLS[6]).xyz;
        entry.boxPointsWS[7] = math::mult(node.nodeData.worldMatrix, boxPointsLS[7]).xyz;
        entry.poolId = n;
    }
}
struct VisibleNodes {
    u32* visible_nodes;
    u32 visible_nodes_count;
};
void computeVisibilityWS(allocator::Arena& frameArena, VisibleNodes& visibilityFrustum,
                         u32* isEachNodeVisible, const Frustum& frustum,
                         const CullEntries& cullEntries) {

    allocator::Buffer<u32> visibleNodes = {};
    visibilityFrustum.visible_nodes_count = 0;
    for (u32 i = 0; i < cullEntries.count; i++) {
        const CullEntry& entry = cullEntries.entries[i];
        
        // quad inside frustum
        bool visible = true;
        for (u32 p = 0; p < 6; p++) {
            int out = 0;
            out += (math::dot(frustum.planes[p], float4(entry.boxPointsWS[0], 1.f)) < 0.f) ? 1 : 0;
            out += (math::dot(frustum.planes[p], float4(entry.boxPointsWS[1], 1.f)) < 0.f) ? 1 : 0;
            out += (math::dot(frustum.planes[p], float4(entry.boxPointsWS[2], 1.f)) < 0.f) ? 1 : 0;
            out += (math::dot(frustum.planes[p], float4(entry.boxPointsWS[3], 1.f)) < 0.f) ? 1 : 0;
            out += (math::dot(frustum.planes[p], float4(entry.boxPointsWS[4], 1.f)) < 0.f) ? 1 : 0;
            out += (math::dot(frustum.planes[p], float4(entry.boxPointsWS[5], 1.f)) < 0.f) ? 1 : 0;
            out += (math::dot(frustum.planes[p], float4(entry.boxPointsWS[6], 1.f)) < 0.f) ? 1 : 0;
            out += (math::dot(frustum.planes[p], float4(entry.boxPointsWS[7], 1.f)) < 0.f) ? 1 : 0;
            if (out == 8) { visible = false; break; }
        }
        // frustum inside quad??

        if (visible) {
            isEachNodeVisible[entry.poolId] = true;
            push(visibleNodes, frameArena) = entry.poolId;
            visibilityFrustum.visible_nodes_count++;
        }
    }
    visibilityFrustum.visible_nodes = visibleNodes.data;
}
void computeVisibilityCS(VisibleNodes& visibleNodes, u32* isEachNodeVisible, float4x4& vpMatrix,
                         const Scene& scene) {
    auto cull_isVisible = [](float4x4 mvp, float3 min, float3 max) -> bool {
        // adapted to clipspace from https://iquilezles.org/articles/frustumcorrect/
        float4 box[8] = {
            {min.x, min.y, min.z, 1.0},
            {max.x, min.y, min.z, 1.0},
            {min.x, max.y, min.z, 1.0},
            {max.x, max.y, min.z, 1.0},
            {min.x, min.y, max.z, 1.0},
            {max.x, min.y, max.z, 1.0},
            {min.x, max.y, max.z, 1.0},
            {max.x, max.y, max.z, 1.0},
        };
        float3 frustum[8] = {
            {-1.f, -1.f, renderer::min_z},
            {1.f, -1.f, renderer::min_z},
            {-1.f, 1.f, renderer::min_z},
            {1.f, 1.f, renderer::min_z},
            {-1.f, -1.f, 1.f},
            {1.f, -1.f, 1.f},
            {-1.f, 1.f, 1.f},
            {1.f, 1.f, 1.f},
        };
        // box in clip space, and min / max in NDC
        float4 box_CS[8];
        float3 boxmin_NDC = { FLT_MAX, FLT_MAX, FLT_MAX };
        float3 boxmax_NDC = { -FLT_MAX,-FLT_MAX,-FLT_MAX };
        for (u32 corner_id = 0; corner_id < 8; corner_id++) {
            box_CS[corner_id] = math::mult(mvp, box[corner_id]);
            boxmax_NDC =
                math::max(math::invScale(box_CS[corner_id].xyz, box_CS[corner_id].w), boxmax_NDC);
            boxmin_NDC =
                math::min(math::invScale(box_CS[corner_id].xyz, box_CS[corner_id].w), boxmin_NDC);
        }
        u32 out;

        // check whether the box is fully out of at least one frustum plane
        // near {0.f,0.f,1.f,-min_z} -> dot(p,box) = box.z - min_z*box.w -> box.z < -min_z*box.w
        out = 0;
        for (u32 corner_id = 0; corner_id < 8; corner_id++)
        { if (box_CS[corner_id].z < renderer::min_z * box_CS[corner_id].w) { out++; } }
        if (out == 8) return false;
        // far {0.f,0.f,-1.f,1.f} -> dot(p,box) = -box.z + box.w -> box.z > box.w
        out = 0;
        for (u32 corner_id = 0; corner_id < 8; corner_id++)
        { if (box_CS[corner_id].z > box_CS[corner_id].w) { out++; } }
        if (out == 8) return false;
        // left {1.f,0.f,0.f,1.f} -> dot(p,box) = box.x + box.w -> box.x < -box.w
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
        { if (box_CS[corner_id].x < -box_CS[corner_id].w) { out++; } }
		if (out == 8) return false;
        // right {1.f,0.f,0.f,1.f} -> dot(p,box) = -box.x + box.w -> box.x > box.w
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
	    { if (box_CS[corner_id].x > box_CS[corner_id].w) { out++; } }
		if (out == 8) return false;
        // bottom {0.f,1.f,0.f,1.f} -> dot(p,box) = box.y + box.w -> box.y < -box.w
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
		{ if (box_CS[corner_id].y < -box_CS[corner_id].w) { out++; } }
		if (out == 8) return false;
        // top {0.f,-1.f,0.f,1.f} -> dot(p,box) = -box.y + box.w -> box.y > box.w
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
		{ if (box_CS[corner_id].y > box_CS[corner_id].w) { out++; } }
		if (out == 8) return false;

        // check whether the frustum is fully out at least one plane of the box
        // todo: figure out whether ndc check here introduces issues with points behind the camera
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
		{ if (frustum[corner_id].z < boxmin_NDC.z) { out++; } }
		if (out == 8) return false;
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
	    { if (frustum[corner_id].z > boxmax_NDC.z) { out++; } }
		if (out == 8) return false;
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
		{ if (frustum[corner_id].x < boxmin_NDC.z) { out++; } }
		if (out == 8) return false;
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
		{ if (frustum[corner_id].x > boxmax_NDC.z) { out++; } }
		if (out == 8) return false;
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
        { if (frustum[corner_id].y < boxmin_NDC.z) { out++; } }
		if (out == 8) return false;
        out = 0;
		for (u32 corner_id = 0; corner_id < 8; corner_id++) 
		{ if (frustum[corner_id].y > boxmax_NDC.z) { out++; } }
		if (out == 8) return false;

        return true;
    };

    for (u32 n = 0, count = 0; n < scene.drawNodes.cap && count < scene.drawNodes.count; n++) {
        if (scene.drawNodes.data[n].alive == 0) { continue; }
        count++;

        const DrawNode& node = scene.drawNodes.data[n].state.live;
        if (cull_isVisible(math::mult(vpMatrix, node.nodeData.worldMatrix), node.min, node.max)) {
            visibleNodes.visible_nodes[visibleNodes.visible_nodes_count++] = n;
            isEachNodeVisible[n] = true;
        }
    }
}
struct SortParams {
    struct Type { enum Enum { Default, BackToFront }; };
    SortParams::Type::Enum type;
    SortKeyValue depthMask;
    SortKeyValue drawNodeMask;
    SortKeyValue shaderTechniqueMask;
    SortKeyValue distSqNormalized;
    SortKeyValue maxDistValue;
    f32 maxDistSq;
    u32 drawNodeBits;
    u32 depthBits;
    u32 shaderTechniqueBits;
};
void makeSortKeyBitParams(
    SortParams& params,
    const SortParams::Type::Enum sortType)
{
    params.drawNodeBits = DrawNodeMeta::HandleBits;
    params.drawNodeMask = (1 << params.drawNodeBits) - 1;
    params.shaderTechniqueBits = ShaderTechniques::Bits;
    params.shaderTechniqueMask = (1 << params.shaderTechniqueBits) - 1;
    params.type = sortType;
    switch (sortType) {
    case SortParams::Type::Default: {
        // very rough depth sorting, 10 bits for 1000 units, sort granularity is 1.023 units
        params.depthBits = 10;
        params.depthMask = (1 << params.depthBits) - 1;
        params.maxDistValue = (1 << params.depthBits) - 1;
        params.maxDistSq = 1000.f * 1000.f;
        assert(DrawNodeMeta::HandleBits + ShaderTechniques::Bits + 10 < sizeof(SortKeyValue) * 8);
    }
    break;
    case SortParams::Type::BackToFront: {
        // very rough depth sorting, 14 bits for 1000 units, sort granularity is 0.06 units,
        // but we only consider mesh centers
        params.depthBits = 14;
        params.depthMask = (1 << params.depthBits) - 1;
        params.maxDistValue = (1 << params.depthBits) - 1;
        params.maxDistSq = 1000.f * 1000.f; // todo: based on camera distance?
        assert(DrawNodeMeta::HandleBits + ShaderTechniques::Bits + 10 < sizeof(SortKeyValue) * 8);
    }
    break;
    };
}
void makeSortKeyDistParams(SortParams& params, const f32 distSq) {
    switch (params.type) {
    case SortParams::Type::Default: {
        params.distSqNormalized =
            SortKeyValue(params.maxDistValue * math::min(distSq / params.maxDistSq, 1.f));
    } break;
    case SortParams::Type::BackToFront: {
        params.distSqNormalized =
            ~SortKeyValue(params.maxDistValue * math::min(distSq / params.maxDistSq, 1.f));
    } break;
    }
}
SortKeyValue makeSortKey(const u32 nodeIdx, const u32 meshType, const SortParams& params) {
    SortKeyValue key = 0;
    SortKeyValue curr_shift = 0;
    switch (params.type) {
    case SortParams::Type::Default: {
        key |= (nodeIdx & params.drawNodeMask) << curr_shift;
        curr_shift += params.drawNodeBits;
        key |= (meshType & params.shaderTechniqueMask) << curr_shift;
        curr_shift += params.shaderTechniqueBits;
        //if (params.drawNodeType != DrawNodeMeta::TypeInstanced) {
        //    key |= (params.distSqNormalized & params.depthMask) << curr_shift;
        //    curr_shift += params.depthBits;
        //}
    }
    break;
    case SortParams::Type::BackToFront: {
        key |= (nodeIdx & params.drawNodeMask) << curr_shift;
        curr_shift += params.drawNodeBits;
        key |= (meshType & params.shaderTechniqueMask) << curr_shift;
        curr_shift += params.shaderTechniqueBits;
        key |= (params.distSqNormalized & params.depthMask) << curr_shift;
        curr_shift += params.depthBits;
    }
    break;
    }
    return key;
}
int partition(SortKey* keys, s32 low, s32 high) {
    SortKeyValue p = keys[low].v;
    int i = low;
    int j = high;
    while (i < j) {
        while (keys[i].v <= p && i <= high - 1) { i++; }
        while (keys[j].v > p && j >= low + 1) { j--; }
        if (i < j) {
            SortKey temp = keys[i];
            keys[i] = keys[j];
            keys[j] = temp;
        }
    }
    SortKey temp = keys[low];
    keys[low] = keys[j];
    keys[j] = temp;
    return j;
};
void qsort_s64(SortKey* keys, s32 low, s32 high) {
    if (low < high) {
        int pi = partition(keys, low, high);
        qsort_s64(keys, low, pi - 1);
        qsort_s64(keys, pi + 1, high);
    };
};

// testing only
void qsort_key(SortKey* keys, s32 low, s32 high) {
    qsort(keys, 0, high, sizeof(SortKey),
        [](const void* a, const void* b) {
            return (s64)((*(const SortKey*)a).v - (*(const SortKey*)b).v);
        });
};

void addNodesToDrawlistSorted(Drawlist& dl, const VisibleNodes& visibleNodes, float3 cameraPos,
                              Scene& scene, const u32 includeFilter, const u32 excludeFilter,
                              const SortParams::Type::Enum sortType) {

    SortParams sortParams;
    makeSortKeyBitParams(sortParams, sortType);
        
    for (u32 i = 0; i < visibleNodes.visible_nodes_count; i++) {
        u32 n = visibleNodes.visible_nodes[i];
        DrawNode& node = scene.drawNodes.data[n].state.live;
            
        if (includeFilter & DrawlistFilter::Alpha && node.nodeData.groupColor.w == 1.f) continue;
        if (excludeFilter & DrawlistFilter::Alpha && node.nodeData.groupColor.w < 1.f) continue;
            
        f32 distSq = math::magSq(math::subtract(node.nodeData.worldMatrix.col3.xyz, cameraPos));
        makeSortKeyDistParams(sortParams, distSq);
            
        for (u32 m = 0; m < countof(node.meshHandles); m++) {
            if (node.meshHandles[m] == 0) { continue; }
            u32 dl_index = dl.count[DrawlistBuckets::Base]++;
            DrawCall_Item& item = dl.items[dl_index];
            item = {};
            SortKey& key = dl.keys[dl_index];
            key.idx = dl_index;
            const DrawMesh& mesh = drawMesh_from_handle(scene, node.meshHandles[m]);
            key.v = makeSortKey(n, mesh.shaderTechnique, sortParams);
            item.shader = scene.shaders[mesh.shaderTechnique];
            item.vertexBuffer = mesh.vertexBuffer;
            item.cbuffers[item.cbuffer_count++] = cbuffer_from_handle(scene, node.cbuffer_node);
            if (node.cbuffer_ext) {
                item.cbuffers[item.cbuffer_count++] =
                    cbuffer_from_handle(scene, node.cbuffer_ext);
            }
            item.texture = mesh.texture;
            if (mesh.shaderTechnique == ShaderTechniques::Textured3DAlphaClip
                || mesh.shaderTechnique == ShaderTechniques::Textured3DAlphaClipSkinned) {
                item.blendState = scene.blendStateOn;
            } else {
                item.blendState = scene.blendStateBlendOff;
            }
            item.name = shaderNames[mesh.shaderTechnique];
        }
    }
    const bool addInstancedNodes = (includeFilter & DrawlistFilter::Mirror) == 0;
    if (addInstancedNodes) {
        for (u32 n = 0, count = 0;n < scene.instancedDrawNodes.cap && count < scene.instancedDrawNodes.count; n++) {
            if (scene.instancedDrawNodes.data[n].alive == 0) { continue; }
            count++;
            
            const DrawNodeInstanced& node = scene.instancedDrawNodes.data[n].state.live;
            
            if (includeFilter & DrawlistFilter::Alpha && node.nodeData.groupColor.w == 1.f) continue; 
            if (excludeFilter & DrawlistFilter::Alpha && node.nodeData.groupColor.w < 1.f) continue;
            
            for (u32 m = 0; m < countof(node.meshHandles); m++) {
                if (node.meshHandles[m] == 0) { continue; }
                u32 dl_index =
                    dl.count[DrawlistBuckets::Instanced]++ + dl.count[DrawlistBuckets::Base];
                DrawCall_Item& item = dl.items[dl_index];
                item = {};
                SortKey& key = dl.keys[dl_index];
                key = {};
                key.idx = dl_index;
                const DrawMesh& mesh = drawMesh_from_handle(scene, node.meshHandles[m]);
                key.v = makeSortKey(n, mesh.shaderTechnique, sortParams);
                item.shader = scene.shaders[mesh.shaderTechnique];
                item.vertexBuffer = mesh.vertexBuffer;
                item.cbuffers[item.cbuffer_count++] =
                    cbuffer_from_handle(scene, node.cbuffer_node);
                item.cbuffers[item.cbuffer_count++] =
                    cbuffer_from_handle(scene, node.cbuffer_instances);
                item.texture = mesh.texture;
                item.blendState = scene.blendStateBlendOff; // todo: support blendstates?
                item.drawcount = node.instanceCount;
                item.name = shaderNames[mesh.shaderTechnique];
            }
        }
    }

    qsort_s64(dl.keys,
              0,
              dl.count[DrawlistBuckets::Base] - 1);
    qsort_s64(dl.keys,
              dl.count[DrawlistBuckets::Base],
              dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced] - 1);
}
}

#endif // __WASTELADNS_DRAWLIST_H__
