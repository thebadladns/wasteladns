#ifndef __WASTELADNS_SCENE_H__
#define __WASTELADNS_SCENE_H__

namespace game {

struct Player {
};
struct Scene {
    camera::Camera camera;
    renderer::Scene renderScene;
    physics::Scene physicsScene;
    animation::Scene animScene;
    game::MovementController player;
    game::OrbitInput orbitCamera;
    u32 playerDrawNodeHandle;
    u32 playerAnimatedNodeHandle;
    u32 ballInstancesDrawHandle;
    u32 particlesDrawHandle; // todo: improve this
};
}

namespace fbx {

void from_ufbx_mat(float4x4& o, const ufbx_matrix& i) {
    o.col0.x = i.cols[0].x; o.col0.y = i.cols[0].y; o.col0.z = i.cols[0].z; o.col0.w = 0.f;
    o.col1.x = i.cols[1].x; o.col1.y = i.cols[1].y; o.col1.z = i.cols[1].z; o.col1.w = 0.f;
    o.col2.x = i.cols[2].x; o.col2.y = i.cols[2].y; o.col2.z = i.cols[2].z; o.col2.w = 0.f;
    o.col3.x = i.cols[3].x; o.col3.y = i.cols[3].y; o.col3.z = i.cols[3].z; o.col3.w = 1.f;
};

void extract_anim_data(animation::Skeleton& skeleton, float4x4*& skinning, 
                       animation::Clip*& clips, u32& clipCount,
                       allocator::Arena& persistentArena, const ufbx_mesh& mesh,
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

    u32* node_indices_skeleton =
        (u32*) allocator::alloc_arena(
            persistentArena, sizeof(u32) * jointCount, alignof(u32));
    s8* nodeIdtoJointId =
        (s8*)allocator::alloc_arena(
            persistentArena, sizeof(s8) * scene.nodes.count, alignof(s8));
    skeleton.jointFromGeometry =
        (float4x4*)allocator::alloc_arena(
            persistentArena, sizeof(float4x4) * jointCount, alignof(s8));
    skeleton.parentFromPosedJoint =
        (float4x4*)allocator::alloc_arena(
            persistentArena, sizeof(float4x4) * jointCount, alignof(s8));
    skeleton.geometryFromPosedJoint =
        (float4x4*)allocator::alloc_arena(
            persistentArena, sizeof(float4x4) * jointCount, alignof(s8));
    skeleton.parentIndices =
        (s8*)allocator::alloc_arena(
            persistentArena, sizeof(s8) * jointCount, alignof(s8));
    skinning =
        (float4x4*)allocator::alloc_arena(
            persistentArena, sizeof(float4x4) * jointCount, alignof(float4x4));
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
    clips =
        (animation::Clip*)allocator::alloc_arena(
            persistentArena,
            sizeof(animation::Clip) * scene.anim_stacks.count,
            alignof(animation::Clip));
    for (size_t i = 0; i < clipCount; i++) {
        const ufbx_anim_stack& stack = *(scene.anim_stacks.data[i]);
        const f64 targetFramerate = 12.f;
        const u32 maxFrames = 4096;
        const f64 duration = (f32)stack.time_end - (f32)stack.time_begin;
        const u32 numFrames = math::clamp((u32)(duration * targetFramerate), 2u, maxFrames);
        const f64 framerate = (numFrames-1) / duration;

        animation::Clip& clip = clips[i];
        clip.frameCount = numFrames;
        clip.joints =
            (animation::JointKeyframes*)allocator::alloc_arena(
                persistentArena,
                sizeof(animation::JointKeyframes) * jointCount,
                alignof(animation::JointKeyframes));
        clip.timeEnd = (f32)stack.time_end;
        for (u32 joint_index = 0; joint_index < jointCount; joint_index++) {
            ufbx_node* node = scene.nodes.data[node_indices_skeleton[joint_index]];
			animation::JointKeyframes& joint = clip.joints[joint_index];
            joint.joint_to_parent_trs =
                (animation::Joint_TRS*)allocator::alloc_arena(
                    persistentArena,
                    sizeof(animation::Joint_TRS) * numFrames,
                    alignof(animation::Joint_TRS));
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
    allocator::Arena scratchArena;
    allocator::Arena& persistentArena;
    const renderer::driver::VertexAttribDesc* vertexAttrs[renderer::DrawlistStreams::Count];
    u32 attr_count[renderer::DrawlistStreams::Count];
};
void load_with_materials(renderer::DrawNode& nodeToAdd, animation::Node& animatedNode,
                         renderer::Scene& renderScene, PipelineAssetContext& pipelineContext,
                         const char* path) {
    ufbx_load_opts opts = {};
    opts.target_axes = {
        UFBX_COORDINATE_AXIS_POSITIVE_X,
        UFBX_COORDINATE_AXIS_POSITIVE_Z,
        UFBX_COORDINATE_AXIS_POSITIVE_Y
    };
    opts.allow_null_material = true;
    ufbx_error error;

    struct Allocator_ufbx {
        static void* alloc_fn(void* user, size_t size) {
            allocator::Arena* arena = (allocator::Arena*)user;
            return allocator::alloc_arena(*arena, size, 16);
        }
        static void* realloc_fn(void* user, void* old_ptr, size_t old_size, size_t new_size) {
            allocator::Arena* arena = (allocator::Arena*)user;
            return allocator::realloc_arena(*arena, old_ptr, old_size, new_size, 16);
        }
        static void free_fn(void* user, void* ptr, size_t size) {
            allocator::Arena* arena = (allocator::Arena*)user;
            allocator::free_arena(*arena, ptr, size);
        }
    };
    opts.temp_allocator.allocator.alloc_fn = &Allocator_ufbx::alloc_fn;
    opts.temp_allocator.allocator.realloc_fn = &Allocator_ufbx::realloc_fn;
    opts.temp_allocator.allocator.free_fn = &Allocator_ufbx::free_fn;
    opts.temp_allocator.allocator.user = &pipelineContext.scratchArena;
    opts.result_allocator.allocator.alloc_fn = &Allocator_ufbx::alloc_fn;
    opts.result_allocator.allocator.realloc_fn = &Allocator_ufbx::realloc_fn;
    opts.result_allocator.allocator.free_fn = &Allocator_ufbx::free_fn;
    opts.result_allocator.allocator.user = &pipelineContext.scratchArena;

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
        animatedNode = {};
        if (scene->meshes.count && scene->meshes[0]->skin_deformers.count) {
            extract_anim_data(
                    animatedNode.skeleton, animatedNode.state.skinning,
                    animatedNode.clips, animatedNode.clipCount,
                    pipelineContext.persistentArena, *(scene->meshes[0]), *scene);
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
                   allocator::Arena & scratchArena, const u32 vertices_src_count,
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
            ctx.dstvertex_to_fbxvertex = (u32*)allocator::alloc_arena(
                pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
            ctx.dstvertex_to_fbxidx = (u32*)allocator::alloc_arena(
                pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
            ctx.fbxvertex_to_dstvertex = (u32*)allocator::alloc_arena(
                pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
            for (size_t m = 0; m < mesh.materials.count; m++) {
                ufbx_mesh_material& mesh_mat = mesh.materials.data[m];
                if (animatedNode.skeleton.jointCount) {
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

        nodeToAdd = {};
        nodeToAdd.max = max;
        nodeToAdd.min = min;
        renderer::driver::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
        nodeToAdd.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
        renderer::driver::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });

        if (animatedNode.skeleton.jointCount) {
            renderer::driver::RscCBuffer& cbufferskinning =
                allocator::alloc_pool(renderScene.cbuffers);
            nodeToAdd.cbuffer_ext = handle_from_cbuffer(renderScene, cbufferskinning);
            renderer::driver::create_cbuffer(cbufferskinning,
                { (u32) sizeof(float4x4) * animatedNode.skeleton.jointCount });
        }
        const renderer::ShaderTechniques::Enum shaderTechniques[renderer::DrawlistStreams::Count] = {
            renderer::ShaderTechniques::Color3D, renderer::ShaderTechniques::Color3DSkinned,
            renderer::ShaderTechniques::Textured3D, renderer::ShaderTechniques::Textured3DAlphaClip,
            renderer::ShaderTechniques::Textured3DSkinned,
            renderer::ShaderTechniques::Textured3DAlphaClipSkinned
        };
		for (u32 i = 0; i < renderer::DrawlistStreams::Count; i++) {
			DstStreams& stream = materialVertexBuffer[i];
			if (stream.vertex.len) {
                renderer::driver::IndexedVertexBufferDesc desc = {};
                desc.vertexData = stream.vertex.data;
                desc.vertexSize = (u32)stream.vertex.len * stream.vertex_size;
				desc.vertexCount = (u32)stream.vertex.len;
                desc.indexData = stream.index.data;
                desc.indexSize = (u32)stream.index.len * sizeof(u32);
                desc.indexCount = (u32)stream.index.len;
                desc.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
                desc.accessType = renderer::driver::BufferAccessType::GPU;
                desc.indexType = renderer::driver::BufferItemType::U32;
                desc.type = renderer::driver::BufferTopologyType::Triangles;

				const renderer::ShaderTechniques::Enum shaderTechnique = shaderTechniques[i];
                renderer::DrawMesh& mesh = allocator::alloc_pool(renderScene.meshes);
                mesh = {};
                mesh.shaderTechnique = shaderTechnique;
                renderer::driver::create_indexed_vertex_buffer(
                    mesh.vertexBuffer, desc, pipelineContext.vertexAttrs[i],
                    pipelineContext.attr_count[i]);
                if (stream.user) {
                    const char* texturefile = ((ufbx_texture*)stream.user)->filename.data;
                    renderer::driver::create_texture_from_file(
                        mesh.texture, { pipelineContext.scratchArena, texturefile });
                }
                nodeToAdd.meshHandles[i] = handle_from_drawMesh(renderScene, mesh);
			}
		}
    }
}
}

struct Camera {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 vpMatrix;
    float3 pos;
};

struct CameraNode {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    renderer::Frustum frustum;
    float3 pos;
    u32 siblingIndex;
    u32 id;
    u32 depth;
    renderer::MeshHandle meshHandle;
    char str[256];
};
struct GatherMirrorTreeContext {
    enum { MAX_CAMERA_DEPTH = 8 };
    allocator::Arena& frameArena;
    allocator::Buffer<CameraNode>& cameraTree;
    const renderer::Mirrors& mirrors;
};
u32 gatherMirrorTreeRecursive(GatherMirrorTreeContext& ctx, u32 index, const CameraNode& parent) {
    for (u32 i = 0; i < ctx.mirrors.count; i++) {
        // do not self-reflect
        if (parent.id == i) { continue; }
        // extract mirror quad from transform
        const Transform& mirror_t = ctx.mirrors.transforms[i];
        enum { MAX_MIRROR_QUAD_VERTICES = 8 };
        static_assert(MAX_MIRROR_QUAD_VERTICES <= countof(parent.frustum.planes) + 2, "check");
        float3 quad[MAX_MIRROR_QUAD_VERTICES];
        u32 quad_count = 4;
        quad[0] = math::add(
            mirror_t.matrix.col3.xyz, math::add(mirror_t.matrix.col0.xyz, mirror_t.matrix.col2.xyz));
        quad[1] = math::add(
            mirror_t.matrix.col3.xyz,
            math::add(mirror_t.matrix.col0.xyz, math::negate(mirror_t.matrix.col2.xyz)));
        quad[2] = math::add(
            mirror_t.matrix.col3.xyz,
            math::add(math::negate(mirror_t.matrix.col0.xyz), math::negate(mirror_t.matrix.col2.xyz)));
        quad[3] = math::add(
            mirror_t.matrix.col3.xyz,
            math::add(math::negate(mirror_t.matrix.col0.xyz), mirror_t.matrix.col2.xyz));

        // cull backfacing mirrors
        float3 normalWS = mirror_t.matrix.col1.xyz;
        if (math::dot(normalWS, math::subtract(parent.pos, quad[0])) < 0.f) { continue; }

        // cull quad by each frustum plane via Sutherland–Hodgman
        float3 quad_copy[MAX_MIRROR_QUAD_VERTICES];
        u32 quad_copy_count;
        for (u32 p = 0; p < parent.frustum.count
            // if we haven't culled the quad, or the previous cut didn't introduce too many cuts, 
            // keep iterating over the culling planes
            && quad_count > 2 && quad_count < countof(quad);
            p++) {
            const float4 plane = parent.frustum.planes[p];
            // copy our quad, so that we can iterate over the original data
            memcpy(quad_copy, quad, quad_count * sizeof(float3));
            quad_copy_count = quad_count;
            quad_count = 0;
            float3 va = quad_copy[quad_copy_count - 1];
            f32 va_d = math::dot(float4(va, 1.f), plane);
            for (u32 curr_v = 0; curr_v < quad_copy_count; curr_v++) {
                float3 vb = quad_copy[curr_v];
                f32 vb_d = math::dot(float4(vb, 1.f), plane);
                float3 intersection;

                const f32 eps = 0.001f;
                if (vb_d > eps) { 
                    // current vertex in positive zone,
                    // will be add to poly
                    if (va_d < -eps) { 
                        // edge entering the positive zone,
                        // add intersection point first
                        float3 ab = math::subtract(vb, va);
                        f32 t = (-va_d) / math::dot(plane.xyz, ab);
                        intersection = math::add(va, math::scale(ab, t));
                        quad[quad_count++] = intersection;
                    }
                    quad[quad_count++] = vb;
                } else if (vb_d < -eps) { 
                    // current vertex in negative zone,
                    // will not be added to poly
                    if (va_d > eps) {
                         // edge entering the negative zone,
                         // add intersection to face
                        float3 ab = math::subtract(vb, va);
                        f32 t = (-va_d) / math::dot(plane.xyz, ab);
                        intersection = math::add(va, math::scale(ab, t));
                        quad[quad_count++] = intersection;
                    }
                } else {
                    // current vertex on plane,
                    // add to poly
                    quad[quad_count++] = vb;
                }

                va = vb;
                va_d = vb_d;
            }
        }
        // quad fully culled
        if (quad_count < 3) continue;


        // acknowledge this mirror as part of the tree
        CameraNode& curr = allocator::push(ctx.cameraTree, ctx.frameArena);
        curr.depth = parent.depth + 1;
        curr.id = i;
        // todo: text for debug only???
        platform::format(curr.str, sizeof(curr.str), "%s-%d", parent.str, i, curr.depth);
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
        float3 posWS = mirror_t.matrix.col3.xyz;
        float4 planeWS(normalWS.x, normalWS.y, normalWS.z, -math::dot(posWS, normalWS));
        float4x4 reflect = reflectionMatrix(planeWS);
        curr.viewMatrix = math::mult(parent.viewMatrix, reflect);
        curr.projectionMatrix = parent.projectionMatrix;
        // Eye Space (ES) values
        float3 normalES = math::mult(curr.viewMatrix, float4(normalWS, 0.f)).xyz;
        float3 posES = math::mult(curr.viewMatrix, float4(posWS, 1.f)).xyz;
        float4 planeES(normalES.x, normalES.y, normalES.z, -math::dot(posES, normalES));
        renderer::add_oblique_plane_to_persp(curr.projectionMatrix, planeES);
        // camera position from inverse orthogonal transform
        curr.pos = float3(-math::dot(curr.viewMatrix.col3, curr.viewMatrix.col0),
                          -math::dot(curr.viewMatrix.col3, curr.viewMatrix.col1),
                          -math::dot(curr.viewMatrix.col3, curr.viewMatrix.col2));

        {
            // frustum planes: near and far come from the projection matrix
            // the rest come from the poly
            float4x4 vpMatrix = math::mult(curr.projectionMatrix, curr.viewMatrix);
            float4x4 transpose = math::transpose(vpMatrix);
            curr.frustum.planes[0] =
                math::add(math::scale(transpose.col3, -renderer::min_z), transpose.col2);   // near
            curr.frustum.planes[1] = math::subtract(transpose.col3, transpose.col2);        // far
            curr.frustum.planes[0] =
                math::invScale(curr.frustum.planes[0], math::mag(curr.frustum.planes[0].xyz));
            curr.frustum.planes[1] =
                math::invScale(curr.frustum.planes[1], math::mag(curr.frustum.planes[1].xyz));
            curr.frustum.count = 2;

            float3 prev_v = quad[quad_count - 1];
            for (u32 v = 0; v < quad_count; v++) {
                float3 curr_v = quad[v];
                float3 normal =
                    math::cross(math::subtract(prev_v, curr_v), math::subtract(curr_v, curr.pos));
                normal = math::normalize(normal);
                curr.frustum.planes[curr.frustum.count++] =
                    float4(normal, -math::dot(curr_v, normal));
                prev_v = curr_v;
            }
        }

        // recurse if there is room for one more mirror
        if (curr.depth + 1 < GatherMirrorTreeContext::MAX_CAMERA_DEPTH) {
            index = gatherMirrorTreeRecursive(ctx, index, curr);
        }
        curr.meshHandle = ctx.mirrors.meshHandles[i];
        curr.siblingIndex = index;
    }
    return index;
}

void renderBaseScene(allocator::Arena& scratchArenaRoot, game::Scene& gameScene,
                     const CameraNode& camera, const renderer::VisibleNodes& visibleNodes) {

    using namespace renderer;
    Scene& scene = gameScene.renderScene;

    driver::RscCBuffer& scene_cbuffer = cbuffer_from_handle(scene, scene.cbuffer_scene);
    SceneData cbufferPerScene;
    {
        cbufferPerScene.projectionMatrix = camera.projectionMatrix;
        cbufferPerScene.viewMatrix = camera.viewMatrix;
        cbufferPerScene.viewPos = camera.pos;
        cbufferPerScene.lightPos = float3(3.f, 8.f, 15.f);
        driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);
    }

    driver::Marker_t marker;
    driver::set_marker_name(marker, "BASE SCENE"); driver::start_event(marker);
    {
        driver::bind_RT(scene.gameRT);
        Color32 skycolor(0.2f, 0.344f, 0.59f, 1.f);
        driver::clear_RT(scene.gameRT,
              driver::RenderTargetClearFlags::Color
            | driver::RenderTargetClearFlags::Depth
            | driver::RenderTargetClearFlags::Stencil, skycolor);
        driver::bind_RS(scene.rasterizerStateFillFrontfaces);
        driver::set_marker_name(marker, "SKY"); driver::start_event(marker); // todo: quad?
        {
            driver::bind_DS(scene.depthStateOff);
            driver::bind_shader(scene.shaders[ShaderTechniques::Color3D]);
            driver::bind_blend_state(scene.blendStateBlendOff);
            for (u32 i = 0; i < countof(scene.sky.buffers); i++) {
                driver::bind_indexed_vertex_buffer(scene.sky.buffers[i]);
                driver::RscCBuffer buffers[] = { scene_cbuffer, scene.sky.cbuffer };
                driver::bind_cbuffers(scene.shaders[ShaderTechniques::Color3D], buffers, 2);
                driver::draw_indexed_vertex_buffer(scene.sky.buffers[i]);
            }
        }
        driver::end_event();
        driver::set_marker_name(marker, "OPAQUE"); driver::start_event(marker);
        {
            driver::bind_DS(scene.depthStateOn);
            allocator::Arena scratchArena = scratchArenaRoot;
            Drawlist dl = {};
            u32 maxDrawCalls =
                  (visibleNodes.visible_nodes_count
                + (u32)scene.instancedDrawNodes.count) * DrawlistStreams::Count;
            dl.items =
                (DrawCall_Item*)allocator::alloc_arena(
                    scratchArena, maxDrawCalls * sizeof(DrawCall_Item), alignof(DrawCall_Item));
            dl.keys =
                (SortKey*)allocator::alloc_arena(
                    scratchArena, maxDrawCalls * sizeof(SortKey), alignof(SortKey));
            addNodesToDrawlistSorted(
                dl, visibleNodes, camera.pos, scene,
                0, renderer::DrawlistFilter::Alpha, renderer::SortParams::Type::Default);
            Drawlist_Context ctx = {};
            Drawlist_Overrides overrides = {};
            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
            draw_drawlist(dl, ctx, overrides);
        }
        driver::end_event();
        #if __DEBUG
        {
            im::present3d(camera.projectionMatrix, camera.viewMatrix);
        }
        #endif
        driver::set_marker_name(marker, "ALPHA"); driver::start_event(marker);
        {
            allocator::Arena scratchArena = scratchArenaRoot;
            driver::bind_DS(scene.depthStateReadOnly);
            Drawlist dl = {};
            u32 maxDrawCalls =
                  (visibleNodes.visible_nodes_count
                + (u32)scene.instancedDrawNodes.count) * DrawlistStreams::Count;
            dl.items =
                (DrawCall_Item*)allocator::alloc_arena(
                    scratchArena, maxDrawCalls * sizeof(DrawCall_Item), alignof(DrawCall_Item));
            dl.keys =
                (SortKey*)allocator::alloc_arena(
                    scratchArena, maxDrawCalls * sizeof(SortKey), alignof(SortKey));
            driver::bind_blend_state(scene.blendStateOn);
            addNodesToDrawlistSorted(
                dl, visibleNodes, camera.pos, scene,
                renderer::DrawlistFilter::Alpha, 0, renderer::SortParams::Type::BackToFront);
            Drawlist_Context ctx = {};
            Drawlist_Overrides overrides = {};
            overrides.forced_blendState = true;
            ctx.blendState = &scene.blendStateOn;
            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
            draw_drawlist(dl, ctx, overrides);
        }
        driver::end_event();
    }
    driver::end_event();
}

struct RenderMirrorContext {
    allocator::Arena& scratchArenaRoot;
    game::Scene& gameScene;
    const CameraNode* mirrorTreeRoot;
    const renderer::VisibleNodes* visibleNodesTreeRoot;
    renderer::driver::RscRasterizerState* rasterizerState;
};
void renderMirrorTreeRecursive(
        RenderMirrorContext& treeCtx, const CameraNode& parent, const u32 cameraIndex) {

    using namespace renderer;

    renderer::Scene& scene = treeCtx.gameScene.renderScene;

    driver::RscRasterizerState* rasterizerStateParent = treeCtx.rasterizerState;
    driver::RscRasterizerState* rasterizerStateMirror = 
        treeCtx.rasterizerState == &scene.rasterizerStateFillFrontfaces ?
              &scene.rasterizerStateFillBackfaces
            : &scene.rasterizerStateFillFrontfaces;

    // render this mirror
    const CameraNode& camera = treeCtx.mirrorTreeRoot[cameraIndex];
    const renderer::VisibleNodes& visibleNodes = treeCtx.visibleNodesTreeRoot[cameraIndex];

    driver::Marker_t marker;
    driver::set_marker_name(marker, camera.str); driver::start_event(marker);
    {

    driver::RscCBuffer& scene_cbuffer =
        cbuffer_from_handle(scene, scene.cbuffer_scene);
    driver::RscCBuffer& identity_cbuffer =
        cbuffer_from_handle(scene, scene.cbuffer_nodeIdentity);
    driver::Marker_t marker;

    // mark this mirror on the stencil (using the parent's camera)
    driver::set_marker_name(marker, "MARK MIRROR"); driver::start_event(marker);
    {
        driver::bind_DS(scene.depthStateMarkMirror, parent.depth);
        driver::bind_RS(*rasterizerStateParent);
        driver::bind_blend_state(scene.blendStateOff);

        renderer::DrawMesh& mesh = renderer::drawMesh_from_handle(scene, camera.meshHandle);
        driver::bind_shader(scene.shaders[mesh.shaderTechnique]);
        driver::bind_indexed_vertex_buffer(mesh.vertexBuffer);
        driver::RscCBuffer buffers[] = { scene_cbuffer, identity_cbuffer };
        driver::bind_cbuffers(scene.shaders[ShaderTechniques::Color3D], buffers, 2);
        driver::draw_indexed_vertex_buffer(mesh.vertexBuffer);
    }
    driver::end_event();

    driver::set_marker_name(marker, "CLEAR MIRROR CONTENTS"); driver::start_event(marker);
    {
        renderer::driver::bind_blend_state(scene.blendStateBlendOff);
        driver::bind_DS(scene.depthStateMirrorReflectionsDepthAlways, camera.depth);
        driver::bind_RS(scene.rasterizerStateFillFrontfaces);
        driver::bind_shader(scene.shaders[renderer::ShaderTechniques::FullscreenBlitClearWhite]);
        driver::draw_fullscreen();
        driver::bind_RS(*rasterizerStateMirror);
    }
    driver::end_event();
     
    // render reflection of this mirror
    driver::set_marker_name(marker, "REFLECTION SCENE"); driver::start_event(marker);
    {
        // perspective cam set up
        {
            renderer::SceneData cbufferPerSceneMirrored = {};
            cbufferPerSceneMirrored.projectionMatrix = camera.projectionMatrix;
            cbufferPerSceneMirrored.viewMatrix = camera.viewMatrix;
            cbufferPerSceneMirrored.viewPos = camera.pos;
            cbufferPerSceneMirrored.lightPos = float3(3.f, 8.f, 15.f); //todo: improve
            renderer::driver::update_cbuffer(scene_cbuffer, &cbufferPerSceneMirrored);
        }

        driver::set_marker_name(marker, "SKY"); driver::start_event(marker);
        {
            driver::bind_DS(scene.depthStateMirrorReflectionsDepthReadOnly, camera.depth);
            driver::bind_shader(scene.shaders[ShaderTechniques::Color3D]);
            driver::bind_blend_state(scene.blendStateBlendOff);
            for (u32 i = 0; i < countof(scene.sky.buffers); i++) {
                driver::bind_indexed_vertex_buffer(scene.sky.buffers[i]);
                driver::RscCBuffer buffers[] = { scene_cbuffer, scene.sky.cbuffer };
                driver::bind_cbuffers(scene.shaders[ShaderTechniques::Color3D], buffers, 2);
                driver::draw_indexed_vertex_buffer(scene.sky.buffers[i]);
            }
        }
        driver::end_event();

        driver::set_marker_name(marker, "OPAQUE"); driver::start_event(marker);
        {
            driver::bind_DS(scene.depthStateMirrorReflections, camera.depth);
            allocator::Arena scratchArena = treeCtx.scratchArenaRoot;
            Drawlist dl = {};
            u32 maxDrawCallsThisFrame =
                   (visibleNodes.visible_nodes_count
                 + (u32)scene.instancedDrawNodes.count) * DrawlistStreams::Count;
            dl.items =
                (DrawCall_Item*)allocator::alloc_arena(
                        scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
            dl.keys = 
                (SortKey*)allocator::alloc_arena(
                        scratchArena, maxDrawCallsThisFrame * sizeof(SortKey), alignof(SortKey));
            addNodesToDrawlistSorted(
                    dl, visibleNodes, camera.pos, scene,
                    0, renderer::DrawlistFilter::Alpha, renderer::SortParams::Type::Default);
            Drawlist_Context ctx = {};
            Drawlist_Overrides overrides = {};
            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
            draw_drawlist(dl, ctx, overrides);
        }
        driver::end_event();

        #if __DEBUG
        {
            im::present3d(camera.projectionMatrix, camera.viewMatrix);
        }
        #endif

        driver::set_marker_name(marker, "ALPHA"); driver::start_event(marker);
        {
            driver::bind_DS(scene.depthStateMirrorReflectionsDepthReadOnly, camera.depth);
            driver::bind_blend_state(scene.blendStateOn);

            allocator::Arena scratchArena = treeCtx.scratchArenaRoot;
            Drawlist dl = {};
            u32 maxDrawCallsThisFrame =
                   (visibleNodes.visible_nodes_count
                 + (u32)scene.instancedDrawNodes.count) * DrawlistStreams::Count;
            dl.items =
                (DrawCall_Item*)allocator::alloc_arena(
                        scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
            dl.keys =
                (SortKey*)allocator::alloc_arena(
                        scratchArena, maxDrawCallsThisFrame * sizeof(SortKey), alignof(SortKey));
            addNodesToDrawlistSorted(
                    dl, visibleNodes, camera.pos, scene,
                    renderer::DrawlistFilter::Alpha, 0, renderer::SortParams::Type::BackToFront);
            Drawlist_Context ctx = {};
            Drawlist_Overrides overrides = {};
            overrides.forced_blendState = true;
            ctx.blendState = &scene.blendStateOn;
            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
            draw_drawlist(dl, ctx, overrides);
        }
        driver::end_event();
    }
    driver::end_event();

    // render any other mirrors seen by this mirror
    if (cameraIndex + 1 < camera.siblingIndex) {
        treeCtx.rasterizerState = rasterizerStateMirror;
        renderMirrorTreeRecursive(treeCtx, camera, cameraIndex + 1);
        treeCtx.rasterizerState = rasterizerStateParent;
    }

    {
        renderer::SceneData cbufferPerScene;
        cbufferPerScene.projectionMatrix = parent.projectionMatrix;
        cbufferPerScene.viewMatrix = parent.viewMatrix;
        cbufferPerScene.viewPos = parent.pos;
        cbufferPerScene.lightPos = float3(3.f, 8.f, 15.f); // todo: save off somewhere?
        renderer::driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);
        driver::bind_RS(*rasterizerStateParent);
    }

    driver::set_marker_name(marker, "UNMARK MIRROR"); driver::start_event(marker);
    {
        driver::bind_DS(scene.depthStateUnmarkMirror, camera.depth);
        driver::bind_blend_state(scene.blendStateOn);

        renderer::DrawMesh& mesh = renderer::drawMesh_from_handle(scene, camera.meshHandle);
        driver::bind_shader(scene.shaders[mesh.shaderTechnique]);
        driver::bind_indexed_vertex_buffer(mesh.vertexBuffer);
        driver::RscCBuffer buffers[] = { scene_cbuffer, identity_cbuffer };
        driver::bind_cbuffers(scene.shaders[ShaderTechniques::Color3D], buffers, 2);
        driver::draw_indexed_vertex_buffer(mesh.vertexBuffer);
    }
    driver::end_event();

    }
    driver::end_event();

    // todo: test render sibling mirrors
    if (camera.siblingIndex < parent.siblingIndex) {
        renderMirrorTreeRecursive(treeCtx, parent, camera.siblingIndex);
    }
}

struct SceneMemory {
    allocator::Arena& persistentArena;
    allocator::Arena scratchArena; // explicit copy, makes it a stack allocator for this context
    __DEBUGDEF(allocator::Arena& debugArena;)
};
void init_scene(game::Scene& scene, SceneMemory& memory, const platform::Screen& screen) {

    allocator::Arena& persistentArena = memory.persistentArena;

    renderer::Scene& renderScene = scene.renderScene;
    physics::Scene& physicsScene = scene.physicsScene;
    animation::Scene& animScene = scene.animScene;

    const size_t drawNodePoolSize =
        math::min((size_t)maxRenderNodes, (size_t)renderer::DrawNodeMeta::MaxNodes);
    const size_t instancedNodePoolSize =
		math::min((size_t)maxRenderNodesInstanced, (size_t)renderer::DrawNodeMeta::MaxNodes);
    const size_t cbufferSize =
		(drawNodePoolSize * 2 + instancedNodePoolSize * 2) + 16;
    const size_t meshPoolSize =
		(drawNodePoolSize + instancedNodePoolSize) * renderer::ShaderTechniques::Count;
    const size_t animatedNodesSize =
        math::min((size_t)maxAnimatedNodes, (size_t)renderer::DrawNodeMeta::MaxNodes);
    allocator::init_pool(renderScene.cbuffers, cbufferSize, persistentArena);
	__DEBUGDEF(renderScene.cbuffers.name = "cbuffers";)
    allocator::init_pool(renderScene.meshes, meshPoolSize, persistentArena);
	__DEBUGDEF(renderScene.meshes.name = "meshes";)
    allocator::init_pool(renderScene.instancedDrawNodes, instancedNodePoolSize, persistentArena);
	__DEBUGDEF(renderScene.instancedDrawNodes.name = "instanced draw nodes";)
    allocator::init_pool(renderScene.drawNodes, drawNodePoolSize, persistentArena);
	__DEBUGDEF(renderScene.drawNodes.name = "draw nodes";)
    allocator::init_pool(animScene.nodes, animatedNodesSize, persistentArena);
	__DEBUGDEF(animScene.nodes.name = "anim nodes";)

    camera::WindowProjection::Config& ortho = renderScene.windowProjection.config;
    ortho.right = screen.window_width * 0.5f;
    ortho.top = screen.window_height * 0.5f;
    ortho.left = -ortho.right;
    ortho.bottom = -ortho.top;
    ortho.near = 0.f;
    ortho.far = 1000.f;
    renderer::generate_matrix_ortho(renderScene.windowProjection.matrix, ortho);

    // Todo: consider whether precision needs special handling
    camera::PerspProjection::Config& frustum = renderScene.perspProjection.config;
    frustum.fov = 45.f;
    frustum.aspect = screen.width / (f32)screen.height;
    frustum.near = 1.f;
    frustum.far = 1000.f;
    renderer::generate_matrix_persp(renderScene.perspProjection.matrix, frustum);

    renderer::driver::MainRenderTargetParams windowRTparams = {};
    windowRTparams.depth = true;
    windowRTparams.width = screen.window_width;
    windowRTparams.height = screen.window_height;
    renderer::driver::create_main_RT(renderScene.windowRT, windowRTparams);

    renderer::driver::RenderTargetParams gameRTparams;
    gameRTparams.depth = true;
    gameRTparams.width = screen.width;
    gameRTparams.height = screen.height;
    gameRTparams.textureFormat = renderer::driver::TextureFormat::V4_8;
    gameRTparams.textureInternalFormat = renderer::driver::InternalTextureFormat::V4_8;
    gameRTparams.textureFormatType = renderer::driver::Type::Float;
    gameRTparams.count = 1;
    renderer::driver::create_RT(renderScene.gameRT, gameRTparams);

    {
        renderer::driver::BlendStateParams blendState_params;
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            renderer::driver::RenderTargetWriteMask::All; blendState_params.blendEnable = true;
        renderer::driver::create_blend_state(renderScene.blendStateOn, blendState_params);
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            renderer::driver::RenderTargetWriteMask::All;
        renderer::driver::create_blend_state(renderScene.blendStateBlendOff, blendState_params);
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            renderer::driver::RenderTargetWriteMask::None;
        renderer::driver::create_blend_state(renderScene.blendStateOff, blendState_params);
    }
    renderer::driver::create_RS(renderScene.rasterizerStateFillFrontfaces,
            { renderer::driver::RasterizerFillMode::Fill,
              renderer::driver::RasterizerCullMode::CullBack });
    renderer::driver::create_RS(renderScene.rasterizerStateFillBackfaces,
            { renderer::driver::RasterizerFillMode::Fill,
              renderer::driver::RasterizerCullMode::CullFront });
    renderer::driver::create_RS(renderScene.rasterizerStateFillCullNone,
            { renderer::driver::RasterizerFillMode::Fill,
              renderer::driver::RasterizerCullMode::CullNone });
    renderer::driver::create_RS(renderScene.rasterizerStateLine,
            { renderer::driver::RasterizerFillMode::Line,
              renderer::driver::RasterizerCullMode::CullNone });
    {
        // BASE depth states
        renderer::driver::DepthStencilStateParams dsParams;
        dsParams = {};
        dsParams.depth_enable = true;
        dsParams.depth_func = renderer::driver::CompFunc::Less;
        dsParams.depth_writemask = renderer::driver::DepthWriteMask::All;
        renderer::driver::create_DS(renderScene.depthStateOn, dsParams);
        dsParams = {};
        dsParams.depth_enable = true;
        dsParams.depth_func = renderer::driver::CompFunc::Less;
        dsParams.depth_writemask = renderer::driver::DepthWriteMask::Zero;
        renderer::driver::create_DS(renderScene.depthStateReadOnly, dsParams);
        dsParams = {};
        dsParams.depth_enable = false;
        renderer::driver::create_DS(renderScene.depthStateOff, dsParams);
        { // MIRROR depth states
            dsParams = {};
            dsParams.depth_enable = true;
            dsParams.depth_func = renderer::driver::CompFunc::Less;
            dsParams.depth_writemask = renderer::driver::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = dsParams.stencil_writemask = 0xff;
            dsParams.stencil_failOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_depthFailOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_passOp = renderer::driver::StencilOp::Incr;
            dsParams.stencil_func = renderer::driver::CompFunc::Equal;
            renderer::driver::create_DS(renderScene.depthStateMarkMirror, dsParams);
            dsParams = {};
            dsParams.depth_enable = true;
            dsParams.depth_func = renderer::driver::CompFunc::Always;
            dsParams.depth_writemask = renderer::driver::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = dsParams.stencil_writemask = 0xff;
            dsParams.stencil_failOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_depthFailOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_passOp = renderer::driver::StencilOp::Decr;
            dsParams.stencil_func = renderer::driver::CompFunc::Equal;
            renderer::driver::create_DS(renderScene.depthStateUnmarkMirror, dsParams);
            dsParams = {};
            dsParams.depth_enable = true;
            dsParams.depth_func = renderer::driver::CompFunc::Always;
            dsParams.depth_writemask = renderer::driver::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = dsParams.stencil_writemask = 0xff;
            dsParams.stencil_failOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_depthFailOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_passOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_func = renderer::driver::CompFunc::Equal;
            renderer::driver::create_DS(renderScene.depthStateClearDepthInMirror, dsParams);
            dsParams.depth_enable = true;
            dsParams.depth_func = renderer::driver::CompFunc::Less;
            dsParams.depth_writemask = renderer::driver::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = 0xff;
            dsParams.stencil_writemask = 0x00;
            dsParams.stencil_failOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_depthFailOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_passOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_func = renderer::driver::CompFunc::Equal;
            renderer::driver::create_DS(renderScene.depthStateMirrorReflections, dsParams);
            dsParams.depth_enable = true;
            dsParams.depth_func = renderer::driver::CompFunc::Less;
            dsParams.depth_writemask = renderer::driver::DepthWriteMask::Zero;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = 0xff;
            dsParams.stencil_writemask = 0x00;
            dsParams.stencil_failOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_depthFailOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_passOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_func = renderer::driver::CompFunc::Equal;
            renderer::driver::create_DS(renderScene.depthStateMirrorReflectionsDepthReadOnly, dsParams);
            dsParams.depth_enable = true;
            dsParams.depth_func = renderer::driver::CompFunc::Always;
            dsParams.depth_writemask = renderer::driver::DepthWriteMask::All;
            dsParams.stencil_enable = true;
            dsParams.stencil_readmask = 0xff;
            dsParams.stencil_writemask = 0x00;
            dsParams.stencil_failOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_depthFailOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_passOp = renderer::driver::StencilOp::Keep;
            dsParams.stencil_func = renderer::driver::CompFunc::Equal;
            renderer::driver::create_DS(renderScene.depthStateMirrorReflectionsDepthAlways, dsParams);
        }
    }

    // known cbuffers
    renderer::driver::RscCBuffer& cbufferscene = allocator::alloc_pool(renderScene.cbuffers);
    renderScene.cbuffer_scene = handle_from_cbuffer(renderScene, cbufferscene);
    renderer::driver::create_cbuffer(cbufferscene, { sizeof(renderer::SceneData) });
    renderer::driver::RscCBuffer& cbufferNodeIdentity = allocator::alloc_pool(renderScene.cbuffers);
    renderScene.cbuffer_nodeIdentity = handle_from_cbuffer(renderScene, cbufferNodeIdentity);
    renderer::driver::create_cbuffer(cbufferNodeIdentity, { sizeof(renderer::NodeData) });
    renderer::NodeData nodeIdentity = {};
    nodeIdentity.groupColor = float4(1.f, 1.f, 1.f, 1.f);
    math::identity4x4(*(Transform*)&nodeIdentity.worldMatrix);
    renderer::driver::update_cbuffer(cbufferNodeIdentity, &nodeIdentity);

    // input layouts
    const renderer::driver::VertexAttribDesc attribs_3d[] = {
        renderer::driver::make_vertexAttribDesc(
                "POSITION", 0,
                sizeof(float3),
                renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
    };
    const renderer::driver::VertexAttribDesc attribs_color3d[] = {
        renderer::driver::make_vertexAttribDesc(
				"POSITION", offsetof(renderer::VertexLayout_Color_3D, pos),
                sizeof(renderer::VertexLayout_Color_3D),
                renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
        renderer::driver::make_vertexAttribDesc(
				"COLOR", offsetof(renderer::VertexLayout_Color_3D, color),
                sizeof(renderer::VertexLayout_Color_3D),
                renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM) };
    const renderer::driver::VertexAttribDesc attribs_color3d_skinned[] = {
        renderer::driver::make_vertexAttribDesc(
				"POSITION", offsetof(renderer::VertexLayout_Color_Skinned_3D, pos),
                sizeof(renderer::VertexLayout_Color_Skinned_3D),
                renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
        renderer::driver::make_vertexAttribDesc(
				"COLOR", offsetof(renderer::VertexLayout_Color_Skinned_3D, color),
                sizeof(renderer::VertexLayout_Color_Skinned_3D),
                renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM),
        renderer::driver::make_vertexAttribDesc(
				"JOINTINDICES", offsetof(renderer::VertexLayout_Color_Skinned_3D, joint_indices),
                sizeof(renderer::VertexLayout_Color_Skinned_3D),
                renderer::driver::BufferAttributeFormat::R8G8B8A8_SINT),
        renderer::driver::make_vertexAttribDesc(
				"JOINTWEIGHTS", offsetof(renderer::VertexLayout_Color_Skinned_3D, joint_weights),
                sizeof(renderer::VertexLayout_Color_Skinned_3D),
                renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM)
    };
    const renderer::driver::VertexAttribDesc attribs_textured3d[] = {
        renderer::driver::make_vertexAttribDesc(
				"POSITION", offsetof(renderer::VertexLayout_Textured_3D, pos),
                sizeof(renderer::VertexLayout_Textured_3D),
                renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
        renderer::driver::make_vertexAttribDesc(
				"TEXCOORD", offsetof(renderer::VertexLayout_Textured_3D, uv),
                sizeof(renderer::VertexLayout_Textured_3D),
                renderer::driver::BufferAttributeFormat::R32G32_FLOAT)
    };
    const renderer::driver::VertexAttribDesc attribs_textured3d_skinned[] = {
        renderer::driver::make_vertexAttribDesc(
				"POSITION", offsetof(renderer::VertexLayout_Textured_Skinned_3D, pos),
                sizeof(renderer::VertexLayout_Textured_Skinned_3D),
                renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
        renderer::driver::make_vertexAttribDesc(
				"TEXCOORD", offsetof(renderer::VertexLayout_Textured_Skinned_3D, uv),
                sizeof(renderer::VertexLayout_Textured_Skinned_3D),
                renderer::driver::BufferAttributeFormat::R32G32_FLOAT),
        renderer::driver::make_vertexAttribDesc(
				"JOINTINDICES", offsetof(renderer::VertexLayout_Textured_Skinned_3D, joint_indices),
                sizeof(renderer::VertexLayout_Textured_Skinned_3D),
                renderer::driver::BufferAttributeFormat::R8G8B8A8_SINT),
        renderer::driver::make_vertexAttribDesc(
				"JOINTWEIGHTS", offsetof(renderer::VertexLayout_Textured_Skinned_3D, joint_weights),
                sizeof(renderer::VertexLayout_Textured_Skinned_3D),
                renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM)
    };

    // cbuffer bindings
    const renderer::driver::CBufferBindingDesc bufferBindings_untextured_base[] = {
        { "type_PerScene", renderer::driver::CBufferStageMask::VS },
        { "type_PerGroup", renderer::driver::CBufferStageMask::VS }
    };
    const renderer::driver::CBufferBindingDesc bufferBindings_skinned_untextured_base[] = {
        { "type_PerScene", renderer::driver::CBufferStageMask::VS },
        { "type_PerGroup", renderer::driver::CBufferStageMask::VS },
        { "type_PerJoint", renderer::driver::CBufferStageMask::VS }
    };
    const renderer::driver::CBufferBindingDesc bufferBindings_textured_base[] = {
        { "type_PerScene", renderer::driver::CBufferStageMask::VS },
        { "type_PerGroup",
            renderer::driver::CBufferStageMask::VS | renderer::driver::CBufferStageMask::PS }
    };
    const renderer::driver::CBufferBindingDesc bufferBindings_skinned_textured_base[] = {
        { "type_PerScene", renderer::driver::CBufferStageMask::VS },
        { "type_PerGroup",
            renderer::driver::CBufferStageMask::VS | renderer::driver::CBufferStageMask::PS },
        { "type_PerJoint", renderer::driver::CBufferStageMask::VS }
    };
    const renderer::driver::CBufferBindingDesc bufferBindings_instanced_base[] = {
        { "type_PerScene", renderer::driver::CBufferStageMask::VS },
        { "type_PerGroup", renderer::driver::CBufferStageMask::VS },
        { "type_PerInstance", renderer::driver::CBufferStageMask::VS }
    };

    // texture bindings
    const renderer::driver::TextureBindingDesc textureBindings_base[] = { { "texDiffuse" } };
    const renderer::driver::TextureBindingDesc textureBindings_fullscreenblit[] = {{ "texSrc" }};

    // shaders
    {
        allocator::Arena scratchArena = memory.scratchArena; // explicit copy
        // Initialize cache with room for a VS and PS shader of each technique
        renderer::driver::ShaderCache shader_cache = {};
        renderer::driver::load_shader_cache(
            shader_cache, "shaderCache.bin", &scratchArena,
            renderer::ShaderTechniques::Count * 2);
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_3d;
            desc.vertexAttr_count = countof(attribs_3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = nullptr;
            desc.bufferBinding_count = 0;
            // reuse 3d shaders
            desc.vs_name = renderer::shaders::vs_fullscreen_bufferless_clear_blit.name;
            desc.vs_src = renderer::shaders::vs_fullscreen_bufferless_clear_blit.src;
            desc.ps_name = renderer::shaders::ps_fullscreen_blit_white.name;
            desc.ps_src = renderer::shaders::ps_fullscreen_blit_white.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                renderScene.shaders[renderer::ShaderTechniques::FullscreenBlitClearWhite], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d;
            desc.vertexAttr_count = countof(attribs_textured3d);
            desc.textureBindings = textureBindings_fullscreenblit;
            desc.textureBinding_count = countof(textureBindings_fullscreenblit);
            desc.bufferBindings = nullptr;
            desc.bufferBinding_count = 0;
            // reuse 3d shaders
            desc.vs_name = renderer::shaders::vs_fullscreen_bufferless_textured_blit.name;
            desc.vs_src = renderer::shaders::vs_fullscreen_bufferless_textured_blit.src;
            desc.ps_name = renderer::shaders::ps_fullscreen_blit_textured.name;
            desc.ps_src = renderer::shaders::ps_fullscreen_blit_textured.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                    renderScene.shaders[renderer::ShaderTechniques::FullscreenBlitTextured], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_3d;
            desc.vertexAttr_count = countof(attribs_3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_instanced_base;
            desc.bufferBinding_count = countof(bufferBindings_instanced_base);
            desc.vs_name = renderer::shaders::vs_3d_instanced_base.name;
            desc.vs_src = renderer::shaders::vs_3d_instanced_base.src;
            desc.ps_name = renderer::shaders::ps_color3d_unlit.name;
            desc.ps_src = renderer::shaders::ps_color3d_unlit.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                    renderScene.shaders[renderer::ShaderTechniques::Instanced3D], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_color3d;
            desc.vertexAttr_count = countof(attribs_color3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_untextured_base;
            desc.bufferBinding_count = countof(bufferBindings_untextured_base);
            desc.vs_name = renderer::shaders::vs_color3d_base.name;
            desc.vs_src = renderer::shaders::vs_color3d_base.src;
            desc.ps_name = renderer::shaders::ps_color3d_unlit.name;
            desc.ps_src = renderer::shaders::ps_color3d_unlit.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(renderScene.shaders[renderer::ShaderTechniques::Color3D], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_color3d_skinned;
            desc.vertexAttr_count = countof(attribs_color3d_skinned);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_skinned_untextured_base;
            desc.bufferBinding_count = countof(bufferBindings_skinned_untextured_base);
            desc.vs_name = renderer::shaders::vs_color3d_skinned_base.name;
            desc.vs_src = renderer::shaders::vs_color3d_skinned_base.src;
            desc.ps_name = renderer::shaders::ps_color3d_unlit.name;
            desc.ps_src = renderer::shaders::ps_color3d_unlit.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                    renderScene.shaders[renderer::ShaderTechniques::Color3DSkinned], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d;
            desc.vertexAttr_count = countof(attribs_textured3d);
            desc.textureBindings = textureBindings_base;
            desc.textureBinding_count = countof(textureBindings_base);
            desc.bufferBindings = bufferBindings_textured_base;
            desc.bufferBinding_count = countof(bufferBindings_textured_base);
            desc.vs_name = renderer::shaders::vs_textured3d_base.name;
            desc.vs_src = renderer::shaders::vs_textured3d_base.src;
            desc.ps_name = renderer::shaders::ps_textured3d_base.name;
            desc.ps_src = renderer::shaders::ps_textured3d_base.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                    renderScene.shaders[renderer::ShaderTechniques::Textured3D], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d;
            desc.vertexAttr_count = countof(attribs_textured3d);
            desc.textureBindings = textureBindings_base;
            desc.textureBinding_count = countof(textureBindings_base);
            desc.bufferBindings = bufferBindings_textured_base;
            desc.bufferBinding_count = countof(bufferBindings_textured_base);
            desc.vs_name = renderer::shaders::vs_textured3d_base.name;
            desc.vs_src = renderer::shaders::vs_textured3d_base.src;
            desc.ps_name = renderer::shaders::ps_textured3dalphaclip_base.name;
            desc.ps_src = renderer::shaders::ps_textured3dalphaclip_base.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                    renderScene.shaders[renderer::ShaderTechniques::Textured3DAlphaClip], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d_skinned;
            desc.vertexAttr_count = countof(attribs_textured3d_skinned);
            desc.textureBindings = textureBindings_base;
            desc.textureBinding_count = countof(textureBindings_base);
            desc.bufferBindings = bufferBindings_skinned_textured_base;
            desc.bufferBinding_count = countof(bufferBindings_skinned_textured_base);
            desc.vs_name = renderer::shaders::vs_textured3d_skinned_base.name;
            desc.vs_src = renderer::shaders::vs_textured3d_skinned_base.src;
            desc.ps_name = renderer::shaders::ps_textured3d_base.name;
            desc.ps_src = renderer::shaders::ps_textured3d_base.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                    renderScene.shaders[renderer::ShaderTechniques::Textured3DSkinned], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_textured3d_skinned;
            desc.vertexAttr_count = countof(attribs_textured3d_skinned);
            desc.textureBindings = textureBindings_base;
            desc.textureBinding_count = countof(textureBindings_base);
            desc.bufferBindings = bufferBindings_skinned_textured_base;
            desc.bufferBinding_count = countof(bufferBindings_skinned_textured_base);
            desc.vs_name = renderer::shaders::vs_textured3d_skinned_base.name;
            desc.vs_src = renderer::shaders::vs_textured3d_skinned_base.src;
            desc.ps_name = renderer::shaders::ps_textured3dalphaclip_base.name;
            desc.ps_src = renderer::shaders::ps_textured3dalphaclip_base.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                    renderScene.shaders[renderer::ShaderTechniques::Textured3DAlphaClipSkinned], desc);
        }
        renderer::driver::write_shader_cache(shader_cache);
    }

    allocator::Arena scratchArena = memory.scratchArena; // explicit copy

    struct AssetData {
        const char* path;
        float3 asset_init_pos;
        u32 count;
        bool player; // only first count
    };
    // from blender: export fbx -> Apply Scalings: FBX All
    // -> Forward: the one in Blender -> Use Space Transform: yes
    const AssetData assets[] = {
          { "assets/meshes/boar.fbx", { -5.f, 10.f, 2.30885f }, 1, false }
        , { "assets/meshes/bird.fbx", { 1.f, 3.f, 2.23879f }, 1, true }
        , { "assets/meshes/bird.fbx", { -8.f, 12.f, 2.23879f }, 1, false }
    };

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
        const AssetData& asset = assets[asset_idx];

        for (u32 asset_rep = 0; asset_rep < asset.count; asset_rep++) {
            animation::Node animatedNode = {};

            float3 asset_init_pos = asset.asset_init_pos;
            const f32 spacing = 10.f;
            const s32 grid_size = 16;
            const s32 max_row = (asset.count - 1) % grid_size;
            const s32 max_column = ((asset.count - 1) / grid_size) % grid_size;
            s32 row = asset_rep % grid_size;
            s32 column = (asset_rep / grid_size) % grid_size;
            s32 stride = asset_rep / (grid_size * grid_size);
            asset_init_pos = math::add(asset_init_pos,
                                      {(row - max_row /2) * spacing,
                                       (column - max_column/2) * spacing,
                                        stride * spacing });

            u32 nodeHandle = 0;
		    ctx.scratchArena = scratchArena; // explicit copy
            renderer::DrawNode nodeToAdd = {};
            fbx::load_with_materials(nodeToAdd, animatedNode, renderScene, ctx, asset.path);

            renderer::DrawNode& node = allocator::alloc_pool(renderScene.drawNodes);
            node = nodeToAdd; // todo: validate that node was loaded?
            nodeHandle = renderer::handle_from_node(renderScene, node);
            math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
            node.nodeData.worldMatrix.col3 = { asset_init_pos, 1.f };
            node.nodeData.groupColor = Color32(1.f, 1.f, 1.f, 1.f).RGBAv4();
            animation::Handle animHandle = {};
            if (animatedNode.skeleton.jointCount) {
                for (u32 m = 0; m < animatedNode.skeleton.jointCount; m++) {
                    float4x4& matrix = animatedNode.state.skinning[m];
                    math::identity4x4(*(Transform*)&(matrix));
                }
                renderer::driver::RscCBuffer& cbufferskinning =
                    allocator::alloc_pool(renderScene.cbuffers);
                node.cbuffer_ext = handle_from_cbuffer(renderScene, cbufferskinning);
                renderer::driver::create_cbuffer(
                    cbufferskinning,
                    { (u32)sizeof(float4x4) * animatedNode.skeleton.jointCount });
                node.ext_data = animatedNode.state.skinning; // hack: improve
                animatedNode.state.animIndex = 0;
                animatedNode.state.time = 0.f;
                animatedNode.state.scale = 1.f;
                animation::Node& animatedNodeToAdd = allocator::alloc_pool(animScene.nodes);
                animatedNodeToAdd = animatedNode;
                animHandle = animation::handle_from_node(scene.animScene, animatedNodeToAdd);
		    }

            if (asset.player) {
                scene.playerAnimatedNodeHandle = animHandle;
                scene.playerDrawNodeHandle = nodeHandle;
                math::identity4x4(scene.player.transform);
                scene.player.transform.pos = asset_init_pos;
            }
        }
    }

    // sky
    {
        renderer::driver::create_cbuffer(renderScene.sky.cbuffer, { sizeof(renderer::NodeData) });
        math::identity4x4(*(Transform*)&(renderScene.sky.nodeData.worldMatrix));
        renderScene.sky.nodeData.worldMatrix.col3.xyz = float3(-5.f, -8.f, 5.f);
        renderScene.sky.nodeData.groupColor = Color32(1.f, 1.f, 1.f, 1.f).RGBAv4();
        renderer::driver::update_cbuffer(renderScene.sky.cbuffer, &renderScene.sky.nodeData);

        renderer::driver::IndexedVertexBufferDesc bufferParams;
        bufferParams.vertexSize = 4 * sizeof(renderer::VertexLayout_Color_3D);
        bufferParams.vertexCount = 4;
        bufferParams.indexSize = 6 * sizeof(u16);
        bufferParams.indexCount = 6;
        bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
        bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
        bufferParams.indexType = renderer::driver::BufferItemType::U16;
        bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
        renderer::driver::VertexAttribDesc attribs[] = {
            renderer::driver::make_vertexAttribDesc(
                    "POSITION", offsetof(renderer::VertexLayout_Color_3D, pos),
                    sizeof(renderer::VertexLayout_Color_3D),
                    renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
            renderer::driver::make_vertexAttribDesc(
                    "COLOR", offsetof(renderer::VertexLayout_Color_3D, color),
                    sizeof(renderer::VertexLayout_Color_3D),
                    renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM)
        };

        u32 c = Color32(0.2f, 0.344f, 0.59f, 1.f).ABGR();
        f32 w = 150.f;
        f32 h = 500.f;
        {
            renderer::VertexLayout_Color_3D v[] = {
                { { w, w, -h }, c }, { { -w, w, -h }, c },
                { { -w, w, h }, c }, { { w, w, h }, c } };
            u16 i[] = { 0, 1, 2, 2, 3, 0 };
            bufferParams.vertexData = v;
            bufferParams.indexData = i;
            renderer::driver::create_indexed_vertex_buffer(
                    renderScene.sky.buffers[0], bufferParams, attribs, countof(attribs));
        }
        {
            renderer::VertexLayout_Color_3D v[] = {
				{ { w, -w, -h }, c }, { { -w, -w, -h }, c },
				{ { -w, -w, h }, c }, { { w, -w, h }, c } };
            u16 i[] = { 2, 1, 0, 0, 3, 2 };
            bufferParams.vertexData = v;
            bufferParams.indexData = i;
            renderer::driver::create_indexed_vertex_buffer(
					renderScene.sky.buffers[1], bufferParams, attribs, countof(attribs));
        }
        {
            renderer::VertexLayout_Color_3D v[] = {
				{ { w, w, -h }, c }, { { w, -w, -h }, c },
				{ { w, -w, h }, c }, { { w, w, h }, c } };
            u16 i[] = { 2, 1, 0, 0, 3, 2 };
            bufferParams.vertexData = v;
            bufferParams.indexData = i;
            renderer::driver::create_indexed_vertex_buffer(
					renderScene.sky.buffers[2], bufferParams, attribs, countof(attribs));
        }
        {
            renderer::VertexLayout_Color_3D v[] = {
				{ { -w, w, -h }, c }, { { -w, -w, -h }, c },
				{ { -w, -w, h }, c }, { { -w, w, h }, c } };
            u16 i[] = { 0, 1, 2, 0, 2, 3 };
            bufferParams.vertexData = v;
            bufferParams.indexData = i;
            renderer::driver::create_indexed_vertex_buffer(
					renderScene.sky.buffers[3], bufferParams, attribs, countof(attribs));
        }
        {
            renderer::VertexLayout_Color_3D v[] = {
				{ { -w, w, h }, c }, { { -w, -w, h }, c },
				{ { w, -w, h }, c }, { { w, w, h }, c } };
            u16 i[] = { 0, 1, 2, 0, 2, 3 };
            bufferParams.vertexData = v;
            bufferParams.indexData = i;
            renderer::driver::create_indexed_vertex_buffer(
					renderScene.sky.buffers[4], bufferParams, attribs, countof(attribs));
        }
        {
            renderer::VertexLayout_Color_3D v[] = {
				{ { -w, w, -h/4 }, c }, { { -w, -w, -h/4 }, c },
				{ { w, -w, -h/4 }, c }, { { w, w, -h/4 }, c } };
            u16 i[] = { 2, 1, 0, 0, 3, 2 };
            bufferParams.vertexData = v;
            bufferParams.indexData = i;
            renderer::driver::create_indexed_vertex_buffer(
					renderScene.sky.buffers[5], bufferParams, attribs, countof(attribs));
        }
    }

    // alpha unit cubes (todo: shouldn't be alpha and instanced)
    {
        renderer::DrawMesh& mesh = allocator::alloc_pool(renderScene.meshes);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Instanced3D;
        renderer::UntexturedCube cube;
        renderer::create_cube_coords(
				(uintptr_t)cube.vertices, sizeof(cube.vertices[0]),
                cube.indices, float3(1.f, 1.f, 1.f), float3(0.f, 0.f, 0.f));
        renderer::create_indexed_vertex_buffer_from_untextured_mesh(
				mesh.vertexBuffer, cube.vertices, countof(cube.vertices),
                cube.indices, countof(cube.indices));
        renderer::DrawNodeInstanced& node = allocator::alloc_pool(renderScene.instancedDrawNodes);
        node = {};
		node.meshHandles[0] = handle_from_drawMesh(renderScene, mesh);
        math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
        node.nodeData.groupColor = Color32(0.9f, 0.7f, 0.8f, 0.6f).RGBAv4();
        node.instanceCount = maxPlayerParticleCubes;
        renderer::driver::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
        renderer::driver::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });
        renderer::driver::RscCBuffer& cbufferinstances =
            allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_instances = handle_from_cbuffer(renderScene, cbufferinstances);
        renderer::driver::create_cbuffer(
                cbufferinstances,
                { (u32) sizeof(float4x4) * node.instanceCount });
        for (u32 m = 0; m < countof(node.instanceMatrices.data); m++) {
            float4x4& matrix = node.instanceMatrices.data[m];
            math::identity4x4(*(Transform*)&(matrix));
        }
        scene.particlesDrawHandle = handle_from_instanced_node(renderScene, node);
    }

    // ground
    {
        renderer::DrawMesh& mesh = allocator::alloc_pool(renderScene.meshes);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Color3D;
        f32 w = 30.f, h = 30.f;
        u32 c = Color32(1.f, 1.f, 1.f, 1.f).ABGR();
        renderer::VertexLayout_Color_3D v[] = {
            { { w, -h, 0.f }, c }, { { -w, -h, 0.f }, c },
			{ { -w, h, 0.f }, c }, { { w, h, 0.f }, c } };
        u16 i[] = { 0, 1, 2, 2, 3, 0 };
        renderer::driver::IndexedVertexBufferDesc bufferParams;
        bufferParams.vertexData = v;
        bufferParams.indexData = i;
        bufferParams.vertexSize = sizeof(v);
        bufferParams.vertexCount = countof(v);
        bufferParams.indexSize = sizeof(i);
        bufferParams.indexCount = countof(i);
        bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
        bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
        bufferParams.indexType = renderer::driver::BufferItemType::U16;
        bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
        renderer::driver::VertexAttribDesc attribs[] = {
            renderer::driver::make_vertexAttribDesc(
                    "POSITION", offsetof(renderer::VertexLayout_Color_3D, pos),
                    sizeof(renderer::VertexLayout_Color_3D),
                    renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
            renderer::driver::make_vertexAttribDesc(
                    "COLOR", offsetof(renderer::VertexLayout_Color_3D, color),
                    sizeof(renderer::VertexLayout_Color_3D),
                    renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM)
        };
        renderer::driver::create_indexed_vertex_buffer(
                mesh.vertexBuffer, bufferParams, attribs, countof(attribs));
        renderer::DrawNode& node = allocator::alloc_pool(renderScene.drawNodes);
        node = {};
        node.meshHandles[0] = handle_from_drawMesh(renderScene, mesh);
        math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
        node.nodeData.worldMatrix.col3.xyz = float3(0.f, 0.f, 0.f);
        node.nodeData.groupColor = Color32(0.13f, 0.51f, 0.23f, 1.f).RGBAv4();
        node.max = float3(w, h, 0.f); // todo: improve node generation!!! I KEEP MISSING THIS
        node.min = float3(-w, -h, 0.f);
        renderer::driver::RscCBuffer& cbuffernode = allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_node = handle_from_cbuffer(renderScene, cbuffernode);
        renderer::driver::create_cbuffer(cbuffernode, { sizeof(renderer::NodeData) });
    }

    // mirrors
    for (u32 m = 0; m < 8; m++) {
        renderer::DrawMesh& mesh = allocator::alloc_pool(renderScene.meshes);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Color3D;
        f32 w = 8.4f, h = 10.f;
        u32 c = Color32(0.01f, 0.19f, 0.3f, 0.32f).ABGR();
        renderer::VertexLayout_Color_3D v[] = {
            { { 1.f, 0.f, -1.f }, c }, { { -1.f, 0.f, -1.f }, c },
            { { -1.f, 0.f, 1.f }, c }, { { 1.f, 0.f, 1.f }, c } };
        const u32 mirrorId = renderScene.mirrors.count++;
        Transform& t = renderScene.mirrors.transforms[mirrorId];
        Transform rot = math::fromOffsetAndOrbit(
            float3(0.f, 0.f, 0.f), float3(0.f, 0.f, f32(m) * math::twopi32 / f32(8) ));
        Transform translate = {};
        math::identity4x4(translate);
        translate.pos = float3(0.f, -20.f, 0.f);
        t.matrix = math::mult(rot.matrix, translate.matrix);
        t.matrix.col0.xyz = math::scale(t.matrix.col0.xyz, w);
        t.matrix.col2.xyz = math::scale(t.matrix.col2.xyz, h);
        for (u32 i = 0; i < countof(v); i++) {
            v[i].pos = math::mult(t.matrix, float4(v[i].pos, 1.f)).xyz;
        }
        u16 i[] = { 2, 1, 0, 0, 3, 2 };
        renderer::driver::IndexedVertexBufferDesc bufferParams;
        bufferParams.vertexData = v;
        bufferParams.indexData = i;
        bufferParams.vertexSize = sizeof(v);
        bufferParams.vertexCount = countof(v);
        bufferParams.indexSize = sizeof(i);
        bufferParams.indexCount = countof(i);
        bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
        bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
        bufferParams.indexType = renderer::driver::BufferItemType::U16;
        bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
        renderer::driver::VertexAttribDesc attribs[] = {
            renderer::driver::make_vertexAttribDesc(
                    "POSITION", offsetof(renderer::VertexLayout_Color_3D, pos),
                    sizeof(renderer::VertexLayout_Color_3D),
                    renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
            renderer::driver::make_vertexAttribDesc(
                    "COLOR", offsetof(renderer::VertexLayout_Color_3D, color),
                    sizeof(renderer::VertexLayout_Color_3D),
                    renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM)
        };
        renderer::driver::create_indexed_vertex_buffer(
                mesh.vertexBuffer, bufferParams, attribs, countof(attribs));
        {
            renderScene.mirrors.meshHandles[mirrorId] = handle_from_drawMesh(renderScene, mesh);
            renderer::driver::RscCBuffer& cbuffernode =
                allocator::alloc_pool(renderScene.cbuffers);
            renderer::driver::create_cbuffer(cbuffernode, { sizeof(renderer::NodeData) });
        }
    }

    // physics
    {
        struct BallAssets {
            float3 position;
            float radius;
            float3 velocity;
            float mass;
        };
        BallAssets ballAssets[8];
        const u32 numballs = countof(ballAssets);
        const f32 w = 30.f;
        const f32 h = 30.f;
        const f32 maxspeed = 20.f;
        //physicsScene.dt = 1 / 60.f;
        physicsScene.bounds = float2(w, h);
        physicsScene.restitution = 1.f;
        for (u32 i = 0; i < numballs; i++) {
            physics::Ball& ball = physicsScene.balls[i];
            ball.radius = math::rand() * 3.f;
            ball.mass = math::pi32 * ball.radius * ball.radius;
            ball.pos = float3(w * (-1.f + 2.f * math::rand()), h * (-1.f + 2.f * math::rand()), 0.f);
            ball.vel = float3(maxspeed * (-1.f + 2.f * math::rand()), maxspeed * (-1.f + 2.f * math::rand()), 0.f);
        }
        renderer::DrawMesh& mesh = allocator::alloc_pool(renderScene.meshes);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Instanced3D;

        renderer::UntexturedSphere sphere;
        renderer::create_untextured_sphere_coords(sphere, 1.f);
        renderer::create_indexed_vertex_buffer_from_untextured_mesh(
                mesh.vertexBuffer, sphere.vertices, countof(sphere.vertices),
                sphere.indices, countof(sphere.indices));
        renderer::DrawNodeInstanced& node = allocator::alloc_pool(renderScene.instancedDrawNodes);
        node = {};
        node.meshHandles[0] = handle_from_drawMesh(renderScene, mesh);
        math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
        node.nodeData.groupColor = Color32(0.72f, 0.74f, 0.12f, 1.f).RGBAv4();
        node.instanceCount = numballs;
        renderer::driver::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
        renderer::driver::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });
        renderer::driver::RscCBuffer& cbufferinstances =
            allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_instances = 
            handle_from_cbuffer(renderScene, cbufferinstances);
        renderer::driver::create_cbuffer(cbufferinstances, { sizeof(float4x4) * numballs });
        for (u32 m = 0; m < countof(node.instanceMatrices.data); m++) {
            float4x4& matrix = node.instanceMatrices.data[m];
            math::identity4x4(*(Transform*)&(matrix));
        }
        scene.ballInstancesDrawHandle = handle_from_instanced_node(renderScene, node);
    }

    // camera
    scene.orbitCamera.offset = float3(0.f, -100.f, 0.f);
    scene.orbitCamera.eulers = float3(-25.f * math::d2r32, 0.f, 135.f * math::d2r32);
    scene.orbitCamera.scale = 1.f;
    scene.orbitCamera.origin = float3(0.f, 0.f, 0.f);

    __DEBUGDEF(renderer::im::init(memory.debugArena);)
}

#endif // __WASTELADNS_SCENE_H__
