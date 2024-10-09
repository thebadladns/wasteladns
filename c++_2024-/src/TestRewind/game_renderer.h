#ifndef __WASTELADNS_GAMERENDERER_H__
#define __WASTELADNS_GAMERENDERER_H__

struct Key {
    u32 v;
    s32 idx;
};
int partition(Key* keys, s32 low, s32 high) {
    u32 p = keys[low].v;
    int i = low;
    int j = high;
    while (i < j) {
        while (keys[i].v <= p && i <= high - 1) { i++; }
        while (keys[j].v > p && j >= low + 1) { j--; }
        if (i < j) {
            Key temp = keys[i];
            keys[i] = keys[j];
            keys[j] = temp;
        }
    }
    Key temp = keys[low];
    keys[low] = keys[j];
    keys[j] = temp;
    return j;
};
void qsort(Key* keys, s32 low, s32 high) {
    if (low < high) {
        int pi = partition(keys, low, high);
        qsort(keys, low, pi - 1);
        qsort(keys, pi + 1, high);
    };
};

namespace Renderer {

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
    Driver::RscCBuffer cbuffers[CBuffer_Binding::Count];
    Driver::RscShaderSet* shader;
    Driver::RscTexture* texture;
    Driver::RscBlendState* blendState;
    Driver::RscIndexedVertexBuffer* vertexBuffer;
    u32 cbuffer_count;
};
struct Drawlist_Overrides {
    bool forced_shader;
    bool forced_texture;
    bool forced_blendState;
    bool forced_vertexBuffer;
    u32 forced_cbuffer_count;
};

struct DrawCall_Item {
    Driver::RscShaderSet shader;
    Driver::RscTexture texture;
    Driver::RscBlendState blendState;
    Driver::RscIndexedVertexBuffer vertexBuffer;
    Driver::RscCBuffer cbuffers[2];
    const char* name;
    u32 cbuffer_count;
    u32 drawcount;
};

struct DrawlistFilter { enum Enum { Alpha = 1 }; };
struct DrawlistBuckets { enum Enum { Base, Instanced, Count }; };
struct Drawlist {
	enum { MaxItems = 2048 };
    Key keys[MaxItems];
    DrawCall_Item items[MaxItems];
    u32 count[DrawlistBuckets::Count];
};
struct DrawlistStreams { enum Enum { Color3D, Color3DSkinned, Textured3D, Textured3DAlphaClip, Textured3DSkinned, Textured3DAlphaClipSkinned, Count }; };
struct DrawNodeMeshType { enum Enum { Invalid = 0, Default = 1, Skinned, Instanced, Count }; }; // invalid type is 0, to support 0 initialization
struct ShaderType { enum Enum {
    FullscreenBlit,
    Instanced3D,
    Color3D, Color3DSkinned,
    Textured3D, Textured3DAlphaClip, Textured3DSkinned, Textured3DAlphaClipSkinned,
    Count
}; };
struct DrawMesh {
    ShaderType::Enum type;
    Driver::RscIndexedVertexBuffer vertexBuffer;
    Driver::RscTexture texture;
};
struct DrawNode {
    u32 meshHandles[DrawlistStreams::Count];
    float3 min;
    float3 max;
    u32 cbuffer_node;
    NodeData nodeData;
};
struct DrawNodeSkinned {
    u32 meshHandles[DrawlistStreams::Count];
    float3 min;
    float3 max;
    u32 cbuffer_node;
    u32 cbuffer_skinning;
    NodeData nodeData;
    Matrices32 skinningMatrixPalette;
};
struct DrawNodeInstanced {
    u32 meshHandles[DrawlistStreams::Count];
    u32 cbuffer_node;
    u32 cbuffer_instances;
    NodeData nodeData;
    Matrices64 instanceMatrices;
    u32 instanceCount;
};

void draw_drawlist(Drawlist& dl, Drawlist_Context& ctx, const Drawlist_Overrides& overrides) {
    qsort(dl.keys, 0, dl.count[DrawlistBuckets::Base] - 1);
    qsort(dl.keys, dl.count[DrawlistBuckets::Base], dl.count[DrawlistBuckets::Instanced] - 1);
    
	u32 count = dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced];
    for (u32 i = 0; i < count; i++) {
        DrawCall_Item& item = dl.items[dl.keys[i].idx];
        Driver::Marker_t marker;
        Driver::set_marker_name(marker, item.name);
        Renderer::Driver::start_event(marker);
        {
            if (!overrides.forced_shader && ctx.shader != &item.shader) {
                Driver::bind_shader(item.shader);
                ctx.shader = &item.shader;
            }
			if (!overrides.forced_blendState && ctx.blendState != &item.blendState) {
				Driver::bind_blend_state(item.blendState);
				ctx.blendState = &item.blendState;
			}
            if (!overrides.forced_texture && ctx.texture != &item.texture) {
                Driver::bind_textures(&item.texture, 1);
                ctx.texture = &item.texture;
            }
            if (!overrides.forced_vertexBuffer && ctx.vertexBuffer != &item.vertexBuffer) {
                Driver::bind_indexed_vertex_buffer(item.vertexBuffer);
                ctx.vertexBuffer = &item.vertexBuffer;
            }
            for (u32 i = 0; i < item.cbuffer_count; i++) {
                ctx.cbuffers[i + overrides.forced_cbuffer_count] = item.cbuffers[i];
            }
            Driver::bind_cbuffers(*ctx.shader, ctx.cbuffers, ctx.cbuffer_count);
			if (item.drawcount) {
                Driver::draw_instances_indexed_vertex_buffer(item.vertexBuffer, item.drawcount);
            } else {
                Driver::draw_indexed_vertex_buffer(item.vertexBuffer);
            }
        }
        Renderer::Driver::end_event();
    }
}
struct Store {
    Driver::RscCBuffer cbuffers[512]; // todo: check size?
    Driver::RscShaderSet shaders[ShaderType::Count];
	Allocator::Arena persistentArena;
    Allocator::Pool<Animation::AnimatedNode> animatedNodes;
    Allocator::Pool<DrawNode> drawNodes;
    Allocator::Pool<DrawNodeSkinned> drawNodesSkinned;
    Allocator::Pool<DrawNodeInstanced> drawNodesInstanced;
    Allocator::Pool<DrawMesh> meshes;
    Renderer::Driver::RscRasterizerState rasterizerStateFill, rasterizerStateFillCullNone, rasterizerStateLine;
    Renderer::Driver::RscDepthStencilState depthStateOn, depthStateReadOnly, depthStateOff;
    Renderer::Driver::RscBlendState blendStateOn;
    Renderer::Driver::RscBlendState blendStateOff;
    Renderer::Driver::RscMainRenderTarget windowRT;
    Renderer::Driver::RscRenderTarget gameRT;
    u32 cbuffer_count;
    u32 cbuffer_scene;
    u32 playerDrawNodeHandle;
    u32 playerAnimatedNodeHandle;
    u32 particlesDrawHandle;
};

NodeData& get_draw_node_data(Store& store, const u32 nodeHandle) {
    if ((nodeHandle >> 16) & DrawNodeMeshType::Skinned) {
        DrawNodeSkinned& node = Allocator::get_pool_slot(store.drawNodesSkinned, nodeHandle & 0xffff);
        return node.nodeData;
    } else if ((nodeHandle >> 16) & DrawNodeMeshType::Instanced) {
        DrawNodeInstanced& node = Allocator::get_pool_slot(store.drawNodesInstanced, nodeHandle & 0xffff);
        return node.nodeData;
    } else { // if ((nodeHandle >> 16) & DrawNodeMeshType::Default) {
        DrawNode& node = Allocator::get_pool_slot(store.drawNodes, nodeHandle & 0xffff);
        return node.nodeData;
    }
}
Matrices32& get_draw_skinning_data(Store& store, const u32 nodeHandle) {
    //if ((nodeHandle >> 16) & DrawNodeMeshType::Skinned) {
    DrawNodeSkinned& node = Allocator::get_pool_slot(store.drawNodesSkinned, nodeHandle & 0xffff);
    return node.skinningMatrixPalette;
}
void get_draw_instanced_data(Matrices64*& matrices, u32*& count, Store& store, const u32 nodeHandle) {
    //if ((nodeHandle >> 16) & DrawNodeMeshType::Instanced) {
    DrawNodeInstanced& node = Allocator::get_pool_slot(store.drawNodesInstanced, nodeHandle & 0xffff);
    matrices = &node.instanceMatrices;
	count = &node.instanceCount;
}
DrawMesh get_drawMesh(Store& store, const u32 meshHandle) { return Allocator::get_pool_slot(store.meshes, meshHandle - 1); }
u32 handle_from_drawMesh(Store& store, DrawMesh& mesh) { return Allocator::get_pool_index(store.meshes, mesh) + 1; }
Animation::AnimatedNode& get_animatedNode(Store& store, const u32 nodeHandle) { return Allocator::get_pool_slot(store.animatedNodes, nodeHandle - 1); }
u32 handle_from_animatedNode(Store& store, Animation::AnimatedNode& animatedNode) { return Allocator::get_pool_index(store.animatedNodes, animatedNode) + 1; }

