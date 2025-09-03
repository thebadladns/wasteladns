#ifndef __WASTELADNS_SCENE_H__
#define __WASTELADNS_SCENE_H__

namespace game {

const f32 SDF_scene_radius = 8.5f;

struct Mirrors { // todo: figure out delete, unhack sizes
    struct Poly { float3 v[4]; float3 normal; u32 numPts; };
    Poly* polys; // xy from 0.f to 1.f describe the mirror quad, z is the normal
    renderer::DrawMesh* drawMeshes;
    bvh::Tree bvh; // used to accelerate visibility queries
    u32 count;
};
struct GPUCPUMesh {
    renderer::CPUMesh cpuBuffer;
    gfx::rhi::RscIndexedVertexBuffer gpuBuffer;
};

struct Scene {
    struct InstancedTypes { enum Enum { PlayerTrail, PhysicsBalls, SceneLimits, Count }; };
    camera::Camera camera;
    renderer::Scene renderScene;
    physics::Scene physicsScene;
    animation::Scene animScene;
    MovementController player;
    Mirrors mirrors;
    game::OrbitInput orbitCamera;
    u32 instancedNodesHandles[InstancedTypes::Count];
    u32 playerDrawNodeHandle;
    u32 playerAnimatedNodeHandle;
    u32 playerPhysicsNodeHandle;
    u32 maxMirrorBounces;
};
struct AssetInMemory {
    // render
    float3 min;
    float3 max;
    renderer::MeshHandle meshHandles[renderer::DrawlistStreams::Count];
    // animation
    animation::Skeleton skeleton;
    animation::Clip* clips;
    u32 clipCount;
};
struct Resources {
    struct AssetsMeta { enum Enum { Bird, Ground, BackMirrors, Count }; };
    struct MirrorHallMeta { enum Enum { Count = 4 }; };
    renderer::CoreResources renderCore;
    AssetInMemory assets[AssetsMeta::Count];
    GPUCPUMesh mirrorHallMesh;
    renderer::MeshHandle instancedUnitCubeMesh;
    renderer::MeshHandle instancedUnitSphereMesh;
    renderer::MeshHandle groundMesh;
    // UI
    __DEBUGDEF(gfx::rhi::RscIndexedVertexBuffer gpuBufferDebugText;)
    gfx::rhi::RscIndexedVertexBuffer gpuBufferHeaderText;
    gfx::rhi::RscIndexedVertexBuffer gpuBufferOrbitText;
    gfx::rhi::RscIndexedVertexBuffer gpuBufferPanText;
    gfx::rhi::RscIndexedVertexBuffer gpuBufferScaleText;
    gfx::rhi::RscIndexedVertexBuffer gpuBufferCycleRoomText;
};
struct RoomDefinition {
    float3 minCameraEulers;
    float3 maxCameraEulers;
    f32 minCameraZoom;
    f32 maxCameraZoom;
    u32 maxMirrorBounces;
    bool physicsBalls;
};
const RoomDefinition roomDefinitions[] = {
    { float3(-180.f * math::d2r32, 0.f, -180 * math::d2r32), // min camera eulers
      float3(-3.f * math::d2r32, 0.f, 180 * math::d2r32), // max camera eulers
      0.3f, 2.f,
      8, true }
};

void spawnAsset(
    renderer::DrawNodeHandle& renderHandle, animation::Handle& animationHandle,
Scene& scene, const AssetInMemory& def, const float3& spawnPos) {
    // render data
    renderer::Scene& renderScene = scene.renderScene;
    renderer::DrawNode& renderNode = allocator::alloc_pool(renderScene.drawNodes);
    renderHandle = renderer::handle_from_node(renderScene, renderNode);
    renderNode = {};
    math::identity4x4(*(Transform*)&(renderNode.nodeData.worldMatrix));
    renderNode.nodeData.worldMatrix.col3.xyz = { spawnPos };
    renderNode.nodeData.groupColor = Color32(1.f, 1.f, 1.f, 1.f).RGBAv4();
    renderNode.min = def.min;
    renderNode.max = def.max;
    memcpy(renderNode.meshHandles, def.meshHandles, sizeof(renderNode.meshHandles));
    gfx::rhi::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
    renderNode.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
    gfx::rhi::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });
    if (def.skeleton.jointCount) {
        animation::Scene& animScene = scene.animScene;
        animation::Node& animNode = allocator::alloc_pool(animScene.nodes);
        animationHandle = animation::handle_from_node(animScene, animNode);
        animNode = {};
        animNode.skeleton = def.skeleton;
        animNode.clips = def.clips;
        animNode.clipCount = def.clipCount;
        animNode.state.animIndex = 0;
        animNode.state.time = 0.f;
        animNode.state.scale = 1.f;
        for (u32 m = 0; m < animNode.skeleton.jointCount; m++) {
            float4x4& matrix = animNode.state.skinning[m];
            math::identity4x4(*(Transform*)&(matrix));
        }
        // skinning data for rendering
        gfx::rhi::RscCBuffer& cbufferskinning =
            allocator::alloc_pool(renderScene.cbuffers);
        renderNode.cbuffer_ext = handle_from_cbuffer(renderScene, cbufferskinning);
        gfx::rhi::create_cbuffer(cbufferskinning,
            { (u32) sizeof(float4x4) * animNode.skeleton.jointCount });
        renderNode.ext_data = animNode.state.skinning;
    }
    // todo: physics??
}

void spawn_model_as_mirrors(
    game::Mirrors& mirrors, const game::GPUCPUMesh& loadedMesh,
    allocator::PagedArena scratchArena, allocator::PagedArena& sceneArena, bool accelerateBVH) {

    const renderer::CPUMesh& cpuMesh = loadedMesh.cpuBuffer;
    u32* triangleIds;
    if (accelerateBVH) {
        triangleIds = ALLOC_ARRAY(scratchArena, u32, (cpuMesh.indexCount / 3));
    }

    u32 index = 0;
    u32 triangles = 0;
    while (index + 3 <= cpuMesh.indexCount) {
        float3 v0 = cpuMesh.vertices[cpuMesh.indices[index]];
        float3 v1 = cpuMesh.vertices[cpuMesh.indices[index + 1]];
        float3 v2 = cpuMesh.vertices[cpuMesh.indices[index + 2]];

        const u32 mirrorId = mirrors.count++;
        game::Mirrors::Poly& poly = mirrors.polys[mirrorId];
        renderer::DrawMesh& mesh = mirrors.drawMeshes[mirrorId];

        // reference triangles in the gpu mesh
        mesh.shaderTechnique = renderer::ShaderTechniques::Color3D;
        mesh.vertexBuffer = loadedMesh.gpuBuffer; // copy buffer
        mesh.vertexBuffer.indexOffset = index;
        mesh.vertexBuffer.indexCount = 3;

        poly.numPts = 3;
        poly.v[0] = v0;
        poly.v[1] = v1;
        poly.v[2] = v2;
        if (accelerateBVH) { triangleIds[triangles++] = mirrorId; }

        if (index + 6 <= cpuMesh.indexCount) {
            float3 v3 = cpuMesh.vertices[cpuMesh.indices[index + 3]];
            float3 v4 = cpuMesh.vertices[cpuMesh.indices[index + 4]];
            float3 v5 = cpuMesh.vertices[cpuMesh.indices[index + 5]];
            const float3 normal012 = math::normalize(math::cross(math::subtract(v1, v0), math::subtract(v2, v0)));
            const float3 normal345 = math::normalize(math::cross(math::subtract(v4, v3), math::subtract(v5, v3)));
            const float3 normalCross = math::cross(normal012, normal345);
            if (math::isCloseAll(normalCross, float3(0.f, 0.f, 0.f), 0.01f)) {
                // coplanar enough (may still be degenerate, but we otherwise have too many mirrors
                // add a quad
                poly.numPts = 4;
                poly.v[3] = v3;
                mesh.vertexBuffer.indexCount = 6;
                if (accelerateBVH) { triangleIds[triangles++] = mirrorId; }
            }
        }
        poly.normal = math::normalize(math::cross(math::subtract(poly.v[2], poly.v[0]), math::subtract(poly.v[1], poly.v[0])));
        index += mesh.vertexBuffer.indexCount;
    }

    if (accelerateBVH) {
        bvh::buildTree(
            sceneArena, scratchArena, mirrors.bvh, &(loadedMesh.cpuBuffer.vertices[0].x),
            loadedMesh.cpuBuffer.indices, loadedMesh.cpuBuffer.indexCount, triangleIds);
    }
}
}

