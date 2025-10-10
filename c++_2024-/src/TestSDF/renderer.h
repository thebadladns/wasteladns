#ifndef __WASTELADNS_RENDERER_H__
#define __WASTELADNS_RENDERER_H__

namespace renderer {

// vertex input layouts

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
// create the input layout attributes for each vertex layout, as variables on the stack
#define LOAD_VERTEX_LAYOUT_ATTRIBS \
    const gfx::rhi::VertexAttribDesc attribs_2d[] = { \
        gfx::rhi::make_vertexAttribDesc( \
            "POSITION", offsetof(renderer::VertexLayout_Color_2D, pos), \
            sizeof(renderer::VertexLayout_Color_2D), \
            gfx::rhi::BufferAttributeFormat::R32G32_FLOAT), \
        gfx::rhi::make_vertexAttribDesc( \
            "COLOR", offsetof(renderer::VertexLayout_Color_2D, color), \
            sizeof(renderer::VertexLayout_Color_2D), \
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM) }; \
    const gfx::rhi::VertexAttribDesc attribs_3d[] = { \
        gfx::rhi::make_vertexAttribDesc( \
            "POSITION", 0, \
            sizeof(float3), \
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT) }; \
    const gfx::rhi::VertexAttribDesc attribs_color3d[] = { \
        gfx::rhi::make_vertexAttribDesc( \
			"POSITION", offsetof(renderer::VertexLayout_Color_3D, pos), \
            sizeof(renderer::VertexLayout_Color_3D), \
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT), \
        gfx::rhi::make_vertexAttribDesc( \
			"COLOR", offsetof(renderer::VertexLayout_Color_3D, color), \
            sizeof(renderer::VertexLayout_Color_3D), \
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM) }; \
    const gfx::rhi::VertexAttribDesc attribs_color3d_skinned[] = { \
        gfx::rhi::make_vertexAttribDesc( \
			"POSITION", offsetof(renderer::VertexLayout_Color_Skinned_3D, pos), \
            sizeof(renderer::VertexLayout_Color_Skinned_3D), \
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT), \
        gfx::rhi::make_vertexAttribDesc( \
			"COLOR", offsetof(renderer::VertexLayout_Color_Skinned_3D, color), \
            sizeof(renderer::VertexLayout_Color_Skinned_3D), \
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM), \
        gfx::rhi::make_vertexAttribDesc( \
			"JOINTINDICES", offsetof(renderer::VertexLayout_Color_Skinned_3D, joint_indices), \
            sizeof(renderer::VertexLayout_Color_Skinned_3D), \
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_SINT), \
        gfx::rhi::make_vertexAttribDesc( \
			"JOINTWEIGHTS", offsetof(renderer::VertexLayout_Color_Skinned_3D, joint_weights), \
            sizeof(renderer::VertexLayout_Color_Skinned_3D), \
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM) }; \
    const gfx::rhi::VertexAttribDesc attribs_textured3d[] = { \
        gfx::rhi::make_vertexAttribDesc( \
			"POSITION", offsetof(renderer::VertexLayout_Textured_3D, pos), \
            sizeof(renderer::VertexLayout_Textured_3D), \
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT), \
        gfx::rhi::make_vertexAttribDesc( \
			"TEXCOORD", offsetof(renderer::VertexLayout_Textured_3D, uv), \
            sizeof(renderer::VertexLayout_Textured_3D), \
            gfx::rhi::BufferAttributeFormat::R32G32_FLOAT) }; \
    const gfx::rhi::VertexAttribDesc attribs_textured3d_skinned[] = { \
        gfx::rhi::make_vertexAttribDesc( \
			"POSITION", offsetof(renderer::VertexLayout_Textured_Skinned_3D, pos), \
            sizeof(renderer::VertexLayout_Textured_Skinned_3D), \
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT), \
        gfx::rhi::make_vertexAttribDesc( \
			"TEXCOORD", offsetof(renderer::VertexLayout_Textured_Skinned_3D, uv), \
            sizeof(renderer::VertexLayout_Textured_Skinned_3D), \
            gfx::rhi::BufferAttributeFormat::R32G32_FLOAT), \
        gfx::rhi::make_vertexAttribDesc( \
			"JOINTINDICES", offsetof(renderer::VertexLayout_Textured_Skinned_3D, joint_indices), \
            sizeof(renderer::VertexLayout_Textured_Skinned_3D), \
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_SINT), \
        gfx::rhi::make_vertexAttribDesc( \
			"JOINTWEIGHTS", offsetof(renderer::VertexLayout_Textured_Skinned_3D, joint_weights), \
            sizeof(renderer::VertexLayout_Textured_Skinned_3D), \
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM) \
    };

// cbuffers

struct CBuffer_Binding { enum { Binding_0 = 0, Binding_1 = 1, Binding_2 = 2, Count }; };

struct CBufferBlitColor {
    float4 color;
};
struct CBufferSDF {
    float4 viewMatrix0;
    float4 viewMatrix1;
    float4 viewMatrix2;
    float4 viewMatrix3;
    // row necessary to project the z coordinate. Note that we need more than the z and w components
    // of this row in the case of oblique projections (used in mirrors)
    float4 proj_row2; 
    float tanfov; float aspect; float near; float far;
    float time; float platformSize; float2 padding;
};
struct CBufferSceneData {
    float4x4 vpMatrix;
};
struct CBufferNodeData {
    float4x4 worldMatrix;
    float4 groupColor;
};

struct Drawlist_Context {
    gfx::rhi::RscCBuffer cbuffers[CBuffer_Binding::Count];
    gfx::rhi::RscShaderSet* shader;
    gfx::rhi::RscTexture* texture;
    gfx::rhi::RscBlendState* blendState;
    gfx::rhi::RscIndexedVertexBuffer* vertexBuffer;
};
struct Drawlist_Overrides {
    bool forced_shader;
    bool forced_texture;
    bool forced_blendState;
    bool forced_vertexBuffer;
    u32 forced_cbuffer_count;
};

struct DrawCall_Item {
    gfx::rhi::RscShaderSet shader;
    gfx::rhi::RscTexture texture;
    gfx::rhi::RscBlendState blendState;
    gfx::rhi::RscIndexedVertexBuffer vertexBuffer;
    gfx::rhi::RscCBuffer cbuffers[2];
    const char* name;
    u32 cbuffer_count;
    u32 drawcount;
};

typedef u64 SortKeyValue;
typedef u32 DrawNodeHandle;
struct DrawNodeMeta { enum Enum : u32 {
    HandleBits = 32, HandleMask = 0xffffffff, MaxNodes = HandleMask - 1
}; };
typedef u32 InstancedNodeHandle;

struct SortKey { SortKeyValue v; s32 idx; };
struct DrawlistType { enum Enum {
    Alpha = 1 << 0, BeforeShadows = 1 << 1, AfterShadows = 1 << 2}; };

struct Drawlist {
    SortKey* keys;
    DrawCall_Item* items;
    u32 count;
};

typedef u32 MeshHandle;

struct ShaderPrograms { enum Enum { NullShader = -1,
    FullscreenBlitClearColor = 0, FullscreenBlitSDFScene,
    FullscreenBlitTextured, FullscreenBlitTexturedDepth,
    DefaultColor2D, DefaultInstanced3D, DefaultColor3D, DefaultColor3DSkinned,
    DefaultTextured3D, DefaultTextured3DAlphaClip,
    DefaultTextured3DSkinned, DefaultTextured3DAlphaClipSkinned,
    Count, Bits = math::ceillog2(Count) }; };

const char* shaderProgramNames[] = {
    "FullscreenBlitClearColor", "FullscreenBlitSDFScene",
    "FullscreenBlitTextured", "FullscreenBlitTexturedDepth",
    "Color2D", "Instanced3D", "Color3D", "Color3DSkinned",
    "Textured3D", "Textured3DAlphaClip", "Textured3DSkinned", "Textured3DAlphaClipSkinned"
};

static_assert(countof(shaderProgramNames) == ShaderPrograms::Count,
    "Make sure there are enough shaderShaderProgramNames strings as there are ShaderPrograms::Enum values");

// the different ways a technique can render
struct DrawType { enum Enum { Default, DefaultAlphaClip, Count }; };

// materials, of sorts
struct ShaderTechnique {
    enum Type { 
        Instanced, VertexColor, VertexColorSkinned,
        Diffuse, DiffuseSkinned, CutoutDiffuse, CutoutDiffuseSkinned, Count };
    ShaderPrograms::Enum shaders[DrawType::Count];
};

struct DrawMesh { // id of a piece of geometry loaded on the gpu
    ShaderTechnique::Type shaderTechnique;
    gfx::rhi::RscIndexedVertexBuffer vertexBuffer;
    gfx::rhi::RscTexture texture; // should this be here?
};
struct CPUMesh { // representation of a mesh, where all the vertices are on the cpu
    float3* vertices;
    u16* indices;
    u32 vertexCount;
    u32 indexCount;
};
struct Drawable {
    void* ext_data;
    float3 min;
    float3 max;
    float3 shadowpos;
    u32 instanceCount;
    u32 cbuffer_node;
    u32 cbuffer_ext;
    u32 meshCount;
    MeshHandle meshHandles[8];
    CBufferNodeData nodeData;
    DrawlistType::Enum drawlistType;
};

void draw_drawlist(Drawlist& dl, Drawlist_Context& ctx, const Drawlist_Overrides& overrides) {
    for (u32 i = 0; i < dl.count; i++) {
        DrawCall_Item& item = dl.items[dl.keys[i].idx];
        gfx::rhi::start_event(item.name);
        if (!overrides.forced_shader && ctx.shader != &item.shader) {
            gfx::rhi::bind_shader(item.shader);
            ctx.shader = &item.shader;
        }
		if (!overrides.forced_blendState && ctx.blendState != &item.blendState) {
			gfx::rhi::bind_blend_state(item.blendState);
			ctx.blendState = &item.blendState;
		}
        if (!overrides.forced_texture && ctx.texture != &item.texture) {
            gfx::rhi::bind_textures(&item.texture, 1);
            ctx.texture = &item.texture;
        }
        if (!overrides.forced_vertexBuffer && ctx.vertexBuffer != &item.vertexBuffer) {
            gfx::rhi::bind_indexed_vertex_buffer(item.vertexBuffer);
            ctx.vertexBuffer = &item.vertexBuffer;
        }
        for (u32 i = 0; i < item.cbuffer_count; i++) {
            ctx.cbuffers[i + overrides.forced_cbuffer_count] = item.cbuffers[i];
        }
        gfx::rhi::bind_cbuffers(
            *ctx.shader, ctx.cbuffers,item.cbuffer_count + overrides.forced_cbuffer_count);
		if (item.drawcount) {
            gfx::rhi::draw_instances_indexed_vertex_buffer(item.vertexBuffer, item.drawcount);
        } else {
            gfx::rhi::draw_indexed_vertex_buffer(item.vertexBuffer);
        }
        gfx::rhi::end_event();
    }
}

struct ReloadableShader {
    gfx::rhi::RscShaderSet shader;
    #if __DEBUG
        const char* vs_binFile;
        const char* ps_binFile;
        const char* vs_srcFile;
        const char* ps_srcFile;
        time_t vs_lastCompileTime;
        time_t ps_lastCompileTime;
    #endif
};

struct Scene {
    allocator::Pool<Drawable> drawables;
    allocator::Pool<gfx::rhi::RscCBuffer> cbuffers;
};
struct CoreResources {
    ReloadableShader shaderPrograms[ShaderPrograms::Count]; // raw shader programs used in draw calls
    ShaderTechnique shaderTechniques[ShaderTechnique::Count]; // techniques compiling several programs
    DrawMesh* meshes;
    u32 num_meshes;
    struct CBuffersMeta { enum {
        SDF, ClearColor, Scene, NodeIdentity, UIText, Instances64, Count }; };
    gfx::rhi::RscCBuffer cbuffers[CBuffersMeta::Count];
    gfx::rhi::RscRasterizerState rasterizerStateFillFrontfaces;
    gfx::rhi::RscRasterizerState rasterizerStateFillBackfaces;
    gfx::rhi::RscRasterizerState rasterizerStateFillCullNone;
    gfx::rhi::RscRasterizerState rasterizerStateLine;
    gfx::rhi::RscDepthStencilState depthStateOn;
    gfx::rhi::RscDepthStencilState depthStateReadOnly;
    gfx::rhi::RscDepthStencilState depthStateOff;
    gfx::rhi::RscDepthStencilState depthStateAlways;
    gfx::rhi::RscDepthStencilState depthStateMarkMirror;
    gfx::rhi::RscDepthStencilState depthStateUnmarkMirror;
    gfx::rhi::RscDepthStencilState depthStateMirrorReflections; 
    gfx::rhi::RscDepthStencilState depthStateMirrorReflectionsDepthReadOnly;
    gfx::rhi::RscDepthStencilState depthStateMirrorReflectionsDepthAlways;
    gfx::rhi::RscBlendState blendStateOn;
    gfx::rhi::RscBlendState blendStateBlendOff;
    gfx::rhi::RscBlendState blendStateOff;
    gfx::rhi::RscMainRenderTarget windowRT;
    gfx::rhi::RscRenderTarget gameRT;
    camera::WindowProjection windowProjection;
    camera::PerspProjection perspProjection;
};

force_inline DrawNodeHandle handle_from_drawable(Scene& scene, Drawable& node) {
    return allocator::get_pool_index(scene.drawables, node) + 1;
}
force_inline Drawable& drawable_from_handle(Scene& scene, const DrawNodeHandle handle) {
    return allocator::get_pool_slot(scene.drawables, handle - 1);
}
force_inline DrawMesh& alloc_drawMesh(CoreResources& core) {
    return core.meshes[core.num_meshes++];
}
force_inline DrawMesh& drawMesh_from_handle(CoreResources& core, const u32 handle) {
    return core.meshes[handle - 1];
}
force_inline u32 handle_from_drawMesh(CoreResources& core, DrawMesh& mesh) {
    return u32(&mesh - &core.meshes[0]) + 1;
}
force_inline gfx::rhi::RscCBuffer& cbuffer_from_handle(Scene& scene, const u32 handle) {
    return allocator::get_pool_slot(scene.cbuffers, handle - 1);
}
force_inline u32 handle_from_cbuffer(Scene& scene, gfx::rhi::RscCBuffer& cbuffer) {
    return allocator::get_pool_index(scene.cbuffers, cbuffer) + 1;
}

struct Frustum {
    enum { MAX_PLANE_COUNT = 10 };
    float4 planes[MAX_PLANE_COUNT];
    u32 numPlanes;
};
struct CullEntry {
    float3 boxPointsWS[8]; // todo: surely we can do better?
    u32 poolId;
};
struct CullEntries {
    CullEntry* entries;
    u32 count;
};
void allocCullEntries(allocator::PagedArena scratchArena, CullEntries& cullEntries, const Scene& scene) {
    
    cullEntries.entries = ALLOC_ARRAY(scratchArena, CullEntry, scene.drawables.count);

    for (u32 n = 0, count = 0; n < scene.drawables.cap && count < scene.drawables.count; n++) {
        if (scene.drawables.data[n].alive == 0) { continue; }
        count++;

        CullEntry& entry = cullEntries.entries[cullEntries.count++];
        const Drawable& node = scene.drawables.data[n].state.live;
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
void computeVisibilityWS(allocator::PagedArena& frameArena, VisibleNodes& visibilityFrustum,
                         u32* isEachNodeVisible, const Frustum& frustum,
                         const CullEntries& cullEntries) {

    allocator::Buffer<u32> visibleNodes = {};
    visibilityFrustum.visible_nodes_count = 0;
    for (u32 i = 0; i < cullEntries.count; i++) {
        const CullEntry& entry = cullEntries.entries[i];
        
        // quad inside frustum
        bool visible = true;
        for (u32 p = 0; p < frustum.numPlanes; p++) {
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
            {-1.f, -1.f, gfx::min_z},
            {1.f, -1.f, gfx::min_z},
            {-1.f, 1.f, gfx::min_z},
            {1.f, 1.f, gfx::min_z},
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
        { if (box_CS[corner_id].z < gfx::min_z * box_CS[corner_id].w) { out++; } }
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

    for (u32 n = 0, count = 0; n < scene.drawables.cap && count < scene.drawables.count; n++) {
        if (scene.drawables.data[n].alive == 0) { continue; }
        count++;

        const Drawable& node = scene.drawables.data[n].state.live;
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
    params.shaderTechniqueBits = ShaderPrograms::Bits;
    params.shaderTechniqueMask = (1 << params.shaderTechniqueBits) - 1;
    params.type = sortType;
    switch (sortType) {
    case SortParams::Type::Default: {
        // very rough depth sorting, 10 bits for 1000 units, sort granularity is 1.023 units
        params.depthBits = 10;
        params.depthMask = (1 << params.depthBits) - 1;
        params.maxDistValue = (1 << params.depthBits) - 1;
        params.maxDistSq = 1000.f * 1000.f;
        assert(DrawNodeMeta::HandleBits + ShaderPrograms::Bits + 10 < sizeof(SortKeyValue) * 8);
    }
    break;
    case SortParams::Type::BackToFront: {
        // very rough depth sorting, 14 bits for 1000 units, sort granularity is 0.06 units,
        // but we only consider mesh centers
        params.depthBits = 14;
        params.depthMask = (1 << params.depthBits) - 1;
        params.maxDistValue = (1 << params.depthBits) - 1;
        params.maxDistSq = 1000.f * 1000.f; // todo: based on camera distance?
        assert(DrawNodeMeta::HandleBits + ShaderPrograms::Bits + 10 < sizeof(SortKeyValue) * 8);
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
SortKeyValue makeSortKey(const u32 nodeIdx, const u32 shaderProgram, const SortParams& params) {
    SortKeyValue key = 0;
    SortKeyValue curr_shift = 0;
    switch (params.type) {
    case SortParams::Type::Default: {
        key |= (nodeIdx & params.drawNodeMask) << curr_shift;
        curr_shift += params.drawNodeBits;
        key |= (shaderProgram & params.shaderTechniqueMask) << curr_shift;
        curr_shift += params.shaderTechniqueBits;
        #if 0 // no depth sorting on opaques
        if (params.drawNodeType != DrawNodeMeta::TypeInstanced) {
            key |= (params.distSqNormalized & params.depthMask) << curr_shift;
            curr_shift += params.depthBits;
        }
        #endif
    }
    break;
    case SortParams::Type::BackToFront: {
        key |= (nodeIdx & params.drawNodeMask) << curr_shift;
        curr_shift += params.drawNodeBits;
        key |= (shaderProgram & params.shaderTechniqueMask) << curr_shift;
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

void addNodesToDrawlistSorted(
    Drawlist& dl, const VisibleNodes& visibleNodes, float3 cameraPos,
    Scene& scene, CoreResources& rsc, const u32 includeFilter, const u32 excludeFilter,
    const SortParams::Type::Enum sortType) {

    SortParams sortParams;
    makeSortKeyBitParams(sortParams, sortType);
        
    for (u32 i = 0; i < visibleNodes.visible_nodes_count; i++) {
        u32 n = visibleNodes.visible_nodes[i];
        Drawable& node = scene.drawables.data[n].state.live;
            
        // alpha filter is computed based on the current group color
        u32 nodeDrawlistType = node.drawlistType;
        if (node.nodeData.groupColor.w < 1.f) { nodeDrawlistType |= DrawlistType::Alpha; }
        else { nodeDrawlistType &= ~DrawlistType::Alpha; }

        if ((includeFilter & nodeDrawlistType) == 0) { continue; }
        if ((excludeFilter & nodeDrawlistType) != 0) { continue; }
            
        f32 distSq = math::magSq(math::subtract(node.nodeData.worldMatrix.col3.xyz, cameraPos));
        makeSortKeyDistParams(sortParams, distSq);

        // if on an alpha drawlist, or not an instanced draw, select alpha draw technique
        DrawType::Enum drawType =
            ((nodeDrawlistType & DrawlistType::Alpha) != 0 || node.instanceCount > 0) ?
            DrawType::Default : DrawType::DefaultAlphaClip;
            
        for (u32 m = 0; m < node.meshCount; m++) {
            u32 dl_index = dl.count++;
            DrawCall_Item& item = dl.items[dl_index];
            item = {};
            SortKey& key = dl.keys[dl_index];
            key.idx = dl_index;
            const DrawMesh& mesh = drawMesh_from_handle(rsc, node.meshHandles[m]);
            const ShaderTechnique& shaderTechnique = rsc.shaderTechniques[mesh.shaderTechnique];
            ShaderPrograms::Enum shaderProgram = shaderTechnique.shaders[drawType];

            // todo: do not hardcore this here
            gfx::rhi::RscBlendState bs = (mesh.shaderTechnique == ShaderTechnique::CutoutDiffuse 
                || mesh.shaderTechnique == ShaderTechnique::CutoutDiffuseSkinned
                || drawType == DrawType::DefaultAlphaClip)
                ? item.blendState = rsc.blendStateOn
                : item.blendState = rsc.blendStateBlendOff;

            key.v = makeSortKey(n, shaderProgram, sortParams);

            item.shader = rsc.shaderPrograms[shaderProgram].shader;
            item.vertexBuffer = mesh.vertexBuffer;
            item.cbuffers[item.cbuffer_count++] = cbuffer_from_handle(scene, node.cbuffer_node);
            if (node.cbuffer_ext) {
                item.cbuffers[item.cbuffer_count++] =
                    cbuffer_from_handle(scene, node.cbuffer_ext);
            }
            item.texture = mesh.texture;
            item.blendState = bs;
            item.name = shaderProgramNames[shaderProgram];
            item.drawcount = node.instanceCount;
        }
    }

    qsort_s64(dl.keys, 0, dl.count - 1);
}

#if __DEBUG
void recompileShaders(CoreResources& rsc) {

    for (u32 i = 0; i < ShaderPrograms::Count; i++) {

        struct stat binFile_lastModified;
        char buff[1024];
        errno_t err;

        renderer::ReloadableShader& shader = rsc.shaderPrograms[i];

        // recompile vertex shaders
        io::format(buff, sizeof(buff), SRC_PATH_FROM_BIN"%s", shader.vs_binFile);
        err = stat(buff, &binFile_lastModified);
        if (err == 0) {
            struct stat srcFile_lastModified;
            io::format(buff, sizeof(buff), SRC_PATH_FROM_BIN"%s", shader.vs_srcFile);
            err = stat(buff, &srcFile_lastModified);
            if (err == 0 &&
                math::max(binFile_lastModified.st_mtime, shader.vs_lastCompileTime) < srcFile_lastModified.st_mtime) {

                gfx::rhi::ShaderResult result = 
                    gfx::rhi::recompile_shaderfile_vs(shader.shader, buff);
                if (result.compiled) {
                    shader.vs_lastCompileTime = srcFile_lastModified.st_mtime;
                } else {
                    io::debuglog("%s: %s\n", shader.vs_srcFile, result.error);
                }
            }
        }

        // recompile pixel shaders
        io::format(buff, sizeof(buff), SRC_PATH_FROM_BIN"%s", shader.ps_binFile);
        err = stat(buff, &binFile_lastModified);
        if (err == 0) {
            struct stat srcFile_lastModified;
            io::format(buff, sizeof(buff), SRC_PATH_FROM_BIN"%s", shader.ps_srcFile);
            err = stat(buff, &srcFile_lastModified);
            if (err == 0 &&
                math::max(binFile_lastModified.st_mtime, shader.ps_lastCompileTime) < srcFile_lastModified.st_mtime) {

                gfx::rhi::ShaderResult result =
                    gfx::rhi::recompile_shaderfile_ps(shader.shader, buff);
                if (result.compiled) {
                    shader.ps_lastCompileTime = srcFile_lastModified.st_mtime;
                }
                else {
                    io::debuglog("%s: %s\n", shader.vs_srcFile, result.error);
                }
            }
        }
    }
}
#endif

}

#endif // __WASTELADNS_RENDERER_H__