struct VisibleNodes {
    u32 visible_nodes[256];
    u32 visible_nodes_skinned[256];
    u32 visible_nodes_count;
    u32 visible_nodes_skinned_count;
};
void addNodesToDrawlist(Drawlist& dl, const VisibleNodes& visibleNodes, float3 cameraPos, Store& store, const u32 includeFilter, const u32 excludeFilter) {

    // hack
    const char* shaderNames[DrawNodeMeshType::Count][ShaderType::Count] = {
    { 0 }, {
        "NODE FullscreenBlit",
        "NODE Instanced3D", "NODE Color3D", "NODE Color3DSkinned", "NODE Textured3D", "NODE Textured3DAlphaClip", "NODE Textured3DSkinned", "NODE Textured3DAlphaClipSkinned"
	}, {
		"NODE SKINNED FullscreenBlit",
		"NODE SKINNED Instanced3D", "NODE SKINNED Color3D", "NODE SKINNED Color3DSkinned", "NODE SKINNED Textured3D", "NODE SKINNED Textured3DAlphaClip", "NODE SKINNED Textured3DSkinned", "NODE SKINNED Textured3DAlphaClipSkinned"
    }, {
        "NODE INSTANCED FullscreenBlit",
        "NODE INSTANCED Instanced3D", "NODE INSTANCED Color3D", "NODE INSTANCED Color3DSkinned", "NODE INSTANCED Textured3D", "NODE INSTANCED Textured3DAlphaClip", "NODE INSTANCED Textured3DSkinned", "NODE INSTANCED Textured3DAlphaClipSkinned"
    } };
    {
        // very rough depth sorting, 10 bits for 1000 units, sort granularity is 1.023 units
        const u32 depthBits = 10;
        const u32 depthMask = (1 << depthBits) - 1;
        const u32 maxValue = (1 << depthBits) - 1;
        const f32 maxDistSq = 1000.f * 1000.f;
        const u32 nodeBits = 10; // 1024 nodes
        const u32 nodeMask = (1 << nodeBits) - 1;
        const u32 shaderBits = 8; // 255 shader types
        const u32 shaderMask = (1 << shaderBits) - 1;
        
        for (u32 i = 0; i < visibleNodes.visible_nodes_count; i++) {
            u32 n = visibleNodes.visible_nodes[i];
            const DrawNode& node = store.drawNodes.data[n].state.live;
            const u32 maxMeshCount = COUNT_OF(node.meshHandles);
            
            if (includeFilter & DrawlistFilter::Alpha) { if (node.nodeData.groupColor.w == 1.f) { continue; } }
            if (excludeFilter & DrawlistFilter::Alpha) { if (node.nodeData.groupColor.w < 1.f) { continue; } }
            
            f32 distSq = Math::magSq(Math::subtract(node.nodeData.worldMatrix.col3.xyz, cameraPos));
            u32 distSqNormalized = u32(maxValue * Math::min(distSq / maxDistSq, 1.f));
            
            for (u32 m = 0; m < maxMeshCount; m++) {
                if (node.meshHandles[m] == 0) { continue; }
                u32 dl_index = dl.count[DrawlistBuckets::Base]++;
                DrawCall_Item& item = dl.items[dl_index];
                item = {};
                Key& key = dl.keys[dl_index];
                key.idx = dl_index;
                const DrawMesh& mesh = get_drawMesh(store, node.meshHandles[m]);
                u32 curr_shift = 0;
                key.v = 0;
                key.v |= (mesh.type & shaderMask) << curr_shift, curr_shift += shaderBits;
                key.v |= (n & nodeMask) << curr_shift, curr_shift += nodeBits;
                key.v |= (distSqNormalized & depthMask) << curr_shift, curr_shift += depthBits;
                item.shader = store.shaders[mesh.type];
                item.vertexBuffer = mesh.vertexBuffer;
                item.cbuffers[item.cbuffer_count++] = store.cbuffers[node.cbuffer_node];
                item.texture = mesh.texture;
                item.blendState = mesh.type == ShaderType::Textured3DAlphaClip ? store.blendStateOn : store.blendStateOff;
                item.name = shaderNames[DrawNodeMeshType::Default][mesh.type];
            }
        }
    }
    {
        // skinned nodes are sorted by distance, then node, then program (important to minimize cbuffer writes)
        const u32 depthBits = 10;
        const u32 depthMask = (1 << depthBits) - 1;
        const u32 maxValue = (1 << depthBits) - 1;
        const f32 maxDistSq = 1000.f * 1000.f;
        const u32 nodeBits = 10; // 1024 nodes
        const u32 nodeMask = (1 << nodeBits) - 1;
        const u32 shaderBits = 8; // 255 shader types
        const u32 shaderMask = (1 << shaderBits) - 1;
        for (u32 i = 0; i < visibleNodes.visible_nodes_skinned_count; i++) {
            u32 n = visibleNodes.visible_nodes_skinned[i];
            const DrawNodeSkinned& node = store.drawNodesSkinned.data[n].state.live;
            const u32 maxMeshCount = COUNT_OF(node.meshHandles);
            
            if (includeFilter & DrawlistFilter::Alpha) { if (node.nodeData.groupColor.w == 1.f) { continue; } }
            if (excludeFilter & DrawlistFilter::Alpha) { if (node.nodeData.groupColor.w < 1.f) { continue; } }
            
            f32 distSq = Math::magSq(Math::subtract(node.nodeData.worldMatrix.col3.xyz, cameraPos));
            u32 distSqNormalized = u32(maxValue * Math::min(distSq / maxDistSq, 1.f));
            
            for (u32 m = 0; m < maxMeshCount; m++) {
                if (node.meshHandles[m] == 0) { continue; }
                u32 dl_index = dl.count[DrawlistBuckets::Base]++;
                DrawCall_Item& item = dl.items[dl_index];
                Key& key = dl.keys[dl_index];
                key.idx = dl_index;
                const DrawMesh& mesh = get_drawMesh(store, node.meshHandles[m]);
                u32 curr_shift = 0;
                key.v = 0;
                key.v |= (mesh.type & shaderMask) << curr_shift, curr_shift += shaderBits;
                key.v |= (n & nodeMask) << curr_shift, curr_shift += nodeBits;
                key.v |= (distSqNormalized & depthMask) << curr_shift, curr_shift += depthBits;
                item.shader = store.shaders[mesh.type];
                item.vertexBuffer = mesh.vertexBuffer;
                item.cbuffers[item.cbuffer_count++] = store.cbuffers[node.cbuffer_node];
                item.cbuffers[item.cbuffer_count++] = store.cbuffers[node.cbuffer_skinning];
                item.texture = mesh.texture;
                item.blendState = mesh.type == ShaderType::Textured3DAlphaClipSkinned ? store.blendStateOn : store.blendStateOff;
                item.name = shaderNames[DrawNodeMeshType::Skinned][mesh.type];
            }
        }
    }
    {
        // instance nodes are sorted by node, then program (no distance sort)
        const u32 nodeBits = 10; // 1024 nodes
        const u32 nodeMask = (1 << nodeBits) - 1;
        const u32 shaderBits = 8; // 255 shader types
        const u32 shaderMask = (1 << shaderBits) - 1;
        for (u32 n = 0, count = 0; n < store.drawNodesInstanced.cap && count < store.drawNodesInstanced.count; n++) {
            if (store.drawNodesInstanced.data[n].alive == 0) { continue; }
            count++;
            
            const DrawNodeInstanced& node = store.drawNodesInstanced.data[n].state.live;
            const u32 maxMeshCount = COUNT_OF(node.meshHandles);
            
            if (includeFilter & DrawlistFilter::Alpha) { if (node.nodeData.groupColor.w == 1.f) { continue; } }
            if (excludeFilter & DrawlistFilter::Alpha) { if (node.nodeData.groupColor.w < 1.f) { continue; } }
            
            for (u32 m = 0; m < maxMeshCount; m++) {
                if (node.meshHandles[m] == 0) { continue; }
                u32 dl_index = dl.count[DrawlistBuckets::Instanced]++;
                DrawCall_Item& item = dl.items[dl_index];
                Key& key = dl.keys[dl_index];
                key.idx = dl_index;
                const DrawMesh& mesh = get_drawMesh(store, node.meshHandles[m]);
                u32 curr_shift = 0;
                key.v = 0;
                key.v |= (mesh.type & shaderMask) << curr_shift, curr_shift += shaderBits;
                key.v |= (n & nodeMask) << curr_shift, curr_shift += nodeBits;
                item.shader = store.shaders[mesh.type];
                item.vertexBuffer = mesh.vertexBuffer;
                item.cbuffers[item.cbuffer_count++] = store.cbuffers[node.cbuffer_node];
                item.cbuffers[item.cbuffer_count++] = store.cbuffers[node.cbuffer_instances];
                item.texture = mesh.texture;
                item.blendState = store.blendStateOff; // todo: support other blendstates when doing instances
                item.drawcount = node.instanceCount;
                item.name = shaderNames[DrawNodeMeshType::Instanced][mesh.type];
            }
        }
    }
}