namespace fbx {

void from_ufbx_mat(float4x4& o, const ufbx_matrix& i) {
    o.col0.x = i.cols[0].x; o.col0.y = i.cols[0].y; o.col0.z = i.cols[0].z; o.col0.w = 0.f;
    o.col1.x = i.cols[1].x; o.col1.y = i.cols[1].y; o.col1.z = i.cols[1].z; o.col1.w = 0.f;
    o.col2.x = i.cols[2].x; o.col2.y = i.cols[2].y; o.col2.z = i.cols[2].z; o.col2.w = 0.f;
    o.col3.x = i.cols[3].x; o.col3.y = i.cols[3].y; o.col3.z = i.cols[3].z; o.col3.w = 1.f;
};

void extract_anim_data(game::AssetInMemory& assetToAdd,
                       allocator::PagedArena& persistentArena, const ufbx_mesh& mesh,
                       const ufbx_scene& scene) {

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

    animation::Skeleton& skeleton = assetToAdd.skeleton;
    u32* node_indices_skeleton = ALLOC_ARRAY(persistentArena, u32, jointCount);
    s8* nodeIdtoJointId = ALLOC_ARRAY(persistentArena, s8, scene.nodes.count);
    skeleton.jointFromGeometry = ALLOC_ARRAY(persistentArena, float4x4, jointCount);
    skeleton.parentFromPosedJoint = ALLOC_ARRAY(persistentArena, float4x4, jointCount);
    skeleton.geometryFromPosedJoint = ALLOC_ARRAY(persistentArena, float4x4, jointCount);
    skeleton.parentIndices = ALLOC_ARRAY(persistentArena, s8, jointCount);
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
    assetToAdd.clipCount = (u32)scene.anim_stacks.count;
    assetToAdd.clips = ALLOC_ARRAY(persistentArena, animation::Clip, scene.anim_stacks.count);
    for (size_t i = 0; i < assetToAdd.clipCount; i++) {
        const ufbx_anim_stack& stack = *(scene.anim_stacks.data[i]);
        const f64 targetFramerate = 12.f;
        const u32 maxFrames = 4096;
        const f64 duration = (f32)stack.time_end - (f32)stack.time_begin;
        const u32 numFrames = math::clamp((u32)(duration * targetFramerate), 2u, maxFrames);
        const f64 framerate = (numFrames-1) / duration;

        animation::Clip& clip = assetToAdd.clips[i];
        clip.frameCount = numFrames;
        clip.joints = ALLOC_ARRAY(persistentArena, animation::JointKeyframes, jointCount);
        clip.timeEnd = (f32)stack.time_end;
        for (u32 joint_index = 0; joint_index < jointCount; joint_index++) {
            ufbx_node* node = scene.nodes.data[node_indices_skeleton[joint_index]];
			animation::JointKeyframes& joint = clip.joints[joint_index];
            joint.joint_to_parent_trs =
                ALLOC_ARRAY(persistentArena, animation::Joint_TRS, numFrames);
            for (u32 f = 0; f < numFrames; f++) {
                animation::Joint_TRS& keyframeJoint = joint.joint_to_parent_trs[f];
                f64 time = stack.time_begin + (double)f / framerate;
                ufbx_transform transform = ufbx_evaluate_transform(&stack.anim, node, time);
                keyframeJoint.rotation.x = transform.rotation.x;
                keyframeJoint.rotation.y = transform.rotation.y;
                keyframeJoint.rotation.z = transform.rotation.z;
                keyframeJoint.rotation.w = transform.rotation.w;
                keyframeJoint.translation.x = transform.translation.x;
                keyframeJoint.translation.y = transform.translation.y;
                keyframeJoint.translation.z = transform.translation.z;
                keyframeJoint.scale.x = transform.scale.x;
                keyframeJoint.scale.y = transform.scale.y;
                keyframeJoint.scale.z = transform.scale.z;
            }
        }
    }
}
void extract_skinning_attribs(u8* joint_indices, u8* joint_weights,
                              const ufbx_mesh& mesh,const u32 vertex_id) {
    ufbx_skin_deformer& skin = *(mesh.skin_deformers.data[0]);
    ufbx_skin_vertex& skinned = skin.vertices.data[vertex_id];
    f32 total_weight = 0.f;
    const u32 num_weights = math::min(skinned.num_weights, 4u);
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
    allocator::PagedArena scratchArena;
    allocator::PagedArena& persistentArena;
    const gfx::rhi::VertexAttribDesc* vertexAttrs[renderer::DrawlistStreams::Count];
    u32 attr_count[renderer::DrawlistStreams::Count];
};
bool load_with_materials(
    game::AssetInMemory& assetToAdd, renderer::CoreResources& renderCore,
    PipelineAssetContext& pipelineContext, const char* path) {
   
    bool success = false;

    ufbx_load_opts opts = {};
    opts.target_axes = {
        UFBX_COORDINATE_AXIS_POSITIVE_X,
        UFBX_COORDINATE_AXIS_POSITIVE_Z,
        UFBX_COORDINATE_AXIS_POSITIVE_Y
    };
    opts.allow_null_material = true;
    ufbx_error error;
    
    setup_fbx_arena(opts.temp_allocator.allocator, pipelineContext.scratchArena);
    setup_fbx_arena(opts.result_allocator.allocator, pipelineContext.scratchArena);

    ufbx_scene* scene = ufbx_load_file(path, &opts, &error);
    if (scene) {
        // Process all vertices first, and then split then into the supported material buffers
        u32 maxVertices = 0;
        for (size_t i = 0; i < scene->meshes.count; i++) {
            maxVertices += (u32)scene->meshes.data[i]->num_vertices;
        }

        allocator::Buffer<float3> vertices = {};
        allocator::reserve(vertices, maxVertices, pipelineContext.scratchArena);

        struct DstStreams {
            allocator::Buffer_t vertex;
            allocator::Buffer<u32> index;
            void* user;
            u32 vertex_size;
            u32 vertex_align;
        };
        DstStreams materialVertexBuffer[renderer::DrawlistStreams::Count] = {};
		materialVertexBuffer[renderer::DrawlistStreams::Color3D].vertex_size =
            sizeof(renderer::VertexLayout_Color_3D);
		materialVertexBuffer[renderer::DrawlistStreams::Color3D].vertex_align =
            alignof(renderer::VertexLayout_Color_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Color3DSkinned].vertex_size =
            sizeof(renderer::VertexLayout_Color_Skinned_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Color3DSkinned].vertex_align =
            alignof(renderer::VertexLayout_Color_Skinned_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Textured3D].vertex_size =
            sizeof(renderer::VertexLayout_Textured_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Textured3D].vertex_align =
            alignof(renderer::VertexLayout_Textured_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Textured3DAlphaClip].vertex_size =
            sizeof(renderer::VertexLayout_Textured_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Textured3DAlphaClip].vertex_align =
            alignof(renderer::VertexLayout_Textured_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Textured3DSkinned].vertex_size =
            sizeof(renderer::VertexLayout_Textured_Skinned_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Textured3DSkinned].vertex_align =
            alignof(renderer::VertexLayout_Textured_Skinned_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Textured3DAlphaClipSkinned].vertex_size =
            sizeof(renderer::VertexLayout_Textured_Skinned_3D);
        materialVertexBuffer[renderer::DrawlistStreams::Textured3DAlphaClipSkinned].vertex_align =
            alignof(renderer::VertexLayout_Textured_Skinned_3D);

		for (u32 i = 0; i < renderer::DrawlistStreams::Count; i++) {
            DstStreams& stream = materialVertexBuffer[i];
            allocator::reserve(stream.vertex, maxVertices, stream.vertex_size,
                               stream.vertex_align, pipelineContext.scratchArena);
            allocator::reserve(stream.index, maxVertices * 3 * 2, pipelineContext.scratchArena);
		}

        // hack: only consider skinning from first mesh
        if (scene->meshes.count && scene->meshes[0]->skin_deformers.count) {
            extract_anim_data(
                assetToAdd, pipelineContext.persistentArena, *(scene->meshes[0]), *scene);
        }

        // TODO: consider skinning
        float3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
        float3 max = {-FLT_MAX,-FLT_MAX,-FLT_MAX };

        u32 vertexOffset = 0;
        for (size_t i = 0; i < scene->meshes.count; i++) {
            ufbx_mesh& mesh = *scene->meshes.data[i];
            // Extract vertices from this mesh and flatten any transform trees
            // This assumes there's only one instance of this mesh, we don't support more yet
            {
                // todo: consider asset transform
                assert(mesh.instances.count == 1);
                const ufbx_matrix& m = mesh.instances.data[0]->geometry_to_world;
                for (size_t v = 0; v < mesh.num_vertices; v++) {
                    float3 vertex;
                    const ufbx_float3& v_fbx_ls = mesh.vertices[v];
                        
                    float3 transformed;
                    transformed.x =
                        v_fbx_ls.x * m.m00 + v_fbx_ls.y * m.m01 + v_fbx_ls.z * m.m02 + m.m03;
                    transformed.y =
                        v_fbx_ls.x * m.m10 + v_fbx_ls.y * m.m11 + v_fbx_ls.z * m.m12 + m.m13;
                    transformed.z =
                        v_fbx_ls.x * m.m20 + v_fbx_ls.y * m.m21 + v_fbx_ls.z * m.m22 + m.m23;
                    max = math::max(transformed, max);
                    min = math::min(transformed, min);

                    // If using skinning, we'll store the node hierarchy in the joints
                    if (mesh.skin_deformers.count) {
                        vertex.x = v_fbx_ls.x;
                        vertex.y = v_fbx_ls.y;
                        vertex.z = v_fbx_ls.z;
                        allocator::push(vertices, pipelineContext.scratchArena) = vertex;
                    } else {
                        allocator::push(vertices, pipelineContext.scratchArena) = transformed;
                    }
                }
            }

            struct MaterialVertexBufferContext {
                u32* fbxvertex_to_dstvertex;
                u32* dstvertex_to_fbxvertex;
                u32* dstvertex_to_fbxidx;
            };
            const auto add_material_indices_to_stream =
                [](DstStreams & stream_dst,MaterialVertexBufferContext & ctx,
                   allocator::PagedArena & scratchArena, const u32 vertices_src_count,
                   const ufbx_mesh & mesh, const ufbx_mesh_material & mesh_mat) {

                memset(ctx.fbxvertex_to_dstvertex, 0xffffffff, sizeof(u32) * vertices_src_count);

                for (size_t f = 0; f < mesh_mat.num_faces; f++) {
                    const u32 maxTriIndices = 32;
                    u32 triIndices[maxTriIndices];

                    ufbx_face& face = mesh.faces.data[mesh_mat.face_indices.data[f]];
                    const size_t num_tris =
                        ufbx_triangulate_face(triIndices, maxTriIndices, &mesh, face);
                    for (size_t v = 0; v < num_tris * 3; v++) {
                        const uint32_t triangulatedFaceIndex = triIndices[v];
                        const u32 src_index = mesh.vertex_indices[triangulatedFaceIndex];

                        if (ctx.fbxvertex_to_dstvertex[src_index] != 0xffffffff) {
                            // Vertex has been updated in the new buffer, copy the index over
                            allocator::push(stream_dst.index, scratchArena) =
                                ctx.fbxvertex_to_dstvertex[src_index];
                        }
                        else {
                            // Vertex needs to be added to the new buffer, and index updated
                            u32 dst_index = (u32)stream_dst.vertex.len;
                            // mark where the ufbx data is in relation to the destination vertex
                            ctx.dstvertex_to_fbxvertex[dst_index] = src_index;
                            ctx.dstvertex_to_fbxidx[dst_index] = triangulatedFaceIndex;
                            allocator::push(stream_dst.vertex, stream_dst.vertex_size,
                                            stream_dst.vertex_align, scratchArena);
                            // mark where this vertex is in the destination array
                            ctx.fbxvertex_to_dstvertex[src_index] = dst_index;

                            allocator::push(stream_dst.index, scratchArena) = dst_index;
                        }
                    }
                }
            };
            // Extract all indices, separated by supported drawlist
            // They will all point the same vertex buffer array, which is not ideal,
            // but I don't want to deal with re-building a vertex buffer for each index buffer
            const u32 mesh_vertices_count = (u32)mesh.num_vertices;
            const float3* mesh_vertices = &vertices.data[vertexOffset];
            MaterialVertexBufferContext ctx = {};
			// tmp buffers to store the mapping between fbx mesh vertices and dest vertex streams
            ctx.dstvertex_to_fbxvertex =
                ALLOC_ARRAY(pipelineContext.scratchArena, u32, mesh_vertices_count);
            ctx.dstvertex_to_fbxidx =
                ALLOC_ARRAY(pipelineContext.scratchArena, u32, mesh_vertices_count);
            ctx.fbxvertex_to_dstvertex =
                ALLOC_ARRAY(pipelineContext.scratchArena, u32, mesh_vertices_count);
            for (size_t m = 0; m < mesh.materials.count; m++) {
                ufbx_mesh_material& mesh_mat = mesh.materials.data[m];
                if (assetToAdd.skeleton.jointCount) {
                    if (mesh_mat.material && mesh_mat.material->pbr.base_color.has_value) {
                        if (mesh_mat.material->pbr.base_color.texture_enabled) { // textured
                            // Assume that opacity tied to a texture means that
                            // we should use the alpha clip shader
                            DstStreams& stream = (mesh_mat.material->pbr.opacity.texture_enabled) ?
                                  materialVertexBuffer[renderer::DrawlistStreams::Textured3DAlphaClipSkinned]
							    : materialVertexBuffer[renderer::DrawlistStreams::Textured3DSkinned];

                            assert(!stream.user); // we only support one textured-material of each pass
                            u32 stream_current_offset = (u32)stream.vertex.len;
                            add_material_indices_to_stream(
                                stream, ctx,pipelineContext.scratchArena,
                                mesh_vertices_count, mesh, mesh_mat);
                            for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                renderer::VertexLayout_Textured_Skinned_3D& dst =
                                    *(renderer::VertexLayout_Textured_Skinned_3D*)(stream.vertex.data + i * stream.vertex_size);
                                ufbx_float2& uv = mesh.vertex_uv[ctx.dstvertex_to_fbxidx[i]];
                                const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                dst.pos = { src.x, src.y, src.z };
                                dst.uv.x = uv.x;
                                dst.uv.y = uv.y;
                                extract_skinning_attribs(
                                    dst.joint_indices, dst.joint_weights,
                                    mesh, ctx.dstvertex_to_fbxvertex[i]);
                            }
                            stream.user = mesh_mat.material->pbr.base_color.texture;
                        } else { // material color
                            DstStreams& stream = materialVertexBuffer[renderer::DrawlistStreams::Color3DSkinned];
                            u32 stream_current_offset = (u32)stream.vertex.len;
                            add_material_indices_to_stream(
                                stream, ctx, pipelineContext.scratchArena,
                                mesh_vertices_count, mesh, mesh_mat);
                            for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                renderer::VertexLayout_Color_Skinned_3D& dst =
                                    *(renderer::VertexLayout_Color_Skinned_3D*)(stream.vertex.data + i * stream.vertex_size);
                                ufbx_float4& c = mesh_mat.material->pbr.base_color.value_float4;
                                const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                dst.pos = { src.x, src.y, src.z };
                                dst.color = Color32(c.x, c.y, c.z, c.w).ABGR();
                                extract_skinning_attribs(
                                    dst.joint_indices, dst.joint_weights,
                                    mesh, ctx.dstvertex_to_fbxvertex[i]);
                            }
                        }
                    } else if (mesh.vertex_color.exists) { // vertex color
                        DstStreams& stream = materialVertexBuffer[renderer::DrawlistStreams::Color3DSkinned];
                        u32 stream_current_offset = (u32)stream.vertex.len;
                        add_material_indices_to_stream(
                            stream, ctx, pipelineContext.scratchArena,
                            mesh_vertices_count, mesh, mesh_mat);
                        for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                            renderer::VertexLayout_Color_Skinned_3D& dst =
                                *(renderer::VertexLayout_Color_Skinned_3D*)(stream.vertex.data + i * stream.vertex_size);
                            const ufbx_float4& c = mesh.vertex_color[ctx.dstvertex_to_fbxidx[i]];
                            const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                            dst.pos = { src.x, src.y, src.z };
                            dst.color = Color32(c.x, c.y, c.z, c.w).ABGR();
                            extract_skinning_attribs(
                                dst.joint_indices, dst.joint_weights,
                                mesh, ctx.dstvertex_to_fbxvertex[i]);
                        }
                    } else { // no material info
                        DstStreams& stream = materialVertexBuffer[renderer::DrawlistStreams::Color3DSkinned];
                        u32 stream_current_offset = (u32)stream.vertex.len;
                        add_material_indices_to_stream(
                            stream, ctx, pipelineContext.scratchArena,
                            mesh_vertices_count, mesh, mesh_mat);
                        for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                            renderer::VertexLayout_Color_Skinned_3D& dst =
                                *(renderer::VertexLayout_Color_Skinned_3D*)(stream.vertex.data + i * stream.vertex_size);
                            const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                            dst.pos = { src.x, src.y, src.z };
                            // no vertex color available, write a dummy
                            dst.color = Color32(1.f, 0.f, 1.f, 0.f).ABGR();
                            extract_skinning_attribs(
                                dst.joint_indices, dst.joint_weights,
                                mesh, ctx.dstvertex_to_fbxvertex[i]);
                        }
                    }
                } else {
                    if (mesh_mat.material && mesh_mat.material->pbr.base_color.has_value) {
                        if (mesh_mat.material->pbr.base_color.texture_enabled) { // textured
                            // Assume that opacity tied to a texture means that we should use the alpha clip shader
                            DstStreams& stream = (mesh_mat.material->pbr.opacity.texture_enabled) ?
                                  materialVertexBuffer[renderer::DrawlistStreams::Textured3DAlphaClip]
                                : materialVertexBuffer[renderer::DrawlistStreams::Textured3D];
                            // we only support one textured-material of each pass
                            assert(!stream.user);
                            u32 stream_current_offset = (u32)stream.vertex.len;
                            add_material_indices_to_stream(
                                stream, ctx, pipelineContext.scratchArena,
                                mesh_vertices_count, mesh, mesh_mat);
                            for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                renderer::VertexLayout_Textured_3D& dst =
                                    *(renderer::VertexLayout_Textured_3D*)(stream.vertex.data + i * stream.vertex_size);
                                ufbx_float2& uv = mesh.vertex_uv[ctx.dstvertex_to_fbxidx[i]];
                                const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                dst.pos = { src.x, src.y, src.z };
                                dst.uv.x = uv.x;
                                dst.uv.y = uv.y;
                            }
                            stream.user = mesh_mat.material->pbr.base_color.texture;
                        } else { // material color
                            DstStreams& stream = materialVertexBuffer[renderer::DrawlistStreams::Color3D];
                            u32 stream_current_offset = (u32)stream.vertex.len;
                            add_material_indices_to_stream(
                                stream, ctx, pipelineContext.scratchArena,
                                mesh_vertices_count, mesh, mesh_mat);
                            for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                                renderer::VertexLayout_Color_3D& dst =
                                    *(renderer::VertexLayout_Color_3D*)(stream.vertex.data + i * stream.vertex_size);
                                ufbx_float4& c = mesh_mat.material->pbr.base_color.value_float4;
                                const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                                dst.pos = { src.x, src.y, src.z };
                                dst.color = Color32(c.x, c.y, c.z, c.w).ABGR();
                            }
                        }
                    } else if (mesh.vertex_color.exists) { // vertex color
                        DstStreams& stream = materialVertexBuffer[renderer::DrawlistStreams::Color3D];
                        u32 stream_current_offset = (u32)stream.vertex.len;
                        add_material_indices_to_stream(
                            stream, ctx, pipelineContext.scratchArena,
                            mesh_vertices_count, mesh, mesh_mat);
                        for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                            renderer::VertexLayout_Color_3D& dst =
                                *(renderer::VertexLayout_Color_3D*)(stream.vertex.data + i * stream.vertex_size);
                            const ufbx_float4& c = mesh.vertex_color[ctx.dstvertex_to_fbxidx[i]];
                            const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                            dst.pos = { src.x, src.y, src.z };
                            dst.color = Color32(c.x, c.y, c.z, c.w).ABGR();
                        }
                    } else { // no material info
                        DstStreams& stream = materialVertexBuffer[renderer::DrawlistStreams::Color3D];
                        u32 stream_current_offset = (u32)stream.vertex.len;
                        add_material_indices_to_stream(
                            stream, ctx, pipelineContext.scratchArena,
                            mesh_vertices_count, mesh, mesh_mat);
                        for (u32 i = stream_current_offset; i < stream.vertex.len; i++) {
                            renderer::VertexLayout_Color_3D& dst =
                                *(renderer::VertexLayout_Color_3D*)(stream.vertex.data + i * stream.vertex_size);
                            const float3& src = mesh_vertices[ctx.dstvertex_to_fbxvertex[i]];
                            dst.pos = { src.x, src.y, src.z };
                            dst.color = Color32(1.f, 0.f, 1.f, 0.f).ABGR(); // no vertex color available, write a dummy
                        }
                    }
                }
            }
            vertexOffset += (u32)mesh.num_vertices;
        }

        assetToAdd.max = max;
        assetToAdd.min = min;

        const renderer::ShaderTechniques::Enum shaderTechniques[renderer::DrawlistStreams::Count] = {
            renderer::ShaderTechniques::Color3D, renderer::ShaderTechniques::Color3DSkinned,
            renderer::ShaderTechniques::Textured3D, renderer::ShaderTechniques::Textured3DAlphaClip,
            renderer::ShaderTechniques::Textured3DSkinned,
            renderer::ShaderTechniques::Textured3DAlphaClipSkinned
        };
		for (u32 i = 0; i < renderer::DrawlistStreams::Count; i++) {
			DstStreams& stream = materialVertexBuffer[i];
			if (!stream.vertex.len) { continue; }

            gfx::rhi::IndexedVertexBufferDesc desc = {};
            desc.vertexData = stream.vertex.data;
            desc.vertexSize = (u32)stream.vertex.len * stream.vertex_size;
            desc.vertexCount = (u32)stream.vertex.len;
            desc.indexData = stream.index.data;
            desc.indexSize = (u32)stream.index.len * sizeof(u32);
            desc.indexCount = (u32)stream.index.len;
            desc.memoryUsage = gfx::rhi::BufferMemoryUsage::GPU;
            desc.accessType = gfx::rhi::BufferAccessType::GPU;
            desc.indexType = gfx::rhi::BufferItemType::U32;
            desc.type = gfx::rhi::BufferTopologyType::Triangles;

            const renderer::ShaderTechniques::Enum shaderTechnique =
                shaderTechniques[i];
            renderer::DrawMesh& mesh =
                renderer::alloc_drawMesh(renderCore);
            mesh = {};
            mesh.shaderTechnique = shaderTechnique;
            gfx::rhi::create_indexed_vertex_buffer(
                mesh.vertexBuffer, desc, pipelineContext.vertexAttrs[i],
                pipelineContext.attr_count[i]);
            if (stream.user) {
                const char* texturefile =
                    ((ufbx_texture*)stream.user)->filename.data;
                gfx::rhi::create_texture_from_file(
                    mesh.texture,
                    { pipelineContext.scratchArena, texturefile });
            }
            assetToAdd.meshHandles[i] =
                handle_from_drawMesh(renderCore, mesh);
		}
        success = true;
    }
    return success;
}
} // fbx

