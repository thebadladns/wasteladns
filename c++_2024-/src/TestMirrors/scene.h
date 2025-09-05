#ifndef __WASTELADNS_SCENE_H__
#define __WASTELADNS_SCENE_H__

namespace game {

struct Mirrors { // todo: figure out delete, unhack sizes
    struct Poly { float3 v[4]; float3 normal; u32 numPts; };
    Poly* polys; // xy from 0.f to 1.f describe the mirror quad, z is the normal
    renderer::DrawMesh* drawMeshes;
    bvh::Tree bvh; // used to accelerate visibility queries
    u32 count;
};
struct GPUCPUMesh {
    renderer::CPUMesh cpuBuffer;
    renderer::driver::RscIndexedVertexBuffer gpuBuffer;
};

struct Scene {
    struct InstancedTypes { enum Enum { PlayerTrail, PhysicsBalls, Count }; };
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
    struct MeshesMeta { enum Enum { ToiletLo, ToiletHi, HallOfMirrors, Count }; };
    struct AssetsMeta { enum Enum { Bird, Boar, Ground, Count }; };
    struct MirrorHallMeta { enum Enum { Count = 8 }; };
    renderer::CoreResources renderCore;
    AssetInMemory assets[AssetsMeta::Count];
    GPUCPUMesh meshes[MeshesMeta::Count];
    renderer::MeshHandle instancedUnitCubeMesh;
    renderer::MeshHandle instancedUnitSphereMesh;
    renderer::MeshHandle groundMesh;
    // UI
    renderer::driver::RscIndexedVertexBuffer gpuBufferHeaderText;
    renderer::driver::RscIndexedVertexBuffer gpuBufferOrbitText;
    renderer::driver::RscIndexedVertexBuffer gpuBufferPanText;
    renderer::driver::RscIndexedVertexBuffer gpuBufferScaleText;
    renderer::driver::RscIndexedVertexBuffer gpuBufferCycleRoomText;
};
struct RoomDefinition {
    Resources::MeshesMeta::Enum mirrorMesh;
    float3 minCameraEulers;
    float3 maxCameraEulers;
    f32 minCameraZoom;
    f32 maxCameraZoom;
    u32 maxMirrorBounces;
    bool physicsBalls;
};
const RoomDefinition roomDefinitions[] = {
#if __DEBUG
    { Resources::MeshesMeta::ToiletLo,
      float3(-180.f * math::d2r32, 0.f, -180 * math::d2r32), // min camera eulers
      float3(180.f * math::d2r32, 0.f, 180 * math::d2r32), // max camera eulers
      0.3f, 2.f,
      4, true },
    { Resources::MeshesMeta::ToiletHi,
      float3(-180.f * math::d2r32, 0.f, -180 * math::d2r32), // min camera eulers
      float3(180.f * math::d2r32, 0.f, 180 * math::d2r32), // max camera eulers
      0.2f, 0.5f,
      2, true },
    { Resources::MeshesMeta::HallOfMirrors,
      float3(-180.f * math::d2r32, 0.f, -180 * math::d2r32), // min camera eulers
      float3(180.f * math::d2r32, 0.f, 180 * math::d2r32), // max camera eulers
      0.3f, 2.f,
      8, true }
#else
    { Resources::MeshesMeta::ToiletLo,
      float3(-45 * math::d2r32, 0.f, -180 * math::d2r32), // min camera eulers
      float3(-1.f * math::d2r32, 0.f, 180 * math::d2r32), // max camera eulers
      0.3f, 2.f,
      4, true },
    { Resources::MeshesMeta::ToiletHi,
      float3(-30 * math::d2r32, 0.f, -180 * math::d2r32), // min camera eulers
      float3(-1.f * math::d2r32, 0.f, 180 * math::d2r32), // max camera eulers
      0.2f, 0.5f,
      3, true },
    { Resources::MeshesMeta::HallOfMirrors,
      float3(-45 * math::d2r32, 0.f, -180 * math::d2r32), // min camera eulers
      float3(-1.f * math::d2r32, 0.f, 180 * math::d2r32), // max camera eulers
      0.3f, 2.f,
      8, true }
#endif
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
    renderer::driver::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
    renderNode.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
    renderer::driver::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });
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
        renderer::driver::RscCBuffer& cbufferskinning =
            allocator::alloc_pool(renderScene.cbuffers);
        renderNode.cbuffer_ext = handle_from_cbuffer(renderScene, cbufferskinning);
        renderer::driver::create_cbuffer(cbufferskinning,
            { (u32) sizeof(float4x4) * animNode.skeleton.jointCount });
        renderNode.ext_data = animNode.state.skinning;
    }
    // todo: physics??
}

void spawn_model_as_mirrors(
    game::Mirrors& mirrors, const game::GPUCPUMesh& loadedMesh,
    allocator::PagedArena scratchArena, allocator::PagedArena& sceneArena) {

    const renderer::CPUMesh& cpuMesh = loadedMesh.cpuBuffer;
    u32* triangleIds =
        (u32*)allocator::alloc_arena(
            scratchArena, sizeof(u32) * (cpuMesh.indexCount / 3), alignof(u32));

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
        triangleIds[triangles++] = mirrorId;

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
                triangleIds[triangles++] = mirrorId;
            }
        }
        poly.normal = math::normalize(math::cross(math::subtract(poly.v[2], poly.v[0]), math::subtract(poly.v[1], poly.v[0])));
        index += mesh.vertexBuffer.indexCount;
    }

    bvh::buildTree(
        sceneArena, scratchArena, mirrors.bvh, &(loadedMesh.cpuBuffer.vertices[0].x),
        loadedMesh.cpuBuffer.indices, loadedMesh.cpuBuffer.indexCount, triangleIds);
}
}