namespace FBX {

    void extract_anim_data(Animation::Skeleton& skeleton, Animation::Clip*& clips, u32& clipCount, Store& store, const ufbx_mesh& mesh, const ufbx_scene& scene) {
        auto from_ufbx_mat = [](float4x4& dst, const ufbx_matrix& src) {
            dst.col0.x = src.cols[0].x; dst.col0.y = src.cols[0].y; dst.col0.z = src.cols[0].z; dst.col0.w = 0.f;
            dst.col1.x = src.cols[1].x; dst.col1.y = src.cols[1].y; dst.col1.z = src.cols[1].z; dst.col1.w = 0.f;
            dst.col2.x = src.cols[2].x; dst.col2.y = src.cols[2].y; dst.col2.z = src.cols[2].z; dst.col2.w = 0.f;
            dst.col3.x = src.cols[3].x; dst.col3.y = src.cols[3].y; dst.col3.z = src.cols[3].z; dst.col3.w = 1.f;
        };

        const u32 maxjoints = 128;
        ufbx_matrix jointFromGeometry[maxjoints];
        u32 node_indices_raw[maxjoints];
        u8 jointCount = 0;

        // Extract all constant matrices from geometry to joints
        ufbx_skin_deformer& skin = *(mesh.skin_deformers.data[0]);
        for (size_t ci = 0; ci < skin.clusters.count; ci++) {
            ufbx_skin_cluster* cluster = skin.clusters.data[ci];
            if (jointCount == maxjoints) { continue; }
            jointFromGeometry[jointCount] = cluster->geometry_to_bone;
            node_indices_raw[jointCount] = (s8)cluster->bone_node->typed_id;
            jointCount++;
        }

        // todo: use arenas
        u32* node_indices_skeleton = (u32*)malloc(sizeof(u32) * jointCount);
        s8* nodeIdtoJointId = (s8*)malloc(sizeof(s8) * scene.nodes.count);
        skeleton.jointFromGeometry = (float4x4*)malloc(sizeof(float4x4) * jointCount);
        skeleton.parentFromPosedJoint = (float4x4*)malloc(sizeof(float4x4) * jointCount);
        skeleton.geometryFromPosedJoint = (float4x4*)malloc(sizeof(float4x4) * jointCount);
        skeleton.parentIndices = (s8*)malloc(sizeof(s8) * jointCount);
        skeleton.jointCount = jointCount;

        // joints are listed in hierarchical order, first one is the root
        for (u32 joint_index = 0; joint_index < jointCount; joint_index++) {
            u32 node_index = node_indices_raw[joint_index];
            ufbx_node& node = *(scene.nodes.data[node_index]);
            from_ufbx_mat(skeleton.jointFromGeometry[joint_index], jointFromGeometry[joint_index]);
            if (joint_index == 0) {
                skeleton.parentIndices[joint_index] = -1;
                from_ufbx_mat(skeleton.geometryFromRoot, node.parent->node_to_world);
            } else { // if (node.parent) { // todo: multiple roots??
                skeleton.parentIndices[joint_index] = nodeIdtoJointId[node.parent->typed_id];
            }
            from_ufbx_mat(skeleton.geometryFromPosedJoint[joint_index], node.node_to_world); // default
            nodeIdtoJointId[node_index] = joint_index;
            node_indices_skeleton[joint_index] = node_index;
        }

        // todo: default
        for (u32 joint_index = 0; joint_index < skeleton.jointCount; joint_index++) {
            float4x4& m = skeleton.parentFromPosedJoint[joint_index];
            m.col0 = { 1.f, 0.f, 0.f, 0.f };
            m.col1 = { 0.f, 1.f, 0.f, 0.f };
            m.col2 = { 0.f, 0.f, 1.f, 0.f };
            m.col3 = { 0.f, 0.f, 0.f, 1.f };
        }

        // anim clip
        clipCount = (u32)scene.anim_stacks.count;
        clips = (Animation::Clip*)malloc(sizeof(Animation::Clip) * scene.anim_stacks.count);
        for (size_t i = 0; i < clipCount; i++) {
            const ufbx_anim_stack& stack = *(scene.anim_stacks.data[i]);
            const f64 targetFramerate = 12.f;
            const u32 maxFrames = 4096;
            const f64 duration = (f32)stack.time_end - (f32)stack.time_begin;
            const u32 numFrames = Math::clamp((u32)(duration * targetFramerate), 2u, maxFrames);
            const f64 framerate = (numFrames-1) / duration;

            Animation::Clip& clip = clips[i];
            clip.frameCount = numFrames;
            clip.joints = (Animation::JointKeyframes*)malloc(sizeof(Animation::JointKeyframes) * jointCount); // todo: remove malloc
            clip.timeEnd = (f32)stack.time_end;
            for (u32 joint_index = 0; joint_index < jointCount; joint_index++) {
                ufbx_node* node = scene.nodes.data[node_indices_skeleton[joint_index]];
				Animation::JointKeyframes& joint = clip.joints[joint_index];
                joint.joint_to_parent_trs = (Animation::Joint_TRS*)malloc(sizeof(Animation::Joint_TRS) * numFrames);
                for (u32 f = 0; f < numFrames; f++) {
                    Animation::Joint_TRS& keyframeJoint = joint.joint_to_parent_trs[f];
                    f64 time = stack.time_begin + (double)f / framerate;
                    ufbx_transform transform = ufbx_evaluate_transform(&stack.anim, node, time);
                    keyframeJoint.rotation.x = transform.rotation.x; keyframeJoint.rotation.y = transform.rotation.y; keyframeJoint.rotation.z = transform.rotation.z; keyframeJoint.rotation.w = transform.rotation.w;
                    keyframeJoint.translation.x = transform.translation.x; keyframeJoint.translation.y = transform.translation.y; keyframeJoint.translation.z = transform.translation.z;
                    keyframeJoint.scale.x = transform.scale.x; keyframeJoint.scale.y = transform.scale.y; keyframeJoint.scale.z = transform.scale.z;
                }
            }
        }
    }
    void extract_skinning_attribs(u8* joint_indices, u8* joint_weights, const ufbx_mesh& mesh, const u32 vertex_id) {
        ufbx_skin_deformer& skin = *(mesh.skin_deformers.data[0]);
        ufbx_skin_vertex& skinned = skin.vertices.data[vertex_id];
        f32 total_weight = 0.f;
        const u32 num_weights = Math::min(skinned.num_weights, 4u);
        u8 bone_index[4]{};
        f32 bone_weights[4]{};
        u32 num_bones = 0;
        for (u32 wi = 0; wi < num_weights; wi++) {
            ufbx_skin_weight weight = skin.weights.data[skinned.weight_begin + wi];
            if (weight.cluster_index >= 255) { continue; }
            total_weight += (f32)weight.weight;
            bone_index[num_bones] = (u8)weight.cluster_index;
            bone_weights[num_bones] = (f32)weight.weight;
            num_bones++;
        }
        if (total_weight > 0.0f) {
            u32 quantized_sum = 0;
            for (size_t i = 0; i < 4; i++) {
                u8 quantized_weight = (u8)(255.f * (bone_weights[i] / total_weight));
                quantized_sum += quantized_weight;
                joint_indices[i] = bone_index[i];
                joint_weights[i] = quantized_weight;
            }
            joint_weights[0] += 255 - quantized_sum; // if the sum is not 255, adjust first weight
        }
    }
    struct PipelineAssetContext {
        Allocator::Arena scratchArena;
        const Driver::VertexAttribDesc* vertexAttrs[DrawlistStreams::Count];
        u32 attr_count[DrawlistStreams::Count];
    };
    void load_with_materials(u32& nodeHandle, Animation::AnimatedNode& animatedNode, Store& store, PipelineAssetContext& pipelineContext, const char* path) {
        ufbx_load_opts opts = {};
        opts.target_axes = { UFBX_COORDINATE_AXIS_POSITIVE_X, UFBX_COORDINATE_AXIS_POSITIVE_Z, UFBX_COORDINATE_AXIS_POSITIVE_Y };
        opts.allow_null_material = true;
        ufbx_error error;
        ufbx_scene* scene = ufbx_load_file(path, &opts, &error);
        if (scene) {
            // We'll process all vertices first, and then split then into the supported material buffers
            u32 maxVertices = 0;
            for (size_t i = 0; i < scene->meshes.count; i++) { maxVertices += (u32)scene->meshes.data[i]->num_vertices; }

            Allocator::Buffer<float3> vertices = {};
            Allocator::reserve(vertices, maxVertices, pipelineContext.scratchArena);

            struct DstStreams {
                Allocator::Buffer_t vertex;
                Allocator::Buffer<u32> index;
                void* user;
                u32 vertex_size;
                u32 vertex_align;
            };
            DstStreams materialVertexBuffer[DrawlistStreams::Count] = {};
			materialVertexBuffer[DrawlistStreams::Color3D].vertex_size = sizeof(VertexLayout_Color_3D);
			materialVertexBuffer[DrawlistStreams::Color3D].vertex_align = alignof(VertexLayout_Color_3D);
            materialVertexBuffer[DrawlistStreams::Color3DSkinned].vertex_size = sizeof(VertexLayout_Color_Skinned_3D);
            materialVertexBuffer[DrawlistStreams::Color3DSkinned].vertex_align = alignof(VertexLayout_Color_Skinned_3D);
            materialVertexBuffer[DrawlistStreams::Textured3D].vertex_size = sizeof(VertexLayout_Textured_3D);
            materialVertexBuffer[DrawlistStreams::Textured3D].vertex_align = alignof(VertexLayout_Textured_3D);
            materialVertexBuffer[DrawlistStreams::Textured3DAlphaClip].vertex_size = sizeof(VertexLayout_Textured_3D);
            materialVertexBuffer[DrawlistStreams::Textured3DAlphaClip].vertex_align = alignof(VertexLayout_Textured_3D);
            materialVertexBuffer[DrawlistStreams::Textured3DSkinned].vertex_size = sizeof(VertexLayout_Textured_Skinned_3D);
            materialVertexBuffer[DrawlistStreams::Textured3DSkinned].vertex_align = alignof(VertexLayout_Textured_Skinned_3D);
            materialVertexBuffer[DrawlistStreams::Textured3DAlphaClipSkinned].vertex_size = sizeof(VertexLayout_Textured_Skinned_3D);
            materialVertexBuffer[DrawlistStreams::Textured3DAlphaClipSkinned].vertex_align = alignof(VertexLayout_Textured_Skinned_3D);

			for (u32 i = 0; i < DrawlistStreams::Count; i++) {
                DstStreams& stream = materialVertexBuffer[i];
                Allocator::reserve(stream.vertex, maxVertices, stream.vertex_size, stream.vertex_align, pipelineContext.scratchArena);
                Allocator::reserve(stream.index, maxVertices * 3 * 2, pipelineContext.scratchArena);
			}

            // hack: only consider skinning from first mesh
            animatedNode = {};
            if (scene->meshes.count && scene->meshes[0]->skin_deformers.count) {
                extract_anim_data(animatedNode.skeleton, animatedNode.clips, animatedNode.clipCount, store, *(scene->meshes[0]), *scene);
            }

            // TODO: consider skinning
            float3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
            float3 max = {-FLT_MAX,-FLT_MAX,-FLT_MAX };

            u32 vertexOffset = 0;
            for (size_t i = 0; i < scene->meshes.count; i++) {
                ufbx_mesh& mesh = *scene->meshes.data[i];
                // Extract vertices from this mesh and flatten any transform trees
                // This assumes there's only one instance of this mesh, we don't support more at the moment
                {
                    // todo: consider asset transform
                    assert(mesh.instances.count == 1);
                    const ufbx_matrix& m = mesh.instances.data[0]->geometry_to_world;
                    for (size_t v = 0; v < mesh.num_vertices; v++) {
                        float3 vertex;
                        const ufbx_float3& v_fbx_ls = mesh.vertices[v];
                        
                        float3 transformed;
                        transformed.x = v_fbx_ls.x * m.m00 + v_fbx_ls.y * m.m01 + v_fbx_ls.z * m.m02 + m.m03;
                        transformed.y = v_fbx_ls.x * m.m10 + v_fbx_ls.y * m.m11 + v_fbx_ls.z * m.m12 + m.m13;
                        transformed.z = v_fbx_ls.x * m.m20 + v_fbx_ls.y * m.m21 + v_fbx_ls.z * m.m22 + m.m23;
                        max = Math::max(transformed, max);
                        min = Math::min(transformed, min);

                        // If using skinning, we'll store the node hierarchy in the joints
                        if (mesh.skin_deformers.count) {
                            vertex.x = v_fbx_ls.x;
                            vertex.y = v_fbx_ls.y;
                            vertex.z = v_fbx_ls.z;
                            Allocator::push(vertices, pipelineContext.scratchArena) = vertex;
                        } else {
                            Allocator::push(vertices, pipelineContext.scratchArena) = transformed;
                        }
                    }
                }

                struct MaterialVertexBufferContext {
                    u32* fbxvertex_to_dstvertex;
                    u32* dstvertex_to_fbxvertex;
                    u32* dstvertex_to_fbxidx;
                };
                const auto add_material_indices_to_stream = [](DstStreams & stream_dst, MaterialVertexBufferContext & ctx, Allocator::Arena & scratchArena, const u32 vertices_src_count, const ufbx_mesh & mesh, const ufbx_mesh_material & mesh_mat) {

                    memset(ctx.fbxvertex_to_dstvertex, 0xffffffff, sizeof(u32) * vertices_src_count);

                    for (size_t f = 0; f < mesh_mat.num_faces; f++) {
                        const u32 maxTriIndices = 32;
                        u32 triIndices[maxTriIndices];

                        ufbx_face& face = mesh.faces.data[mesh_mat.face_indices.data[f]];
                        size_t num_tris = ufbx_triangulate_face(triIndices, maxTriIndices, &mesh, face);
                        for (size_t v = 0; v < num_tris * 3; v++) {
                            const uint32_t triangulatedFaceIndex = triIndices[v];
                            const u32 src_index = mesh.vertex_indices[triangulatedFaceIndex];

                            if (ctx.fbxvertex_to_dstvertex[src_index] != 0xffffffff) {
                                // Vertex has been updated in the new buffer, copy the index over
                                Allocator::push(stream_dst.index, scratchArena) = ctx.fbxvertex_to_dstvertex[src_index];
                            }
                            else {
                                // Vertex needs to be added to the new buffer, and index updated
                                u32 dst_index = (u32)stream_dst.vertex.len;
                                // mark where the ufbx data is in relation to the destination vertex
                                ctx.dstvertex_to_fbxvertex[dst_index] = src_index;
                                ctx.dstvertex_to_fbxidx[dst_index] = triangulatedFaceIndex;
                                Allocator::push(stream_dst.vertex, stream_dst.vertex_size, stream_dst.vertex_align, scratchArena);
                                // mark where this vertex is in the destination array
                                ctx.fbxvertex_to_dstvertex[src_index] = dst_index;

                                Allocator::push(stream_dst.index, scratchArena) = dst_index;
                            }
                        }
                    }
                };
                // Extract all indices, separated by supported drawlist
                // They will all point the same vertex buffer array, which is not ideal, but I don't want to deal with
                // re-building a vertex buffer for each index buffer
                const u32 mesh_vertices_count = (u32)mesh.num_vertices;
                const float3* mesh_vertices = &vertices.data[vertexOffset];
                MaterialVertexBufferContext ctx = {};
				// tmp buffers to store the mapping between the fbx mesh vertices and the destination vertex streams
                ctx.dstvertex_to_fbxvertex = (u32*)Allocator::alloc_arena(pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
                ctx.dstvertex_to_fbxidx = (u32*)Allocator::alloc_arena(pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
                ctx.fbxvertex_to_dstvertex = (u32*)Allocator::alloc_arena(pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
                for (size_t m = 0; m < mesh.materials.count; m++) {
                    ufbx_mesh_material& mesh_mat = mesh.materials.data[m];
                    if (animatedNode.skeleton.jointCount) {
                        if (mesh_mat.material && mesh_mat.material->pbr.base_color.has_value) {
                            if (mesh_mat.material->pbr.base_color.texture_enabled) { // textured
                                // Assume that opacity tied to a texture means that we should use the alpha clip shader
                                DstStreams& stream = (mesh_mat.material->pbr.opacity.texture_enabled) ?
                                                      materialVertexBuffer[DrawlistStreams::Textured3DAlphaClipSkinned]
									                : materialVertexBuffer[DrawlistStreams::Textured3DSkinned];

                                assert(!stream.user); // we only support one textured-material of each pass
                                u32 stream_current_offset = (u32)stream.vertex.len;
                                add_material_indices_to_stream(stream, ctx, pipelineContext.scratchArena, mesh_vertices_count, mesh, mesh_mat);
                                for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                    VertexLayout_Textured_Skinned_3D& dst = *(VertexLayout_Textured_Skinned_3D*)(stream.vertex.data + i * stream.vertex_size);
                                    ufbx_float2& uv = mesh.vertex_uv[ctx.dstvertex_to_fbxidx[i]];
                                    const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                    dst.pos = { src.x, src.y, src.z };
                                    dst.uv.x = uv.x;
                                    dst.uv.y = uv.y;
                                    extract_skinning_attribs(dst.joint_indices, dst.joint_weights, mesh, ctx.dstvertex_to_fbxvertex[i]);
                                }
                                stream.user = mesh_mat.material->pbr.base_color.texture;
                            } else { // material color
                                DstStreams& stream = materialVertexBuffer[DrawlistStreams::Color3DSkinned];
                                u32 stream_current_offset = (u32)stream.vertex.len;
                                add_material_indices_to_stream(stream, ctx, pipelineContext.scratchArena, mesh_vertices_count, mesh, mesh_mat);
                                for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                    VertexLayout_Color_Skinned_3D& dst = *(VertexLayout_Color_Skinned_3D*)(stream.vertex.data + i * stream.vertex_size);
                                    ufbx_float4& c = mesh_mat.material->pbr.base_color.value_float4;
                                    const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                    dst.pos = { src.x, src.y, src.z };
                                    dst.color = Color32(c.x, c.y, c.z, c.w).ABGR();
                                    extract_skinning_attribs(dst.joint_indices, dst.joint_weights, mesh, ctx.dstvertex_to_fbxvertex[i]);
                                }
                            }
                        } else if (mesh.vertex_color.exists) { // vertex color
                            DstStreams& stream = materialVertexBuffer[DrawlistStreams::Color3DSkinned];
                            u32 stream_current_offset = (u32)stream.vertex.len;
                            add_material_indices_to_stream(stream, ctx, pipelineContext.scratchArena, mesh_vertices_count, mesh, mesh_mat);
                            for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                VertexLayout_Color_Skinned_3D& dst = *(VertexLayout_Color_Skinned_3D*)(stream.vertex.data + i * stream.vertex_size);
                                const ufbx_float4& c = mesh.vertex_color[ctx.dstvertex_to_fbxidx[i]];
                                const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                dst.pos = { src.x, src.y, src.z };
                                dst.color = Color32(c.x, c.y, c.z, c.w).ABGR();
                                extract_skinning_attribs(dst.joint_indices, dst.joint_weights, mesh, ctx.dstvertex_to_fbxvertex[i]);
                            }
                        } else { // no material info
                            DstStreams& stream = materialVertexBuffer[DrawlistStreams::Color3DSkinned];
                            u32 stream_current_offset = (u32)stream.vertex.len;
                            add_material_indices_to_stream(stream, ctx, pipelineContext.scratchArena, mesh_vertices_count, mesh, mesh_mat);
                            for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                VertexLayout_Color_Skinned_3D& dst = *(VertexLayout_Color_Skinned_3D*)(stream.vertex.data + i * stream.vertex_size);
                                const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                dst.pos = { src.x, src.y, src.z };
                                dst.color = Color32(1.f, 0.f, 1.f, 0.f).ABGR(); // no vertex color available, write a dummy
                                extract_skinning_attribs(dst.joint_indices, dst.joint_weights, mesh, ctx.dstvertex_to_fbxvertex[i]);
                            }
                        }
                    } else {
                        if (mesh_mat.material && mesh_mat.material->pbr.base_color.has_value) {
                            if (mesh_mat.material->pbr.base_color.texture_enabled) { // textured
                                // Assume that opacity tied to a texture means that we should use the alpha clip shader
                                DstStreams& stream = (mesh_mat.material->pbr.opacity.texture_enabled) ?
                                      materialVertexBuffer[DrawlistStreams::Textured3DAlphaClip]
                                    : materialVertexBuffer[DrawlistStreams::Textured3D];

                                assert(!stream.user); // we only support one textured-material of each pass
                                u32 stream_current_offset = (u32)stream.vertex.len;
                                add_material_indices_to_stream(stream, ctx, pipelineContext.scratchArena, mesh_vertices_count, mesh, mesh_mat);
                                for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                    VertexLayout_Textured_3D& dst = *(VertexLayout_Textured_3D*)(stream.vertex.data + i * stream.vertex_size);
                                    ufbx_float2& uv = mesh.vertex_uv[ctx.dstvertex_to_fbxidx[i]];
                                    const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                    dst.pos = { src.x, src.y, src.z };
                                    dst.uv.x = uv.x;
                                    dst.uv.y = uv.y;
                                }
                                stream.user = mesh_mat.material->pbr.base_color.texture;
                            } else { // material color
                                DstStreams& stream = materialVertexBuffer[DrawlistStreams::Color3D];
                                u32 stream_current_offset = (u32)stream.vertex.len;
                                add_material_indices_to_stream(stream, ctx, pipelineContext.scratchArena, mesh_vertices_count, mesh, mesh_mat);
                                for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                    VertexLayout_Color_3D& dst = *(VertexLayout_Color_3D*)(stream.vertex.data + i * stream.vertex_size);
                                    ufbx_float4& c = mesh_mat.material->pbr.base_color.value_float4;
                                    const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                    dst.pos = { src.x, src.y, src.z };
                                    dst.color = Color32(c.x, c.y, c.z, c.w).ABGR();
                                }
                            }
                        } else if (mesh.vertex_color.exists) { // vertex color
                            DstStreams& stream = materialVertexBuffer[DrawlistStreams::Color3D];
                            u32 stream_current_offset = (u32)stream.vertex.len;
                            add_material_indices_to_stream(stream, ctx, pipelineContext.scratchArena, mesh_vertices_count, mesh, mesh_mat);
                            for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                VertexLayout_Color_3D& dst = *(VertexLayout_Color_3D*)(stream.vertex.data + i * stream.vertex_size);
                                const ufbx_float4& c = mesh.vertex_color[ctx.dstvertex_to_fbxidx[i]];
                                const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                dst.pos = { src.x, src.y, src.z };
                                dst.color = Color32(c.x, c.y, c.z, c.w).ABGR();
                            }
                        } else { // no material info
                            DstStreams& stream = materialVertexBuffer[DrawlistStreams::Color3D];
                            u32 stream_current_offset = (u32)stream.vertex.len;
                            add_material_indices_to_stream(stream, ctx, pipelineContext.scratchArena, mesh_vertices_count, mesh, mesh_mat);
                            for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                VertexLayout_Color_3D& dst = *(VertexLayout_Color_3D*)(stream.vertex.data + i * stream.vertex_size);
                                const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                dst.pos = { src.x, src.y, src.z };
                                dst.color = Color32(1.f, 0.f, 1.f, 0.f).ABGR(); // no vertex color available, write a dummy
                            }
                        }
                    }
                }
                vertexOffset += (u32)mesh.num_vertices;
            }

            u32* meshHandles;
            if (animatedNode.skeleton.jointCount) {
				DrawNodeSkinned& nodeToAdd = Allocator::alloc_pool(store.drawNodesSkinned);
                nodeHandle = (DrawNodeMeshType::Skinned << 16) | (Allocator::get_pool_index(store.drawNodesSkinned, nodeToAdd) & 0xffff);
                nodeToAdd = {};
                nodeToAdd.max = max;
                nodeToAdd.min = min;
                nodeToAdd.cbuffer_node = store.cbuffer_count;
                Driver::create_cbuffer(store.cbuffers[store.cbuffer_count++], { sizeof(NodeData) });
                nodeToAdd.cbuffer_skinning = store.cbuffer_count;
                Driver::create_cbuffer(store.cbuffers[store.cbuffer_count++], { sizeof(Matrices32) });
                meshHandles = nodeToAdd.meshHandles;
            } else {
                DrawNode& nodeToAdd = Allocator::alloc_pool(store.drawNodes);
                nodeHandle = (DrawNodeMeshType::Default << 16) | (Allocator::get_pool_index(store.drawNodes, nodeToAdd) & 0xffff);
                nodeToAdd = {};
                nodeToAdd.max = max;
                nodeToAdd.min = min;
                nodeToAdd.cbuffer_node = store.cbuffer_count;
                Driver::create_cbuffer(store.cbuffers[store.cbuffer_count++], { sizeof(NodeData) });
                meshHandles = nodeToAdd.meshHandles;
            }
            const ShaderType::Enum shaderTypes[DrawlistStreams::Count] = {
                ShaderType::Color3D, ShaderType::Color3DSkinned, ShaderType::Textured3D, ShaderType::Textured3DAlphaClip, ShaderType::Textured3DSkinned, ShaderType::Textured3DAlphaClipSkinned
            };
			for (u32 i = 0; i < DrawlistStreams::Count; i++) {
				DstStreams& stream = materialVertexBuffer[i];
				if (stream.vertex.len) {
                    Driver::IndexedVertexBufferDesc desc = {};
                    desc.vertexData = stream.vertex.data;
                    desc.vertexSize = (u32)stream.vertex.len * stream.vertex_size;
					desc.vertexCount = (u32)stream.vertex.len;
                    desc.indexData = stream.index.data;
                    desc.indexSize = (u32)stream.index.len * sizeof(u32);
                    desc.indexCount = (u32)stream.index.len;
                    desc.memoryUsage = Renderer::Driver::BufferMemoryUsage::GPU;
                    desc.accessType = Renderer::Driver::BufferAccessType::GPU;
                    desc.indexType = Renderer::Driver::BufferItemType::U32;
                    desc.type = Renderer::Driver::BufferTopologyType::Triangles;

					const ShaderType::Enum shaderType = shaderTypes[i];
                    DrawMesh& mesh = Allocator::alloc_pool(store.meshes);
                    mesh = {};
                    mesh.type = shaderType;
                    Renderer::Driver::create_indexed_vertex_buffer(mesh.vertexBuffer, desc, pipelineContext.vertexAttrs[i], pipelineContext.attr_count[i]);
                    if (stream.user) { Driver::create_texture_from_file(mesh.texture, { ((ufbx_texture*)stream.user)->filename.data }); }
                    meshHandles[i] = handle_from_drawMesh(store, mesh);
				}
			}
        }
    }
}