void clip_poly_in_frustum(
    float3* poly, u32& poly_count, const float4* planes, const u32 planeCount, const u32 polyCountCap) {

    // todo: consider speeding this up somehow, as it's pretty expensive when called 1000+ times

    assert(polyCountCap <= 8);

    // cull quad by each frustum plane via Sutherland-Hodgman
    // todo: degenerate polys can end up with more than 1 extra vertex per plane;
    // we "fix it" by adding the second plane intertersections in place of the vertex before

    // we'll use a temporary buffer to loop over the input, then swap buffers each iteration
    float3 quadBuffer[8];
    float3* inputPoly = quadBuffer;
    u32 inputPoly_count = 0;
    float3* outputQuad = poly;
    u32 outputPoly_count = poly_count;

    // keep iterating over the culling planes until
    // we have fully culled the quad, or the previous cut introduced too many cuts
    for (u32 p = 0; p < planeCount && outputPoly_count > 2 && outputPoly_count < polyCountCap; p++) {

        float3* tmpQuad = inputPoly;
        inputPoly = outputQuad;
        inputPoly_count = outputPoly_count;
        outputQuad = tmpQuad;
        outputPoly_count = 0;

        const float4 plane = planes[p];
        u32 numPlaneCuts = 0;

        // compute all distances ahead of time
        // even if the poly doesn't have the maximum number of vertices, this speeds things up
        f32 distances[8];
        distances[0] = math::dot(float4(inputPoly[0], 1.f), plane);
        distances[1] = math::dot(float4(inputPoly[1], 1.f), plane);
        distances[2] = math::dot(float4(inputPoly[2], 1.f), plane);
        distances[3] = math::dot(float4(inputPoly[3], 1.f), plane);
        distances[4] = math::dot(float4(inputPoly[4], 1.f), plane);
        distances[5] = math::dot(float4(inputPoly[5], 1.f), plane);
        distances[6] = math::dot(float4(inputPoly[6], 1.f), plane);
        distances[7] = math::dot(float4(inputPoly[7], 1.f), plane);
        u32 prev_v = inputPoly_count - 1;

        for (u32 curr_v = 0; curr_v < inputPoly_count; curr_v++) {
            const f32 eps = 0.001f;
            if (distances[curr_v] > eps) {
                // current vertex in positive zone,
                // will be added to poly
                if (distances[prev_v] < -eps) {
                    // edge entering the positive zone,
                    // add intersection point first
                    float3 ab = math::subtract(inputPoly[curr_v], inputPoly[prev_v]);
                    f32 t = (-distances[prev_v]) / math::dot(plane.xyz, ab);
                    float3 intersection = math::add(inputPoly[prev_v], math::scale(ab, t));

                    numPlaneCuts++;
                    if (numPlaneCuts <= 1) { outputQuad[outputPoly_count++] = intersection; }
                    else {
                        // poly is cutting the same plane a second time: degenerate
                        // simply place the intersection in the place of the last vertex
                        outputQuad[outputPoly_count - 1] = intersection;
                    }
                }
                outputQuad[outputPoly_count++] = inputPoly[curr_v];

            } else if (distances[curr_v] < -eps) {
                // current vertex in negative zone,
                // will not be added to poly
                if (distances[prev_v] > eps) {
                    // edge entering the negative zone,
                    // add intersection to face
                    float3 ab = math::subtract(inputPoly[curr_v], inputPoly[prev_v]);
                    f32 t = (-distances[prev_v]) / math::dot(plane.xyz, ab);
                    float3 intersection = math::add(inputPoly[prev_v], math::scale(ab, t));
                    outputQuad[outputPoly_count++] = intersection;
                }
            } else {
                // current vertex on plane,
                // add to poly
                outputQuad[outputPoly_count++] = inputPoly[curr_v];
            }

            prev_v = curr_v;
        }
    }
    memcpy(poly, outputQuad, outputPoly_count * sizeof(float3));
    poly_count = outputPoly_count;
}

struct Camera {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 vpMatrix;
    float3 pos;
};

struct CameraNode { // Node in camera tree (stored as depth-first)
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 vpMatrix;
    renderer::Frustum frustum;
    float3 pos;
    u32 depth;          // depth of this node in the tree (0 == root)
    u32 siblingIndex;   // next sibling index in the tree
    u32 parentIndex;    // next sibling index in the tree
    u32 sourceId;
    renderer::DrawMesh drawMesh;
    __PROFILEONLY(char str[256];)   // used in profile for GPU markers
};
struct GatherMirrorTreeContext {
    allocator::PagedArena& frameArena;
    allocator::PagedArena scratchArenaRoot;
    allocator::Buffer<CameraNode>& cameraTree;
    const game::Mirrors& mirrors;
    u32 maxDepth;
};
u32 gatherMirrorTreeRecursive(GatherMirrorTreeContext& ctx, u32 index, const CameraNode& parent) {

    bool* mirrorVisibility = ALLOC_ARRAY(ctx.scratchArenaRoot, bool, ctx.mirrors.count);
    memset(mirrorVisibility, 0, ctx.mirrors.count * sizeof(bool));
    if (ctx.mirrors.bvh.nodeCount) {
        memset(mirrorVisibility, 0, ctx.mirrors.count * sizeof(bool));
        m256_4 planes_256[renderer::Frustum::MAX_PLANE_COUNT];
        for (u32 p = 0; p < parent.frustum.numPlanes; p++) {
            planes_256[p].vx = _mm256_set1_ps(parent.frustum.planes[p].x);
            planes_256[p].vy = _mm256_set1_ps(parent.frustum.planes[p].y);
            planes_256[p].vz = _mm256_set1_ps(parent.frustum.planes[p].z);
            planes_256[p].vw = _mm256_set1_ps(parent.frustum.planes[p].w);
        }
        bvh::findTrianglesIntersectingFrustum(
            ctx.scratchArenaRoot,mirrorVisibility, ctx.mirrors.bvh,
            planes_256, parent.frustum.numPlanes);
    } else {
        memset(mirrorVisibility, 1, ctx.mirrors.count * sizeof(bool));
    }

    u32 parentIndex = index - 1;
    for (u32 i = 0; i < ctx.mirrors.count; i++) {

        // didn't pass visibility pre-pass, if appropriate
        if (!mirrorVisibility[i]) { continue; }
        // do not self-reflect
        if (parent.sourceId == i) { continue; }

        const game::Mirrors::Poly& mirrorGeo = ctx.mirrors.polys[i];
        // cull backfacing mirrors
        // normal is v2-v0xv1-v0 assuming clockwise winding and right handed coordinates
        if (math::dot(mirrorGeo.normal, math::subtract(parent.pos, mirrorGeo.v[0])) < 0.f) { continue; }

        // copy mirror quad (we'll modify it during clipping)
        enum { MAX_MIRROR_POLY_VERTICES = 8 };
        static_assert(MAX_MIRROR_POLY_VERTICES <= countof(parent.frustum.planes) + 2,
            "mirror poly has too many vertices, it will generate too many frustum planes");
        float3 poly[MAX_MIRROR_POLY_VERTICES];
        u32 poly_count = ctx.mirrors.polys[i].numPts;
        memcpy(poly, ctx.mirrors.polys[i].v, sizeof(float3) * ctx.mirrors.polys[i].numPts);

        clip_poly_in_frustum(
            poly, poly_count, parent.frustum.planes, parent.frustum.numPlanes, countof(poly));

        if (poly_count < 3) { continue; } // resulting mirror poly is fully culled

        // acknowledge this mirror as part of the tree
        CameraNode& curr = allocator::push(ctx.cameraTree, ctx.frameArena);
        curr.parentIndex = parentIndex;
        curr.depth = parent.depth + 1;
        curr.sourceId = i;
        // store mirror id only when using GPU markers
        __PROFILEONLY(io::format(curr.str, sizeof(curr.str), "%s-%d", parent.str, i);)
        index++;

        // compute mirror matrices
        auto reflectionMatrix = [](float4 p) -> float4x4 { // todo: understand properly
            float4x4 o;
            o.col0.x = 1 - 2.f * p.x * p.x; o.col1.x = -2.f * p.x * p.y;    o.col2.x = -2.f * p.x * p.z;        o.col3.x = -2.f * p.x * p.w;
            o.col0.y = -2.f * p.y * p.x;    o.col1.y = 1 - 2.f * p.y * p.y; o.col2.y = -2.f * p.y * p.z;        o.col3.y = -2.f * p.y * p.w;
            o.col0.z = -2.f * p.z * p.x;    o.col1.z = -2.f * p.z * p.y;    o.col2.z = 1.f - 2.f * p.z * p.z;   o.col3.z = -2.f * p.z * p.w;
            o.col0.w = 0.f;                 o.col1.w = 0.f;                 o.col2.w = 0.f;                     o.col3.w = 1.f;
            return o;
        };
        // World Space (WS) values
        float3 posWS = poly[0];
        float4 planeWS(
            mirrorGeo.normal.x, mirrorGeo.normal.y, mirrorGeo.normal.z,
            -math::dot(posWS, mirrorGeo.normal));
        float4x4 reflect = reflectionMatrix(planeWS);
        curr.viewMatrix = math::mult(parent.viewMatrix, reflect);
        curr.projectionMatrix = parent.projectionMatrix;
        // Eye Space (ES) values
        float3 normalES = math::mult(curr.viewMatrix, float4(mirrorGeo.normal, 0.f)).xyz;
        float3 posES = math::mult(curr.viewMatrix, float4(posWS, 1.f)).xyz;
        float4 planeES(normalES.x, normalES.y, normalES.z, -math::dot(posES, normalES));
        gfx::add_oblique_plane_to_persp(curr.projectionMatrix, planeES);
        curr.vpMatrix = math::mult(curr.projectionMatrix, curr.viewMatrix);
        // camera position from inverse orthogonal transform
        curr.pos = float3(-math::dot(curr.viewMatrix.col3, curr.viewMatrix.col0),
                          -math::dot(curr.viewMatrix.col3, curr.viewMatrix.col1),
                          -math::dot(curr.viewMatrix.col3, curr.viewMatrix.col2));

        {
            // frustum planes: near and far come from the projection matrix
            // the rest come from the poly
            float4x4 transpose = math::transpose(curr.vpMatrix);
            curr.frustum.planes[0] =
                math::add(math::scale(transpose.col3, -gfx::min_z), transpose.col2);   // near
            curr.frustum.planes[1] = math::subtract(transpose.col3, transpose.col2);        // far
            curr.frustum.planes[0] =
                math::invScale(curr.frustum.planes[0], math::mag(curr.frustum.planes[0].xyz));
            curr.frustum.planes[1] =
                math::invScale(curr.frustum.planes[1], math::mag(curr.frustum.planes[1].xyz));
            curr.frustum.numPlanes = 2;

            float3 prev_v = poly[poly_count - 1];
            for (u32 v = 0; v < poly_count; v++) {
                float3 curr_v = poly[v];
                // normal is cam-v0xv1-v0 assuming clockwise winding and right handed coordinates
                float3 normal =
                    math::cross(math::subtract(curr.pos, curr_v), math::subtract(curr_v, prev_v));
                normal = math::normalize(normal);
                curr.frustum.planes[curr.frustum.numPlanes++] =
                    float4(normal, -math::dot(curr_v, normal));
                prev_v = curr_v;
            }
        }

        // recurse if there is room for one more mirror
        if (curr.depth + 1 < ctx.maxDepth) {
            index = gatherMirrorTreeRecursive(ctx, index, curr);
        }
        curr.drawMesh = ctx.mirrors.drawMeshes[i];
        curr.siblingIndex = index;
    }
    return index;
}

