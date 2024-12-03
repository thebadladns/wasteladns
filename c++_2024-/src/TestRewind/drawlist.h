#ifndef __WASTELADNS_DRAWLIST_H__
#define __WASTELADNS_DRAWLIST_H__

namespace renderer {

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
struct VertexLayout_Color_2D {
    float2 pos;
    u32 color;
};
struct VertexLayout_Color_3D {
    float3 pos;
    u32 color;
};
struct VertexLayout_Textured_3D {
    float3 pos;
    float2 uv;
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

#if USE_DEBUG_MEMORY
typedef u64 SortKeyValue;
typedef u32 DrawNodeHandle;
struct DrawNodeMeta { enum Enum : u64 {
          TypeInvalid = 0, TypeDefault = 1, TypeSkinned, TypeInstanced, TypesCount // invalid type is 0, to support 0 initialization
        , TypeBits = 2, HandleBits = sizeof(DrawNodeHandle) * 8 - TypeBits, TypeShift = HandleBits, TypeMask = (1 << TypeBits) - 1, HandleMask = (1 << HandleBits) - 1, MaxNodes = HandleMask
}; };
#else 
typedef u32 SortKeyValue;
typedef u32 DrawNodeHandle;
struct DrawNodeMeta { enum Enum : u32 {
          TypeInvalid = 0, TypeDefault = 1, TypeSkinned, TypeInstanced, TypesCount // invalid type is 0, to support 0 initialization
        , TypeBits = 2, HandleBits = 16, TypeShift = HandleBits, TypeMask = (1 << TypeBits) - 1, HandleMask = (1 << HandleBits) - 1, MaxNodes = HandleMask
}; };
#endif

struct SortKey {
    SortKeyValue v;
    s32 idx;
};
struct DrawlistFilter { enum Enum { Alpha = 1, Mirror = 2 }; };
struct DrawlistBuckets { enum Enum { Base, Instanced, Count }; };
struct Drawlist {
    SortKey* keys;
    DrawCall_Item* items;
    u32 count[DrawlistBuckets::Count];
};
struct DrawlistStreams { enum Enum { Color3D, Color3DSkinned, Textured3D, Textured3DAlphaClip, Textured3DSkinned, Textured3DAlphaClipSkinned, Count }; };

typedef u32 MeshHandle;
struct ShaderTechniques { enum Enum {
        FullscreenBlit,
        Instanced3D,
        Color3D, Color3DSkinned,
        Textured3D, Textured3DAlphaClip, Textured3DSkinned, Textured3DAlphaClipSkinned,
        Count, Bits = 3
}; };
const char* shaderNames[] = {
    "FullscreenBlit", "Instanced3D", "Color3D", "Color3DSkinned", "Textured3D", "Textured3DAlphaClip", "Textured3DSkinned", "Textured3DAlphaClipSkinned"
};
static_assert(countof(shaderNames) == ShaderTechniques::Count, "Make sure there are enough shaderNames strings as there are ShaderTechniques::Enum values");

struct DrawMesh { // id of a piece of geometry loaded on the gpu
    ShaderTechniques::Enum shaderTechnique;
    driver::RscIndexedVertexBuffer vertexBuffer;
    driver::RscTexture texture; // should this be here?
};
struct DrawNode { // List of meshes (one of each type), and their render data in the scene
    MeshHandle meshHandles[DrawlistStreams::Count];
    float3 min;
    float3 max;
    u32 cbuffer_node;
    NodeData nodeData;
};
struct DrawNodeSkinned {
    DrawNode core;
    u32 cbuffer_skinning;
    Matrices32 skinningMatrixPalette;
};
struct DrawNodeInstanced {
    DrawNode core;
    u32 cbuffer_instances;
    Matrices64 instanceMatrices; // todo: dynamic?
    u32 instanceCount;
};

void draw_drawlist(Drawlist& dl, Drawlist_Context& ctx, const Drawlist_Overrides& overrides) {
	u32 count = dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced];
    for (u32 i = 0; i < count; i++) {
        DrawCall_Item& item = dl.items[dl.keys[i].idx];
        driver::Marker_t marker;
        driver::set_marker_name(marker, item.name);
        renderer::driver::start_event(marker);
        {
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
            driver::bind_cbuffers(*ctx.shader, ctx.cbuffers, item.cbuffer_count + overrides.forced_cbuffer_count);
			if (item.drawcount) {
                driver::draw_instances_indexed_vertex_buffer(item.vertexBuffer, item.drawcount);
            } else {
                driver::draw_indexed_vertex_buffer(item.vertexBuffer);
            }
        }
        renderer::driver::end_event();
    }
}
struct Scene {
    struct Mirror { // todo: make this not so temp
        float3 normal;
        DrawNodeHandle drawHandle;
    };
    struct Sky {
        driver::RscCBuffer cbuffer;
        NodeData nodeData;
        driver::RscIndexedVertexBuffer buffers[6];
    };
    driver::RscShaderSet shaders[ShaderTechniques::Count];
	allocator::Arena persistentArena;
    allocator::Pool<DrawNode> drawNodes;
    allocator::Pool<DrawNodeSkinned> drawNodesSkinned;
    allocator::Pool<DrawNodeInstanced> drawNodesInstanced;
    allocator::Pool<DrawMesh> meshes;
    allocator::Pool<driver::RscCBuffer> cbuffers;
    renderer::driver::RscRasterizerState rasterizerStateFillFrontfaces, rasterizerStateFillBackfaces, rasterizerStateFillCullNone, rasterizerStateLine;
    renderer::driver::RscDepthStencilState depthStateOn, depthStateReadOnly, depthStateOff;
    renderer::driver::RscDepthStencilState depthStateMarkMirrors, depthStateMirrorReflections, depthStateMirrorReflectionsDepthReadOnly;
    renderer::driver::RscBlendState blendStateOn, blendStateBlendOff, blendStateOff;
    renderer::driver::RscMainRenderTarget windowRT;
    renderer::driver::RscRenderTarget gameRT;
    Mirror mirror;
    renderer::WindowProjection windowProjection;
    renderer::PerspProjection perspProjection;
    Sky sky;
    u32 cbuffer_scene;
};

DrawNodeHandle handle_from_node(Scene& scene, DrawNode& node) {
    return (DrawNodeMeta::TypeDefault << DrawNodeMeta::TypeShift) | (allocator::get_pool_index(scene.drawNodes, node) & DrawNodeMeta::HandleMask);
}
DrawNodeHandle handle_from_node(Scene& scene, DrawNodeSkinned& node) {
    return (DrawNodeMeta::TypeSkinned << DrawNodeMeta::TypeShift) | (allocator::get_pool_index(scene.drawNodesSkinned, node) & DrawNodeMeta::HandleMask);
}
DrawNodeHandle handle_from_node(Scene& scene, DrawNodeInstanced& node) {
    return  (DrawNodeMeta::TypeInstanced << DrawNodeMeta::TypeShift) | (allocator::get_pool_index(scene.drawNodesInstanced, node) & DrawNodeMeta::HandleMask);
}
DrawNode& get_draw_node_core(Scene& scene, const DrawNodeHandle meshHandle) {
    if ((meshHandle >> DrawNodeMeta::TypeShift) == DrawNodeMeta::TypeSkinned) {
        DrawNodeSkinned& node = allocator::get_pool_slot(scene.drawNodesSkinned, meshHandle & DrawNodeMeta::HandleMask);
        return node.core;
    } else if ((meshHandle >> DrawNodeMeta::TypeShift) == DrawNodeMeta::TypeInstanced) {
        DrawNodeInstanced& node = allocator::get_pool_slot(scene.drawNodesInstanced, meshHandle & DrawNodeMeta::HandleMask);
        return node.core;
    } else { // if ((meshHandle >> DrawNodeMeta::TypeShift) == DrawNodeMeta::TypeDefault) {
        DrawNode& node = allocator::get_pool_slot(scene.drawNodes, meshHandle & DrawNodeMeta::HandleMask);
        return node;
    }
}
Matrices32& get_draw_skinning_data(Scene& scene, const DrawNodeHandle nodeHandle) {
    //if ((meshHandle >> DrawNodeMeta::TypeShift) == DrawNodeMeta::TypeSkinned) {
    DrawNodeSkinned& node = allocator::get_pool_slot(scene.drawNodesSkinned, nodeHandle & DrawNodeMeta::HandleMask);
    return node.skinningMatrixPalette;
}
void get_draw_instanced_data(Matrices64*& matrices, u32*& count, Scene& scene, const u32 nodeHandle) {
    //if ((meshHandle >> DrawNodeMeta::TypeShift) == DrawNodeMeta::TypeInstanced) {
    DrawNodeInstanced& node = allocator::get_pool_slot(scene.drawNodesInstanced, nodeHandle & DrawNodeMeta::HandleMask);
    matrices = &node.instanceMatrices;
	count = &node.instanceCount;
}
DrawMesh get_drawMesh(Scene& scene, const u32 meshHandle) { return allocator::get_pool_slot(scene.meshes, meshHandle - 1); }
u32 handle_from_drawMesh(Scene& scene, DrawMesh& mesh) { return allocator::get_pool_index(scene.meshes, mesh) + 1; }

struct SortParams {
    struct Type { enum Enum { Default, BackToFront }; };
    SortParams::Type::Enum type;
    DrawNodeMeta::Enum drawNodeType;
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
void makeSortKeyBitParams(SortParams& params, const DrawNodeMeta::Enum drawNodeType, const SortParams::Type::Enum sortType) {
    params.drawNodeBits = DrawNodeMeta::HandleBits;
    params.drawNodeMask = (1 << params.drawNodeBits) - 1;
    params.shaderTechniqueBits = ShaderTechniques::Bits;
    params.shaderTechniqueMask = (1 << params.shaderTechniqueBits) - 1;
    params.type = sortType;
    params.drawNodeType = drawNodeType;
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
        // very rough depth sorting, 14 bits for 1000 units, sort granularity is 0.06 units, but we only consider mesh centers
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
        params.distSqNormalized = SortKeyValue(params.maxDistValue * math::min(distSq / params.maxDistSq, 1.f));
    }
    break;
    case SortParams::Type::BackToFront: {
        params.distSqNormalized = ~SortKeyValue(params.maxDistValue * math::min(distSq / params.maxDistSq, 1.f));
    }
    break;
    }
}
SortKeyValue makeSortKey(const u32 nodeIdx, const u32 meshType, const SortParams& params) {
    SortKeyValue key = 0;
    SortKeyValue curr_shift = 0;
    switch (params.type) {
    case SortParams::Type::Default: {
        key |= (nodeIdx & params.drawNodeMask) << curr_shift, curr_shift += params.drawNodeBits;
        key |= (meshType & params.shaderTechniqueMask) << curr_shift, curr_shift += params.shaderTechniqueBits;
        //if (params.drawNodeType != DrawNodeMeta::TypeInstanced) {
        //    key |= (params.distSqNormalized & params.depthMask) << curr_shift, curr_shift += params.depthBits;
        //}
    }
    break;
    case SortParams::Type::BackToFront: {
        key |= (nodeIdx & params.drawNodeMask) << curr_shift, curr_shift += params.drawNodeBits;
        key |= (meshType & params.shaderTechniqueMask) << curr_shift, curr_shift += params.shaderTechniqueBits;
        key |= (params.distSqNormalized & params.depthMask) << curr_shift, curr_shift += params.depthBits;
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
void qsort(SortKey* keys, s32 low, s32 high) {
    if (low < high) {
        int pi = partition(keys, low, high);
        qsort(keys, low, pi - 1);
        qsort(keys, pi + 1, high);
    };
};

struct VisibleNodes {
    u32* visible_nodes;
    u32* visible_nodes_skinned;
    u32* is_node_visible;
    u32* is_skinnednode_visible;
    u32 visible_nodes_count;
    u32 visible_nodes_skinned_count;
};
void allocVisibleNodes(VisibleNodes& visibleNodes, u32 nodes, u32 nodesSkinned, allocator::Arena& arena) {
    visibleNodes.visible_nodes = (u32*)allocator::alloc_arena(arena, nodes * sizeof(u32), alignof(u32));
    visibleNodes.is_node_visible = (u32*)allocator::alloc_arena(arena, nodes * sizeof(u32), alignof(u32));
    visibleNodes.visible_nodes_skinned = (u32*)allocator::alloc_arena(arena, nodesSkinned * sizeof(u32), alignof(u32));
    visibleNodes.is_skinnednode_visible = (u32*)allocator::alloc_arena(arena, nodesSkinned * sizeof(u32), alignof(u32));
    memset(visibleNodes.visible_nodes, 0, nodes * sizeof(u32));
    memset(visibleNodes.is_node_visible, 0, nodes * sizeof(u32));
    memset(visibleNodes.visible_nodes_skinned, 0, nodesSkinned * sizeof(u32));
    memset(visibleNodes.is_skinnednode_visible, 0, nodesSkinned * sizeof(u32));

};
void computeVisibility(VisibleNodes& visibleNodes, float4x4& vpMatrix, Scene& scene) {
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
        float4 box_CS[8]; float3 boxmin_NDC = { FLT_MAX, FLT_MAX, FLT_MAX }, boxmax_NDC = { -FLT_MAX,-FLT_MAX,-FLT_MAX };
        for (u32 corner_id = 0; corner_id < 8; corner_id++) {
            box_CS[corner_id] = math::mult(mvp, box[corner_id]);
            boxmax_NDC = math::max(math::invScale(box_CS[corner_id].xyz, box_CS[corner_id].w), boxmax_NDC);
            boxmin_NDC = math::min(math::invScale(box_CS[corner_id].xyz, box_CS[corner_id].w), boxmin_NDC);
        }
        u32 out;

        // check whether the box is fully out of at least one frustum plane
        // near plane { 0.f, 0.f, 1.f, -renderer::min_z } -> dot(p,box) = box.z - renderer::min_z & box.w -> box.z < -renderer::min_z & box.w
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (box_CS[corner_id].z < renderer::min_z * box_CS[corner_id].w) { out++; } } if (out == 8) return false;
        // far plane { 0.f, 0.f, -1.f, 1.f } -> dot(p,box) = -box.z + box.w -> box.z > box.w
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (box_CS[corner_id].z > box_CS[corner_id].w) { out++; } } if (out == 8) return false;
        // left plane { 1.f, 0.f, 0.f, 1.f } -> dot(p,box) = box.x + box.w -> box.x < -box.w
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (box_CS[corner_id].x < -box_CS[corner_id].w) { out++; } } if (out == 8) return false;
        // right plane { 1.f, 0.f, 0.f, 1.f } -> dot(p,box) = -box.x + box.w -> box.x > box.w
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (box_CS[corner_id].x > box_CS[corner_id].w) { out++; } } if (out == 8) return false;
        // bottom plane { 0.f, 1.f, 0.f, 1.f } -> dot(p,box) = box.y + box.w -> box.y < -box.w
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (box_CS[corner_id].y < -box_CS[corner_id].w) { out++; } } if (out == 8) return false;
        // top plane { 0.f, -1.f, 0.f, 1.f } -> dot(p,box) = -box.y + box.w -> box.y > box.w
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (box_CS[corner_id].y > box_CS[corner_id].w) { out++; } } if (out == 8) return false;

        // check whether the frustum is fully out at least one plane of the box
        // todo: figure out whether ndc check here introduces issues with elements behind the camera
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (frustum[corner_id].z < boxmin_NDC.z) { out++; } } if (out == 8) return false;
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (frustum[corner_id].z > boxmax_NDC.z) { out++; } } if (out == 8) return false;
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (frustum[corner_id].x < boxmin_NDC.z) { out++; } } if (out == 8) return false;
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (frustum[corner_id].x > boxmax_NDC.z) { out++; } } if (out == 8) return false;
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (frustum[corner_id].y < boxmin_NDC.z) { out++; } } if (out == 8) return false;
        out = 0; for (u32 corner_id = 0; corner_id < 8; corner_id++) { if (frustum[corner_id].y > boxmax_NDC.z) { out++; } } if (out == 8) return false;

        return true;
    };

    for (u32 n = 0, count = 0; n < scene.drawNodes.cap && count < scene.drawNodes.count; n++) {
        if (scene.drawNodes.data[n].alive == 0) { continue; }
        count++;

        const DrawNode& node = scene.drawNodes.data[n].state.live;
        if (cull_isVisible(math::mult(vpMatrix, node.nodeData.worldMatrix), node.min, node.max)) {
            visibleNodes.visible_nodes[visibleNodes.visible_nodes_count++] = n;
            visibleNodes.is_node_visible[n] = true;
        }

    }
    for (u32 n = 0, count = 0; n < scene.drawNodesSkinned.cap && count < scene.drawNodesSkinned.count; n++) {
        if (scene.drawNodesSkinned.data[n].alive == 0) { continue; }
        count++;

        const DrawNodeSkinned& node = scene.drawNodesSkinned.data[n].state.live;
        if (cull_isVisible(math::mult(vpMatrix, node.core.nodeData.worldMatrix), node.core.min, node.core.max)) {
            visibleNodes.visible_nodes_skinned[visibleNodes.visible_nodes_skinned_count++] = n;
            visibleNodes.is_skinnednode_visible[n] = true;
        }
    }
}