namespace obj {

void load_mesh_cpu_gpu(
    game::GPUCPUMesh& meshToLoad, renderer::CoreResources& renderCore,
    allocator::PagedArena scratchArena, allocator::PagedArena& persistentArena,
    const char* path, const f32 scale) {

    allocator::Buffer<renderer::VertexLayout_Color_3D> vertices = {};
    allocator::Buffer<u16> indices = {};

    FILE* f;
    if (platform::fopen(&f, path, "r") == 0) {
        char c;
        do {
            c = platform::fgetc(f);
            if (c == 'v') { // read vertices
                renderer::VertexLayout_Color_3D& v = allocator::push(vertices, scratchArena);
                f32 x, y, z, r, g, b;
                s32 n = platform::fscanf(f, "%f%f%f%f%f%f", &x, &y, &z, &r, &g, &b);
                if (n == 6) {
                    v.pos.x = scale * x; v.pos.y = scale * y; v.pos.z = scale * z;
                    v.color = Color32(r, g, b, 0.32f).ABGR();
                }
            } else if (c == 'f') { // read face indices (can't read any more vertices from now on)
                int a, b, c;
                s32 n = platform::fscanf(f, "%d%d%d", &a, &b, &c);
                if (n == 3) {
                    allocator::push(indices, scratchArena) = c - 1;
                    allocator::push(indices, scratchArena) = b - 1;
                    allocator::push(indices, scratchArena) = a - 1;
                    int d;
                    n = platform::fscanf(f, "%d", &d);
                    if (n > 0) {
                        // read four values: divide quad in two triangles
                        allocator::push(indices, scratchArena) = d - 1;
                        allocator::push(indices, scratchArena) = c - 1;
                        allocator::push(indices, scratchArena) = a - 1;
                    }
                }
            }
        } while (c > 0);
        platform::fclose(f);
    }

    if (vertices.len) {
        meshToLoad = {};

        renderer::CPUMesh& mesh = meshToLoad.cpuBuffer;
        mesh.vertices = (float3*)allocator::alloc_arena(
            persistentArena,
            sizeof(float3) * vertices.len, alignof(float3));
        mesh.indices = (u16*)allocator::alloc_arena(
            persistentArena,
            sizeof(u16) * indices.len, alignof(u16));
        for (u32 i = 0; i < vertices.len; i++) { mesh.vertices[i] = vertices.data[i].pos; }
        memcpy(mesh.indices, indices.data, sizeof(u16) * indices.len);
        mesh.indexCount = (u32)indices.len;
        mesh.vertexCount = (u32)vertices.len;

        // create the global vertex buffer for all the mirrors
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
        renderer::driver::IndexedVertexBufferDesc bufferParams;
        bufferParams.vertexData = vertices.data;
        bufferParams.indexData = indices.data;
        bufferParams.vertexSize = (u32) (sizeof(renderer::VertexLayout_Color_3D) * vertices.len);
        bufferParams.vertexCount = (u32) vertices.len;
        bufferParams.indexSize = (u32)(sizeof(u16)* indices.len);
        bufferParams.indexCount = (u32) indices.len;
        bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
        bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
        bufferParams.indexType = renderer::driver::BufferItemType::U16;
        bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
        renderer::driver::create_indexed_vertex_buffer(
            meshToLoad.gpuBuffer, bufferParams, attribs, countof(attribs));
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
    assetToAdd.clips =
        (animation::Clip*)allocator::alloc_arena(
            persistentArena,
            sizeof(animation::Clip) * scene.anim_stacks.count,
            alignof(animation::Clip));
    for (size_t i = 0; i < assetToAdd.clipCount; i++) {
        const ufbx_anim_stack& stack = *(scene.anim_stacks.data[i]);
        const f64 targetFramerate = 12.f;
        const u32 maxFrames = 4096;
        const f64 duration = (f32)stack.time_end - (f32)stack.time_begin;
        const u32 numFrames = math::clamp((u32)(duration * targetFramerate), 2u, maxFrames);
        const f64 framerate = (numFrames-1) / duration;

        animation::Clip& clip = assetToAdd.clips[i];
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
    allocator::PagedArena scratchArena;
    allocator::PagedArena& persistentArena;
    const renderer::driver::VertexAttribDesc* vertexAttrs[renderer::DrawlistStreams::Count];
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

    struct Allocator_ufbx {
        static void* alloc_fn(void* user, size_t size) {
            allocator::PagedArena* arena = (allocator::PagedArena*)user;
            return allocator::alloc_arena(*arena, size, 16);
        }
        static void* realloc_fn(void* user, void* old_ptr, size_t old_size, size_t new_size) {
            allocator::PagedArena* arena = (allocator::PagedArena*)user;
            return allocator::realloc_arena(*arena, old_ptr, old_size, new_size, 16);
        }
        static void free_fn(void* user, void* ptr, size_t size) {
            allocator::PagedArena* arena = (allocator::PagedArena*)user;
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
            ctx.dstvertex_to_fbxvertex = (u32*)allocator::alloc_arena(
                pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
            ctx.dstvertex_to_fbxidx = (u32*)allocator::alloc_arena(
                pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
            ctx.fbxvertex_to_dstvertex = (u32*)allocator::alloc_arena(
                pipelineContext.scratchArena, sizeof(u32) * mesh_vertices_count, alignof(u32));
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

            const renderer::ShaderTechniques::Enum shaderTechnique =
                shaderTechniques[i];
            renderer::DrawMesh& mesh =
                renderer::alloc_drawMesh(renderCore);
            mesh = {};
            mesh.shaderTechnique = shaderTechnique;
            renderer::driver::create_indexed_vertex_buffer(
                mesh.vertexBuffer, desc, pipelineContext.vertexAttrs[i],
                pipelineContext.attr_count[i]);
            if (stream.user) {
                const char* texturefile =
                    ((ufbx_texture*)stream.user)->filename.data;
                renderer::driver::create_texture_from_file(
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

#if DISABLE_INTRINSICS
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
#else
void clip_poly_in_frustum_256(
    float3* poly, u32& poly_count, const m256_4* planes, const u32 planeCount, const u32 polyCountCap) {

    assert(polyCountCap <= 8);
    // cull quad by each frustum plane via Sutherland-Hodgman using AVX2's 256-bit vectors
    // a lookup-table tells us which edges we should be doing intersection checks on, as well as
    // handles degenerate cases by picking just 2 intersecting edges

    // duplicate last entry to ensure no new intersections
    for (u32 i = poly_count; i < 8; i++) { poly[i] = poly[poly_count - 1]; }
    // expand input:
    // (x0,x1,x2,x3,x4,x5,x6,x7)
    // (y0,y1,y2,y3,y4,y5,y6,y7)
    // (z0,z1,z2,z3,z4,z5,z6,z7)
    __m256 vx_in, vy_in, vz_in;
    vx_in = _mm256_set_ps(poly[7].x, poly[6].x, poly[5].x, poly[4].x, poly[3].x, poly[2].x, poly[1].x, poly[0].x);
    vy_in = _mm256_set_ps(poly[7].y, poly[6].y, poly[5].y, poly[4].y, poly[3].y, poly[2].y, poly[1].y, poly[0].y);
    vz_in = _mm256_set_ps(poly[7].z, poly[6].z, poly[5].z, poly[4].z, poly[3].z, poly[2].z, poly[1].z, poly[0].z);

    const f32 eps = 0.001f;
    __m256 veps = _mm256_set1_ps(eps);

    u32 input_poly_count = 0;
    u32 output_poly_count = poly_count;

    // keep iterating over the culling planes until
    // we have fully culled the quad, or the previous cut introduced too many cuts
    for (u32 p = 0; p < planeCount && output_poly_count > 2 && output_poly_count < polyCountCap; p++) {

        input_poly_count = output_poly_count;
        output_poly_count = 0;

        // compute all clip flags
        const m256_4& plane = planes[p];
        __m256 vdotxw = _mm256_fmadd_ps(vx_in, plane.vx, plane.vw);
        __m256 vdotxyw = _mm256_fmadd_ps(vy_in, plane.vy, vdotxw);
        __m256 vdot = _mm256_fmadd_ps(vz_in, plane.vz, vdotxyw);
        __m256 vdoteps = _mm256_sub_ps(vdot, veps);
        const u32 clipFlags = _mm256_movemask_ps(vdoteps);
        /*
        // Code to generate the couple of indices that mark the first intersection from a positive
        // region (0) into a negative region (1), followed by the reversed intersection, using the
        // clip flags (whether a vertex is in the positive or negative region) as input
        printf("const u8 transitionLUT[] = {\n");
        for (int n = 0; n < 256;n++) {
            int i, j;
            if (n == 0)         { i = j = 0; }  // 0b00000000 -> all in positive region: cull none
            else if (n == 0xff) { i = j = 8; }  // 0b11111111 -> all in negative region: cull all
            else {
                // capture start and ends of a group of 1s closest to the least significant bits
                // i.e.: 0b00001100 -> { 2, 4 }
                // i.e.: 0b00000111 -> { 0, 3 }
                // it is important that we select the group of 1s closest to the right,
                // so the flags work with polygons with less than 8 vertices
                // i.e.: 0b01100101 -> { 0, 1 }, not { 2, 3 } nor { 5, 7 }
                int k = 0;
                if ((n & (1 << k)) == 0) {
                    // first digit is 0, poly starts in the positive zone
                    // find the next digit in the negative zone
                    k = 1;
                    while (k < 8 && (n & (1 << k)) == 0) { k++; }
                    i = k;
                    // next, find the next digit in the positive zone (we may loop to the beginning)
                    while (k < 8 && (n & (1 << k)) != 0) { k++; }
                    j = k % 8;
                } else {
                    // first digit is 1, poly starts in the negative zone
                    // find the last digit inside the negative zone, towards the left
                    k = 1;
                    while (k < 8 && (n & (1 << k)) != 0) { k++; }
                    j = k;
                    // find the next digit on the right from the start inside a positive zone
                    k = 7;
                    while (k > 0 && (n & (1 << k)) != 0) { k--; }
                    i = (k + 1) % 8;
                }
            }
            // generate code
            printf("    0x%02x,", i << 4 | j);  // actual array byte content, one index in each half
            printf(" // {%d, %d} : 0b", i, j);  // readable indexes for reference, and clip flags as
            for (int s = 128; s; s>>=1)         // binary: print each digit separately
            { printf("%d", (n & s) != 0); }     // print as bool digit (0 or 1)
            printf(" (%d)\n", n);               // clip flags in base 10
        };
        printf("};\n");
        */
        static const u8 transitionLUT[] = {
            0x00, // {0, 0} : 0b00000000 (0)
            0x01, // {0, 1} : 0b00000001 (1)
            0x12, // {1, 2} : 0b00000010 (2)
            0x02, // {0, 2} : 0b00000011 (3)
            0x23, // {2, 3} : 0b00000100 (4)
            0x01, // {0, 1} : 0b00000101 (5)
            0x13, // {1, 3} : 0b00000110 (6)
            0x03, // {0, 3} : 0b00000111 (7)
            0x34, // {3, 4} : 0b00001000 (8)
            0x01, // {0, 1} : 0b00001001 (9)
            0x12, // {1, 2} : 0b00001010 (10)
            0x02, // {0, 2} : 0b00001011 (11)
            0x24, // {2, 4} : 0b00001100 (12)
            0x01, // {0, 1} : 0b00001101 (13)
            0x14, // {1, 4} : 0b00001110 (14)
            0x04, // {0, 4} : 0b00001111 (15)
            0x45, // {4, 5} : 0b00010000 (16)
            0x01, // {0, 1} : 0b00010001 (17)
            0x12, // {1, 2} : 0b00010010 (18)
            0x02, // {0, 2} : 0b00010011 (19)
            0x23, // {2, 3} : 0b00010100 (20)
            0x01, // {0, 1} : 0b00010101 (21)
            0x13, // {1, 3} : 0b00010110 (22)
            0x03, // {0, 3} : 0b00010111 (23)
            0x35, // {3, 5} : 0b00011000 (24)
            0x01, // {0, 1} : 0b00011001 (25)
            0x12, // {1, 2} : 0b00011010 (26)
            0x02, // {0, 2} : 0b00011011 (27)
            0x25, // {2, 5} : 0b00011100 (28)
            0x01, // {0, 1} : 0b00011101 (29)
            0x15, // {1, 5} : 0b00011110 (30)
            0x05, // {0, 5} : 0b00011111 (31)
            0x56, // {5, 6} : 0b00100000 (32)
            0x01, // {0, 1} : 0b00100001 (33)
            0x12, // {1, 2} : 0b00100010 (34)
            0x02, // {0, 2} : 0b00100011 (35)
            0x23, // {2, 3} : 0b00100100 (36)
            0x01, // {0, 1} : 0b00100101 (37)
            0x13, // {1, 3} : 0b00100110 (38)
            0x03, // {0, 3} : 0b00100111 (39)
            0x34, // {3, 4} : 0b00101000 (40)
            0x01, // {0, 1} : 0b00101001 (41)
            0x12, // {1, 2} : 0b00101010 (42)
            0x02, // {0, 2} : 0b00101011 (43)
            0x24, // {2, 4} : 0b00101100 (44)
            0x01, // {0, 1} : 0b00101101 (45)
            0x14, // {1, 4} : 0b00101110 (46)
            0x04, // {0, 4} : 0b00101111 (47)
            0x46, // {4, 6} : 0b00110000 (48)
            0x01, // {0, 1} : 0b00110001 (49)
            0x12, // {1, 2} : 0b00110010 (50)
            0x02, // {0, 2} : 0b00110011 (51)
            0x23, // {2, 3} : 0b00110100 (52)
            0x01, // {0, 1} : 0b00110101 (53)
            0x13, // {1, 3} : 0b00110110 (54)
            0x03, // {0, 3} : 0b00110111 (55)
            0x36, // {3, 6} : 0b00111000 (56)
            0x01, // {0, 1} : 0b00111001 (57)
            0x12, // {1, 2} : 0b00111010 (58)
            0x02, // {0, 2} : 0b00111011 (59)
            0x26, // {2, 6} : 0b00111100 (60)
            0x01, // {0, 1} : 0b00111101 (61)
            0x16, // {1, 6} : 0b00111110 (62)
            0x06, // {0, 6} : 0b00111111 (63)
            0x67, // {6, 7} : 0b01000000 (64)
            0x01, // {0, 1} : 0b01000001 (65)
            0x12, // {1, 2} : 0b01000010 (66)
            0x02, // {0, 2} : 0b01000011 (67)
            0x23, // {2, 3} : 0b01000100 (68)
            0x01, // {0, 1} : 0b01000101 (69)
            0x13, // {1, 3} : 0b01000110 (70)
            0x03, // {0, 3} : 0b01000111 (71)
            0x34, // {3, 4} : 0b01001000 (72)
            0x01, // {0, 1} : 0b01001001 (73)
            0x12, // {1, 2} : 0b01001010 (74)
            0x02, // {0, 2} : 0b01001011 (75)
            0x24, // {2, 4} : 0b01001100 (76)
            0x01, // {0, 1} : 0b01001101 (77)
            0x14, // {1, 4} : 0b01001110 (78)
            0x04, // {0, 4} : 0b01001111 (79)
            0x45, // {4, 5} : 0b01010000 (80)
            0x01, // {0, 1} : 0b01010001 (81)
            0x12, // {1, 2} : 0b01010010 (82)
            0x02, // {0, 2} : 0b01010011 (83)
            0x23, // {2, 3} : 0b01010100 (84)
            0x01, // {0, 1} : 0b01010101 (85)
            0x13, // {1, 3} : 0b01010110 (86)
            0x03, // {0, 3} : 0b01010111 (87)
            0x35, // {3, 5} : 0b01011000 (88)
            0x01, // {0, 1} : 0b01011001 (89)
            0x12, // {1, 2} : 0b01011010 (90)
            0x02, // {0, 2} : 0b01011011 (91)
            0x25, // {2, 5} : 0b01011100 (92)
            0x01, // {0, 1} : 0b01011101 (93)
            0x15, // {1, 5} : 0b01011110 (94)
            0x05, // {0, 5} : 0b01011111 (95)
            0x57, // {5, 7} : 0b01100000 (96)
            0x01, // {0, 1} : 0b01100001 (97)
            0x12, // {1, 2} : 0b01100010 (98)
            0x02, // {0, 2} : 0b01100011 (99)
            0x23, // {2, 3} : 0b01100100 (100)
            0x01, // {0, 1} : 0b01100101 (101)
            0x13, // {1, 3} : 0b01100110 (102)
            0x03, // {0, 3} : 0b01100111 (103)
            0x34, // {3, 4} : 0b01101000 (104)
            0x01, // {0, 1} : 0b01101001 (105)
            0x12, // {1, 2} : 0b01101010 (106)
            0x02, // {0, 2} : 0b01101011 (107)
            0x24, // {2, 4} : 0b01101100 (108)
            0x01, // {0, 1} : 0b01101101 (109)
            0x14, // {1, 4} : 0b01101110 (110)
            0x04, // {0, 4} : 0b01101111 (111)
            0x47, // {4, 7} : 0b01110000 (112)
            0x01, // {0, 1} : 0b01110001 (113)
            0x12, // {1, 2} : 0b01110010 (114)
            0x02, // {0, 2} : 0b01110011 (115)
            0x23, // {2, 3} : 0b01110100 (116)
            0x01, // {0, 1} : 0b01110101 (117)
            0x13, // {1, 3} : 0b01110110 (118)
            0x03, // {0, 3} : 0b01110111 (119)
            0x37, // {3, 7} : 0b01111000 (120)
            0x01, // {0, 1} : 0b01111001 (121)
            0x12, // {1, 2} : 0b01111010 (122)
            0x02, // {0, 2} : 0b01111011 (123)
            0x27, // {2, 7} : 0b01111100 (124)
            0x01, // {0, 1} : 0b01111101 (125)
            0x17, // {1, 7} : 0b01111110 (126)
            0x07, // {0, 7} : 0b01111111 (127)
            0x70, // {7, 0} : 0b10000000 (128)
            0x71, // {7, 1} : 0b10000001 (129)
            0x12, // {1, 2} : 0b10000010 (130)
            0x72, // {7, 2} : 0b10000011 (131)
            0x23, // {2, 3} : 0b10000100 (132)
            0x71, // {7, 1} : 0b10000101 (133)
            0x13, // {1, 3} : 0b10000110 (134)
            0x73, // {7, 3} : 0b10000111 (135)
            0x34, // {3, 4} : 0b10001000 (136)
            0x71, // {7, 1} : 0b10001001 (137)
            0x12, // {1, 2} : 0b10001010 (138)
            0x72, // {7, 2} : 0b10001011 (139)
            0x24, // {2, 4} : 0b10001100 (140)
            0x71, // {7, 1} : 0b10001101 (141)
            0x14, // {1, 4} : 0b10001110 (142)
            0x74, // {7, 4} : 0b10001111 (143)
            0x45, // {4, 5} : 0b10010000 (144)
            0x71, // {7, 1} : 0b10010001 (145)
            0x12, // {1, 2} : 0b10010010 (146)
            0x72, // {7, 2} : 0b10010011 (147)
            0x23, // {2, 3} : 0b10010100 (148)
            0x71, // {7, 1} : 0b10010101 (149)
            0x13, // {1, 3} : 0b10010110 (150)
            0x73, // {7, 3} : 0b10010111 (151)
            0x35, // {3, 5} : 0b10011000 (152)
            0x71, // {7, 1} : 0b10011001 (153)
            0x12, // {1, 2} : 0b10011010 (154)
            0x72, // {7, 2} : 0b10011011 (155)
            0x25, // {2, 5} : 0b10011100 (156)
            0x71, // {7, 1} : 0b10011101 (157)
            0x15, // {1, 5} : 0b10011110 (158)
            0x75, // {7, 5} : 0b10011111 (159)
            0x56, // {5, 6} : 0b10100000 (160)
            0x71, // {7, 1} : 0b10100001 (161)
            0x12, // {1, 2} : 0b10100010 (162)
            0x72, // {7, 2} : 0b10100011 (163)
            0x23, // {2, 3} : 0b10100100 (164)
            0x71, // {7, 1} : 0b10100101 (165)
            0x13, // {1, 3} : 0b10100110 (166)
            0x73, // {7, 3} : 0b10100111 (167)
            0x34, // {3, 4} : 0b10101000 (168)
            0x71, // {7, 1} : 0b10101001 (169)
            0x12, // {1, 2} : 0b10101010 (170)
            0x72, // {7, 2} : 0b10101011 (171)
            0x24, // {2, 4} : 0b10101100 (172)
            0x71, // {7, 1} : 0b10101101 (173)
            0x14, // {1, 4} : 0b10101110 (174)
            0x74, // {7, 4} : 0b10101111 (175)
            0x46, // {4, 6} : 0b10110000 (176)
            0x71, // {7, 1} : 0b10110001 (177)
            0x12, // {1, 2} : 0b10110010 (178)
            0x72, // {7, 2} : 0b10110011 (179)
            0x23, // {2, 3} : 0b10110100 (180)
            0x71, // {7, 1} : 0b10110101 (181)
            0x13, // {1, 3} : 0b10110110 (182)
            0x73, // {7, 3} : 0b10110111 (183)
            0x36, // {3, 6} : 0b10111000 (184)
            0x71, // {7, 1} : 0b10111001 (185)
            0x12, // {1, 2} : 0b10111010 (186)
            0x72, // {7, 2} : 0b10111011 (187)
            0x26, // {2, 6} : 0b10111100 (188)
            0x71, // {7, 1} : 0b10111101 (189)
            0x16, // {1, 6} : 0b10111110 (190)
            0x76, // {7, 6} : 0b10111111 (191)
            0x60, // {6, 0} : 0b11000000 (192)
            0x61, // {6, 1} : 0b11000001 (193)
            0x12, // {1, 2} : 0b11000010 (194)
            0x62, // {6, 2} : 0b11000011 (195)
            0x23, // {2, 3} : 0b11000100 (196)
            0x61, // {6, 1} : 0b11000101 (197)
            0x13, // {1, 3} : 0b11000110 (198)
            0x63, // {6, 3} : 0b11000111 (199)
            0x34, // {3, 4} : 0b11001000 (200)
            0x61, // {6, 1} : 0b11001001 (201)
            0x12, // {1, 2} : 0b11001010 (202)
            0x62, // {6, 2} : 0b11001011 (203)
            0x24, // {2, 4} : 0b11001100 (204)
            0x61, // {6, 1} : 0b11001101 (205)
            0x14, // {1, 4} : 0b11001110 (206)
            0x64, // {6, 4} : 0b11001111 (207)
            0x45, // {4, 5} : 0b11010000 (208)
            0x61, // {6, 1} : 0b11010001 (209)
            0x12, // {1, 2} : 0b11010010 (210)
            0x62, // {6, 2} : 0b11010011 (211)
            0x23, // {2, 3} : 0b11010100 (212)
            0x61, // {6, 1} : 0b11010101 (213)
            0x13, // {1, 3} : 0b11010110 (214)
            0x63, // {6, 3} : 0b11010111 (215)
            0x35, // {3, 5} : 0b11011000 (216)
            0x61, // {6, 1} : 0b11011001 (217)
            0x12, // {1, 2} : 0b11011010 (218)
            0x62, // {6, 2} : 0b11011011 (219)
            0x25, // {2, 5} : 0b11011100 (220)
            0x61, // {6, 1} : 0b11011101 (221)
            0x15, // {1, 5} : 0b11011110 (222)
            0x65, // {6, 5} : 0b11011111 (223)
            0x50, // {5, 0} : 0b11100000 (224)
            0x51, // {5, 1} : 0b11100001 (225)
            0x12, // {1, 2} : 0b11100010 (226)
            0x52, // {5, 2} : 0b11100011 (227)
            0x23, // {2, 3} : 0b11100100 (228)
            0x51, // {5, 1} : 0b11100101 (229)
            0x13, // {1, 3} : 0b11100110 (230)
            0x53, // {5, 3} : 0b11100111 (231)
            0x34, // {3, 4} : 0b11101000 (232)
            0x51, // {5, 1} : 0b11101001 (233)
            0x12, // {1, 2} : 0b11101010 (234)
            0x52, // {5, 2} : 0b11101011 (235)
            0x24, // {2, 4} : 0b11101100 (236)
            0x51, // {5, 1} : 0b11101101 (237)
            0x14, // {1, 4} : 0b11101110 (238)
            0x54, // {5, 4} : 0b11101111 (239)
            0x40, // {4, 0} : 0b11110000 (240)
            0x41, // {4, 1} : 0b11110001 (241)
            0x12, // {1, 2} : 0b11110010 (242)
            0x42, // {4, 2} : 0b11110011 (243)
            0x23, // {2, 3} : 0b11110100 (244)
            0x41, // {4, 1} : 0b11110101 (245)
            0x13, // {1, 3} : 0b11110110 (246)
            0x43, // {4, 3} : 0b11110111 (247)
            0x30, // {3, 0} : 0b11111000 (248)
            0x31, // {3, 1} : 0b11111001 (249)
            0x12, // {1, 2} : 0b11111010 (250)
            0x32, // {3, 2} : 0b11111011 (251)
            0x20, // {2, 0} : 0b11111100 (252)
            0x21, // {2, 1} : 0b11111101 (253)
            0x10, // {1, 0} : 0b11111110 (254)
            0x88, // {8, 8} : 0b11111111 (255)
        };

        u8 intersection_indexes = transitionLUT[clipFlags];
        // first index that enters the negative zone
        u32 first_intersection_curr = 0xf & (intersection_indexes >> 4);
        // first index that enters the positive zone
        u32 second_intersection_curr = 0xf & intersection_indexes;
        u32 clipped_vertices_count = (second_intersection_curr < first_intersection_curr) ?
            (second_intersection_curr + input_poly_count - first_intersection_curr)
            : (second_intersection_curr - first_intersection_curr);
        u32 first_intersection_prev =
            first_intersection_curr == 0 ? input_poly_count - 1 : first_intersection_curr - 1;
        u32 second_intersection_prev =
            second_intersection_curr == 0 ? input_poly_count - 1 : second_intersection_curr - 1;
        if (intersection_indexes == 0) { // no intersections: keep all vertices
            output_poly_count = input_poly_count;
            continue;
        }
        else if (intersection_indexes == 0x88) { // all vertices are clipped: keep no vertices
            output_poly_count = 0;
            break;
        }

        // example: first intersection 1, second intersection 5, this is (1,1,0,0,5,5,4,4)
        __m256i permute_indexes_xy = _mm256_set_epi32(
            second_intersection_prev, second_intersection_prev,
            second_intersection_curr, second_intersection_curr,
            first_intersection_prev, first_intersection_prev,
            first_intersection_curr, first_intersection_curr);
        // in the example, this yields:
        // (x1,x1,x0,x0,x5,x5,x4,x4)
        // (y1,y1,y0,y0,y5,y5,y4,y4)
        // (x1,y1,x0,y0,x5,y5,x4,y4)
        __m256 permuted_x = _mm256_permutevar8x32_ps(vx_in, permute_indexes_xy);
        __m256 permuted_y = _mm256_permutevar8x32_ps(vy_in, permute_indexes_xy);
        __m256 permuted_xy = _mm256_blend_ps(permuted_x, permuted_y, 0b10101010);
        // in the example: (0,0,1,1,4,4,5,5)
        // (z0,z0,z1,z1,z4,z4,z5,z5)
        __m256i permute_indexes_z = _mm256_set_epi32(
            second_intersection_curr, second_intersection_curr,
            second_intersection_prev, second_intersection_prev,
            first_intersection_curr, first_intersection_curr,
            first_intersection_prev, first_intersection_prev);
        __m256 permuted_z = _mm256_permutevar8x32_ps(vz_in, permute_indexes_z);
        // in the example, this yields:
        // (d0,d0,d0,d0,d4,d4,d4,d4)
        __m256i permute_indexes_dot = _mm256_set_epi32(
            second_intersection_prev, second_intersection_prev,
            second_intersection_prev, second_intersection_prev,
            first_intersection_prev, first_intersection_prev,
            first_intersection_prev, first_intersection_prev);

        __m256 permuted_dot = _mm256_permutevar8x32_ps(vdoteps, permute_indexes_dot);
        __m256 permuted_zdot = _mm256_blend_ps(permuted_z, permuted_dot, 0b10101010);
        // in the example: (x1,y1,z1,d0,x5,y5,z5,d4)
        __m256 vcurr = _mm256_blend_ps(permuted_xy, permuted_zdot, 0b11001100);
        // in the example: (z0,d0,x0,y0,z4,d4,x4,y4)
        __m256 permuted_tmp = _mm256_blend_ps(permuted_xy, permuted_zdot, 0b00110011);
        __m256i permuted_indexes_prev = _mm256_set_epi32(5, 4, 7, 6, 1, 0, 3, 2);
        // in the example: (x0,y0,z0,d0,x4,y4,z4,d4)
        __m256 vprev = _mm256_permutevar8x32_ps(permuted_tmp, permuted_indexes_prev);
        __m256i permuted_indexes_dot = _mm256_set_epi32(7, 7, 7, 7, 3, 3, 3, 3);
        // in the example: (d0,d0,d0,d0,d4,d4,d4,d4)
        __m256 distance_distance = _mm256_permutevar8x32_ps(vprev, permuted_indexes_dot);

        // plane is xxxxxxxx, yyyyyyyy, zzzzzzzz, wwwwwwww
        // this yields (x,y,x,x,x,y,x,x)
        __m256 plane_xy = _mm256_blend_ps(plane.vx, plane.vy, 0b10100010);
        // this yields (z,z,z,w,z,z,z,w)
        __m256 plane_zw = _mm256_blend_ps(plane.vz, plane.vw, 0b10001000);
        __m256 plane_plane = _mm256_blend_ps(plane_xy, plane_zw, 0b11001100);

        // (curr-prev)0.xyzw (curr-prev)1.xyzw = ab0 ab1, w component cancels out
        __m256 ab_ab = _mm256_sub_ps(vcurr, vprev);
        // ab0*p.xyzw ab1*p.xyzw
        __m256 planeab_planeab = _mm256_mul_ps(plane_plane, ab_ab);
        // sum0_0 sum0_1 sum0_0 sum0_1 sum1_0 sum1_1 sum1_0 sum1_1
        __m256 dottmp = _mm256_hadd_ps(planeab_planeab, planeab_planeab);
        // dot0 dot0 dot0 dot0 dot1 dot1 dot1 dot1
        __m256 dot_dot = _mm256_hadd_ps(dottmp, dottmp);
        __m256 negt_negt = _mm256_div_ps(distance_distance, dot_dot);
        // two intersection points
        __m256 i_i = _mm256_fnmadd_ps(ab_ab, negt_negt, vprev);

        // Now that we have the intersection points, we need to reformat the output poly, so we
        // replace the values outside (o) and keep the ones inside (i), for example:
        //  i  i  i  o  o  o  i
        // p0 p1 p2 p3 p4 p5 p6
        // with the intersection points (x0 and x1), repeating the last vertex so the count stays 8
        // p0 p1 p2 x0 x1 p6
        // we'll change the ordering so they start at the intersection points
        // x0 x1 p6 p0 p1 p2
        u8 output_indexes[8] = {};
        output_indexes[output_poly_count++] = first_intersection_curr;
        output_indexes[output_poly_count++] = second_intersection_curr;
        u32 remaining = input_poly_count - clipped_vertices_count;
        for (u32 step = 0; step < remaining; step++) {
            u32 index = (step + second_intersection_curr) % input_poly_count;
            output_indexes[output_poly_count++] = index;
        }
        for (u32 step = output_poly_count; step < 8; step++) {
            output_indexes[step] = output_indexes[output_poly_count - 1];
        }

        // in the example, this yields:
        // (x1,x5,x5,x6,x7,x0,x0,x0)
        // (y1,y5,y5,y6,y7,y0,y0,y0)
        // (z1,z5,z5,z6,z7,z0,z0,z0)
        __m256i permute_output_xyz = _mm256_set_epi32(
            output_indexes[7], output_indexes[6], output_indexes[5], output_indexes[4],
            output_indexes[3], output_indexes[2], output_indexes[1], output_indexes[0]);
        __m256 vx_tmp = _mm256_permutevar8x32_ps(vx_in, permute_output_xyz);
        __m256 vy_tmp = _mm256_permutevar8x32_ps(vy_in, permute_output_xyz);
        __m256 vz_tmp = _mm256_permutevar8x32_ps(vz_in, permute_output_xyz);
        // blend with intersection points
        // in the example, this yields:
        // (i0x,i1x,x5,x6,x7,x0,x0,x0)
        // (i0y,i1y,y5,y6,y7,y0,y0,y0)
        // (i0z,i1z,z5,z6,z7,z0,z0,z0)
        __m256i permute_output_intersect_x = _mm256_set_epi32(0, 0, 0, 0, 0, 0, 4, 0);
        __m256 ix = _mm256_permutevar8x32_ps(i_i, permute_output_intersect_x);
        vx_in = _mm256_blend_ps(vx_tmp, ix, 0b00000011);
        __m256i permute_output_intersect_y = _mm256_set_epi32(0, 0, 0, 0, 0, 0, 5, 1);
        __m256 iy = _mm256_permutevar8x32_ps(i_i, permute_output_intersect_y);
        vy_in = _mm256_blend_ps(vy_tmp, iy, 0b00000011);
        __m256i permute_output_intersect_z = _mm256_set_epi32(0, 0, 0, 0, 0, 0, 6, 2);
        __m256 iz = _mm256_permutevar8x32_ps(i_i, permute_output_intersect_z);
        vz_in = _mm256_blend_ps(vz_tmp, iz, 0b00000011);
    }

    // output the poly in non-SIMD format
    for (u32 i = 0; i < output_poly_count; i++) {
        poly[i].x = ((const float*)&vx_in)[i];
        poly[i].y = ((const float*)&vy_in)[i];
        poly[i].z = ((const float*)&vz_in)[i];
    }
    poly_count = output_poly_count;
}
#endif

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

    #if !DISABLE_INTRINSICS
    m256_4 planes_256[renderer::Frustum::MAX_PLANE_COUNT];
    for (u32 p = 0; p < parent.frustum.numPlanes; p++) {
        planes_256[p].vx = _mm256_set1_ps(parent.frustum.planes[p].x);
        planes_256[p].vy = _mm256_set1_ps(parent.frustum.planes[p].y);
        planes_256[p].vz = _mm256_set1_ps(parent.frustum.planes[p].z);
        planes_256[p].vw = _mm256_set1_ps(parent.frustum.planes[p].w);
    }
    #endif

    bool* mirrorVisibility =
        (bool*)allocator::alloc_arena(
            ctx.scratchArenaRoot, sizeof(bool) * ctx.mirrors.count, alignof(bool));
    memset(mirrorVisibility, 0, ctx.mirrors.count * sizeof(bool));
    if (ctx.mirrors.bvh.nodeCount) {
        memset(mirrorVisibility, 0, ctx.mirrors.count * sizeof(bool));
        #if DISABLE_INTRINSICS
            bvh::findTrianglesIntersectingFrustum(
                ctx.scratchArenaRoot,
                mirrorVisibility, ctx.mirrors.bvh,
                parent.frustum.planes, parent.frustum.numPlanes);
        #else
            bvh::findTrianglesIntersectingFrustum(
                ctx.scratchArenaRoot,
                mirrorVisibility, ctx.mirrors.bvh,
                planes_256, parent.frustum.numPlanes);
        #endif
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

        #if DISABLE_INTRINSICS
            clip_poly_in_frustum(
                poly, poly_count, parent.frustum.planes, parent.frustum.numPlanes, countof(poly));
        #else
            clip_poly_in_frustum_256(
                poly, poly_count, planes_256, parent.frustum.numPlanes, countof(poly));
        #endif

        if (poly_count < 3) { continue; } // resulting mirror poly is fully culled

        // acknowledge this mirror as part of the tree
        CameraNode& curr = allocator::push(ctx.cameraTree, ctx.frameArena);
        curr.parentIndex = parentIndex;
        curr.depth = parent.depth + 1;
        curr.sourceId = i;
        // store mirror id only when using GPU markers
        __PROFILEONLY(platform::format(curr.str, sizeof(curr.str), "%s-%d", parent.str, i);)
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
        renderer::add_oblique_plane_to_persp(curr.projectionMatrix, planeES);
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
                math::add(math::scale(transpose.col3, -renderer::min_z), transpose.col2);   // near
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

struct RenderSceneContext {
    const CameraNode& camera;
    const renderer::VisibleNodes& visibleNodes;
    game::Scene& gameScene;
    renderer::CoreResources& core;
    renderer::driver::RscDepthStencilState& ds_always;
    renderer::driver::RscDepthStencilState& ds_opaque;
    renderer::driver::RscDepthStencilState& ds_alpha;
    renderer::driver::RscRasterizerState& rs;
    allocator::PagedArena scratchArena;
};
void renderBaseScene(RenderSceneContext& sceneCtx) {

    using namespace renderer;
    Scene& scene = sceneCtx.gameScene.renderScene;
    renderer::CoreResources& rsc = sceneCtx.core;
    driver::Marker_t marker;

    driver::RscCBuffer& scene_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
    SceneData cbufferPerScene;
    {
        cbufferPerScene.vpMatrix = 
            math::mult(sceneCtx.camera.projectionMatrix, sceneCtx.camera.viewMatrix);
        driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);
    }

    driver::set_marker_name(marker, "SKY"); driver::start_event(marker);
    {
        driver::RscCBuffer& clearColor_cbuffer =
            rsc.cbuffers[renderer::CoreResources::CBuffersMeta::ClearColor];
        renderer::driver::bind_blend_state(rsc.blendStateBlendOff);
        driver::bind_DS(sceneCtx.ds_always, sceneCtx.camera.depth);
        driver::bind_RS(rsc.rasterizerStateFillFrontfaces);
        driver::bind_cbuffers(
            rsc.shaders[renderer::ShaderTechniques::FullscreenBlitClearColor],
            &clearColor_cbuffer, 1);
        driver::bind_shader(rsc.shaders[renderer::ShaderTechniques::FullscreenBlitClearColor]);
        driver::draw_fullscreen();
    }
    driver::end_event();

    driver::bind_RS(sceneCtx.rs);
    {
        allocator::PagedArena scratchArena = sceneCtx.scratchArena;
        Drawlist dl = {};
        u32 maxDrawCalls =
              (sceneCtx.visibleNodes.visible_nodes_count
            + (u32)scene.instancedDrawNodes.count) * DrawlistStreams::Count;
        dl.items =
            (DrawCall_Item*)allocator::alloc_arena(
                scratchArena, maxDrawCalls * sizeof(DrawCall_Item), alignof(DrawCall_Item));
        dl.keys =
            (SortKey*)allocator::alloc_arena(
                scratchArena, maxDrawCalls * sizeof(SortKey), alignof(SortKey));
        addNodesToDrawlistSorted(
            dl, sceneCtx.visibleNodes, sceneCtx.camera.pos, scene, rsc,
            0, renderer::DrawlistFilter::Alpha, renderer::SortParams::Type::Default);
        if (dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced] > 0) {
            driver::set_marker_name(marker, "OPAQUE"); driver::start_event(marker);
            driver::bind_DS(sceneCtx.ds_opaque, sceneCtx.camera.depth);
            Drawlist_Context ctx = {};
            Drawlist_Overrides overrides = {};
            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
            draw_drawlist(dl, ctx, overrides);
            driver::end_event();
        }
    }
    #if __DEBUG
    {
        im::present3d(sceneCtx.camera.projectionMatrix, sceneCtx.camera.viewMatrix);
    }
    #endif
    {
        allocator::PagedArena scratchArena = sceneCtx.scratchArena;
        Drawlist dl = {};
        u32 maxDrawCalls =
              (sceneCtx.visibleNodes.visible_nodes_count
            + (u32)scene.instancedDrawNodes.count) * DrawlistStreams::Count;
        dl.items =
            (DrawCall_Item*)allocator::alloc_arena(
                scratchArena, maxDrawCalls * sizeof(DrawCall_Item), alignof(DrawCall_Item));
        dl.keys =
            (SortKey*)allocator::alloc_arena(
                scratchArena, maxDrawCalls * sizeof(SortKey), alignof(SortKey));
        driver::bind_blend_state(rsc.blendStateOn);
        addNodesToDrawlistSorted(
            dl, sceneCtx.visibleNodes, sceneCtx.camera.pos, scene, rsc,
            renderer::DrawlistFilter::Alpha, 0, renderer::SortParams::Type::BackToFront);
        if (dl.count[DrawlistBuckets::Base] + dl.count[DrawlistBuckets::Instanced] > 0) {
            driver::bind_DS(sceneCtx.ds_alpha, sceneCtx.camera.depth);
            driver::set_marker_name(marker, "ALPHA"); driver::start_event(marker);
            Drawlist_Context ctx = {};
            Drawlist_Overrides overrides = {};
            overrides.forced_blendState = true;
            ctx.blendState = &rsc.blendStateOn;
            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
            draw_drawlist(dl, ctx, overrides);
            driver::end_event();
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

    driver::RscRasterizerState& rasterizerStateParent =
        (mirrorCtx.camera.depth & 1) != 0 ?
              rsc.rasterizerStateFillFrontfaces
            : rsc.rasterizerStateFillBackfaces;

    driver::RscCBuffer& scene_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
    driver::RscCBuffer& identity_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::NodeIdentity];
    driver::Marker_t marker;

    // mark this mirror on the stencil (using the parent's camera)
    driver::set_marker_name(marker, "MARK MIRROR"); driver::start_event(marker);
    {
        driver::bind_DS(rsc.depthStateMarkMirror, mirrorCtx.parent.depth);
        driver::bind_RS(rasterizerStateParent);

        driver::bind_blend_state(rsc.blendStateOff);

        const renderer::DrawMesh& mesh =
            mirrorCtx.camera.drawMesh;
        driver::bind_shader(rsc.shaders[mesh.shaderTechnique]);
        driver::bind_indexed_vertex_buffer(mesh.vertexBuffer);
        driver::RscCBuffer buffers[] = { scene_cbuffer, identity_cbuffer };
        driver::bind_cbuffers(rsc.shaders[mesh.shaderTechnique], buffers, 2);
        driver::draw_indexed_vertex_buffer(mesh.vertexBuffer);
    }
    driver::end_event();
}
void unmarkMirror(RenderMirrorContext& mirrorCtx) {

    using namespace renderer;
    renderer::CoreResources& rsc = mirrorCtx.renderCore;
    driver::RscRasterizerState& rasterizerStateParent =
        (mirrorCtx.camera.depth & 1) != 0 ?
              rsc.rasterizerStateFillFrontfaces
            : rsc.rasterizerStateFillBackfaces;

    driver::RscCBuffer& scene_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
    driver::RscCBuffer& identity_cbuffer =
        rsc.cbuffers[renderer::CoreResources::CBuffersMeta::NodeIdentity];

    {
        renderer::SceneData cbufferPerScene;
        cbufferPerScene.vpMatrix =
            math::mult(mirrorCtx.parent.projectionMatrix, mirrorCtx.parent.viewMatrix);
        renderer::driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);
    }

    driver::Marker_t marker;
    driver::set_marker_name(marker, "UNMARK MIRROR"); driver::start_event(marker);
    {
        driver::bind_DS(rsc.depthStateUnmarkMirror, mirrorCtx.camera.depth);
        driver::bind_RS(rasterizerStateParent);
        driver::bind_blend_state(rsc.blendStateOn);

        renderer::DrawMesh mesh = //renderer::drawMesh_from_handle(rsc, mirrorCtx.camera.meshHandle);
            mirrorCtx.camera.drawMesh;
        driver::bind_shader(rsc.shaders[mesh.shaderTechnique]);
        driver::bind_indexed_vertex_buffer(mesh.vertexBuffer);
        driver::RscCBuffer buffers[] = { scene_cbuffer, identity_cbuffer };
        driver::bind_cbuffers(rsc.shaders[ShaderTechniques::Color3D], buffers, 2);
        driver::draw_indexed_vertex_buffer(mesh.vertexBuffer);
    }
    driver::end_event();
}

void renderMirrorTree(
        const CameraNode* cameraTree,
        const renderer::VisibleNodes* visibleNodes,
        game::Scene& gameScene,
        renderer::CoreResources& renderCore,
        allocator::PagedArena scratchArena) {

    using namespace renderer;
    u32 numCameras = cameraTree[0].siblingIndex;
    u32* parents =
        (u32*) allocator::alloc_arena(
                scratchArena, sizeof(u32) * numCameras,
                alignof(u32));
    u32 parentCount = 0;
    parents[parentCount++] = 0;
    for (u32 index = 1; index < numCameras; index++) {

        const CameraNode& camera = cameraTree[index];
        const CameraNode& parent = cameraTree[parents[parentCount - 1]];

        // render this mirror
        driver::Marker_t marker;
        __PROFILEONLY(driver::set_marker_name(marker, camera.str); driver::start_event(marker);)
        

        // mark mirror
        RenderMirrorContext mirrorContext {
            camera, parent, gameScene, renderCore
        };
        markMirror(mirrorContext);

        // render base scene
        driver::set_marker_name(marker, "REFLECTION SCENE"); driver::start_event(marker);
        {
            driver::RscRasterizerState& rasterizerStateMirror =
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
        __PROFILEONLY(driver::end_event();)

        parents[parentCount++] = index;
        while (parentCount > 1 && index + 1 >= cameraTree[parents[parentCount - 1]].siblingIndex) {
            // unmark mirror
            const CameraNode& camera = cameraTree[parents[parentCount - 1]];
            const CameraNode& parent = cameraTree[parents[parentCount - 2]];
            RenderMirrorContext mirrorContext {
                camera, parent, gameScene, renderCore
            };
            unmarkMirror(mirrorContext);
            driver::end_event();
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
          { "assets/meshes/boar.fbx", game::Resources::AssetsMeta::Boar }
        , { "assets/meshes/bird.fbx", game::Resources::AssetsMeta::Bird }
    };
    // very very hack: number of assets * 4 + 16 (for the meshes we load ourselves)
    const size_t meshArenaSize = (countof(assets) * 4 + 16) * sizeof(renderer::DrawMesh);
    renderCore.meshes = (renderer::DrawMesh*)allocator::alloc_arena(
            persistentArena, meshArenaSize, alignof(renderer::DrawMesh));
    renderCore.num_meshes = 0;

    camera::WindowProjection::Config& ortho =
        renderCore.windowProjection.config;
    ortho.right = screen.window_width * 0.5f;
    ortho.top = screen.window_height * 0.5f;
    ortho.left = -ortho.right;
    ortho.bottom = -ortho.top;
    ortho.near = 0.f;
    ortho.far = 1000.f;
    renderer::generate_matrix_ortho(renderCore.windowProjection.matrix, ortho);

    // Todo: consider whether precision needs special handling
    camera::PerspProjection::Config& frustum =
        renderCore.perspProjection.config;
    frustum.fov = 45.f;
    frustum.aspect = screen.width / (f32)screen.height;
    frustum.near = 1.f;
    frustum.far = 1000.f;
    renderer::generate_matrix_persp(
        renderCore.perspProjection.matrix, frustum);

    renderer::driver::MainRenderTargetParams windowRTparams = {};
    windowRTparams.depth = true;
    windowRTparams.width = screen.window_width;
    windowRTparams.height = screen.window_height;
    renderer::driver::create_main_RT(
        renderCore.windowRT, windowRTparams);

    renderer::driver::RenderTargetParams gameRTparams;
    gameRTparams.depth = true;
    gameRTparams.width = screen.width;
    gameRTparams.height = screen.height;
    gameRTparams.textureFormat = renderer::driver::TextureFormat::V4_8;
    gameRTparams.textureInternalFormat =
        renderer::driver::InternalTextureFormat::V4_8;
    gameRTparams.textureFormatType =
        renderer::driver::Type::Float;
    gameRTparams.count = 1;
    renderer::driver::create_RT(renderCore.gameRT, gameRTparams);

    {
        renderer::driver::BlendStateParams blendState_params;
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            renderer::driver::RenderTargetWriteMask::All; blendState_params.blendEnable = true;
        renderer::driver::create_blend_state(renderCore.blendStateOn, blendState_params);
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            renderer::driver::RenderTargetWriteMask::All;
        renderer::driver::create_blend_state(renderCore.blendStateBlendOff, blendState_params);
        blendState_params = {};
        blendState_params.renderTargetWriteMask =
            renderer::driver::RenderTargetWriteMask::None;
        renderer::driver::create_blend_state(renderCore.blendStateOff, blendState_params);
    }
    renderer::driver::create_RS(renderCore.rasterizerStateFillFrontfaces,
            { renderer::driver::RasterizerFillMode::Fill,
              renderer::driver::RasterizerCullMode::CullBack, false });
    renderer::driver::create_RS(renderCore.rasterizerStateFillBackfaces,
            { renderer::driver::RasterizerFillMode::Fill,
              renderer::driver::RasterizerCullMode::CullFront, false });
    renderer::driver::create_RS(renderCore.rasterizerStateFillCullNone,
            { renderer::driver::RasterizerFillMode::Fill,
              renderer::driver::RasterizerCullMode::CullNone, false });
    renderer::driver::create_RS(renderCore.rasterizerStateLine,
            { renderer::driver::RasterizerFillMode::Line,
              renderer::driver::RasterizerCullMode::CullNone, false });
    {
        // BASE depth states
        renderer::driver::DepthStencilStateParams dsParams;
        dsParams = {};
        dsParams.depth_enable = true;
        dsParams.depth_func = renderer::driver::CompFunc::Less;
        dsParams.depth_writemask = renderer::driver::DepthWriteMask::All;
        renderer::driver::create_DS(renderCore.depthStateOn, dsParams);
        dsParams = {};
        dsParams.depth_enable = true;
        dsParams.depth_func = renderer::driver::CompFunc::Less;
        dsParams.depth_writemask = renderer::driver::DepthWriteMask::Zero;
        renderer::driver::create_DS(renderCore.depthStateReadOnly, dsParams);
        dsParams = {};
        dsParams.depth_enable = false;
        renderer::driver::create_DS(renderCore.depthStateOff, dsParams);
        dsParams = {};
        dsParams.depth_enable = true;
        dsParams.depth_func = renderer::driver::CompFunc::Always;
        dsParams.depth_writemask = renderer::driver::DepthWriteMask::All;
        renderer::driver::create_DS(renderCore.depthStateAlways, dsParams);
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
            renderer::driver::create_DS(renderCore.depthStateMarkMirror, dsParams);
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
            renderer::driver::create_DS(renderCore.depthStateUnmarkMirror, dsParams);
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
            renderer::driver::create_DS(renderCore.depthStateMirrorReflections, dsParams);
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
            renderer::driver::create_DS(renderCore.depthStateMirrorReflectionsDepthReadOnly, dsParams);
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
            renderer::driver::create_DS(renderCore.depthStateMirrorReflectionsDepthAlways, dsParams);
        }
    }

    // known cbuffers
    renderer::driver::RscCBuffer& cbufferclear =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::ClearColor];
    renderer::driver::create_cbuffer(cbufferclear, { sizeof(renderer::BlitColor) });
    renderer::BlitColor clearColor = { float4(0.2f, 0.344f, 0.59f, 1.f) };
    renderer::driver::update_cbuffer(cbufferclear, &clearColor);
    renderer::driver::RscCBuffer& cbufferscene =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
    renderer::driver::create_cbuffer(cbufferscene, { sizeof(renderer::SceneData) });
    renderer::driver::RscCBuffer& cbufferNodeIdentity =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::NodeIdentity];
    renderer::driver::create_cbuffer(cbufferNodeIdentity, { sizeof(renderer::NodeData) });
    renderer::NodeData nodeIdentity = {};
    nodeIdentity.groupColor = float4(1.f, 1.f, 1.f, 1.f);
    math::identity4x4(*(Transform*)&nodeIdentity.worldMatrix);
    renderer::driver::update_cbuffer(cbufferNodeIdentity, &nodeIdentity);
    renderer::driver::RscCBuffer& cbufferNodeUItext =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::UIText];
    renderer::driver::create_cbuffer(cbufferNodeUItext, { sizeof(renderer::NodeData) });
    renderer::NodeData noteUItext = {};
    noteUItext.groupColor = float4(1.f, 1.f, 1.f, 1.f);
    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
    renderer::driver::update_cbuffer(cbufferNodeUItext, &noteUItext);
    renderer::driver::RscCBuffer& cbufferinstances64 =
        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::Instances64];
    renderer::driver::create_cbuffer(cbufferinstances64, { u32(sizeof(float4x4)) * 64 });

    // input layouts
    const renderer::driver::VertexAttribDesc attribs_2d[] = {
        renderer::driver::make_vertexAttribDesc(
            "POSITION", offsetof(renderer::VertexLayout_Color_2D, pos),
            sizeof(renderer::VertexLayout_Color_2D),
            renderer::driver::BufferAttributeFormat::R32G32_FLOAT),
        renderer::driver::make_vertexAttribDesc(
            "COLOR", offsetof(renderer::VertexLayout_Color_2D, color),
            sizeof(renderer::VertexLayout_Color_2D),
            renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM) };
    const renderer::driver::VertexAttribDesc attribs_3d[] = {
        renderer::driver::make_vertexAttribDesc(
            "POSITION", 0,
            sizeof(float3),
            renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT) };
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
            renderer::driver::BufferAttributeFormat::R8G8B8A8_UNORM) };
    const renderer::driver::VertexAttribDesc attribs_textured3d[] = {
        renderer::driver::make_vertexAttribDesc(
			"POSITION", offsetof(renderer::VertexLayout_Textured_3D, pos),
            sizeof(renderer::VertexLayout_Textured_3D),
            renderer::driver::BufferAttributeFormat::R32G32B32_FLOAT),
        renderer::driver::make_vertexAttribDesc(
			"TEXCOORD", offsetof(renderer::VertexLayout_Textured_3D, uv),
            sizeof(renderer::VertexLayout_Textured_3D),
            renderer::driver::BufferAttributeFormat::R32G32_FLOAT) };
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
    const renderer::driver::CBufferBindingDesc bufferBindings_blit_clear_color[] = {
        { "type_BlitColor", renderer::driver::CBufferStageMask::PS }
    };
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
        allocator::PagedArena scratchArena = memory.scratchArena; // explicit copy
        // Initialize cache with room for a VS and PS shader of each technique
        renderer::driver::ShaderCache shader_cache = {};
        renderer::driver::load_shader_cache(
            shader_cache, "assets/data/shaderCache_mirrors.bin", &scratchArena,
            renderer::ShaderTechniques::Count * 2);
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_3d;
            desc.vertexAttr_count = countof(attribs_3d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_blit_clear_color;
            desc.bufferBinding_count = countof(bufferBindings_blit_clear_color);
            // reuse 3d shaders
            desc.vs_name = renderer::shaders::vs_fullscreen_bufferless_clear_blit.name;
            desc.vs_src = renderer::shaders::vs_fullscreen_bufferless_clear_blit.src;
            desc.ps_name = renderer::shaders::ps_fullscreen_blit_clear_colored.name;
            desc.ps_src = renderer::shaders::ps_fullscreen_blit_clear_colored.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::FullscreenBlitClearColor], desc);
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
                renderCore.shaders[renderer::ShaderTechniques::FullscreenBlitTextured], desc);
        }
        {
            renderer::ShaderDesc desc = {};
            desc.vertexAttrs = attribs_2d;
            desc.vertexAttr_count = countof(attribs_2d);
            desc.textureBindings = nullptr;
            desc.textureBinding_count = 0;
            desc.bufferBindings = bufferBindings_untextured_base;
            desc.bufferBinding_count = countof(bufferBindings_untextured_base);
            desc.vs_name = renderer::shaders::vs_color2d_base.name;
            desc.vs_src = renderer::shaders::vs_color2d_base.src;
            desc.ps_name = renderer::shaders::ps_color3d_unlit.name;
            desc.ps_src = renderer::shaders::ps_color3d_unlit.src;
            desc.shader_cache = &shader_cache;
            renderer::compile_shader(
                renderCore.shaders[renderer::ShaderTechniques::Color2D], desc);
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
                renderCore.shaders[renderer::ShaderTechniques::Instanced3D], desc);
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
            renderer::compile_shader(renderCore.shaders[renderer::ShaderTechniques::Color3D], desc);
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
                renderCore.shaders[renderer::ShaderTechniques::Color3DSkinned], desc);
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
                renderCore.shaders[renderer::ShaderTechniques::Textured3D], desc);
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
                renderCore.shaders[renderer::ShaderTechniques::Textured3DAlphaClip], desc);
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
                renderCore.shaders[renderer::ShaderTechniques::Textured3DSkinned], desc);
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
                renderCore.shaders[renderer::ShaderTechniques::Textured3DAlphaClipSkinned], desc);
        }
        renderer::driver::write_shader_cache(shader_cache);
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
        renderer::UntexturedCube cube;
        renderer::create_cube_coords(
            (uintptr_t)cube.vertices, sizeof(cube.vertices[0]),
            cube.indices, float3(1.f, 1.f, 1.f), float3(0.f, 0.f, 0.f));
        renderer::create_indexed_vertex_buffer_from_untextured_mesh(
            mesh.vertexBuffer, cube.vertices, countof(cube.vertices),
            cube.indices, countof(cube.indices));
        core.instancedUnitCubeMesh = renderer::handle_from_drawMesh(renderCore, mesh);
    }

    // instanced spheres
    {
        renderer::DrawMesh& mesh = renderer::alloc_drawMesh(renderCore);
        mesh = {};
        mesh.shaderTechnique = renderer::ShaderTechniques::Instanced3D;
        renderer::UntexturedSphere sphere;
        renderer::create_sphere_coords(
            (uintptr_t)sphere.vertices, sizeof(sphere.vertices[0]),
            sphere.indices, 1.f);
        renderer::create_indexed_vertex_buffer_from_untextured_mesh(
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

        game::AssetInMemory& ground = core.assets[game::Resources::AssetsMeta::Ground];
        ground = {};
        ground.min = float3(-w, -h, 0.f);
        ground.max = float3(w, h, 0.f);
        ground.meshHandles[0] = renderer::handle_from_drawMesh(renderCore, mesh);
        ground.skeleton.jointCount = 0;
    }

    // Mirror models
    obj::load_mesh_cpu_gpu(
        core.meshes[game::Resources::MeshesMeta::ToiletLo], renderCore,
        scratchArena, persistentArena, "assets/meshes/throne.obj", 1.f);
    obj::load_mesh_cpu_gpu(
        core.meshes[game::Resources::MeshesMeta::ToiletHi], renderCore,
        scratchArena, persistentArena, "assets/meshes/mid-smooth.obj", 1.5f);

    // Hall of mirrors
    {
        const u32 mirrorCount = 8;
        renderer::VertexLayout_Color_3D vertices[4 * mirrorCount];
        u16 indices[4 * mirrorCount * 3 / 2]; // 6 indices per quad
        const float3 v0(1.f, 0.f, -1.f);
        const float3 v1(-1.f, 0.f, -1.f);
        const float3 v2(-1.f, 0.f, 1.f);
        const float3 v3(1.f, 0.f, 1.f);
        for (u32 m = 0; m < mirrorCount; m++) {
            f32 w = 12.4f, h = 10.f;
            u32 c = Color32(0.01f, 0.19f, 0.3f, 0.32f).ABGR();
            u32 vertex_idx = m * 4;
            Transform t;
            Transform rot = math::fromOffsetAndOrbit(
                float3(0.f, 0.f, 0.f), float3(0.f, 0.f, f32(m) * math::twopi32 / f32(8)));
            Transform translate = {};
            math::identity4x4(translate);
            translate.pos = float3(0.f, -30.f, 0.f);
            t.matrix = math::mult(rot.matrix, translate.matrix);
            t.matrix.col0.xyz = math::scale(t.matrix.col0.xyz, w);
            t.matrix.col2.xyz = math::scale(t.matrix.col2.xyz, h);
            vertices[vertex_idx + 0] = { math::mult(t.matrix, float4(v0, 1.f)).xyz, c };
            vertices[vertex_idx + 1] = { math::mult(t.matrix, float4(v1, 1.f)).xyz, c };
            vertices[vertex_idx + 2] = { math::mult(t.matrix, float4(v2, 1.f)).xyz, c };
            vertices[vertex_idx + 3] = { math::mult(t.matrix, float4(v3, 1.f)).xyz, c };

            // clock-wise indices
            u32 index_idx = vertex_idx * 3 / 2;
            // todo: start of second tri needs to be the last vertex, fix this weird restriction
            indices[index_idx + 0] = vertex_idx + 2;
            indices[index_idx + 1] = vertex_idx + 1;
            indices[index_idx + 2] = vertex_idx + 0;
            indices[index_idx + 3] = vertex_idx + 3;
            indices[index_idx + 4] = vertex_idx + 2;
            indices[index_idx + 5] = vertex_idx + 0;
        }
        game::GPUCPUMesh& meshToLoad = core.meshes[game::Resources::MeshesMeta::HallOfMirrors];
        renderer::driver::IndexedVertexBufferDesc bufferParams;
        bufferParams.vertexData = vertices;
        bufferParams.indexData = indices;
        bufferParams.vertexSize = sizeof(vertices);
        bufferParams.vertexCount = countof(vertices);
        bufferParams.indexSize = sizeof(indices);
        bufferParams.indexCount = countof(indices);
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
            meshToLoad.gpuBuffer, bufferParams, attribs, countof(attribs));

        renderer::CPUMesh& cpuBuffer = meshToLoad.cpuBuffer;
        cpuBuffer.vertices =
            (float3*)allocator::alloc_arena(
                persistentArena, sizeof(float3) * countof(vertices), alignof(float3));
        cpuBuffer.indices =
            (u16*)allocator::alloc_arena(
                persistentArena, sizeof(u16) * countof(indices), alignof(u16));
        for (u32 i = 0; i < countof(vertices); i++) { cpuBuffer.vertices[i] = vertices[i].pos; }
        memcpy(cpuBuffer.indices, indices, sizeof(u16) * countof(indices));
        cpuBuffer.indexCount = countof(indices);
        cpuBuffer.vertexCount = countof(vertices);
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

            char text[256];
            platform::strncpy(text, str, 256);

            const u32 vertexCount = buffer.vertexCount;
            const u32 indexCount = 3 * vertexCount / 2; // every 4 vertices form a 6 index quad

            unsigned char color[4];
            color[0] = params.color.getRu();
            color[1] = params.color.getGu();
            color[2] = params.color.getBu();
            color[3] = params.color.getAu();
            u8* vertexBuffer = (u8*)&buffer.vertices[vertexCount];
            s32 vertexBufferSize =
                (buffer.vertexCap - vertexCount) * sizeof(renderer::VertexLayout_Color_2D);
            // negate y, since our (0,0) is the center of the screen, stb's is bottom left
            u32 quadCount = stb_easy_font_print(
                params.pos.x, -params.pos.y, params.scale, text,
                color, vertexBuffer, vertexBufferSize);
            buffer.vertexCount += quadCount * 4;

            // stb uses ccw winding, but we are negating the y, so it matches our cw winding
            for (u32 i = 0; i < quadCount; i++) {
                const u32 vertexIndex = vertexCount + i * 4;
                const u32 indexIndex = indexCount + i * 6;
                buffer.indices[indexIndex] = vertexIndex + 1;
                buffer.indices[indexIndex + 1] = vertexIndex + 2;
                buffer.indices[indexIndex + 2] = vertexIndex + 3;

                buffer.indices[indexIndex + 3] = vertexIndex + 3;
                buffer.indices[indexIndex + 4] = vertexIndex + 0;
                buffer.indices[indexIndex + 5] = vertexIndex + 1;
            }
        };

        auto commit2d = [](
            renderer::driver::RscIndexedVertexBuffer& gpuBuffer, Buffer2D& buffer,
            const renderer::driver::VertexAttribDesc* attribs, const u32 attribs_count){
            const u32 vertexCount = buffer.vertexCount;
            const u32 indexCount = 3 * vertexCount / 2; // every 4 vertices form a 6 index quad
            renderer::driver::IndexedVertexBufferDesc bufferParams;
            bufferParams.vertexData = buffer.vertices;
            bufferParams.indexData = buffer.indices;
            bufferParams.vertexSize = vertexCount * sizeof(renderer::VertexLayout_Color_2D);
            bufferParams.vertexCount = vertexCount;
            bufferParams.indexSize = indexCount * sizeof(u32);
            bufferParams.indexCount = indexCount;
            bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
            bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
            bufferParams.indexType = renderer::driver::BufferItemType::U32;
            bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
            renderer::driver::create_indexed_vertex_buffer(
                gpuBuffer, bufferParams, attribs, attribs_count);
            buffer.vertexCount = 0;
        };

        enum { MAX_VERTICES2D_COUNT = 1024 };
        Buffer2D buffer2D = {};
        buffer2D.vertices = (renderer::VertexLayout_Color_2D*)allocator::alloc_arena(
            scratchArena, MAX_VERTICES2D_COUNT * sizeof(renderer::VertexLayout_Color_2D),
            alignof(renderer::VertexLayout_Color_2D));
        buffer2D.vertexCap = MAX_VERTICES2D_COUNT;
        buffer2D.indices = (u32*)allocator::alloc_arena(
            scratchArena, (MAX_VERTICES2D_COUNT * 3 / 2) * sizeof(u32),
            alignof(u32));

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
        text2d(buffer2D, textParams, "Press N to cycle through the three different rooms");
        textParams.pos.y += lineheight;
        commit2d(core.gpuBufferCycleRoomText, buffer2D, attribs_2d, countof(attribs_2d));
        text2d(buffer2D, textParams, "Please enjoy your mirrors");
        textParams.pos.y += lineheight;
        text2d(buffer2D, textParams, "Welcome to the mirror room");
        textParams.pos.y += lineheight;
        commit2d(core.gpuBufferHeaderText, buffer2D, attribs_2d, countof(attribs_2d));
    }

    // debug renderer
    __DEBUGDEF(renderer::im::init(memory.debugArena);)
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
          { game::Resources::AssetsMeta::Boar, { -5.f, 10.f, 2.30885f }, 1, false, true }
        , { game::Resources::AssetsMeta::Bird, { 9.f, 6.f, 2.23879f }, 1, true, true }
        , { game::Resources::AssetsMeta::Bird, { -8.f, 12.f, 2.23879f }, 1, false, true }
        , { game::Resources::AssetsMeta::Ground, { 0.f, 0.f, 0.f }, 1, false, false }
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
        for (u32 i = 0; i < physicsScene.ball_count; i++) {
            physics::DynamicObject_Sphere& ball = physicsScene.balls[i];
            ball.radius = (math::rand() + 1.f) * 2.f;
            ball.mass = math::pi32 * ball.radius * ball.radius;
            ball.pos =
                float3(
                    w * (-1.f + 2.f * math::rand()),
                    h * (-1.f + 2.f * math::rand()),
                    ball.radius * 0.5f);
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
        renderer::driver::RscCBuffer& cbuffercore = allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_node = handle_from_cbuffer(renderScene, cbuffercore);
        renderer::driver::create_cbuffer(cbuffercore, { sizeof(renderer::NodeData) });
        renderer::driver::RscCBuffer& cbufferinstances =
            allocator::alloc_pool(renderScene.cbuffers);
        node.cbuffer_instances =
            handle_from_cbuffer(renderScene, cbufferinstances);
        renderer::driver::create_cbuffer(
            cbufferinstances, { u32(sizeof(float4x4)) * physicsScene.ball_count });
        for (u32 m = 0; m < countof(node.instanceMatrices.data); m++) {
            float4x4& matrix = node.instanceMatrices.data[m];
            math::identity4x4(*(Transform*)&(matrix));
        }
        scene.instancedNodesHandles[game::Scene::InstancedTypes::PhysicsBalls] =
            handle_from_instanced_node(renderScene, node);
    }

    // physics ground
    {
        float width = 12.4f;
        float radius = 30.f;
        float2 ground[] = {
            float2(width, radius),
            float2(-width, radius),
            float2(-radius, width),
            float2(-radius, -width),
            float2(-width, -radius),
            float2(width, -radius),
            float2(radius, -width),
            float2(radius, width)
        };

        u32 prev = countof(ground) - 1;
        for (u32 i = 0; i < countof(ground); i++) {
            physics::StaticObject_Line& wall = physicsScene.walls[physicsScene.wall_count++];
            wall = {};
            wall.start = float3(ground[prev], 0.f);
            wall.end = float3(ground[i], 0.f);

            prev = i;
        }
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

            if (assetDef.physicsObstacle) {
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

    // unit cubes
    {
        renderer::DrawNodeInstanced& node = allocator::alloc_pool(renderScene.instancedDrawNodes);
        node = {};
		node.meshHandles[0] = core.instancedUnitCubeMesh;
        math::identity4x4(*(Transform*)&(node.nodeData.worldMatrix));
        node.nodeData.groupColor = Color32(0.68f, 0.69f, 0.71f, 1.f).RGBAv4();
        node.instanceCount = 4;
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
        scene.instancedNodesHandles[game::Scene::InstancedTypes::PlayerTrail] =
            handle_from_instanced_node(renderScene, node);
    }

    // mirrors
    {
        const u32 numMirrors = 
            (roomDef.mirrorMesh == game::Resources::MeshesMeta::HallOfMirrors) ?
            game::Resources::MirrorHallMeta::Count * 2 : 1024 * 1024;
        scene.mirrors.polys = (game::Mirrors::Poly*)
            allocator::alloc_arena(
                sceneArena,
                sizeof(game::Mirrors::Poly) * numMirrors,
                alignof(game::Mirrors::Poly));
        scene.mirrors.drawMeshes = (renderer::DrawMesh*)
            allocator::alloc_arena(
                sceneArena,
                sizeof(renderer::DrawMesh) * numMirrors,
                alignof(renderer::DrawMesh));
        scene.mirrors.bvh = {};
        game::spawn_model_as_mirrors(
            scene.mirrors, core.meshes[roomDef.mirrorMesh], scratchArena, sceneArena);

        // Manually define the bounding boxes for these obstacles, since their bounding box
        // will otherwise account for the upper water tank
        if (roomDef.mirrorMesh == game::Resources::MeshesMeta::ToiletLo) {
            physics::StaticObject_Sphere& o =
                physicsScene.obstacles[physicsScene.obstacle_count++];
            o.pos = float3(0.f, -1.f, 0.f);
            o.radius = 2.5f;
        } else if (roomDef.mirrorMesh == game::Resources::MeshesMeta::ToiletHi) {
            physics::StaticObject_Sphere& o =
                physicsScene.obstacles[physicsScene.obstacle_count++];
            o.pos = float3(0.f, 0.f, 0.f);
            o.radius = 9.f;
        }
    }
    scene.maxMirrorBounces = roomDef.maxMirrorBounces;

    // camera
    scene.orbitCamera.offset = float3(0.f, -100.f, 0.f);
    scene.orbitCamera.eulers = float3(-25.f * math::d2r32, 0.f, 135.f * math::d2r32);
    scene.orbitCamera.minEulers = roomDef.minCameraEulers;
    scene.orbitCamera.maxEulers = roomDef.maxCameraEulers;
    scene.orbitCamera.scale = 1.f;
    scene.orbitCamera.origin = float3(0.f, 0.f, 0.f);
    scene.orbitCamera.minScale = roomDef.minCameraZoom;
    scene.orbitCamera.maxScale = roomDef.maxCameraZoom;
}

#endif // __WASTELADNS_SCENE_H__