void renderSDFScene(
    const CameraNode& camera, renderer::CoreResources& rsc, gfx::rhi::RscDepthStencilState& ds) {

    const float radius = game::SDF_scene_radius;
    float3 boxPointsWS[8] = {
        float3(-radius, -radius, 0.f),
        float3(-radius, -radius, radius),
        float3(-radius, radius, 0.f),
        float3(-radius, radius, radius),
        float3(radius, -radius, 0.f),
        float3(radius, -radius, radius),
        float3(radius, radius, 0.f),
        float3(radius, radius, radius),
    };
    bool visible = true;
    const renderer::Frustum& frustum = camera.frustum;
    for (u32 p = 0; p < frustum.numPlanes; p++) {
        int out = 0;
        out += (math::dot(frustum.planes[p], float4(boxPointsWS[0], 1.f)) < 0.f) ? 1 : 0;
        out += (math::dot(frustum.planes[p], float4(boxPointsWS[1], 1.f)) < 0.f) ? 1 : 0;
        out += (math::dot(frustum.planes[p], float4(boxPointsWS[2], 1.f)) < 0.f) ? 1 : 0;
        out += (math::dot(frustum.planes[p], float4(boxPointsWS[3], 1.f)) < 0.f) ? 1 : 0;
        out += (math::dot(frustum.planes[p], float4(boxPointsWS[4], 1.f)) < 0.f) ? 1 : 0;
        out += (math::dot(frustum.planes[p], float4(boxPointsWS[5], 1.f)) < 0.f) ? 1 : 0;
        out += (math::dot(frustum.planes[p], float4(boxPointsWS[6], 1.f)) < 0.f) ? 1 : 0;
        out += (math::dot(frustum.planes[p], float4(boxPointsWS[7], 1.f)) < 0.f) ? 1 : 0;
        if (out == 8) { visible = false; break; }
    }

    if (visible) {
        gfx::rhi::start_event("SDF");
        {
            gfx::rhi::RscCBuffer& cbuffersdf =
                rsc.cbuffers[renderer::CoreResources::CBuffersMeta::SDF];

            renderer::SDF sdf;
            sdf.near = rsc.perspProjection.config.near;
            sdf.far = rsc.perspProjection.config.far;
            sdf.tanfov = math::tan(rsc.perspProjection.config.fov * 0.5f * math::d2r32);
            sdf.aspect = ::platform::state.screen.window_width / (f32)::platform::state.screen.window_height;
            sdf.viewMatrix0 = camera.viewMatrix.col0;
            sdf.viewMatrix1 = camera.viewMatrix.col1;
            sdf.viewMatrix2 = camera.viewMatrix.col2;
            sdf.viewMatrix3 = camera.viewMatrix.col3;
            sdf.proj_row2 = float4(
                camera.projectionMatrix.m[2],
                camera.projectionMatrix.m[6],
                camera.projectionMatrix.m[10],
                camera.projectionMatrix.m[14]);
            sdf.time = (f32)::platform::state.time.running;
            sdf.platformSize = radius;

            gfx::rhi::update_cbuffer(cbuffersdf, &sdf);

            gfx::rhi::bind_blend_state(rsc.blendStateOn);
            gfx::rhi::bind_DS(ds, camera.depth);
            gfx::rhi::bind_cbuffers(
                rsc.shaders[renderer::ShaderTechniques::FullscreenBlitSDF],
                &cbuffersdf, 1);
            gfx::rhi::bind_shader(rsc.shaders[renderer::ShaderTechniques::FullscreenBlitSDF]);
            gfx::rhi::draw_fullscreen();
        }
        gfx::rhi::end_event();
    }
}

struct RenderSceneContext {
    const CameraNode& camera;
    const renderer::VisibleNodes& visibleNodes;
    game::Scene& gameScene;
    renderer::CoreResources& core;
    gfx::rhi::RscDepthStencilState& ds_always;
    gfx::rhi::RscDepthStencilState& ds_opaque;
    gfx::rhi::RscDepthStencilState& ds_alpha;
    gfx::rhi::RscRasterizerState& rs;
    allocator::PagedArena scratchArena;
};
void renderBaseScene(RenderSceneContext& sceneCtx) {

    using namespace renderer;
    Scene& scene = sceneCtx.gameScene.renderScene;
    renderer::CoreResources& rsc = sceneCtx.core;

    gfx::rhi::RscCBuffer& scene_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
    SceneData cbufferPerScene;
    {
        cbufferPerScene.vpMatrix = 
            math::mult(sceneCtx.camera.projectionMatrix, sceneCtx.camera.viewMatrix);
        gfx::rhi::update_cbuffer(scene_cbuffer, &cbufferPerScene);
    }

    gfx::rhi::start_event("SKY");
    {
        gfx::rhi::RscCBuffer& clearColor_cbuffer =
            rsc.cbuffers[renderer::CoreResources::CBuffersMeta::ClearColor];
        gfx::rhi::bind_blend_state(rsc.blendStateBlendOff);
        gfx::rhi::bind_DS(sceneCtx.ds_always, sceneCtx.camera.depth);
        gfx::rhi::bind_RS(rsc.rasterizerStateFillFrontfaces);
        gfx::rhi::bind_cbuffers(
            rsc.shaders[renderer::ShaderTechniques::FullscreenBlitClearColor],
            &clearColor_cbuffer, 1);
        gfx::rhi::bind_shader(rsc.shaders[renderer::ShaderTechniques::FullscreenBlitClearColor]);
        gfx::rhi::draw_fullscreen();
    }
    gfx::rhi::end_event();

    // Opaque pass
    gfx::rhi::bind_RS(sceneCtx.rs);
    {
        allocator::PagedArena scratchArena = sceneCtx.scratchArena;
        Drawlist dl = {};
        u32 maxDrawCalls =
              (sceneCtx.visibleNodes.visible_nodes_count
            + (u32)scene.instancedDrawNodes.count) * DrawlistStreams::Count;
        dl.items = ALLOC_ARRAY(scratchArena, DrawCall_Item, maxDrawCalls);
        dl.keys = ALLOC_ARRAY(scratchArena, SortKey, maxDrawCalls);
        addNodesToDrawlistSorted(
            dl, sceneCtx.visibleNodes, sceneCtx.camera.pos, scene, rsc,
            0, renderer::DrawlistFilter::Alpha, renderer::SortParams::Type::Default);
        if (dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced] > 0) {
            gfx::rhi::start_event("OPAQUE");
            gfx::rhi::bind_DS(sceneCtx.ds_opaque, sceneCtx.camera.depth);
            Drawlist_Context ctx = {};
            Drawlist_Overrides overrides = {};
            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
            draw_drawlist(dl, ctx, overrides);
            gfx::rhi::end_event();
        }
    }
    #if __DEBUG
    {
        gfx::rhi::bind_blend_state(rsc.blendStateOn);
        im::present3d(sceneCtx.camera.projectionMatrix, sceneCtx.camera.viewMatrix);
    }
    #endif

    // SDF scene
    gfx::rhi::bind_RS(rsc.rasterizerStateFillFrontfaces);
    renderSDFScene(sceneCtx.camera, rsc, sceneCtx.ds_opaque);

    // Alpha pass
    {
        gfx::rhi::bind_RS(sceneCtx.rs);
        allocator::PagedArena scratchArena = sceneCtx.scratchArena;
        Drawlist dl = {};
        u32 maxDrawCalls =
              (sceneCtx.visibleNodes.visible_nodes_count
            + (u32)scene.instancedDrawNodes.count) * DrawlistStreams::Count;
        dl.items = ALLOC_ARRAY(scratchArena, DrawCall_Item, maxDrawCalls);
        dl.keys = ALLOC_ARRAY(scratchArena, SortKey, maxDrawCalls);
        gfx::rhi::bind_blend_state(rsc.blendStateOn);
        addNodesToDrawlistSorted(
            dl, sceneCtx.visibleNodes, sceneCtx.camera.pos, scene, rsc,
            renderer::DrawlistFilter::Alpha, 0, renderer::SortParams::Type::BackToFront);
        if (dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced] > 0) {
            gfx::rhi::bind_DS(sceneCtx.ds_alpha, sceneCtx.camera.depth);
            gfx::rhi::start_event("ALPHA");
            Drawlist_Context ctx = {};
            Drawlist_Overrides overrides = {};
            overrides.forced_blendState = true;
            ctx.blendState = &rsc.blendStateOn;
            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
            draw_drawlist(dl, ctx, overrides);
            gfx::rhi::end_event();
        }
    }
}

struct RenderMirrorContext {
    const CameraNode& camera;
    const CameraNode& parent;
    game::Scene& gameScene;
    renderer::CoreResources& renderCore;
};
void markMirror(RenderMirrorContext& mirrorCtx) {
    using namespace renderer;
    renderer::CoreResources& rsc = mirrorCtx.renderCore;

    gfx::rhi::RscRasterizerState& rasterizerStateParent =
        (mirrorCtx.camera.depth & 1) != 0 ?
              rsc.rasterizerStateFillFrontfaces
            : rsc.rasterizerStateFillBackfaces;

    gfx::rhi::RscCBuffer& scene_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
    gfx::rhi::RscCBuffer& identity_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::NodeIdentity];

    // mark this mirror on the stencil (using the parent's camera)
    gfx::rhi::start_event("MARK MIRROR");
    {
        gfx::rhi::bind_DS(rsc.depthStateMarkMirror, mirrorCtx.parent.depth);
        gfx::rhi::bind_RS(rasterizerStateParent);

        gfx::rhi::bind_blend_state(rsc.blendStateOff);

        const renderer::DrawMesh& mesh =
            mirrorCtx.camera.drawMesh;
        gfx::rhi::bind_shader(rsc.shaders[mesh.shaderTechnique]);
        gfx::rhi::bind_indexed_vertex_buffer(mesh.vertexBuffer);
        gfx::rhi::RscCBuffer buffers[] = { scene_cbuffer, identity_cbuffer };
        gfx::rhi::bind_cbuffers(rsc.shaders[mesh.shaderTechnique], buffers, 2);
        gfx::rhi::draw_indexed_vertex_buffer(mesh.vertexBuffer);
    }
    gfx::rhi::end_event();
}
void unmarkMirror(RenderMirrorContext& mirrorCtx) {

    using namespace renderer;
    renderer::CoreResources& rsc = mirrorCtx.renderCore;
    gfx::rhi::RscRasterizerState& rasterizerStateParent =
        (mirrorCtx.camera.depth & 1) != 0 ?
              rsc.rasterizerStateFillFrontfaces
            : rsc.rasterizerStateFillBackfaces;

    gfx::rhi::RscCBuffer& scene_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
    gfx::rhi::RscCBuffer& identity_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::NodeIdentity];

    {
        renderer::SceneData cbufferPerScene;
        cbufferPerScene.vpMatrix =
            math::mult(mirrorCtx.parent.projectionMatrix, mirrorCtx.parent.viewMatrix);
        gfx::rhi::update_cbuffer(scene_cbuffer, &cbufferPerScene);
    }

    gfx::rhi::start_event("UNMARK MIRROR");
    {
        gfx::rhi::bind_DS(rsc.depthStateUnmarkMirror, mirrorCtx.camera.depth);
        gfx::rhi::bind_RS(rasterizerStateParent);
        gfx::rhi::bind_blend_state(rsc.blendStateOn);

        renderer::DrawMesh mesh = //gfx::drawMesh_from_handle(rsc, mirrorCtx.camera.meshHandle);
            mirrorCtx.camera.drawMesh;
        gfx::rhi::bind_shader(rsc.shaders[mesh.shaderTechnique]);
        gfx::rhi::bind_indexed_vertex_buffer(mesh.vertexBuffer);
        gfx::rhi::RscCBuffer buffers[] = { scene_cbuffer, identity_cbuffer };
        gfx::rhi::bind_cbuffers(rsc.shaders[ShaderTechniques::Color3D], buffers, 2);
        gfx::rhi::draw_indexed_vertex_buffer(mesh.vertexBuffer);
    }
    gfx::rhi::end_event();
}

void renderMirrorTree(
        const CameraNode* cameraTree,
        const renderer::VisibleNodes* visibleNodes,
        game::Scene& gameScene,
        renderer::CoreResources& renderCore,
        allocator::PagedArena scratchArena) {

    using namespace renderer;
    u32 numCameras = cameraTree[0].siblingIndex;
    u32* parents = ALLOC_ARRAY(scratchArena, u32, numCameras);
    u32 parentCount = 0;
    parents[parentCount++] = 0;
    for (u32 index = 1; index < numCameras; index++) {

        const CameraNode& camera = cameraTree[index];
        const CameraNode& parent = cameraTree[parents[parentCount - 1]];

        // render this mirror
        __PROFILEONLY(gfx::rhi::start_event(camera.str);)
        
        // mark mirror
        RenderMirrorContext mirrorContext {
            camera, parent, gameScene, renderCore
        };
        markMirror(mirrorContext);

        // render base scene
        gfx::rhi::start_event("REFLECTION SCENE");
        {
            gfx::rhi::RscRasterizerState& rasterizerStateMirror =
                (camera.depth & 1) == 0 ?
                      renderCore.rasterizerStateFillFrontfaces
                    : renderCore.rasterizerStateFillBackfaces;
            RenderSceneContext renderSceneContext = {
                camera, visibleNodes[index], gameScene, renderCore,
                renderCore.depthStateMirrorReflectionsDepthAlways,
                renderCore.depthStateMirrorReflections,
                renderCore.depthStateMirrorReflectionsDepthReadOnly,
                rasterizerStateMirror,
                scratchArena };
            renderBaseScene(renderSceneContext);
        }
        __PROFILEONLY(gfx::rhi::end_event();)

        parents[parentCount++] = index;
        while (parentCount > 1 && index + 1 >= cameraTree[parents[parentCount - 1]].siblingIndex) {
            // unmark mirror
            const CameraNode& camera = cameraTree[parents[parentCount - 1]];
            const CameraNode& parent = cameraTree[parents[parentCount - 2]];
            RenderMirrorContext mirrorContext {
                camera, parent, gameScene, renderCore
            };
            unmarkMirror(mirrorContext);
            gfx::rhi::end_event();
            parentCount--;
        }
    }
}