void addNodesToDrawlistSorted(Drawlist& dl, const VisibleNodes& visibleNodes, float3 cameraPos, Scene& scene, const u32 includeFilter, const u32 excludeFilter, const SortParams::Type::Enum sortType) {

    {
        SortParams sortParams;
        makeSortKeyBitParams(sortParams, DrawNodeMeta::TypeDefault, sortType);
        
        for (u32 i = 0; i < visibleNodes.visible_nodes_count; i++) {
            u32 n = visibleNodes.visible_nodes[i];
            DrawNode& node = scene.drawNodes.data[n].state.live;
            const u32 maxMeshCount = countof(node.meshHandles);
            
            if (includeFilter & DrawlistFilter::Alpha) { if (node.nodeData.groupColor.w == 1.f) { continue; } }
            if (excludeFilter & DrawlistFilter::Alpha) { if (node.nodeData.groupColor.w < 1.f) { continue; } }
            if (includeFilter & DrawlistFilter::Mirror) { if (scene.mirror.drawHandle != handle_from_node(scene, node)) { continue; } }
            if (excludeFilter & DrawlistFilter::Mirror) { if (scene.mirror.drawHandle == handle_from_node(scene, node)) { continue; } }
            
            f32 distSq = math::magSq(math::subtract(node.nodeData.worldMatrix.col3.xyz, cameraPos));
            makeSortKeyDistParams(sortParams, distSq);
            
            for (u32 m = 0; m < maxMeshCount; m++) {
                if (node.meshHandles[m] == 0) { continue; }
                u32 dl_index = dl.count[DrawlistBuckets::Base]++;
                DrawCall_Item& item = dl.items[dl_index];
                item = {};
                SortKey& key = dl.keys[dl_index];
                key.idx = dl_index;
                const DrawMesh& mesh = get_drawMesh(scene, node.meshHandles[m]);
                key.v = makeSortKey(n, mesh.shaderTechnique, sortParams);
                item.shader = scene.shaders[mesh.shaderTechnique];
                item.vertexBuffer = mesh.vertexBuffer;
                item.cbuffers[item.cbuffer_count++] = scene.cbuffers.data[node.cbuffer_node].state.live;
                item.texture = mesh.texture;
                item.blendState = mesh.shaderTechnique == ShaderTechniques::Textured3DAlphaClip ? scene.blendStateOn : scene.blendStateBlendOff;
                item.name = shaderNames[mesh.shaderTechnique];
            }
        }
    }
    const bool addSkinnedNodes = (includeFilter & DrawlistFilter::Mirror) == 0;
    if (addSkinnedNodes)
    {
        SortParams sortParams;
        makeSortKeyBitParams(sortParams, DrawNodeMeta::TypeSkinned, sortType);
        for (u32 i = 0; i < visibleNodes.visible_nodes_skinned_count; i++) {
            u32 n = visibleNodes.visible_nodes_skinned[i];
            const DrawNodeSkinned& node = scene.drawNodesSkinned.data[n].state.live;
            const u32 maxMeshCount = countof(node.core.meshHandles);
            
            if (includeFilter & DrawlistFilter::Alpha) { if (node.core.nodeData.groupColor.w == 1.f) { continue; } }
            if (excludeFilter & DrawlistFilter::Alpha) { if (node.core.nodeData.groupColor.w < 1.f) { continue; } }
            
            f32 distSq = math::magSq(math::subtract(node.core.nodeData.worldMatrix.col3.xyz, cameraPos));
            makeSortKeyDistParams(sortParams, distSq);
            
            for (u32 m = 0; m < maxMeshCount; m++) {
                if (node.core.meshHandles[m] == 0) { continue; }
                u32 dl_index = dl.count[DrawlistBuckets::Base]++;
                DrawCall_Item& item = dl.items[dl_index];
                item = {};
                SortKey& key = dl.keys[dl_index];
                key = {};
                key.idx = dl_index;
                const DrawMesh& mesh = get_drawMesh(scene, node.core.meshHandles[m]);
                key.v = makeSortKey(n, mesh.shaderTechnique, sortParams);
                item.shader = scene.shaders[mesh.shaderTechnique];
                item.vertexBuffer = mesh.vertexBuffer;
                item.cbuffers[item.cbuffer_count++] = scene.cbuffers.data[node.core.cbuffer_node].state.live;
                item.cbuffers[item.cbuffer_count++] = scene.cbuffers.data[node.cbuffer_skinning].state.live;
                item.texture = mesh.texture;
                item.blendState = mesh.shaderTechnique == ShaderTechniques::Textured3DAlphaClipSkinned ? scene.blendStateOn : scene.blendStateBlendOff;
                item.name = shaderNames[mesh.shaderTechnique];
            }
        }
    }
    const bool addInstancedNodes = (includeFilter & DrawlistFilter::Mirror) == 0;
    if (addInstancedNodes)
    {
        SortParams sortParams;
        makeSortKeyBitParams(sortParams, DrawNodeMeta::TypeInstanced, sortType);
        for (u32 n = 0, count = 0; n < scene.drawNodesInstanced.cap && count < scene.drawNodesInstanced.count; n++) {
            if (scene.drawNodesInstanced.data[n].alive == 0) { continue; }
            count++;
            
            const DrawNodeInstanced& node = scene.drawNodesInstanced.data[n].state.live;
            const u32 maxMeshCount = countof(node.core.meshHandles);
            
            if (includeFilter & DrawlistFilter::Alpha) { if (node.core.nodeData.groupColor.w == 1.f) { continue; } }
            if (excludeFilter & DrawlistFilter::Alpha) { if (node.core.nodeData.groupColor.w < 1.f) { continue; } }
            
            for (u32 m = 0; m < maxMeshCount; m++) {
                if (node.core.meshHandles[m] == 0) { continue; }
                u32 dl_index = dl.count[DrawlistBuckets::Instanced]++ + dl.count[DrawlistBuckets::Base];
                DrawCall_Item& item = dl.items[dl_index];
                item = {};
                SortKey& key = dl.keys[dl_index];
                key = {};
                key.idx = dl_index;
                const DrawMesh& mesh = get_drawMesh(scene, node.core.meshHandles[m]);
                key.v = makeSortKey(n, mesh.shaderTechnique, sortParams);
                item.shader = scene.shaders[mesh.shaderTechnique];
                item.vertexBuffer = mesh.vertexBuffer;
                item.cbuffers[item.cbuffer_count++] = scene.cbuffers.data[node.core.cbuffer_node].state.live;
                item.cbuffers[item.cbuffer_count++] = scene.cbuffers.data[node.cbuffer_instances].state.live;
                item.texture = mesh.texture;
                item.blendState = scene.blendStateBlendOff; // todo: support other blendstates when doing instances
                item.drawcount = node.instanceCount;
                item.name = shaderNames[mesh.shaderTechnique];
            }
        }
    }

    // sort
    qsort(dl.keys, 0, dl.count[DrawlistBuckets::Base] - 1);
    qsort(dl.keys, dl.count[DrawlistBuckets::Base], dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced] - 1);
}
}

#endif // __WASTELADNS_DRAWLIST_H__