void init_pipelines(Store& store, Allocator::Arena scratchArena, const Platform::Screen& screen) {

    Allocator::init_arena(store.persistentArena, 1024 * 1024); // 1MB
    const u32 meshPoolSize = 2048;
    const u32 drawNodePoolSize = 256;
    const u32 drawNodeSkinnedPoolSize = 256;
    const u32 drawNodeInstancedPoolSize = 16;
    const u32 animatedNodesSize = 256;
    Allocator::init_pool(store.meshes, meshPoolSize, store.persistentArena);
    Allocator::init_pool(store.drawNodesInstanced, drawNodeInstancedPoolSize, store.persistentArena);
    Allocator::init_pool(store.drawNodesSkinned, drawNodeSkinnedPoolSize, store.persistentArena);
    Allocator::init_pool(store.drawNodes, drawNodePoolSize, store.persistentArena);
    Allocator::init_pool(store.animatedNodes, animatedNodesSize, store.persistentArena);
    __DEBUGEXP(store.meshes.name = "meshes");
    __DEBUGEXP(store.drawNodesSkinned.name = "skinned draw nodes");
    __DEBUGEXP(store.drawNodes.name = "draw nodes");
    __DEBUGEXP(store.animatedNodes.name = "anim nodes");

    Renderer::Driver::MainRenderTargetParams windowRTparams = {};
    windowRTparams.depth = true;
    windowRTparams.width = screen.window_width;
    windowRTparams.height = screen.window_height;
    Renderer::Driver::create_main_RT(store.windowRT, windowRTparams);

    Renderer::Driver::RenderTargetParams gameRTparams;
    gameRTparams.depth = true;
    gameRTparams.width = screen.width;
    gameRTparams.height = screen.height;
    gameRTparams.textureFormat = Renderer::Driver::TextureFormat::V4_8;
    gameRTparams.textureInternalFormat = Renderer::Driver::InternalTextureFormat::V4_8;
    gameRTparams.textureFormatType = Renderer::Driver::Type::Float;
    gameRTparams.count = 1;
    Renderer::Driver::create_RT(store.gameRT, gameRTparams);

    // rasterizer states
    Renderer::Driver::create_blend_state(store.blendStateOn, { true });
    Renderer::Driver::create_blend_state(store.blendStateOff, { false });
    Renderer::Driver::create_RS(store.rasterizerStateFill, { Renderer::Driver::RasterizerFillMode::Fill, Renderer::Driver::RasterizerCullMode::CullBack });
    Renderer::Driver::create_RS(store.rasterizerStateFillCullNone, { Renderer::Driver::RasterizerFillMode::Fill, Renderer::Driver::RasterizerCullMode::CullNone });
    Renderer::Driver::create_RS(store.rasterizerStateLine, { Renderer::Driver::RasterizerFillMode::Line, Renderer::Driver::RasterizerCullMode::CullNone });
    Renderer::Driver::create_DS(store.depthStateOn, { true, Renderer::Driver::DepthFunc::Less, Renderer::Driver::DepthWriteMask::All });
    Renderer::Driver::create_DS(store.depthStateReadOnly, { true, Renderer::Driver::DepthFunc::Less, Renderer::Driver::DepthWriteMask::Zero });
    Renderer::Driver::create_DS(store.depthStateOff, { false });

    // known cbuffers
    store.cbuffer_scene = store.cbuffer_count;
    Driver::create_cbuffer(store.cbuffers[store.cbuffer_count++], { sizeof(SceneData) });

    // input layouts
    const Driver::VertexAttribDesc attribs_3d[] = {
        Driver::make_vertexAttribDesc("POSITION", 0, sizeof(float3), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
    };
    const Driver::VertexAttribDesc attribs_color3d[] = {
        Driver::make_vertexAttribDesc("POSITION", OFFSET_OF(VertexLayout_Color_3D, pos), sizeof(VertexLayout_Color_3D), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
        Driver::make_vertexAttribDesc("COLOR", OFFSET_OF(VertexLayout_Color_3D, color), sizeof(VertexLayout_Color_3D), Driver::BufferAttributeFormat::R8G8B8A8_UNORM)
    };
    const Driver::VertexAttribDesc attribs_color3d_skinned[] = {
        Driver::make_vertexAttribDesc("POSITION", OFFSET_OF(VertexLayout_Color_Skinned_3D, pos), sizeof(VertexLayout_Color_Skinned_3D), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
        Driver::make_vertexAttribDesc("COLOR", OFFSET_OF(VertexLayout_Color_Skinned_3D, color), sizeof(VertexLayout_Color_Skinned_3D), Driver::BufferAttributeFormat::R8G8B8A8_UNORM),
        Driver::make_vertexAttribDesc("JOINTINDICES", OFFSET_OF(VertexLayout_Color_Skinned_3D, joint_indices), sizeof(VertexLayout_Color_Skinned_3D), Driver::BufferAttributeFormat::R8G8B8A8_SINT),
        Driver::make_vertexAttribDesc("JOINTWEIGHTS", OFFSET_OF(VertexLayout_Color_Skinned_3D, joint_weights), sizeof(VertexLayout_Color_Skinned_3D), Driver::BufferAttributeFormat::R8G8B8A8_UNORM)
    };
    const Driver::VertexAttribDesc attribs_textured3d[] = {
        Driver::make_vertexAttribDesc("POSITION", OFFSET_OF(VertexLayout_Textured_3D, pos), sizeof(VertexLayout_Textured_3D), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
        Driver::make_vertexAttribDesc("TEXCOORD", OFFSET_OF(VertexLayout_Textured_3D, uv), sizeof(VertexLayout_Textured_3D), Driver::BufferAttributeFormat::R32G32_FLOAT)
    };
    const Driver::VertexAttribDesc attribs_textured3d_skinned[] = {
        Driver::make_vertexAttribDesc("POSITION", OFFSET_OF(VertexLayout_Textured_Skinned_3D, pos), sizeof(VertexLayout_Textured_Skinned_3D), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
        Driver::make_vertexAttribDesc("TEXCOORD", OFFSET_OF(VertexLayout_Textured_Skinned_3D, uv), sizeof(VertexLayout_Textured_Skinned_3D), Driver::BufferAttributeFormat::R32G32_FLOAT),
        Driver::make_vertexAttribDesc("JOINTINDICES", OFFSET_OF(VertexLayout_Textured_Skinned_3D, joint_indices), sizeof(VertexLayout_Textured_Skinned_3D), Driver::BufferAttributeFormat::R8G8B8A8_SINT),
        Driver::make_vertexAttribDesc("JOINTWEIGHTS", OFFSET_OF(VertexLayout_Textured_Skinned_3D, joint_weights), sizeof(VertexLayout_Textured_Skinned_3D), Driver::BufferAttributeFormat::R8G8B8A8_UNORM)
    };

    // cbuffer bindings
    const Renderer::Driver::CBufferBindingDesc bufferBindings_base[] = {
        { "type_PerScene", Driver::CBufferStageMask::VS },
        { "type_PerGroup", Driver::CBufferStageMask::VS }
    };
    const Renderer::Driver::CBufferBindingDesc bufferBindings_skinned_base[] = {
        { "type_PerScene", Driver::CBufferStageMask::VS },
        { "type_PerGroup", Driver::CBufferStageMask::VS },
        { "type_PerJoint", Driver::CBufferStageMask::VS }
    };
    const Renderer::Driver::CBufferBindingDesc bufferBindings_instanced_base[] = {
        { "type_PerScene", Driver::CBufferStageMask::VS },
        { "type_PerGroup", Driver::CBufferStageMask::VS },
        { "type_PerInstance", Driver::CBufferStageMask::VS }
    };

    // texture bindings
    const Renderer::Driver::TextureBindingDesc textureBindings_base[] = { { "texDiffuse" } };
    const Renderer::Driver::TextureBindingDesc textureBindings_fullscreenblit[] = {{ "texSrc" }};

    // shaders
    {
        Renderer::ShaderDesc desc = {};
        desc.vertexAttrs = attribs_textured3d;
        desc.vertexAttr_count = COUNT_OF(attribs_textured3d);
        desc.textureBindings = textureBindings_fullscreenblit;
        desc.textureBinding_count = COUNT_OF(textureBindings_fullscreenblit);
        desc.bufferBindings = nullptr;
        desc.bufferBinding_count = 0;
        // reuse 3d shaders
        desc.vs_name = Shaders::vs_fullscreen_bufferless_blit.name;
        desc.vs_src = Shaders::vs_fullscreen_bufferless_blit.src;
        desc.ps_name = Shaders::ps_fullscreen_blit_textured.name;
        desc.ps_src = Shaders::ps_fullscreen_blit_textured.src;
        Renderer::compile_shader(store.shaders[ShaderType::FullscreenBlit], desc);
    }
    {
        Renderer::ShaderDesc desc = {};
        desc.vertexAttrs = attribs_3d;
        desc.vertexAttr_count = COUNT_OF(attribs_3d);
        desc.textureBindings = nullptr;
        desc.textureBinding_count = 0;
        desc.bufferBindings = bufferBindings_instanced_base;
        desc.bufferBinding_count = COUNT_OF(bufferBindings_instanced_base);
        desc.vs_name = Shaders::vs_3d_instanced_base.name;
        desc.vs_src = Shaders::vs_3d_instanced_base.src;
        desc.ps_name = Shaders::ps_color3d_unlit.name;
        desc.ps_src = Shaders::ps_color3d_unlit.src;
        Renderer::compile_shader(store.shaders[ShaderType::Instanced3D], desc);
    }
    {
        Renderer::ShaderDesc desc = {};
        desc.vertexAttrs = attribs_color3d;
        desc.vertexAttr_count = COUNT_OF(attribs_color3d);
        desc.textureBindings = nullptr;
        desc.textureBinding_count = 0;
        desc.bufferBindings = bufferBindings_base;
        desc.bufferBinding_count = COUNT_OF(bufferBindings_base);
        desc.vs_name = Shaders::vs_color3d_base.name;
        desc.vs_src = Shaders::vs_color3d_base.src;
        desc.ps_name = Shaders::ps_color3d_unlit.name;
        desc.ps_src = Shaders::ps_color3d_unlit.src;
        Renderer::compile_shader(store.shaders[ShaderType::Color3D], desc);
    }
    {
        Renderer::ShaderDesc desc = {};
        desc.vertexAttrs = attribs_color3d_skinned;
        desc.vertexAttr_count = COUNT_OF(attribs_color3d_skinned);
        desc.textureBindings = nullptr;
        desc.textureBinding_count = 0;
        desc.bufferBindings = bufferBindings_skinned_base;
        desc.bufferBinding_count = COUNT_OF(bufferBindings_skinned_base);
        desc.vs_name = Shaders::vs_color3d_skinned_base.name;
        desc.vs_src = Shaders::vs_color3d_skinned_base.src;
        desc.ps_name = Shaders::ps_color3d_unlit.name;
        desc.ps_src = Shaders::ps_color3d_unlit.src;
        Renderer::compile_shader(store.shaders[ShaderType::Color3DSkinned], desc);
    }
    {
        Renderer::ShaderDesc desc = {};
        desc.vertexAttrs = attribs_textured3d;
        desc.vertexAttr_count = COUNT_OF(attribs_textured3d);
        desc.textureBindings = textureBindings_base;
        desc.textureBinding_count = COUNT_OF(textureBindings_base);
        desc.bufferBindings = bufferBindings_base;
        desc.bufferBinding_count = COUNT_OF(bufferBindings_base);
        desc.vs_name = Shaders::vs_textured3d_base.name;
        desc.vs_src = Shaders::vs_textured3d_base.src;
        desc.ps_name = Shaders::ps_textured3d_base.name;
        desc.ps_src = Shaders::ps_textured3d_base.src;
        Renderer::compile_shader(store.shaders[ShaderType::Textured3D], desc);
    }
    {
        Renderer::ShaderDesc desc = {};
        desc.vertexAttrs = attribs_textured3d;
        desc.vertexAttr_count = COUNT_OF(attribs_textured3d);
        desc.textureBindings = textureBindings_base;
        desc.textureBinding_count = COUNT_OF(textureBindings_base);
        desc.bufferBindings = bufferBindings_base;
        desc.bufferBinding_count = COUNT_OF(bufferBindings_base);
        desc.vs_name = Shaders::vs_textured3d_base.name;
        desc.vs_src = Shaders::vs_textured3d_base.src;
        desc.ps_name = Shaders::ps_textured3dalphaclip_base.name;
        desc.ps_src = Shaders::ps_textured3dalphaclip_base.src;
        Renderer::compile_shader(store.shaders[ShaderType::Textured3DAlphaClip], desc);
    }
    {
        Renderer::ShaderDesc desc = {};
        desc.vertexAttrs = attribs_textured3d_skinned;
        desc.vertexAttr_count = COUNT_OF(attribs_textured3d_skinned);
        desc.textureBindings = textureBindings_base;
        desc.textureBinding_count = COUNT_OF(textureBindings_base);
        desc.bufferBindings = bufferBindings_skinned_base;
        desc.bufferBinding_count = COUNT_OF(bufferBindings_skinned_base);
        desc.vs_name = Shaders::vs_textured3d_skinned_base.name;
        desc.vs_src = Shaders::vs_textured3d_skinned_base.src;
        desc.ps_name = Shaders::ps_textured3d_base.name;
        desc.ps_src = Shaders::ps_textured3d_base.src;
        Renderer::compile_shader(store.shaders[ShaderType::Textured3DSkinned], desc);
    }
    {
        Renderer::ShaderDesc desc = {};
        desc.vertexAttrs = attribs_textured3d_skinned;
        desc.vertexAttr_count = COUNT_OF(attribs_textured3d_skinned);
        desc.textureBindings = textureBindings_base;
        desc.textureBinding_count = COUNT_OF(textureBindings_base);
        desc.bufferBindings = bufferBindings_skinned_base;
        desc.bufferBinding_count = COUNT_OF(bufferBindings_skinned_base);
        desc.vs_name = Shaders::vs_textured3d_skinned_base.name;
        desc.vs_src = Shaders::vs_textured3d_skinned_base.src;
        desc.ps_name = Shaders::ps_textured3dalphaclip_base.name;
        desc.ps_src = Shaders::ps_textured3dalphaclip_base.src;
        Renderer::compile_shader(store.shaders[ShaderType::Textured3DAlphaClipSkinned], desc);
    }

    struct AssetData {
        const char* path;
        float3 asset_init_pos;
        u32 count;
        bool player; // only first count
    };
    // from blender: export fbx -> Apply Scalings: FBX All -> Forward: the one in Blender -> Use Space Transform: yes
    const AssetData assets[] = {
          { "assets/meshes/boar.fbx", { 5.f, 10.f, 2.30885f }, 1, false }
        , { "assets/meshes/bird.fbx", { 1.f, 3.f, 2.23879f }, 1, true }
        , { "assets/meshes/bird.fbx", { 0.f, 5.f, 2.23879f }, 250, false }
    };

    FBX::PipelineAssetContext ctx = {};
    ctx.vertexAttrs[DrawlistStreams::Color3D] = attribs_color3d;
    ctx.attr_count[DrawlistStreams::Color3D] = COUNT_OF(attribs_color3d);
    ctx.vertexAttrs[DrawlistStreams::Color3DSkinned] = attribs_color3d_skinned;
    ctx.attr_count[DrawlistStreams::Color3DSkinned] = COUNT_OF(attribs_color3d_skinned);
    ctx.vertexAttrs[DrawlistStreams::Textured3D] = attribs_textured3d;
    ctx.attr_count[DrawlistStreams::Textured3D] = COUNT_OF(attribs_textured3d);
    ctx.vertexAttrs[DrawlistStreams::Textured3DAlphaClip] = attribs_textured3d;
    ctx.attr_count[DrawlistStreams::Textured3DAlphaClip] = COUNT_OF(attribs_textured3d);
    ctx.vertexAttrs[DrawlistStreams::Textured3DSkinned] = attribs_textured3d_skinned;
    ctx.attr_count[DrawlistStreams::Textured3DSkinned] = COUNT_OF(attribs_textured3d_skinned);
    ctx.vertexAttrs[DrawlistStreams::Textured3DAlphaClipSkinned] = attribs_textured3d_skinned;
    ctx.attr_count[DrawlistStreams::Textured3DAlphaClipSkinned] = COUNT_OF(attribs_textured3d_skinned);

    for (u32 asset_idx = 0; asset_idx < COUNT_OF(assets); asset_idx++) {
        const AssetData& asset = assets[asset_idx];

        for (u32 asset_rep = 0; asset_rep < asset.count; asset_rep++) {
            Animation::AnimatedNode animatedNode = {};

            float3 asset_init_pos = asset.asset_init_pos;
            const f32 spacing = 10.f;
            const s32 grid_size = 6;
            const s32 max_row = (asset.count - 1) % grid_size;
            const s32 max_column = ((asset.count - 1) / grid_size) % grid_size;
            const s32 max_stride = (asset.count - 1) / (grid_size * grid_size);
            s32 row = asset_rep % grid_size;
            s32 column = (asset_rep / grid_size) % grid_size;
            s32 stride = asset_rep / (grid_size * grid_size);
            asset_init_pos = Math::add(asset_init_pos, { (row - max_row /2) * spacing, (column - max_column/2) * spacing, (stride - max_stride / 2) * spacing });

            u32 nodeHandle = 0;
		    ctx.scratchArena = scratchArena; // explicit copy
            FBX::load_with_materials(nodeHandle, animatedNode, store, ctx, asset.path);

            if ((nodeHandle >> 16) & DrawNodeMeshType::Skinned) {
			    DrawNodeSkinned& node = Allocator::get_pool_slot(store.drawNodesSkinned, nodeHandle & 0xffff);
                Math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
                node.nodeData.worldMatrix.col3 = { asset_init_pos, 1.f };
			    node.nodeData.groupColor = Color32(1.f, 1.f, 1.f, 1.f).RGBAv4();
                for (u32 m = 0; m < animatedNode.skeleton.jointCount; m++) {
                    float4x4& matrix = node.skinningMatrixPalette.data[m];
                    Math::identity4x4(*(Transform*)&(matrix));
                }
		    }
		    else if ((nodeHandle >> 16) & DrawNodeMeshType::Default) {
                DrawNode& node = Allocator::get_pool_slot(store.drawNodes, nodeHandle & 0xffff);
                Math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
                node.nodeData.worldMatrix.col3 = { asset_init_pos, 1.f };
                node.nodeData.groupColor = Color32(1.f, 1.f, 1.f, 1.f).RGBAv4();
            }

            u32 animHandle = 0;
            if (animatedNode.skeleton.jointCount > 0) {
                animatedNode.state.animIndex = 0;
                animatedNode.state.time = 0.f;
                animatedNode.state.scale = 1.f;
                animatedNode.drawNodeHandle = nodeHandle;
                Animation::AnimatedNode& animatedNodeToAdd = Allocator::alloc_pool(store.animatedNodes);
                animatedNodeToAdd = animatedNode;
                animHandle = handle_from_animatedNode(store, animatedNodeToAdd);
            }

            if (asset.player) {
                store.playerAnimatedNodeHandle = animHandle;
                store.playerDrawNodeHandle = nodeHandle;
            }
        }
    }

    // unit cubes
    {
        Renderer::DrawMesh& mesh = Allocator::alloc_pool(store.meshes);
        mesh = {};
        mesh.type = ShaderType::Instanced3D;
        Renderer::create_indexed_vertex_buffer_from_untextured_cube(mesh.vertexBuffer, { 1.f, 1.f, 1.f });
        DrawNodeInstanced& node = Allocator::alloc_pool(store.drawNodesInstanced);
        node = {};
		node.meshHandles[0] = handle_from_drawMesh(store, mesh);
        Math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
        node.nodeData.groupColor = Color32(0.9f, 0.7f, 0.8f, 0.6f).RGBAv4();
        node.instanceCount = 4;
        node.cbuffer_node = store.cbuffer_count;
        Driver::create_cbuffer(store.cbuffers[store.cbuffer_count++], { sizeof(NodeData) });
        node.cbuffer_instances = store.cbuffer_count;
        Driver::create_cbuffer(store.cbuffers[store.cbuffer_count++], { sizeof(Matrices64) });
        for (u32 m = 0; m < COUNT_OF(node.instanceMatrices.data); m++) {
            float4x4& matrix = node.instanceMatrices.data[m];
            Math::identity4x4(*(Transform*)&(matrix));
        }
        store.particlesDrawHandle = (DrawNodeMeshType::Instanced << 16) | (Allocator::get_pool_index(store.drawNodesInstanced, node) & 0xffff); // todo: consolidate this
    }
}
}

#endif // __WASTELADNS_GAMERENDERER_H__