struct SceneMemory {
    allocator::PagedArena& persistentArena;
    // explicit copy, makes it a stack allocator for this context
    allocator::PagedArena scratchArena;
    __DEBUGDEF(allocator::PagedArena& debugArena;)
};
void load_coreResources(
        game::Resources& core, SceneMemory& memory,
        const platform::Screen& screen) {

    allocator::PagedArena& persistentArena = memory.persistentArena;

    renderer::CoreResources& renderCore = core.renderCore;
    renderCore = {};

    // from blender: export fbx -> Apply Scalings: FBX All
    // -> Forward: the one in Blender -> Use Space Transform: yes
    struct AssetDef {
        const char* path;
        game::Resources::AssetsMeta::Enum assetId;
    };
    const AssetDef assets[] = {
        { "assets/meshes/bird.fbx", game::Resources::AssetsMeta::Bird }
    };
    // very very hack: number of assets * 4 + 16 (for the meshes we load ourselves)
    const size_t meshArenaSize = (countof(assets) * 4 + 16) * sizeof(renderer::DrawMesh);
    renderCore.meshes = ALLOC_BYTES(persistentArena, renderer::DrawMesh, meshArenaSize);
    renderCore.num_meshes = 0;

    camera::WindowProjection::Config& ortho =
        renderCore.windowProjection.config;
    ortho.right = screen.window_width * 0.5f;
    ortho.top = screen.window_height * 0.5f;
    ortho.left = -ortho.right;
    ortho.bottom = -ortho.top;
    ortho.near = 0.f;
    ortho.far = 1000.f;
    gfx::generate_matrix_ortho(renderCore.windowProjection.matrix, ortho);

    // Todo: consider whether precision needs special handling
    camera::PerspProjection::Config& frustum =
        renderCore.perspProjection.config;
    frustum.fov = 45.f;
    frustum.aspect = screen.width / (f32)screen.height;
    frustum.near = 1.f;
    frustum.far = 1000.f;
    gfx::generate_matrix_persp(
        renderCore.perspProjection.matrix, frustum);

    gfx::rhi::MainRenderTargetParams windowRTparams = {};
    windowRTparams.depth = true;
    windowRTparams.width = screen.window_width;
    windowRTparams.height = screen.window_height;
    gfx::rhi::create_main_RT(
        renderCore.windowRT, windowRTparams);

    gfx::rhi::RenderTargetParams gameRTparams;
    gameRTparams.flags = (gfx::rhi::RenderTargetParams::Flags::EnableDepth);// | gfx::rhi::RenderTargetParams::Flags::ReadDepth);
    gameRTparams.width = screen.width;
    gameRTparams.height = screen.height;
    gameRTparams.textureFormat = gfx::rhi::TextureFormat::V4_8;
    gameRTparams.textureInternalFormat =
        gfx::rhi::InternalTextureFormat::V4_8;
    gameRTparams.textureFormatType =
        gfx::rhi::Type::Float;
    gameRTparams.count = 1;
    gfx::rhi::create_RT(renderCore.gameRT, gameRTparams);

    {
        gfx::rhi::BlendStateParams blendState_params;
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            gfx::rhi::RenderTargetWriteMask::All; blendState_params.blendEnable = true;
        gfx::rhi::create_blend_state(renderCore.blendStateOn, blendState_params);
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            gfx::rhi::RenderTargetWriteMask::All;
        gfx::rhi::create_blend_state(renderCore.blendStateBlendOff, blendState_params);
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            gfx::rhi::RenderTargetWriteMask::None;
        gfx::rhi::create_blend_state(renderCore.blendStateOff, blendState_params);
    }
    gfx::rhi::create_RS(renderCore.rasterizerStateFillFrontfaces,
            { gfx::rhi::RasterizerFillMode::Fill,
              gfx::rhi::RasterizerCullMode::CullBack, false });
    gfx::rhi::create_RS(renderCore.rasterizerStateFillBackfaces,
            { gfx::rhi::RasterizerFillMode::Fill,
              gfx::rhi::RasterizerCullMode::CullFront, false });
    gfx::rhi::create_RS(renderCore.rasterizerStateFillCullNone,
            { gfx::rhi::RasterizerFillMode::Fill,
              gfx::rhi::RasterizerCullMode::CullNone, false });
    gfx::rhi::create_RS(renderCore.rasterizerStateLine,
            { gfx::rhi::RasterizerFillMode::Line,
              gfx::rhi::RasterizerCullMode::CullNone, false });
    {
        // BASE depth states
        gfx::rhi::DepthStencilStateParams dsParams;
        dsParams = {};
        dsParams.depth_enable = true;
        dsParams.depth_func = gfx::rhi::CompFunc::Less;
        dsParams.depth_writemask = gfx::rhi::DepthWriteMask::All;
        gfx::rhi::create_DS(renderCore.depthStateOn, dsParams);
        dsParams = {};
        dsParams.depth_enable = true;
        dsParams.depth_func = gfx::rhi::CompFunc::Less;
        dsParams.depth_writemask = gfx::rhi::DepthWriteMask::Zero;
        gfx::rhi::create_DS(renderCore.depthStateReadOnly, dsParams);
        dsParams = {};
        dsParams.depth_enable = false;
        gfx::rhi::create_DS(renderCore.depthStateOff, dsParams);
        dsParams = {};
        dsParams.depth_enable = true;
        dsParams.depth_func = gfx::rhi::CompFunc::Always;
        dsParams.depth_writemask = gfx::rhi::DepthWriteMask::All;
        gfx::rhi::create_DS(renderCore.depthStateAlways, dsParams);
        { // MIRROR depth states
            dsParams = {};
            dsParams.depth_enable = true;
            dsParams.depth_func = gfx::rhi::CompFunc::Less;
            dsParams.depth_writemask = gfx::rhi::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = dsParams.stencil_writemask = 0xff;
            dsParams.stencil_failOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_depthFailOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_passOp = gfx::rhi::StencilOp::Incr;
            dsParams.stencil_func = gfx::rhi::CompFunc::Equal;
            gfx::rhi::create_DS(renderCore.depthStateMarkMirror, dsParams);
            dsParams = {};
            dsParams.depth_enable = true;
            dsParams.depth_func = gfx::rhi::CompFunc::Always;
            dsParams.depth_writemask = gfx::rhi::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = dsParams.stencil_writemask = 0xff;
            dsParams.stencil_failOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_depthFailOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_passOp = gfx::rhi::StencilOp::Decr;
            dsParams.stencil_func = gfx::rhi::CompFunc::Equal;
            gfx::rhi::create_DS(renderCore.depthStateUnmarkMirror, dsParams);
            dsParams.depth_enable = true;
            dsParams.depth_func = gfx::rhi::CompFunc::Less;
            dsParams.depth_writemask = gfx::rhi::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = 0xff;
            dsParams.stencil_writemask = 0x00;
            dsParams.stencil_failOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_depthFailOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_passOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_func = gfx::rhi::CompFunc::Equal;
            gfx::rhi::create_DS(renderCore.depthStateMirrorReflections, dsParams);
            dsParams.depth_enable = true;
            dsParams.depth_func = gfx::rhi::CompFunc::Less;
            dsParams.depth_writemask = gfx::rhi::DepthWriteMask::Zero;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = 0xff;
            dsParams.stencil_writemask = 0x00;
            dsParams.stencil_failOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_depthFailOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_passOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_func = gfx::rhi::CompFunc::Equal;
            gfx::rhi::create_DS(renderCore.depthStateMirrorReflectionsDepthReadOnly, dsParams);
            dsParams.depth_enable = true;
            dsParams.depth_func = gfx::rhi::CompFunc::Always;
            dsParams.depth_writemask = gfx::rhi::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = 0xff;
            dsParams.stencil_writemask = 0x00;
            dsParams.stencil_failOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_depthFailOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_passOp = gfx::rhi::StencilOp::Keep;
            dsParams.stencil_func = gfx::rhi::CompFunc::Equal;
            gfx::rhi::create_DS(renderCore.depthStateMirrorReflectionsDepthAlways, dsParams);
        }
    }

    // known cbuffers
    gfx::rhi::RscCBuffer& cbufferclear =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::ClearColor];
    gfx::rhi::create_cbuffer(cbufferclear, { sizeof(renderer::BlitColor) });
    renderer::BlitColor clearColor = { float4(0.2f, 0.344f, 0.59f, 1.f) };
    gfx::rhi::update_cbuffer(cbufferclear, &clearColor);
    gfx::rhi::RscCBuffer& cbuffersdf =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::SDF];
    gfx::rhi::create_cbuffer(cbuffersdf, { sizeof(renderer::SDF) });
    renderer::SDF sdf = { };
    gfx::rhi::update_cbuffer(cbuffersdf, &sdf);
    gfx::rhi::RscCBuffer& cbufferscene =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
    gfx::rhi::create_cbuffer(cbufferscene, { sizeof(renderer::SceneData) });
    gfx::rhi::RscCBuffer& cbufferNodeIdentity =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::NodeIdentity];
    gfx::rhi::create_cbuffer(cbufferNodeIdentity, { sizeof(renderer::NodeData) });
    renderer::NodeData nodeIdentity = {};
    nodeIdentity.groupColor = float4(1.f, 1.f, 1.f, 1.f);
    math::identity4x4(*(Transform*)&nodeIdentity.worldMatrix);
    gfx::rhi::update_cbuffer(cbufferNodeIdentity, &nodeIdentity);
    gfx::rhi::RscCBuffer& cbufferNodeUItext =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::UIText];
    gfx::rhi::create_cbuffer(cbufferNodeUItext, { sizeof(renderer::NodeData) });
    renderer::NodeData noteUItext = {};
    noteUItext.groupColor = float4(1.f, 1.f, 1.f, 1.f);
    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
    gfx::rhi::update_cbuffer(cbufferNodeUItext, &noteUItext);
    gfx::rhi::RscCBuffer& cbufferinstances64 =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::Instances64];
    gfx::rhi::create_cbuffer(cbufferinstances64, { u32(sizeof(float4x4)) * 64 });

    // input layouts
    const gfx::rhi::VertexAttribDesc attribs_2d[] = {
        gfx::rhi::make_vertexAttribDesc(
            "POSITION", offsetof(renderer::VertexLayout_Color_2D, pos),
            sizeof(renderer::VertexLayout_Color_2D),
            gfx::rhi::BufferAttributeFormat::R32G32_FLOAT),
        gfx::rhi::make_vertexAttribDesc(
            "COLOR", offsetof(renderer::VertexLayout_Color_2D, color),
            sizeof(renderer::VertexLayout_Color_2D),
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM) };
    const gfx::rhi::VertexAttribDesc attribs_3d[] = {
        gfx::rhi::make_vertexAttribDesc(
            "POSITION", 0,
            sizeof(float3),
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT) };
    const gfx::rhi::VertexAttribDesc attribs_color3d[] = {
        gfx::rhi::make_vertexAttribDesc(
			"POSITION", offsetof(renderer::VertexLayout_Color_3D, pos),
            sizeof(renderer::VertexLayout_Color_3D),
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT),
        gfx::rhi::make_vertexAttribDesc(
			"COLOR", offsetof(renderer::VertexLayout_Color_3D, color),
            sizeof(renderer::VertexLayout_Color_3D),
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM) };
    const gfx::rhi::VertexAttribDesc attribs_color3d_skinned[] = {
        gfx::rhi::make_vertexAttribDesc(
			"POSITION", offsetof(renderer::VertexLayout_Color_Skinned_3D, pos),
            sizeof(renderer::VertexLayout_Color_Skinned_3D),
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT),
        gfx::rhi::make_vertexAttribDesc(
			"COLOR", offsetof(renderer::VertexLayout_Color_Skinned_3D, color),
            sizeof(renderer::VertexLayout_Color_Skinned_3D),
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM),
        gfx::rhi::make_vertexAttribDesc(
			"JOINTINDICES", offsetof(renderer::VertexLayout_Color_Skinned_3D, joint_indices),
            sizeof(renderer::VertexLayout_Color_Skinned_3D),
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_SINT),
        gfx::rhi::make_vertexAttribDesc(
			"JOINTWEIGHTS", offsetof(renderer::VertexLayout_Color_Skinned_3D, joint_weights),
            sizeof(renderer::VertexLayout_Color_Skinned_3D),
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM) };
    const gfx::rhi::VertexAttribDesc attribs_textured3d[] = {
        gfx::rhi::make_vertexAttribDesc(
			"POSITION", offsetof(renderer::VertexLayout_Textured_3D, pos),
            sizeof(renderer::VertexLayout_Textured_3D),
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT),
        gfx::rhi::make_vertexAttribDesc(
			"TEXCOORD", offsetof(renderer::VertexLayout_Textured_3D, uv),
            sizeof(renderer::VertexLayout_Textured_3D),
            gfx::rhi::BufferAttributeFormat::R32G32_FLOAT) };
    const gfx::rhi::VertexAttribDesc attribs_textured3d_skinned[] = {
        gfx::rhi::make_vertexAttribDesc(
			"POSITION", offsetof(renderer::VertexLayout_Textured_Skinned_3D, pos),
            sizeof(renderer::VertexLayout_Textured_Skinned_3D),
            gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT),
        gfx::rhi::make_vertexAttribDesc(
			"TEXCOORD", offsetof(renderer::VertexLayout_Textured_Skinned_3D, uv),
            sizeof(renderer::VertexLayout_Textured_Skinned_3D),
            gfx::rhi::BufferAttributeFormat::R32G32_FLOAT),
        gfx::rhi::make_vertexAttribDesc(
			"JOINTINDICES", offsetof(renderer::VertexLayout_Textured_Skinned_3D, joint_indices),
            sizeof(renderer::VertexLayout_Textured_Skinned_3D),
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_SINT),
        gfx::rhi::make_vertexAttribDesc(
			"JOINTWEIGHTS", offsetof(renderer::VertexLayout_Textured_Skinned_3D, joint_weights),
            sizeof(renderer::VertexLayout_Textured_Skinned_3D),
            gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM)
    };

    // cbuffer bindings
    const gfx::rhi::CBufferBindingDesc bufferBindings_blit_clear_color[] = {
        { "type_BlitColor", gfx::rhi::CBufferStageMask::PS }
    };
    const gfx::rhi::CBufferBindingDesc bufferBindings_blit_sdf[] = {
        { "type_BlitSDF", gfx::rhi::CBufferStageMask::PS }
    };
    const gfx::rhi::CBufferBindingDesc bufferBindings_untextured_base[] = {
        { "type_PerScene", gfx::rhi::CBufferStageMask::VS },
        { "type_PerGroup", gfx::rhi::CBufferStageMask::VS }
    };
    const gfx::rhi::CBufferBindingDesc bufferBindings_skinned_untextured_base[] = {
        { "type_PerScene", gfx::rhi::CBufferStageMask::VS },
        { "type_PerGroup", gfx::rhi::CBufferStageMask::VS },
        { "type_PerJoint", gfx::rhi::CBufferStageMask::VS }
    };
    const gfx::rhi::CBufferBindingDesc bufferBindings_textured_base[] = {
        { "type_PerScene", gfx::rhi::CBufferStageMask::VS },
        { "type_PerGroup",
            gfx::rhi::CBufferStageMask::VS | gfx::rhi::CBufferStageMask::PS }
    };
    const gfx::rhi::CBufferBindingDesc bufferBindings_skinned_textured_base[] = {
        { "type_PerScene", gfx::rhi::CBufferStageMask::VS },
        { "type_PerGroup",
            gfx::rhi::CBufferStageMask::VS | gfx::rhi::CBufferStageMask::PS },
        { "type_PerJoint", gfx::rhi::CBufferStageMask::VS }
    };
    const gfx::rhi::CBufferBindingDesc bufferBindings_instanced_base[] = {
        { "type_PerScene", gfx::rhi::CBufferStageMask::VS },
        { "type_PerGroup", gfx::rhi::CBufferStageMask::VS },
        { "type_PerInstance", gfx::rhi::CBufferStageMask::VS }
    };

    // texture bindings
    const gfx::rhi::TextureBindingDesc textureBindings_base[] = { { "texDiffuse" } };
    const gfx::rhi::TextureBindingDesc textureBindings_fullscreenblit[] = { { "texSrc" } };
    const gfx::rhi::TextureBindingDesc textureBindings_fullscreenblit_depth[] = { { "texSrc" }, { "depthSrc" }};

    // shaders
    {
        allocator::PagedArena scratchArena = memory.scratchArena; // explicit copy
        // Initialize cache with room for a VS and PS shader of each technique
        gfx::rhi::ShaderCache shader_cache = {};
        gfx::rhi::load_shader_cache(
            shader_cache, "assets/data/shaderCache_physics.bin", &scratchArena,
            renderer::ShaderTechniques::Count * 2);
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_3d;
            desc.vertexAttr_count = countof(attribs_3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_blit_clear_color;
            desc.bufferBinding_count = countof(bufferBindings_blit_clear_color);
            // reuse 3d shaders
            desc.vs_name = gfx::shaders::vs_fullscreen_bufferless_clear_blit.name;
            desc.vs_src = gfx::shaders::vs_fullscreen_bufferless_clear_blit.src;
            desc.ps_name = gfx::shaders::ps_fullscreen_blit_clear_colored.name;
            desc.ps_src = gfx::shaders::ps_fullscreen_blit_clear_colored.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::FullscreenBlitClearColor], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_3d;
            desc.vertexAttr_count = countof(attribs_3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_blit_sdf;
            desc.bufferBinding_count = countof(bufferBindings_blit_sdf);
            // reuse 3d shaders
            desc.vs_name = gfx::shaders::vs_fullscreen_bufferless_textured_blit.name;
            desc.vs_src = gfx::shaders::vs_fullscreen_bufferless_textured_blit.src;
            desc.ps_name = gfx::shaders::ps_fullscreen_blit_sdf.name;
            desc.ps_src = gfx::shaders::ps_fullscreen_blit_sdf.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::FullscreenBlitSDF], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d;
            desc.vertexAttr_count = countof(attribs_textured3d);
            desc.textureBindings = textureBindings_fullscreenblit;
            desc.textureBinding_count = countof(textureBindings_fullscreenblit);
            desc.bufferBindings = nullptr;
            desc.bufferBinding_count = 0;
            // reuse 3d shaders
            desc.vs_name = gfx::shaders::vs_fullscreen_bufferless_textured_blit.name;
            desc.vs_src = gfx::shaders::vs_fullscreen_bufferless_textured_blit.src;
            desc.ps_name = gfx::shaders::ps_fullscreen_blit_textured.name;
            desc.ps_src = gfx::shaders::ps_fullscreen_blit_textured.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::FullscreenBlitTextured], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d;
            desc.vertexAttr_count = countof(attribs_textured3d);
            desc.textureBindings = textureBindings_fullscreenblit_depth;
            desc.textureBinding_count = countof(textureBindings_fullscreenblit_depth);
            desc.bufferBindings = nullptr;
            desc.bufferBinding_count = 0;
            // reuse 3d shaders
            desc.vs_name = gfx::shaders::vs_fullscreen_bufferless_textured_blit.name;
            desc.vs_src = gfx::shaders::vs_fullscreen_bufferless_textured_blit.src;
            desc.ps_name = gfx::shaders::ps_fullscreen_blit_textured_depth.name;
            desc.ps_src = gfx::shaders::ps_fullscreen_blit_textured_depth.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::FullscreenBlitTexturedDepth], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_2d;
            desc.vertexAttr_count = countof(attribs_2d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_untextured_base;
            desc.bufferBinding_count = countof(bufferBindings_untextured_base);
            desc.vs_name = gfx::shaders::vs_color2d_base.name;
            desc.vs_src = gfx::shaders::vs_color2d_base.src;
            desc.ps_name = gfx::shaders::ps_color3d_unlit.name;
            desc.ps_src = gfx::shaders::ps_color3d_unlit.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::Color2D], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_3d;
            desc.vertexAttr_count = countof(attribs_3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_instanced_base;
            desc.bufferBinding_count = countof(bufferBindings_instanced_base);
            desc.vs_name = gfx::shaders::vs_3d_instanced_base.name;
            desc.vs_src = gfx::shaders::vs_3d_instanced_base.src;
            desc.ps_name = gfx::shaders::ps_color3d_unlit.name;
            desc.ps_src = gfx::shaders::ps_color3d_unlit.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::Instanced3D], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_color3d;
            desc.vertexAttr_count = countof(attribs_color3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_untextured_base;
            desc.bufferBinding_count = countof(bufferBindings_untextured_base);
            desc.vs_name = gfx::shaders::vs_color3d_base.name;
            desc.vs_src = gfx::shaders::vs_color3d_base.src;
            desc.ps_name = gfx::shaders::ps_color3d_unlit.name;
            desc.ps_src = gfx::shaders::ps_color3d_unlit.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(renderCore.shaders[renderer::ShaderTechniques::Color3D], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_color3d_skinned;
            desc.vertexAttr_count = countof(attribs_color3d_skinned);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_skinned_untextured_base;
            desc.bufferBinding_count = countof(bufferBindings_skinned_untextured_base);
            desc.vs_name = gfx::shaders::vs_color3d_skinned_base.name;
            desc.vs_src = gfx::shaders::vs_color3d_skinned_base.src;
            desc.ps_name = gfx::shaders::ps_color3d_unlit.name;
            desc.ps_src = gfx::shaders::ps_color3d_unlit.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::Color3DSkinned], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d;
            desc.vertexAttr_count = countof(attribs_textured3d);
            desc.textureBindings = textureBindings_base;
            desc.textureBinding_count = countof(textureBindings_base);
            desc.bufferBindings = bufferBindings_textured_base;
            desc.bufferBinding_count = countof(bufferBindings_textured_base);
            desc.vs_name = gfx::shaders::vs_textured3d_base.name;
            desc.vs_src = gfx::shaders::vs_textured3d_base.src;
            desc.ps_name = gfx::shaders::ps_textured3d_base.name;
            desc.ps_src = gfx::shaders::ps_textured3d_base.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::Textured3D], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d;
            desc.vertexAttr_count = countof(attribs_textured3d);
            desc.textureBindings = textureBindings_base;
            desc.textureBinding_count = countof(textureBindings_base);
            desc.bufferBindings = bufferBindings_textured_base;
            desc.bufferBinding_count = countof(bufferBindings_textured_base);
            desc.vs_name = gfx::shaders::vs_textured3d_base.name;
            desc.vs_src = gfx::shaders::vs_textured3d_base.src;
            desc.ps_name = gfx::shaders::ps_textured3dalphaclip_base.name;
            desc.ps_src = gfx::shaders::ps_textured3dalphaclip_base.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::Textured3DAlphaClip], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d_skinned;
            desc.vertexAttr_count = countof(attribs_textured3d_skinned);
            desc.textureBindings = textureBindings_base;
            desc.textureBinding_count = countof(textureBindings_base);
            desc.bufferBindings = bufferBindings_skinned_textured_base;
            desc.bufferBinding_count = countof(bufferBindings_skinned_textured_base);
            desc.vs_name = gfx::shaders::vs_textured3d_skinned_base.name;
            desc.vs_src = gfx::shaders::vs_textured3d_skinned_base.src;
            desc.ps_name = gfx::shaders::ps_textured3d_base.name;
            desc.ps_src = gfx::shaders::ps_textured3d_base.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::Textured3DSkinned], desc);
        }
        {
            gfx::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d_skinned;
            desc.vertexAttr_count = countof(attribs_textured3d_skinned);
            desc.textureBindings = textureBindings_base;
            desc.textureBinding_count = countof(textureBindings_base);
            desc.bufferBindings = bufferBindings_skinned_textured_base;
            desc.bufferBinding_count = countof(bufferBindings_skinned_textured_base);
            desc.vs_name = gfx::shaders::vs_textured3d_skinned_base.name;
            desc.vs_src = gfx::shaders::vs_textured3d_skinned_base.src;
            desc.ps_name = gfx::shaders::ps_textured3dalphaclip_base.name;
            desc.ps_src = gfx::shaders::ps_textured3dalphaclip_base.src;
            desc.shader_cache = &shader_cache;
            gfx::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::Textured3DAlphaClipSkinned], desc);
        }
        gfx::rhi::write_shader_cache(shader_cache);
    }

    allocator::PagedArena scratchArena = memory.scratchArena; // explicit copy

    fbx::PipelineAssetContext ctx = { scratchArena, persistentArena };
    ctx.vertexAttrs[renderer::DrawlistStreams::Color3D]
		= attribs_color3d;
    ctx.attr_count[renderer::DrawlistStreams::Color3D]
		= countof(attribs_color3d);
    ctx.vertexAttrs[renderer::DrawlistStreams::Color3DSkinned]
		= attribs_color3d_skinned;
    ctx.attr_count[renderer::DrawlistStreams::Color3DSkinned]
		= countof(attribs_color3d_skinned);
    ctx.vertexAttrs[renderer::DrawlistStreams::Textured3D]
		= attribs_textured3d;
    ctx.attr_count[renderer::DrawlistStreams::Textured3D]
		= countof(attribs_textured3d);
    ctx.vertexAttrs[renderer::DrawlistStreams::Textured3DAlphaClip]
		= attribs_textured3d;
    ctx.attr_count[renderer::DrawlistStreams::Textured3DAlphaClip]
		= countof(attribs_textured3d);
    ctx.vertexAttrs[renderer::DrawlistStreams::Textured3DSkinned]
		= attribs_textured3d_skinned;
    ctx.attr_count[renderer::DrawlistStreams::Textured3DSkinned]
		= countof(attribs_textured3d_skinned);
    ctx.vertexAttrs[renderer::DrawlistStreams::Textured3DAlphaClipSkinned]
		= attribs_textured3d_skinned;
    ctx.attr_count[renderer::DrawlistStreams::Textured3DAlphaClipSkinned]
		= countof(attribs_textured3d_skinned);
    for (u32 asset_idx = 0; asset_idx < countof(assets); asset_idx++) {
		ctx.scratchArena = scratchArena; // explicit copy
        game::AssetInMemory& assetToAdd = core.assets[assets[asset_idx].assetId];
        assetToAdd = {};
        fbx::load_with_materials(assetToAdd, renderCore, ctx, assets[asset_idx].path);
    }

    // instanced cubes
    {
        renderer::DrawMesh& mesh = renderer::alloc_drawMesh(renderCore);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Instanced3D;
        gfx::UntexturedCube cube;
        gfx::create_cube_coords(
            (uintptr_t)cube.vertices, sizeof(cube.vertices[0]),
            cube.indices, float3(1.f, 1.f, 1.f), float3(0.f, 0.f, 0.f));
        gfx::create_indexed_vertex_buffer_from_untextured_mesh(
            mesh.vertexBuffer, cube.vertices, countof(cube.vertices),
            cube.indices, countof(cube.indices));
        core.instancedUnitCubeMesh = renderer::handle_from_drawMesh(renderCore, mesh);
    }

    // instanced spheres
    {
        renderer::DrawMesh& mesh = renderer::alloc_drawMesh(renderCore);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Instanced3D;
        gfx::UntexturedSphere sphere;
        gfx::create_sphere_coords(
            (uintptr_t)sphere.vertices, sizeof(sphere.vertices[0]),
            sphere.indices, 1.f);
        gfx::create_indexed_vertex_buffer_from_untextured_mesh(
            mesh.vertexBuffer, sphere.vertices, countof(sphere.vertices),
            sphere.indices, countof(sphere.indices));
        core.instancedUnitSphereMesh = renderer::handle_from_drawMesh(renderCore, mesh);
    }

    // ground
    {
        renderer::DrawMesh& mesh = renderer::alloc_drawMesh(renderCore);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Color3D;
        f32 w = 30.f, h = 30.f;
        u32 c = Color32(0.13f, 0.51f, 0.23f, 1.f).ABGR();
        renderer::VertexLayout_Color_3D v[] = {
            { float3(12.4f, 30.f, 0.f), c },
            { float3(-12.4f, 30.f, 0.f), c },
            { float3(-30.f, 12.4f, 0.f), c },
            { float3(-30.f, -12.4f, 0.f), c },
            { float3(-12.4f, -30.f, 0.f), c },
            { float3(12.4f, -30.f, 0.f), c },
            { float3(30.f, -12.4f, 0.f), c },
            { float3(30.f, 12.4f, 0.f), c },
        };

        u16 i[] = { 0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4, 0, 6, 5, 0, 7, 6 };
        gfx::rhi::IndexedVertexBufferDesc bufferParams;
        bufferParams.vertexData = v;
        bufferParams.indexData = i;
        bufferParams.vertexSize = sizeof(v);
        bufferParams.vertexCount = countof(v);
        bufferParams.indexSize = sizeof(i);
        bufferParams.indexCount = countof(i);
        bufferParams.memoryUsage = gfx::rhi::BufferMemoryUsage::GPU;
        bufferParams.accessType = gfx::rhi::BufferAccessType::GPU;
        bufferParams.indexType = gfx::rhi::BufferItemType::U16;
        bufferParams.type = gfx::rhi::BufferTopologyType::Triangles;
        gfx::rhi::VertexAttribDesc attribs[] = {
            gfx::rhi::make_vertexAttribDesc(
                    "POSITION", offsetof(renderer::VertexLayout_Color_3D, pos),
                    sizeof(renderer::VertexLayout_Color_3D),
                    gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT),
            gfx::rhi::make_vertexAttribDesc(
                    "COLOR", offsetof(renderer::VertexLayout_Color_3D, color),
                    sizeof(renderer::VertexLayout_Color_3D),
                    gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM)
        };
        gfx::rhi::create_indexed_vertex_buffer(
            mesh.vertexBuffer, bufferParams, attribs, countof(attribs));

        game::AssetInMemory& ground = core.assets[game::Resources::AssetsMeta::Ground];
        ground = {};
        ground.min = float3(-w, -h, 0.f);
        ground.max = float3(w, h, 0.f);
        ground.meshHandles[0] = renderer::handle_from_drawMesh(renderCore, mesh);
        ground.skeleton.jointCount = 0;
    }

    // Hall of mirrors
    {
        const u32 mirrorCount = game::Resources::MirrorHallMeta::Count;
        renderer::VertexLayout_Color_3D vertices[4 * mirrorCount];
        u16 indices[4 * mirrorCount * 3 / 2]; // 6 indices per quad
        renderer::VertexLayout_Color_3D verticesBack[countof(vertices)];
        u16 indicesBack[countof(indices)];
        const float3 v0(1.f, 0.f, -1.f);
        const float3 v1(-1.f, 0.f, -1.f);
        const float3 v2(-1.f, 0.f, 1.f);
        const float3 v3(1.f, 0.f, 1.f);
        u32 c = Color32(0.01f, 0.19f, 0.3f, 0.32f).ABGR();
        u32 c_back = Color32(0.21f, 0.24f, 0.22f, 1.f).ABGR();
        for (u32 m = 0; m < mirrorCount; m++) {
            f32 w = 12.4f, h = 10.f;
            u32 vertex_idx = m * 4;
            Transform t;
            // set up mirrors in an octagon
            Transform rot = math::fromOffsetAndOrbit(
                float3(0.f, 0.f, 0.f), float3(0.f, 0.f, f32(m) * math::twopi32 / f32(8)));
            Transform translate = {};
            math::identity4x4(translate);
            translate.pos = float3(0.f, -30.f, h);
            t.matrix = math::mult(rot.matrix, translate.matrix);
            t.matrix.col0.xyz = math::scale(t.matrix.col0.xyz, w);
            t.matrix.col2.xyz = math::scale(t.matrix.col2.xyz, h);
            // shrink vertices horizontally a bit, so they don't cut into the columns
            vertices[vertex_idx + 0] = { math::mult(t.matrix, float4(math::subtract(v0, float3( 0.035f, 0.f, 0.f)), 1.f)).xyz, c };
            vertices[vertex_idx + 1] = { math::mult(t.matrix, float4(math::subtract(v1, float3(-0.035f, 0.f, 0.f)), 1.f)).xyz, c };
            vertices[vertex_idx + 2] = { math::mult(t.matrix, float4(math::subtract(v2, float3(-0.035f, 0.f, 0.f)), 1.f)).xyz, c };
            vertices[vertex_idx + 3] = { math::mult(t.matrix, float4(math::subtract(v3, float3( 0.035f, 0.f, 0.f)), 1.f)).xyz, c };

            // clock-wise indices
            u32 index_idx = vertex_idx * 3 / 2;
            // todo: start of second tri needs to be the last vertex, fix this weird restriction
            indices[index_idx + 0] = vertex_idx + 2;
            indices[index_idx + 1] = vertex_idx + 1;
            indices[index_idx + 2] = vertex_idx + 0;
            indices[index_idx + 3] = vertex_idx + 3;
            indices[index_idx + 4] = vertex_idx + 2;
            indices[index_idx + 5] = vertex_idx + 0;

            // counter clock-wise indices for the back of the mirror
            // shrink vertices horizontally a bit, so they don't cut into the columns, and push them
            // back, to avoid z-fighting with the actual mirrors
            verticesBack[vertex_idx + 0] = { math::mult(t.matrix, float4(math::add(v0, float3(-0.035f, -0.1f, 0.f)), 1.f)).xyz, c_back };
            verticesBack[vertex_idx + 1] = { math::mult(t.matrix, float4(math::add(v1, float3( 0.035f, -0.1f, 0.f)), 1.f)).xyz, c_back };
            verticesBack[vertex_idx + 2] = { math::mult(t.matrix, float4(math::add(v2, float3( 0.035f, -0.1f, 0.f)), 1.f)).xyz, c_back };
            verticesBack[vertex_idx + 3] = { math::mult(t.matrix, float4(math::add(v3, float3(-0.035f, -0.1f, 0.f)), 1.f)).xyz, c_back };
            indicesBack[index_idx + 0] = vertex_idx + 0;
            indicesBack[index_idx + 1] = vertex_idx + 1;
            indicesBack[index_idx + 2] = vertex_idx + 2;
            indicesBack[index_idx + 3] = vertex_idx + 0;
            indicesBack[index_idx + 4] = vertex_idx + 2;
            indicesBack[index_idx + 5] = vertex_idx + 3;
        }
        game::GPUCPUMesh& meshToLoad = core.mirrorHallMesh;
        gfx::rhi::IndexedVertexBufferDesc bufferParams;
        bufferParams.vertexData = vertices;
        bufferParams.indexData = indices;
        bufferParams.vertexSize = sizeof(vertices);
        bufferParams.vertexCount = countof(vertices);
        bufferParams.indexSize = sizeof(indices);
        bufferParams.indexCount = countof(indices);
        bufferParams.memoryUsage = gfx::rhi::BufferMemoryUsage::GPU;
        bufferParams.accessType = gfx::rhi::BufferAccessType::GPU;
        bufferParams.indexType = gfx::rhi::BufferItemType::U16;
        bufferParams.type = gfx::rhi::BufferTopologyType::Triangles;
        gfx::rhi::VertexAttribDesc attribs[] = {
            gfx::rhi::make_vertexAttribDesc(
                    "POSITION", offsetof(renderer::VertexLayout_Color_3D, pos),
                    sizeof(renderer::VertexLayout_Color_3D),
                    gfx::rhi::BufferAttributeFormat::R32G32B32_FLOAT),
            gfx::rhi::make_vertexAttribDesc(
                    "COLOR", offsetof(renderer::VertexLayout_Color_3D, color),
                    sizeof(renderer::VertexLayout_Color_3D),
                    gfx::rhi::BufferAttributeFormat::R8G8B8A8_UNORM)
        };
        gfx::rhi::create_indexed_vertex_buffer(
            meshToLoad.gpuBuffer, bufferParams, attribs, countof(attribs));

        renderer::CPUMesh& cpuBuffer = meshToLoad.cpuBuffer;
        cpuBuffer.vertices = ALLOC_ARRAY(persistentArena, float3, countof(vertices));
        cpuBuffer.indices = ALLOC_ARRAY(persistentArena, u16, countof(indices));
        for (u32 i = 0; i < countof(vertices); i++) { cpuBuffer.vertices[i] = vertices[i].pos; }
        memcpy(cpuBuffer.indices, indices, sizeof(u16) * countof(indices));
        cpuBuffer.indexCount = countof(indices);
        cpuBuffer.vertexCount = countof(vertices);

        // back mirror
        renderer::DrawMesh& mesh = renderer::alloc_drawMesh(renderCore);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Color3D;
        bufferParams = {};
        bufferParams.vertexData = verticesBack;
        bufferParams.indexData = indicesBack;
        bufferParams.vertexSize = sizeof(verticesBack);
        bufferParams.vertexCount = countof(verticesBack);
        bufferParams.indexSize = sizeof(indicesBack);
        bufferParams.indexCount = countof(indicesBack);
        bufferParams.memoryUsage = gfx::rhi::BufferMemoryUsage::GPU;
        bufferParams.accessType = gfx::rhi::BufferAccessType::GPU;
        bufferParams.indexType = gfx::rhi::BufferItemType::U16;
        bufferParams.type = gfx::rhi::BufferTopologyType::Triangles;
        gfx::rhi::create_indexed_vertex_buffer(
            mesh.vertexBuffer, bufferParams, attribs, countof(attribs));

        game::AssetInMemory& ground = core.assets[game::Resources::AssetsMeta::BackMirrors];
        ground = {};
        ground.min = float3(-30.f, -30.f, 0.f);
        ground.max = float3(30.f, 30.f, 0.f);
        ground.meshHandles[0] = renderer::handle_from_drawMesh(renderCore, mesh);
        ground.skeleton.jointCount = 0;
    }

    // UI text
    {
        struct Buffer2D {
            renderer::VertexLayout_Color_2D* vertices;
            u32* indices;
            u32 vertexCount;
            u32 vertexCap;
        };
        struct Text2DParams {
            float2 pos;
            Color32 color;
            u8 scale;
        };
        auto text2d = [](Buffer2D& buffer, const Text2DParams& params, const char* str) {

            char text[512];
            io::strncpy(text, str, sizeof(text));

            u32 vertexCount = buffer.vertexCount;
            const u32 indexCount = 3 * vertexCount / 2; // every 4 vertices form a 6 index quad
            const u32 color = params.color.ABGR();

            // output to a temporary buffer
            float2 textpoly_buffer[2048];

            // negate y, since our (0,0) is the center of the screen, stb's is bottom left
            u32 quadCount = stb_easy_font_print(
                params.pos.x, -params.pos.y, params.scale, text,
                textpoly_buffer, sizeof(textpoly_buffer));

            // stb uses ccw winding, but we are negating the y, so it matches our cw winding
            renderer::VertexLayout_Color_2D* v_dst = &buffer.vertices[buffer.vertexCount];
            float2* v_src = textpoly_buffer;
            for (u32 i = 0; i < quadCount; i++) {

                v_dst[0] = { v_src[0], color }; v_dst[1] = { v_src[1], color };
                v_dst[2] = { v_src[2], color }; v_dst[3] = { v_src[3], color };
                v_dst += 4;
                v_src += 4;

                const u32 vertexIndex = vertexCount + i * 4;
                const u32 indexIndex = indexCount + i * 6;
                buffer.indices[indexIndex] = vertexIndex + 1;
                buffer.indices[indexIndex + 1] = vertexIndex + 2;
                buffer.indices[indexIndex + 2] = vertexIndex + 3;

                buffer.indices[indexIndex + 3] = vertexIndex + 3;
                buffer.indices[indexIndex + 4] = vertexIndex + 0;
                buffer.indices[indexIndex + 5] = vertexIndex + 1;
            }
            buffer.vertexCount += quadCount * 4;
        };

        auto commit2d = [](
            gfx::rhi::RscIndexedVertexBuffer& gpuBuffer, Buffer2D& buffer,
            const gfx::rhi::VertexAttribDesc* attribs, const u32 attribs_count){
            const u32 vertexCount = buffer.vertexCount;
            const u32 indexCount = 3 * vertexCount / 2; // every 4 vertices form a 6 index quad
            gfx::rhi::IndexedVertexBufferDesc bufferParams;
            bufferParams.vertexData = buffer.vertices;
            bufferParams.indexData = buffer.indices;
            bufferParams.vertexSize = vertexCount * sizeof(renderer::VertexLayout_Color_2D);
            bufferParams.vertexCount = vertexCount;
            bufferParams.indexSize = indexCount * sizeof(u32);
            bufferParams.indexCount = indexCount;
            bufferParams.memoryUsage = gfx::rhi::BufferMemoryUsage::GPU;
            bufferParams.accessType = gfx::rhi::BufferAccessType::GPU;
            bufferParams.indexType = gfx::rhi::BufferItemType::U32;
            bufferParams.type = gfx::rhi::BufferTopologyType::Triangles;
            gfx::rhi::create_indexed_vertex_buffer(
                gpuBuffer, bufferParams, attribs, attribs_count);
            buffer.vertexCount = 0;
        };

        enum { MAX_VERTICES2D_COUNT = 2048 };
        Buffer2D buffer2D = {};
        buffer2D.vertices = ALLOC_ARRAY(scratchArena, renderer::VertexLayout_Color_2D, MAX_VERTICES2D_COUNT);
        buffer2D.vertexCap = MAX_VERTICES2D_COUNT;
        buffer2D.indices = ALLOC_ARRAY(scratchArena, u32, MAX_VERTICES2D_COUNT * 3 / 2);

        const f32 lineheight = 15.f * screen.window_scale;
        Text2DParams textParams;
        textParams.scale = (u8)screen.window_scale;
        textParams.pos = float2(
           renderCore.windowProjection.config.left + 10.f * screen.window_scale,
           renderCore.windowProjection.config.bottom + 10.f * screen.window_scale);
        textParams.pos.y += lineheight;
        textParams.color = Color32(0.7f, 0.8f, 0.15f, 1.0f);
        text2d(buffer2D, textParams, "Left click and drag to orbit");
        textParams.pos.y += lineheight;
        commit2d(core.gpuBufferOrbitText, buffer2D, attribs_2d, countof(attribs_2d));
        text2d(buffer2D, textParams, "Right click and drag to pan");
        textParams.pos.y += lineheight;
        commit2d(core.gpuBufferPanText, buffer2D, attribs_2d, countof(attribs_2d));
        text2d(buffer2D, textParams, "Mouse wheel to scale");
        textParams.pos.y += lineheight;
        commit2d(core.gpuBufferScaleText, buffer2D, attribs_2d, countof(attribs_2d));
        text2d(buffer2D, textParams, "There's some physics too");
        textParams.pos.y += lineheight;
        text2d(buffer2D, textParams, "Welcome to the SDF room");
        textParams.pos.y += lineheight;
        commit2d(core.gpuBufferHeaderText, buffer2D, attribs_2d, countof(attribs_2d));
        #if __DEBUG
        text2d(buffer2D, textParams, "Press H to toggle debug menu");
        textParams.pos.y += lineheight;
        commit2d(core.gpuBufferDebugText, buffer2D, attribs_2d, countof(attribs_2d));
        #endif
    }

    // debug renderer
    __DEBUGDEF(im::init(memory.debugArena);)
}
void spawn_scene_mirrorRoom(
    game::Scene& scene, allocator::PagedArena& sceneArena, allocator::PagedArena scratchArena,
    const game::Resources& core, const platform::Screen& screen,
    const game::RoomDefinition& roomDef) {

    renderer::Scene& renderScene = scene.renderScene;
    animation::Scene& animScene = scene.animScene;
    physics::Scene& physicsScene = scene.physicsScene;

    struct AssetData {
        game::Resources::AssetsMeta::Enum assetId;
        float3 asset_init_pos;
        u32 count;
        bool player; // only first count
        bool physicsObstacle;
    };
    const AssetData assetDefs[] = {
          { game::Resources::AssetsMeta::Bird, { 9.f, 6.f, 2.23879f }, 1, true, true }
        , { game::Resources::AssetsMeta::Ground, { 0.f, 0.f, 0.f }, 1, false, false }
        , { game::Resources::AssetsMeta::BackMirrors, { 0.f, 0.f, 0.f }, 1, false, false }
    };

    size_t maxDrawNodes = countof(assetDefs); 
    size_t maxInstancedNodes = game::Scene::InstancedTypes::Count; 
    size_t cbufferCount = (maxDrawNodes + maxInstancedNodes) * 2;
    size_t maxAnimNodes = countof(assetDefs);
    allocator::init_pool(renderScene.cbuffers, cbufferCount, sceneArena);
	__DEBUGDEF(renderScene.cbuffers.name = "cbuffers";)
    allocator::init_pool(renderScene.instancedDrawNodes, maxInstancedNodes, sceneArena);
	__DEBUGDEF(renderScene.instancedDrawNodes.name = "instanced draw nodes";)
    allocator::init_pool(renderScene.drawNodes, maxDrawNodes, sceneArena);
	__DEBUGDEF(renderScene.drawNodes.name = "draw nodes";)
    allocator::init_pool(animScene.nodes, maxAnimNodes, sceneArena);
	__DEBUGDEF(animScene.nodes.name = "anim nodes";)

    // physics
    if (roomDef.physicsBalls)
    {
        struct BallAssets {
            float3 position;
            float radius;
            float3 velocity;
            float mass;
        };
        BallAssets ballAssets[8];
        const f32 w = 30.f;
        const f32 h = 30.f;
        const f32 maxspeed = 20.f;
        //physicsScene.dt = 1 / 60.f;
        physicsScene.ball_count = 8;
        physicsScene.bounds = float2(w, h);
        physicsScene.restitution = 1.f;
        const float origin_x = w - 5.f; const float origin_y = h - 5.f;
        float2 positions[8] = {
            float2(0.f, 1.f), float2(0.5f, 0.5f),
            float2(1.f, 0.f), float2(0.5f, -0.5f),
            float2(0.f, -1.f), float2(-0.5f, -0.5f),
            float2(-1.f, 0.f), float2(-0.5f, 0.5f),
        };
        for (u32 i = 0; i < physicsScene.ball_count; i++) {
            physics::DynamicObject_Sphere& ball = physicsScene.balls[i];
            ball.radius = (math::rand() + 1.f) * 2.f;
            ball.mass = math::pi32 * ball.radius * ball.radius;
            const float2 pos_xy = math::scale(positions[i], float2(origin_x, origin_y));
            ball.pos =
                float3(pos_xy, ball.radius * 0.5f);
            ball.vel =
                float3(
                    maxspeed * (-1.f + 2.f * math::rand()),
                    maxspeed * (-1.f + 2.f * math::rand()),
                    0.f);
        }

        renderer::DrawNodeInstanced& node = allocator::alloc_pool(renderScene.instancedDrawNodes);
        node = {};
        node.meshHandles[0] = core.instancedUnitSphereMesh;
        math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
        node.nodeData.groupColor = Color32(0.72f, 0.74f, 0.12f, 1.f).RGBAv4();
        node.instanceCount = physicsScene.ball_count;
        gfx::rhi::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
        gfx::rhi::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });
        gfx::rhi::RscCBuffer& cbufferinstances =
            allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_instances =
            handle_from_cbuffer(renderScene, cbufferinstances);
        gfx::rhi::create_cbuffer(
            cbufferinstances, { u32(sizeof(float4x4)) * physicsScene.ball_count });
        for (u32 m = 0; m < countof(node.instanceMatrices.data); m++) {
            float4x4& matrix = node.instanceMatrices.data[m];
            math::identity4x4(*(Transform*)&(matrix));
        }
        scene.instancedNodesHandles[game::Scene::InstancedTypes::PhysicsBalls] =
            handle_from_instanced_node(renderScene, node);
    }

    for (u32 asset_idx = 0; asset_idx < countof(assetDefs); asset_idx++) {
        const AssetData& assetDef = assetDefs[asset_idx];
        for (u32 asset_rep = 0; asset_rep < assetDef.count; asset_rep++) {
            float3 asset_init_pos = assetDef.asset_init_pos;
            const f32 spacing = 10.f;
            const s32 grid_size = 16;
            const s32 max_row = (assetDef.count - 1) % grid_size;
            const s32 max_column = ((assetDef.count - 1) / grid_size) % grid_size;
            s32 row = asset_rep % grid_size;
            s32 column = (asset_rep / grid_size) % grid_size;
            s32 stride = asset_rep / (grid_size * grid_size);
            asset_init_pos = math::add(asset_init_pos,
                { (row - max_row / 2) * spacing,
                 (column - max_column / 2) * spacing,
                  stride * spacing });

            const game::AssetInMemory& asset = core.assets[assetDef.assetId];

            renderer::DrawNodeHandle renderHandle = {};
            animation::Handle animHandle = {};
            spawnAsset(renderHandle, animHandle, scene, asset, asset_init_pos); 

            if (assetDef.player) {
                scene.playerAnimatedNodeHandle = animHandle;
                scene.playerDrawNodeHandle = renderHandle;
                math::identity4x4(scene.player.transform);
                scene.player.transform.pos = asset_init_pos;
            }

            if (roomDef.physicsBalls && assetDef.physicsObstacle) {
                physics::StaticObject_Sphere& o =
                    physicsScene.obstacles[physicsScene.obstacle_count++];
                o = {};
                o.pos = assetDef.asset_init_pos;
                f32 radius = math::min(
                    math::min(math::abs(asset.max.x), math::abs(asset.max.y)),
                    math::min(math::abs(asset.min.x), math::abs(asset.min.y)));
                o.radius = radius;

                if (assetDef.player) {
                    scene.playerPhysicsNodeHandle = physics::handleFromObject(o, physicsScene);
                }
            }
        }
    }

    // unit cubes behind player when running
    {
        renderer::DrawNodeInstanced& node = allocator::alloc_pool(renderScene.instancedDrawNodes);
        node = {};
		node.meshHandles[0] = core.instancedUnitCubeMesh;
        math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
        node.nodeData.groupColor = Color32(0.68f, 0.69f, 0.71f, 1.f).RGBAv4();
        node.instanceCount = 4;
        gfx::rhi::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
        gfx::rhi::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });
        gfx::rhi::RscCBuffer& cbufferinstances =
            allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_instances = handle_from_cbuffer(renderScene, cbufferinstances);
        gfx::rhi::create_cbuffer(
            cbufferinstances,
            { (u32) sizeof(float4x4) * node.instanceCount });
        for (u32 m = 0; m < countof(node.instanceMatrices.data); m++) {
            float4x4& matrix = node.instanceMatrices.data[m];
            math::identity4x4(*(Transform*)&(matrix));
        }
        scene.instancedNodesHandles[game::Scene::InstancedTypes::PlayerTrail] =
            handle_from_instanced_node(renderScene, node);
    }

    // instanced bars around scene
    {
        float width = 12.4f;
        float radius = 30.f;
        float2 ground[] = {
            float2(-radius, width),
            float2(-radius, -width),
            float2(-width, -radius),
            float2(width, -radius),
            float2(radius, -width),
            float2(radius, width),
            float2(width, radius),
            float2(-width, radius),
        };

        // physics setup
        {
            u32 prev = countof(ground) - 1;
            for (u32 i = 0; i < countof(ground); i++) {
                float2 coods_xy = ground[i];
                physics::StaticObject_Line& wall = physicsScene.walls[physicsScene.wall_count++];
                wall = {};
                wall.start = float3(ground[prev], 0.f);
                wall.end = float3(ground[i], 0.f);

                prev = i;
            }
        }

        // render setup
        {
            renderer::DrawNodeInstanced& node = allocator::alloc_pool(renderScene.instancedDrawNodes);
            node = {};
            node.meshHandles[0] = core.instancedUnitCubeMesh;
            math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
            node.nodeData.groupColor = Color32(0.82f, 0.64f, 0.12f, 1.f).RGBAv4();
            node.instanceCount = 2 * game::Resources::MirrorHallMeta::Count + 1;
            gfx::rhi::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
            node.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
            gfx::rhi::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });
            gfx::rhi::RscCBuffer& cbufferinstances =
                allocator::alloc_pool(renderScene.cbuffers);
            node.cbuffer_instances = handle_from_cbuffer(renderScene, cbufferinstances);
            gfx::rhi::create_cbuffer(
                cbufferinstances,
                { (u32)sizeof(float4x4) * node.instanceCount });

            // initialize whole buffer
            for (u32 m = 0; m < countof(node.instanceMatrices.data); m++) {
                float4x4& matrix = node.instanceMatrices.data[m];
                math::identity4x4(*(Transform*)&(matrix));
            }
            // set up the horizontal instances
            u32 maxBars = math::min(u32(game::Resources::MirrorHallMeta::Count), (u32)countof(ground));
            u32 prev = countof(ground) - 1;
            for (u32 m = 0; m < maxBars; m++) {

                float3 segment = float3(math::subtract(ground[m], ground[prev]), 0.f);
                float3 dir = math::invScale(segment, 2.f * width);

                float4x4& matrix = node.instanceMatrices.data[m];
                Transform& t = *(Transform*)&(matrix);
                t.right = dir;
                t.up = float3(0.f, 0.f, 1.f);
                t.front = float3(-t.right.y, t.right.x, t.right.z);

                t.pos = math::add(float3(ground[prev], 0.f), math::scale(dir, width));
                t.matrix.col0 = math::scale(t.matrix.col0, width);
                t.matrix.col1 = math::scale(t.matrix.col1, 0.3f);
                t.matrix.col2 = math::scale(t.matrix.col2, 0.3f);

                prev = m;
            }
            // set up the vertical instances
            u32 maxColumns = math::min(u32(game::Resources::MirrorHallMeta::Count + 1), (u32)countof(ground));
            prev = countof(ground) - 1;
            float2 prevSegment = math::subtract(ground[countof(ground) - 1], ground[countof(ground) - 2]);
            for (u32 m = 0; m < maxColumns; m++) {

                float2 segment = math::subtract(ground[m], ground[prev]);
                float3 columnDir = math::normalize(float3(math::add(segment, prevSegment), 0.f));

                float4x4& matrix = node.instanceMatrices.data[m + maxBars];
                Transform& t = *(Transform*)&(matrix);
                t.right = columnDir;
                t.front = float3(-t.right.y, t.right.x, 0.f);
                t.up = float3(0.f, 0.f, 1.f);

                t.pos = float3(ground[prev], 10.f);
                t.matrix.col0 = math::scale(t.matrix.col0, 0.4f);
                t.matrix.col1 = math::scale(t.matrix.col1, 0.8f);
                t.matrix.col2 = math::scale(t.matrix.col2, 10.f);

                prev = m;
                prevSegment = segment;
            }
            scene.instancedNodesHandles[game::Scene::InstancedTypes::SceneLimits] =
                handle_from_instanced_node(renderScene, node);
        }
    }

    // hall of mirrors
    {
        const u32 numMirrors = game::Resources::MirrorHallMeta::Count * 2;
        scene.mirrors.polys = ALLOC_ARRAY(sceneArena, game::Mirrors::Poly, numMirrors);
        scene.mirrors.drawMeshes = ALLOC_ARRAY(sceneArena, renderer::DrawMesh, numMirrors);
        scene.mirrors.bvh = {};
        const game::GPUCPUMesh& mirrorMesh = core.mirrorHallMesh;
        game::spawn_model_as_mirrors(scene.mirrors, mirrorMesh, scratchArena, sceneArena, true);
        scene.maxMirrorBounces = roomDef.maxMirrorBounces;
    }

    // SDF platform
    {
        physics::StaticObject_Sphere& o =
            physicsScene.obstacles[physicsScene.obstacle_count++];
        o = {};
        f32 radius = game::SDF_scene_radius;
        o.radius = radius;
    }

    // camera
    {
        scene.orbitCamera.offset = float3(0.f, -100.f, 0.f);
        scene.orbitCamera.eulers = float3(-25.f * math::d2r32, 0.f, 135.f * math::d2r32);
        scene.orbitCamera.minEulers = roomDef.minCameraEulers;
        scene.orbitCamera.maxEulers = roomDef.maxCameraEulers;
        scene.orbitCamera.scale = 1.f;
        scene.orbitCamera.origin = float3(0.f, 0.f, 0.f);
        scene.orbitCamera.minScale = roomDef.minCameraZoom;
        scene.orbitCamera.maxScale = roomDef.maxCameraZoom;
    }
}

#endif // __WASTELADNS_SCENE_H__
