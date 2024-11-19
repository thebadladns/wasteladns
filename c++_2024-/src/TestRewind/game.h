#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

#include "gameplay.h"
#include "shaders.h"
#include "game_renderer.h"
;
const u32 numCubes = 4;

#if __DEBUG
namespace debug {
struct OverlayMode { enum Enum { All, HelpOnly, ArenaOnly, None, Count }; };
struct Debug3DView { enum Enum { Culling, All, None, Count }; };
struct DebugCameraStage { enum Enum { Scene, SceneMirror, SceneMirrorClipped, Count }; };
struct EventText {
    char text[256];
    f64 time;
};

f64 frameHistory[60];
f64 frameAvg = 0;
u64 frameHistoryIdx = 0;
OverlayMode::Enum overlaymode = OverlayMode::Enum::All;
Debug3DView::Enum debug3Dmode = Debug3DView::Enum::None;
DebugCameraStage::Enum debugCameraStage = DebugCameraStage::Enum::Scene;
float4x4 capturedCameras[DebugCameraStage::Count] = {};
EventText eventLabel = {};
}
#endif

namespace game
{
    struct Time {
        struct Config {
            f64 targetFramerate;
            f64 maxFrameLength;
        };

        Config config;
        f64 lastFrame;
        f64 lastFrameDelta;
        f64 nextFrame;
        f64 updateOverspeed;
        s64 frameCount;
        bool paused;
    };

    namespace input
    {
        constexpr ::input::keyboard::Keys::Enum
              UP = ::input::keyboard::Keys::W
            , DOWN = ::input::keyboard::Keys::S
            , LEFT = ::input::keyboard::Keys::A
            , RIGHT = ::input::keyboard::Keys::D
            ;
        constexpr ::input::keyboard::Keys::Enum
              EXIT = ::input::keyboard::Keys::ESCAPE
            , TOGGLE_OVERLAY = ::input::keyboard::Keys::H
            , TOGGLE_DEBUG3D = ::input::keyboard::Keys::V
            , CAPTURE_CAMERAS = ::input::keyboard::Keys::P
            , TOGGLE_CAPTURED_CAMERA = ::input::keyboard::Keys::C
            , TOGGLE_FRUSTUM_CUT = ::input::keyboard::Keys::NUM0
            , TOGGLE_FRUSTUM_NEAR = ::input::keyboard::Keys::NUM1
            , TOGGLE_FRUSTUM_FAR = ::input::keyboard::Keys::NUM2
            , TOGGLE_FRUSTUM_LEFT = ::input::keyboard::Keys::NUM3
            , TOGGLE_FRUSTUM_RIGHT = ::input::keyboard::Keys::NUM4
            , TOGGLE_FRUSTUM_BOTTOM = ::input::keyboard::Keys::NUM5
            , TOGGLE_FRUSTUM_TOP = ::input::keyboard::Keys::NUM6
            ;
    };

    struct CameraManager {
        enum { FlyCam, CamCount };
        Camera cameras[CamCount];
        gameplay::orbit::State orbitCamera;
        Camera* activeCam;
    };

    struct RenderManager {
        renderer::Store store;
        __DEBUGDEF(renderer::im::Context imCtx;)
        renderer::WindowProjection windowProjection;
        renderer::PerspProjection perspProjection;
    };

    struct Memory {
        allocator::Arena persistentArena;
        __DEBUGDEF(allocator::Arena imDebugArena;)
        allocator::Arena frameArena;
        u8* frameArenaBuffer; // used to reset allocator::frameArena every frame
        __DEBUGDEF(u8* persistentArenaBuffer;) // used for debugging visualization
        __DEBUGDEF(uintptr_t frameArenaHighmark;)
    };

    struct Instance {
        Time time;
        CameraManager cameraMgr;
        RenderManager renderMgr;
        gameplay::movement::State player;
        Memory memory;
    };

    void loadLaunchConfig(platform::LaunchConfig& config) {
        // hardcoded for now
        config.window_width = 320 * 3;
        config.window_height = 240 * 3;
        config.game_width = 320 * 1;
        config.game_height = 240 * 1;
        config.fullscreen = false;
        config.title = "3D Test";
		config.scratchArena_size = 1 << 20; // 1MB
    }

    void start(Instance& game, platform::GameConfig& config, const platform::State& platform) {

        game.time = {};
        game.time.config.maxFrameLength = 0.1;
        game.time.config.targetFramerate = 1.0 / 60.0;
        game.time.lastFrame = platform.time.now;

        config = {};
        config.nextFrame = platform.time.now;

        {
            __DEBUGEXP(allocator::init_arena(allocator::emergencyArena, 1 << 20)); // 1MB
            allocator::init_arena(game.memory.persistentArena, 1 << 20); // 1MB
            __DEBUGEXP(game.memory.persistentArenaBuffer = game.memory.persistentArena.curr);
            allocator::init_arena(game.memory.frameArena, 1 << 20); // 1MB
            game.memory.frameArenaBuffer = game.memory.frameArena.curr;
            __DEBUGEXP(game.memory.frameArenaHighmark = (uintptr_t)game.memory.frameArena.curr; game.memory.frameArena.highmark = &game.memory.frameArenaHighmark);
            __DEBUGEXP(allocator::init_arena(game.memory.imDebugArena, renderer::im::arena_size));
        }


#if READ_SHADERCACHE
        { // shader cache
            FILE* f;
            if (platform::fopen(&f, "shaderCache.bin", "rb") == 0) {
                u64 count;
                fread(&count, sizeof(u64), 1, f);
                renderer::driver::shaderCache.shaderBytecode = (renderer::driver::ShaderBytecode*)malloc(sizeof(renderer::driver::ShaderBytecode) * count);
                for (u64 i = 0; i < count; i++) {
                    u64 size;
                    fread(&size, sizeof(u64), 1, f);
                    renderer::driver::ShaderBytecode& shader = renderer::driver::shaderCache.shaderBytecode[i];
                    shader.data = (u8*)malloc(sizeof(u8) * size);
                    shader.size = size;
                    fread(shader.data, sizeof(u8), size, f);
                }
                platform::fclose(f);
            }
        }
#endif

        game.renderMgr = {};
        {
            RenderManager& mgr = game.renderMgr;
            using namespace renderer;

            WindowProjection::Config& ortho = mgr.windowProjection.config;
            ortho.right = platform.screen.window_width * 0.5f;
            ortho.top = platform.screen.window_height * 0.5f;
            ortho.left = -ortho.right;
            ortho.bottom = -ortho.top;
            ortho.near = 0.f;
            ortho.far = 1000.f;
            generate_matrix_ortho(mgr.windowProjection.matrix, ortho);

            // Todo: consider whether precision needs special handling
            PerspProjection::Config& frustum = mgr.perspProjection.config;
            frustum.fov = 45.f;
            frustum.aspect = platform.screen.width / (f32)platform.screen.height;
            frustum.near = 1.f;
            frustum.far = 1000.f;
            generate_matrix_persp(mgr.perspProjection.matrix, frustum);
        }

        {
            // meshes in the scene
            game.renderMgr.store = {};
            renderer::init_pipelines(game.renderMgr.store, platform.memory.scratchArenaRoot, platform.screen);
            __DEBUGEXP(renderer::im::init(game.renderMgr.imCtx, game.memory.imDebugArena));
        }

        game.player = {};
        {
            math::identity4x4(game.player.transform);
            renderer::NodeData& nodeData = renderer::get_draw_node_core(game.renderMgr.store, game.renderMgr.store.playerDrawNodeHandle).nodeData;
            game.player.transform.pos = nodeData.worldMatrix.col3.xyz;
        }

        game.cameraMgr = {};
        {
            CameraManager& mgr = game.cameraMgr;
            Camera& cam = mgr.cameras[CameraManager::FlyCam];
            mgr.activeCam = &cam;

            mgr.orbitCamera.offset = float3(0.f, -100.f, 0.f);
            mgr.orbitCamera.eulers = float3(45.f * math::d2r_f, 0.f, -25.f * math::d2r_f);
            mgr.orbitCamera.scale = 1.f;
            mgr.orbitCamera.origin = float3(0.f, 0.f, 0.f);
        }

#if WRITE_SHADERCACHE
        { // shader cache
            FILE* f;
            if (platform::fopen(&f, "shaderCache.bin", "wb") == 0) {
                fwrite(&renderer::driver::shaderCache.shaderBytecodeCount, sizeof(u64), 1, f);
                for (size_t i = 0; i < renderer::driver::shaderCache.shaderBytecodeCount; i++) {
                    fwrite(&renderer::driver::shaderCache.shaderBytecode[i].size, sizeof(u64), 1, f);
                    fwrite(renderer::driver::shaderCache.shaderBytecode[i].data, sizeof(u8), renderer::driver::shaderCache.shaderBytecode[i].size, f);
                }
                platform::fclose(f);
            }
        }
#endif
#if WRITE_SHADERCACHE || READ_SHADERCACHE
		{ // shader cache cleanup
			for (u64 i = 0; i < renderer::driver::shaderCache.shaderBytecodeCount; i++) {
				free(renderer::driver::shaderCache.shaderBytecode[i].data);
			}
			free(renderer::driver::shaderCache.shaderBytecode);
        }
#endif

    }

    void update(Instance& game, platform::GameConfig& config, platform::State& platform) {

        // frame arena reset
        game.memory.frameArena.curr = game.memory.frameArenaBuffer;

        // frame timing calculations
        {
            f64 raw_dt = platform.time.now - game.time.lastFrame;
            game.time.lastFrameDelta = math::min(raw_dt, game.time.config.maxFrameLength);
            game.time.lastFrame = platform.time.now;
            config.nextFrame = platform.time.now + game.time.config.targetFramerate;

            #if __DEBUG
            {
                f64 frameAvg = 0.;
                constexpr u64 frameHistoryCount = (sizeof(debug::frameHistory) / sizeof(debug::frameHistory[0]));
                debug::frameHistory[debug::frameHistoryIdx] = raw_dt;
                debug::frameHistoryIdx = (debug::frameHistoryIdx + 1) % frameHistoryCount;
                for (f64 dt : debug::frameHistory) {
                    frameAvg += dt;
                }
                // miscalculated until the queue is full, but that's just the first second
                frameAvg /= frameHistoryCount;
                debug::frameAvg = frameAvg;
            }
            #endif
        }

        // meta input checks
        const ::input::keyboard::State& keyboard = platform.input.keyboard;
        bool step = true;
        bool captureCameras = false;
        {
            if (keyboard.released(input::EXIT)) {
                config.quit = true;
            }
            #if __DEBUG
            if (keyboard.pressed(input::TOGGLE_OVERLAY)) { debug::overlaymode = (debug::OverlayMode::Enum)((debug::overlaymode + 1) % debug::OverlayMode::Count); }
            if (keyboard.pressed(input::TOGGLE_DEBUG3D)) { debug::debug3Dmode = (debug::Debug3DView::Enum)((debug::debug3Dmode + 1) % debug::Debug3DView::Count); }
            if (keyboard.pressed(input::TOGGLE_CAPTURED_CAMERA)) { debug::debugCameraStage = (debug::DebugCameraStage::Enum)((debug::debugCameraStage + 1) % debug::DebugCameraStage::Count); }
            if (keyboard.pressed(input::CAPTURE_CAMERAS)) { captureCameras = true; }
            if (keyboard.pressed(input::TOGGLE_FRUSTUM_CUT)) { debug::force_cut_frustum = !debug::force_cut_frustum; }
            if (keyboard.pressed(::input::keyboard::Keys::Enum(input::TOGGLE_FRUSTUM_NEAR))) { debug::frustum_planes_off[0] = !debug::frustum_planes_off[0]; }
            if (keyboard.pressed(::input::keyboard::Keys::Enum(input::TOGGLE_FRUSTUM_FAR))) { debug::frustum_planes_off[1] = !debug::frustum_planes_off[1]; }
            if (keyboard.pressed(::input::keyboard::Keys::Enum(input::TOGGLE_FRUSTUM_LEFT))) { debug::frustum_planes_off[2] = !debug::frustum_planes_off[2]; }
            if (keyboard.pressed(::input::keyboard::Keys::Enum(input::TOGGLE_FRUSTUM_RIGHT))) { debug::frustum_planes_off[3] = !debug::frustum_planes_off[3]; }
            if (keyboard.pressed(::input::keyboard::Keys::Enum(input::TOGGLE_FRUSTUM_BOTTOM))) { debug::frustum_planes_off[4] = !debug::frustum_planes_off[4]; }
            if (keyboard.pressed(::input::keyboard::Keys::Enum(input::TOGGLE_FRUSTUM_TOP))) { debug::frustum_planes_off[5] = !debug::frustum_planes_off[5]; }
            #endif      
            step = !game.time.paused;
        }

        if (step)
        {
            f32 dt = (f32)(game.time.lastFrameDelta * (1.0 + game.time.updateOverspeed));

            using namespace game;

            Camera* activeCam = game.cameraMgr.activeCam;

            // player movement update
            {
                if (platform.input.padCount > 0) {
                    gameplay::movement::process(game.player.control, platform.input.pads[0]);
                }
                else {
                    gameplay::movement::process(game.player.control, keyboard, { input::UP, input::DOWN, input::LEFT, input::RIGHT });
                }
                gameplay::movement::process_cameraRelative(game.player.transform, game.player.movementController, game.player.control, activeCam->transform, dt);

                // update player render
                renderer::NodeData& nodeData = renderer::get_draw_node_core(game.renderMgr.store, game.renderMgr.store.playerDrawNodeHandle).nodeData;
                nodeData.worldMatrix = game.player.transform.matrix;

                { // animation hack
                    animation::AnimatedNode& animatedData = renderer::get_animatedNode(game.renderMgr.store, game.renderMgr.store.playerAnimatedNodeHandle);

                    enum Cycle { Idle = 0, JerkyRun = 1, Run = 2 };
                    const f32 maxSpeed = 15.f;
                    const f32 runSpeedStart = 0.2f;
                    const f32 jerkyRunSpeedStart = 13.f;
                    const f32 jerkyRunMinScale = 4.5f;
                    const f32 jerkyRunMaxScale = 5.f;
                    const f32 jerkyRunParticleMinScaley = -1.3f;
                    const f32 jerkyRunParticleMaxScaley = -1.5f;
                    const f32 jerkyRunParticleMinScalez = 0.4f;
                    const f32 jerkyRunParticleMaxScalez = 0.5f;
                    const f32 runParticleMinScaley = -0.4f;
                    const f32 runParticleMaxScaley = -0.9f;
                    const f32 runParticleMinScalez = 0.4f;
                    const f32 runParticleMaxScalez = 0.5f;
                    const f32 runParticleMinOffsetz = -1.2f;
                    const f32 runParticleMaxOffsetz = -0.5f;
                    const f32 runMinScale = 5.f;
                    const f32 runMaxScale = 15.f;
                    const f32 idleScale = 2.f;
                    f32 particle_scaley = 0.f, particle_scalez = 0.f, particle_offsetz = -0.5f, particle_offsety = 0.f;
                    f32 particle_totalscale = 0.7f;
                    if (game.player.movementController.speed > jerkyRunSpeedStart) {
						const f32 run_t = (game.player.movementController.speed - jerkyRunSpeedStart) / (maxSpeed - jerkyRunSpeedStart);
                        const f32 scale = 1.f + math::lerp(run_t, jerkyRunMinScale, jerkyRunMaxScale);
                        particle_scaley = math::lerp(run_t, jerkyRunParticleMinScaley, jerkyRunParticleMaxScaley);
                        particle_scalez = math::lerp(run_t, jerkyRunParticleMinScalez, jerkyRunParticleMaxScalez);
                        particle_offsety = 0.3f;
                        animatedData.state.scale = scale;
                        if (animatedData.state.animIndex != 1) {
                            animatedData.state.animIndex = 1;
                            animatedData.state.time = 0.f;
                        }
                    } else if (game.player.movementController.speed > 0.2f) {
                        const f32 run_t = (game.player.movementController.speed - runSpeedStart) / (jerkyRunSpeedStart - runSpeedStart);
                        const f32 scale = 1.f + math::lerp(run_t, runMinScale, runMaxScale);
                        particle_scaley = math::lerp(run_t, runParticleMinScaley, runParticleMaxScaley);
                        particle_scalez = math::lerp(run_t, runParticleMinScalez, runParticleMaxScalez);
                        particle_offsetz = math::lerp(run_t, runParticleMinOffsetz, runParticleMaxOffsetz);
                        particle_totalscale = 0.6f;
                        animatedData.state.scale = scale;
                        if (animatedData.state.animIndex != 2) {
                            animatedData.state.animIndex = 2;
                            animatedData.state.time = 0.f;
                            animatedData.state.scale = 9.f;
                        }
                    } else {
                        particle_scaley = 0.f;
                        particle_scalez = 0.f;
                        animatedData.state.scale = idleScale;
                        if (animatedData.state.animIndex != 0) {
                            animatedData.state.animIndex = 0;
                            animatedData.state.time = 0.f;
                            animatedData.state.scale = 1.f;
                        }
                    }

                    // dust particles hack
                    renderer::Matrices64* instance_matrices;
                    u32* instance_count;
                    renderer::get_draw_instanced_data(instance_matrices, instance_count, game.renderMgr.store, game.renderMgr.store.particlesDrawHandle);
                    for (u32 i = 0; i < *instance_count; i++) {

                        const f32 scaley = particle_scaley;
                        const f32 scalez = particle_scalez;
                        const f32 offsety = particle_offsety;
                        const f32 offsetz = particle_offsetz;
                        const f32 scale = particle_totalscale;

                        Transform t;
                        math::identity4x4(t);
                        t.pos = math::add(game.player.transform.pos, math::scale(game.player.transform.front, offsety + scaley * (i + 2)));
                        t.pos.z = t.pos.z + offsetz + scalez * (math::cos((f32)platform.time.now * 10.f + i * 2.f) - 1.5f);
                        t.matrix.col0.x = 0.2f;
                        t.matrix.col1.y = 0.2f;
                        t.matrix.col2.z = 0.2f;
                        t.matrix.col0 = math::scale(t.matrix.col0, scale);
                        t.matrix.col1 = math::scale(t.matrix.col1, scale);
                        t.matrix.col2 = math::scale(t.matrix.col2, scale);

                        instance_matrices->data[i] = t.matrix;
                    }
                }

                // anim update
                {
                    renderer::Store& store = game.renderMgr.store;
                    for (u32 n = 0, count = 0; n < store.animatedNodes.cap && count < store.animatedNodes.count; n++) {
                        if (store.animatedNodes.data[n].alive == 0) { continue; }
                        count++;

                        animation::AnimatedNode& animatedData = store.animatedNodes.data[n].state.live;
                        renderer::Matrices32& skinningData = renderer::get_draw_skinning_data(game.renderMgr.store, animatedData.drawNodeHandle );
                        animation::updateAnimation(skinningData.data, animatedData.state, animatedData.clips[animatedData.state.animIndex], animatedData.skeleton, dt);
                    }
                }
            }

            // camera update
            {
                if (platform.input.padCount > 0) {
                    gameplay::orbit::process(game.cameraMgr.orbitCamera, platform.input.pads[0]);
                } else {
                    gameplay::orbit::process(game.cameraMgr.orbitCamera, platform.input.mouse);
                }
                gameplay::orbit::process(activeCam->transform, game.cameraMgr.orbitCamera);
                // send camera data to render manager
                renderer::generate_MV_matrix(activeCam->viewMatrix, activeCam->transform);
            }
        }

        //Render update
        renderer::VisibleNodes* visibleNodes, * mirrorVisibleNodes; // todo: move these two somewhere?
        {
            RenderManager& mgr = game.renderMgr;
            renderer::Store& store = mgr.store;
            using namespace renderer;

            Camera* activeCam = game.cameraMgr.activeCam;
            float4x4 viewMatrix = activeCam->viewMatrix;
            float4x4 mirrorViewMatrix, mirrorProjectionMatrix, mirrorVPMatrix;

            // perspective cam set up
            float4x4 vpMatrix;
            driver::RscCBuffer& scene_cbuffer = store.cbuffers[store.cbuffer_scene];
            renderer::SceneData cbufferPerScene;
            {
                cbufferPerScene.projectionMatrix = mgr.perspProjection.matrix;
                cbufferPerScene.viewMatrix = viewMatrix;
                cbufferPerScene.viewPos = viewMatrix.col3.xyz;
                cbufferPerScene.lightPos = float3(3.f, 8.f, 15.f);
                renderer::driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);
                vpMatrix = math::mult(mgr.perspProjection.matrix, viewMatrix);
                __DEBUGEXP(if (captureCameras) { debug::capturedCameras[debug::DebugCameraStage::Scene] = vpMatrix; });
            }

            {
                {
                    driver::ViewportParams vpParams;
                    vpParams.topLeftX = 0;
                    vpParams.topLeftY = 0;
                    vpParams.width = (f32)platform.screen.width;
                    vpParams.height = (f32)platform.screen.height;
                    vpParams.minDepth = 0.f;
                    vpParams.maxDepth = 1.f;
                    driver::set_VP(vpParams);
                }
            
                // cull nodes
                {
                    {
                        visibleNodes = (VisibleNodes*)allocator::alloc_arena(game.memory.frameArena, sizeof(VisibleNodes), alignof(VisibleNodes));
                        memset(visibleNodes, 0, sizeof(VisibleNodes));
                        computeVisibility(*visibleNodes, vpMatrix, store);
                    }

                    // set up mirror scene if mirror is visible
                    mirrorVisibleNodes = (VisibleNodes*)allocator::alloc_arena(game.memory.frameArena, sizeof(VisibleNodes), alignof(VisibleNodes));
                    memset(mirrorVisibleNodes, 0, sizeof(VisibleNodes));
                    for (u32 i = 0; i < visibleNodes->visible_nodes_count; i++) {
                        u32 n = visibleNodes->visible_nodes[i];
                        DrawNode& node = store.drawNodes.data[n].state.live;

                        if (store.mirror.drawHandle != handle_from_node(store, node)) { // todo: improve?
                            
                            auto reflectionMatrix = [](float4 p) -> float4x4 { // todo: understand properly
                                float4x4 o;
                                o.col0.x = 1 - 2.f * p.x * p.x; o.col1.x = -2.f * p.x * p.y;    o.col2.x = -2.f * p.x * p.z;        o.col3.x = -2.f * p.x * p.w;
                                o.col0.y = -2.f * p.y * p.x;    o.col1.y = 1 - 2.f * p.y * p.y; o.col2.y = -2.f * p.y * p.z;        o.col3.y = -2.f * p.y * p.w;
                                o.col0.z = -2.f * p.z * p.x;    o.col1.z = -2.f * p.z * p.y;    o.col2.z = 1.f - 2.f * p.z * p.z;   o.col3.z = -2.f * p.z * p.w;
                                o.col0.w = 0.f;                 o.col1.w = 0.f;                 o.col2.w = 0.f;                     o.col3.w = 1.f;
                                return o;
                            };
                            
                            float4 plane { store.mirror.normal.x, store.mirror.normal.y, store.mirror.normal.z, -math::dot(store.mirror.pos, store.mirror.normal) };
                            float4x4 reflect = reflectionMatrix(plane);
                            mirrorViewMatrix = math::mult(viewMatrix, reflect);
                            mirrorProjectionMatrix = mgr.perspProjection.matrix;
                            __DEBUGEXP(if (captureCameras) { debug::capturedCameras[debug::DebugCameraStage::SceneMirror] = math::mult(mirrorProjectionMatrix, mirrorViewMatrix); });
                            float3 normalCameraSpace = math::mult(mirrorViewMatrix, float4(store.mirror.normal, 0.f)).xyz;
                            float3 posCameraSpace = math::mult(mirrorViewMatrix, float4(store.mirror.pos, 1.f)).xyz;
                            float4 pCameraSpace{ normalCameraSpace.x, normalCameraSpace.y, normalCameraSpace.z, -math::dot(posCameraSpace, normalCameraSpace) };
                            add_oblique_plane_to_persp(mirrorProjectionMatrix, pCameraSpace);
                            mirrorVPMatrix = math::mult(mirrorProjectionMatrix, mirrorViewMatrix);
                            __DEBUGEXP(if (captureCameras) { debug::capturedCameras[debug::DebugCameraStage::SceneMirrorClipped] = mirrorVPMatrix; });
                            computeVisibility(*mirrorVisibleNodes, mirrorVPMatrix, store);

                            break;
                        }
                    }
                }
                // update visible nodes
                {
                    // figure out which nodes are visible among all of the visibility lists
                    allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                    bool* nodesToUpdate = (bool*)allocator::alloc_arena(scratchArena, store.drawNodes.count * sizeof(bool), alignof(bool));
                    memset(nodesToUpdate, 0, store.drawNodes.count * sizeof(bool));
                    for (u32 i = 0; i < visibleNodes->visible_nodes_count; i++) {
                        u32 n = visibleNodes->visible_nodes[i];
                        u32 index = allocator::get_pool_index(store.drawNodes, store.drawNodes.data[n].state.live);
                        nodesToUpdate[index] = true;
                    }
                    bool* nodesSkinnedToUpdate = (bool*)allocator::alloc_arena(scratchArena, store.drawNodesSkinned.count * sizeof(bool), alignof(bool));
                    memset(nodesSkinnedToUpdate, 0, store.drawNodesSkinned.count * sizeof(bool));
                    for (u32 i = 0; i < visibleNodes->visible_nodes_skinned_count; i++) {
                        u32 n = visibleNodes->visible_nodes_skinned[i];
                        u32 index = allocator::get_pool_index(store.drawNodesSkinned, store.drawNodesSkinned.data[n].state.live);
                        nodesSkinnedToUpdate[index] = true;
                    }
                    for (u32 i = 0; i < mirrorVisibleNodes->visible_nodes_count; i++) {
                        u32 n = mirrorVisibleNodes->visible_nodes[i];
                        u32 index = allocator::get_pool_index(store.drawNodes, store.drawNodes.data[n].state.live);
                        nodesToUpdate[index] = true;
                    }
                    for (u32 i = 0; i < mirrorVisibleNodes->visible_nodes_skinned_count; i++) {
                        u32 n = mirrorVisibleNodes->visible_nodes_skinned[i];
                        u32 index = allocator::get_pool_index(store.drawNodesSkinned, store.drawNodesSkinned.data[n].state.live);
                        nodesSkinnedToUpdate[index] = true;
                    }

                    // update cbuffers for all visible nodes
                    for (u32 n = 0, count = 0; n < store.drawNodes.cap && count < store.drawNodes.count; n++) {
                        if (store.drawNodes.data[n].alive == 0) { continue; }
                        count++;
                        const DrawNode& node = store.drawNodes.data[n].state.live;
                        if (!nodesToUpdate[n]) continue;
                        driver::update_cbuffer(store.cbuffers[node.cbuffer_node], &node.nodeData);
                    }
                    for (u32 n = 0, count = 0; n < store.drawNodesSkinned.cap && count < store.drawNodesSkinned.count; n++) {
                        if (store.drawNodesSkinned.data[n].alive == 0) { continue; }
                        count++;
                        const DrawNodeSkinned& node = store.drawNodesSkinned.data[n].state.live;
                        if (!nodesSkinnedToUpdate[n]) continue;
                        driver::update_cbuffer(store.cbuffers[node.core.cbuffer_node], &node.core.nodeData);
                        driver::update_cbuffer(store.cbuffers[node.cbuffer_skinning], &node.skinningMatrixPalette);
                    }
                    for (u32 n = 0, count = 0; n < store.drawNodesInstanced.cap && count < store.drawNodesInstanced.count; n++) {
                        if (store.drawNodesInstanced.data[n].alive == 0) { continue; }
                        count++;
                        const DrawNodeInstanced& node = store.drawNodesInstanced.data[n].state.live;
                        driver::update_cbuffer(store.cbuffers[node.core.cbuffer_node], &node.core.nodeData);
                        driver::update_cbuffer(store.cbuffers[node.cbuffer_instances], &node.instanceMatrices);
                    }
                }

                driver::Marker_t marker;
                driver::set_marker_name(marker, "BASE SCENE"); driver::start_event(marker);
                {
                    driver::bind_RT(store.gameRT);

                    Color32 skycolor(0.2f, 0.344f, 0.59f, 1.f);
                    driver::clear_RT(store.gameRT, driver::RenderTargetClearFlags::Color | driver::RenderTargetClearFlags::Depth | driver::RenderTargetClearFlags::Stencil, skycolor);
                    driver::bind_RS(store.rasterizerStateFillFrontfaces);

                    driver::set_marker_name(marker, "SKY"); driver::start_event(marker); // todo: quad?
                    {
                        driver::bind_DS(store.depthStateOff);
                        driver::bind_shader(store.shaders[ShaderType::Color3D]);
                        driver::bind_blend_state(store.blendStateBlendOff);
                        for (u32 i = 0; i < COUNT_OF(store.sky.buffers); i++) {
                            driver::bind_indexed_vertex_buffer(store.sky.buffers[i]);
                            driver::RscCBuffer buffers[] = { scene_cbuffer, store.sky.cbuffer };
                            driver::bind_cbuffers(store.shaders[ShaderType::Color3D], buffers, 2);
                            driver::draw_indexed_vertex_buffer(store.sky.buffers[i]);
                        }
                    }
                    driver::end_event();

                    driver::set_marker_name(marker, "OPAQUE"); driver::start_event(marker);
                    {
                        driver::bind_DS(store.depthStateOn);
                        allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                        Drawlist dl = {};
                        u32 maxDrawCallsThisFrame = (visibleNodes->visible_nodes_count + visibleNodes->visible_nodes_skinned_count + (u32)store.drawNodesInstanced.count) * DrawlistStreams::Count;
                        dl.items = (DrawCall_Item*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                        dl.keys = (Key*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));
                        addNodesToDrawlistSorted(dl, *visibleNodes, game.cameraMgr.activeCam->transform.pos, store, 0, renderer::DrawlistFilter::Alpha | renderer::DrawlistFilter::Mirror, renderer::SortParams::Type::Default);
                        Drawlist_Context ctx = {};
                        Drawlist_Overrides overrides = {};
                        ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                        draw_drawlist(dl, ctx, overrides);
                    }
                    driver::end_event();

                    #if __DEBUG
                    // Immediate mode debug, to be rendered along with the 3D scene
                    // Note that the buffers will be submitted to the GPU now, anything pushed to the immediate mode from here on will
                    // be rendered on the next frame
                    {
                        im::Context& imCtx = mgr.imCtx;

                        //World axis
                        const Color32 axisX(0.8f, 0.15f, 0.25f, 0.7f); // x is red
                        const Color32 axisY(0.25f, 0.8f, 0.15f, 0.7f); // y is green
                        const Color32 axisZ(0.15f, 0.25f, 0.8f, 0.7f); // z is blue
                        const f32 axisSize = 7.f;
                        const float3 pos(0.f, 0.f, 0.1f);

                        im::segment(imCtx, pos, math::add(pos, math::scale(float3(1.f, 0.f, 0.f), axisSize)), axisX);
                        im::segment(imCtx, pos, math::add(pos, math::scale(float3(0.f, 1.f, 0.f), axisSize)), axisY);
                        im::segment(imCtx, pos, math::add(pos, math::scale(float3(0.f, 0.f, 1.f), axisSize)), axisZ);

                        const f32 thickness = 0.1f;
                        const u32 steps = 10;
                        const f32 spacing = 1.f / (f32)steps;
                        for (int i = 1; i <= steps; i++) {
                            im::segment(imCtx,
                                math::add(pos, float3(spacing * i, 0.f, -thickness))
                                , math::add(pos, float3(spacing * i, 0.f, thickness))
                                , axisX);
                            im::segment(imCtx,
                                math::add(pos, float3(0.f, spacing * i, -thickness))
                                , math::add(pos, float3(0.f, spacing * i, thickness))
                                , axisY);
                            im::segment(imCtx,
                                math::add(pos, float3(-thickness, thickness, spacing * i))
                                , math::add(pos, float3(thickness, -thickness, spacing * i))
                                , axisZ);
                        }

                        if (debug::debug3Dmode == debug::Debug3DView::All || debug::debug3Dmode == debug::Debug3DView::Culling) {
                            const Color32 bbColor(0.8f, 0.15f, 0.25f, 0.7f);

                            renderer::VisibleNodes visibleNodesDebug = {};
                            float4x4 matrix = debug::capturedCameras[debug::debugCameraStage];
                            if (!math::isZeroAll(matrix)) {
                                renderer::computeVisibility(visibleNodesDebug, matrix, store);
                                for (u32 i = 0; i < visibleNodesDebug.visible_nodes_count; i++) {
                                    u32 n = visibleNodesDebug.visible_nodes[i];
                                    const DrawNode& node = store.drawNodes.data[n].state.live;
                                    im::obb(imCtx, node.nodeData.worldMatrix, node.min, node.max, bbColor);
                                }
                                for (u32 i = 0; i < visibleNodesDebug.visible_nodes_skinned_count; i++) {
                                    u32 n = visibleNodesDebug.visible_nodes_skinned[i];
                                    const DrawNodeSkinned& node = store.drawNodesSkinned.data[n].state.live;
                                    im::obb(imCtx, node.core.nodeData.worldMatrix, node.core.min, node.core.max, bbColor);
                                }

                                const Color32 color(0.25f, 0.8f, 0.15f, 0.7f);
                                im::frustum(imCtx, matrix, color);
                            }
                        }

                        im::commit3d(mgr.imCtx);

                        renderer::driver::bind_DS(store.depthStateOn);
                        im::present3d(mgr.imCtx, mgr.perspProjection.matrix, viewMatrix);
                    }
                    #endif

                    driver::set_marker_name(marker, "ALPHA"); driver::start_event(marker);
                    {
                        driver::bind_DS(store.depthStateReadOnly);

                        allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                        Drawlist dl = {};
                        u32 maxDrawCallsThisFrame = (visibleNodes->visible_nodes_count + visibleNodes->visible_nodes_skinned_count + (u32)store.drawNodesInstanced.count) * DrawlistStreams::Count;
                        dl.items = (DrawCall_Item*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                        dl.keys = (Key*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));
                        driver::bind_blend_state(store.blendStateOn);
                        addNodesToDrawlistSorted(dl, *visibleNodes, game.cameraMgr.activeCam->transform.pos, store, renderer::DrawlistFilter::Alpha, renderer::DrawlistFilter::Mirror, renderer::SortParams::Type::BackToFront);
                        Drawlist_Context ctx = {};
                        Drawlist_Overrides overrides = {};
                        overrides.forced_blendState = true;
                        ctx.blendState = &store.blendStateOn;
                        ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                        draw_drawlist(dl, ctx, overrides);
                    }
                    driver::end_event();
                }
                driver::end_event();
                
                // Mirror passes
                if (mirrorVisibleNodes->visible_nodes_count + mirrorVisibleNodes->visible_nodes_skinned_count) {

                    driver::set_marker_name(marker, "MARK MIRROR"); driver::start_event(marker);
                    {
                        allocator::Arena scratchArena = platform.memory.scratchArenaRoot;

                        driver::bind_DS(store.depthStateMarkMirrors);
                        driver::bind_RS(store.rasterizerStateFillFrontfaces);
                        driver::bind_blend_state(store.blendStateOff);
                        Drawlist_Context ctx = {};
                        Drawlist_Overrides overrides = {};
                        overrides.forced_blendState = true;
                        ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                        ctx.blendState = &store.blendStateOff;

                        Drawlist dl = {};
                        u32 maxDrawCallsThisFrame = DrawlistStreams::Count; // support for one single mirror
                        dl.items = (DrawCall_Item*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                        dl.keys = (Key*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));

                        DrawNode& node = get_draw_node_core(store, store.mirror.drawHandle);
                        const u32 maxMeshCount = COUNT_OF(node.meshHandles);
                        for (u32 m = 0; m < maxMeshCount; m++) {
                            if (node.meshHandles[m] == 0) { continue; }
                            u32 dl_index = dl.count[DrawlistBuckets::Base]++;
                            DrawCall_Item& item = dl.items[dl_index];

                            item = {};
                            Key& key = dl.keys[dl_index];
                            key.idx = dl_index;
                            const DrawMesh& mesh = get_drawMesh(store, node.meshHandles[m]);
                            item.shader = store.shaders[mesh.type];
                            item.vertexBuffer = mesh.vertexBuffer;
                            item.cbuffers[item.cbuffer_count++] = store.cbuffers[node.cbuffer_node];
                            item.texture = mesh.texture;
                            item.blendState = store.blendStateOff;
                            item.name = renderer::shaderNames[mesh.type];
                        }
                        draw_drawlist(dl, ctx, overrides);
                    }
                    driver::end_event();

                    driver::set_marker_name(marker, "REFLECTION SCENE"); driver::start_event(marker);
                    {
                        driver::RscCBuffer& scene_cbuffer = store.cbuffers[store.cbuffer_scene];
                        {
                            renderer::SceneData cbufferPerSceneMirrored = cbufferPerScene;
                            cbufferPerSceneMirrored.projectionMatrix = mirrorProjectionMatrix;
                            cbufferPerSceneMirrored.viewMatrix = mirrorViewMatrix;
                            cbufferPerSceneMirrored.viewPos = mirrorViewMatrix.col3.xyz;
                            renderer::driver::update_cbuffer(scene_cbuffer, &cbufferPerSceneMirrored);
                        }

                        driver::clear_RT(store.gameRT, driver::RenderTargetClearFlags::Depth); // todo: can we get away without doing this for all the depth buffer?

                        driver::bind_RS(store.rasterizerStateFillBackfaces);

                        driver::set_marker_name(marker, "SKY"); driver::start_event(marker);
                        {
                            driver::bind_DS(store.depthStateMirrorReflectionsDepthReadOnly);
                            driver::bind_shader(store.shaders[ShaderType::Color3D]);
                            driver::bind_blend_state(store.blendStateBlendOff);
                            for (u32 i = 0; i < COUNT_OF(store.sky.buffers); i++) {
                                driver::bind_indexed_vertex_buffer(store.sky.buffers[i]);
                                driver::RscCBuffer buffers[] = { scene_cbuffer, store.sky.cbuffer };
                                driver::bind_cbuffers(store.shaders[ShaderType::Color3D], buffers, 2);
                                driver::draw_indexed_vertex_buffer(store.sky.buffers[i]);
                            }
                        }
                        driver::end_event();

                        driver::set_marker_name(marker, "OPAQUE"); driver::start_event(marker);
                        {
                            driver::bind_DS(store.depthStateMirrorReflections);
                            allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                            Drawlist dl = {};
                            u32 maxDrawCallsThisFrame = (mirrorVisibleNodes->visible_nodes_count + mirrorVisibleNodes->visible_nodes_skinned_count + (u32)store.drawNodesInstanced.count) * DrawlistStreams::Count;
                            dl.items = (DrawCall_Item*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                            dl.keys = (Key*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));
                            addNodesToDrawlistSorted(dl, *mirrorVisibleNodes, game.cameraMgr.activeCam->transform.pos, store, 0, renderer::DrawlistFilter::Alpha, renderer::SortParams::Type::Default);
                            Drawlist_Context ctx = {};
                            Drawlist_Overrides overrides = {};
                            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                            draw_drawlist(dl, ctx, overrides);
                        }
                        driver::end_event();


                        #if __DEBUG
                        {
                            renderer::driver::bind_DS(store.depthStateMirrorReflections);
                            im::present3d(mgr.imCtx, mirrorProjectionMatrix, mirrorViewMatrix);
                        }
                        #endif

                        driver::set_marker_name(marker, "ALPHA"); driver::start_event(marker);
                        {
                            renderer::driver::bind_RS(store.rasterizerStateFillBackfaces);
                            driver::bind_DS(store.depthStateMirrorReflectionsDepthReadOnly);
                            driver::bind_blend_state(store.blendStateOn);

                            allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                            Drawlist dl = {};
                            u32 maxDrawCallsThisFrame = (mirrorVisibleNodes->visible_nodes_count + mirrorVisibleNodes->visible_nodes_skinned_count + (u32)store.drawNodesInstanced.count) * DrawlistStreams::Count;
                            dl.items = (DrawCall_Item*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                            dl.keys = (Key*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));
                            addNodesToDrawlistSorted(dl, *mirrorVisibleNodes, game.cameraMgr.activeCam->transform.pos, store, renderer::DrawlistFilter::Alpha, 0, renderer::SortParams::Type::BackToFront);
                            Drawlist_Context ctx = {};
                            Drawlist_Overrides overrides = {};
                            overrides.forced_blendState = true;
                            ctx.blendState = &store.blendStateOn;
                            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                            draw_drawlist(dl, ctx, overrides);
                        }
                        driver::end_event();
                    }
                    driver::end_event();

                    driver::set_marker_name(marker, "MIRROR IN SCENE"); driver::start_event(marker);
                    {
                        driver::RscCBuffer& scene_cbuffer = store.cbuffers[store.cbuffer_scene];
                        {
                            renderer::SceneData cbufferPerScene;
                            cbufferPerScene.projectionMatrix = mgr.perspProjection.matrix;
                            cbufferPerScene.viewMatrix = viewMatrix;
                            cbufferPerScene.viewPos = viewMatrix.col3.xyz;
                            cbufferPerScene.lightPos = float3(3.f, 8.f, 15.f); // todo: save off somewhere?
                            renderer::driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);
                        }

                        driver::bind_DS(store.depthStateOn); // todo: this is a hack so opengl can clear depth
                        driver::clear_RT(store.gameRT, driver::RenderTargetClearFlags::Depth); // todo: could we just clear the mirror?

                        driver::bind_DS(store.depthStateMirrorReflectionsDepthReadOnly);
                        driver::bind_RS(store.rasterizerStateFillFrontfaces);
                        driver::bind_blend_state(store.blendStateOn);
                        Drawlist_Context ctx = {};
                        Drawlist_Overrides overrides = {};
                        overrides.forced_blendState = true;
                        ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                        ctx.blendState = &store.blendStateOn;

                        allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                        Drawlist dl = {};
                        u32 maxDrawCallsThisFrame = DrawlistStreams::Count; // support for one single mirror
                        dl.items = (DrawCall_Item*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                        dl.keys = (Key*)allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));

                        DrawNode& node = get_draw_node_core(store, store.mirror.drawHandle);
                        const u32 maxMeshCount = COUNT_OF(node.meshHandles);
                        for (u32 m = 0; m < maxMeshCount; m++) {
                            if (node.meshHandles[m] == 0) { continue; }
                            u32 dl_index = dl.count[DrawlistBuckets::Base]++;
                            DrawCall_Item& item = dl.items[dl_index];

                            item = {};
                            Key& key = dl.keys[dl_index];
                            key.idx = dl_index;
                            const DrawMesh& mesh = get_drawMesh(store, node.meshHandles[m]);
                            item.shader = store.shaders[mesh.type];
                            item.vertexBuffer = mesh.vertexBuffer;
                            item.cbuffers[item.cbuffer_count++] = store.cbuffers[node.cbuffer_node];
                            item.texture = mesh.texture;
                            item.blendState = store.blendStateOn;
                            item.name = renderer::shaderNames[mesh.type];
                        }
                        draw_drawlist(dl, ctx, overrides);
                    }
                    driver::end_event();
                }
            }

            // Immediate-mode debug in 2D. Can be moved out of the render update, it only pushes data to cpu buffers
            #if __DEBUG
            {
                im::Context& imCtx = mgr.imCtx;
                if (debug::overlaymode != debug::OverlayMode::None)
                {
                    Color32 defaultCol(0.7f, 0.8f, 0.15f, 1.0f);
                    Color32 activeCol(1.0f, 0.2f, 0.1f, 1.0f);
                    
                    f32 textscale = platform.screen.text_scale;
                    f32 lineheight = 15.f * textscale;

                    struct EventText {
                        char text[256];
                        f64 time;
                    };
                    renderer::im::TextParams textParamsLeft, textParamsRight, textParamsCenter;
                    textParamsLeft.scale = (u8)platform.screen.text_scale;
                    textParamsLeft.pos = float3(game.renderMgr.windowProjection.config.left + 10.f * textscale, game.renderMgr.windowProjection.config.top - 10.f * textscale, -50);
                    textParamsLeft.color = defaultCol;
                    textParamsRight.scale = (u8)platform.screen.text_scale;
                    textParamsRight.pos = float3(game.renderMgr.windowProjection.config.right - 60.f * textscale, game.renderMgr.windowProjection.config.top - 10.f * textscale, -50);
                    textParamsRight.color = defaultCol;
                    textParamsCenter.scale = (u8)platform.screen.text_scale;
                    textParamsCenter.pos = float3(0.f, game.renderMgr.windowProjection.config.top - 10.f * textscale, -50);
                    textParamsCenter.color = defaultCol;

                    const char* overlaynames[] = { "All", "Help Only", "Arenas only", "None" };
                    if (keyboard.pressed(input::TOGGLE_OVERLAY)) {
                        platform::format(debug::eventLabel.text, sizeof(debug::eventLabel.text), "Overlays: %s", overlaynames[debug::overlaymode]);
                        debug::eventLabel.time = platform.time.now;
                        textParamsLeft.color = activeCol;
                    }
                    else {
                        textParamsLeft.color = defaultCol;
                    }
                    renderer::im::text2d(imCtx, textParamsLeft, "H to toggle overlays: %s", overlaynames[debug::overlaymode]);
                    textParamsLeft.pos.y -= lineheight;

                    const char* visualizationModes[] = { "Culling", "All", "None" };
                    if (keyboard.pressed(input::TOGGLE_DEBUG3D)) {
                        platform::format(debug::eventLabel.text, sizeof(debug::eventLabel.text), "Visualization mode: %s", visualizationModes[debug::debug3Dmode]);
                        debug::eventLabel.time = platform.time.now;
                        textParamsLeft.color = activeCol;
                    }
                    else {
                        textParamsLeft.color = defaultCol;
                    }
                    renderer::im::text2d(imCtx, textParamsLeft, "V to toggle 3D visualization modes: %s", visualizationModes[debug::debug3Dmode]);
                    textParamsLeft.pos.y -= lineheight;

                    const char* cameraStages[] = { "Scene", "Scene Mirror", "Scene Mirror Clipped" };
                    if (keyboard.pressed(input::CAPTURE_CAMERAS)) {
                        platform::format(debug::eventLabel.text, sizeof(debug::eventLabel.text), "Capture cameras");
                        debug::eventLabel.time = platform.time.now;
                        textParamsLeft.color = activeCol;
                    }
                    else {
                        textParamsLeft.color = defaultCol;
                    }
                    renderer::im::text2d(imCtx, textParamsLeft, "P to capture cameras in scene");
                    textParamsLeft.color = defaultCol;
                    textParamsLeft.pos.y -= lineheight;

                    if (keyboard.pressed(input::TOGGLE_CAPTURED_CAMERA)) {
                        platform::format(debug::eventLabel.text,sizeof(debug::eventLabel.text), "Camera: %s (%s)", cameraStages[debug::debugCameraStage], math::isZeroAll(debug::capturedCameras[debug::debugCameraStage]) ? "captured" : "captured");
                        debug::eventLabel.time = platform.time.now;
                        textParamsLeft.color = activeCol;
                    } else {
                        textParamsLeft.color = defaultCol;
                    }
                    renderer::im::text2d(imCtx, textParamsLeft, "C to toggle camera capture stages: %s %s", cameraStages[debug::debugCameraStage], math::isZeroAll(debug::capturedCameras[debug::debugCameraStage]) ? "Camera not captured" : "Camera Captured");
                    textParamsLeft.color = defaultCol;
                    textParamsLeft.pos.y -= lineheight;
                   
                    {
                        constexpr ::input::keyboard::Keys::Enum keys[] = { input::TOGGLE_FRUSTUM_NEAR, input::TOGGLE_FRUSTUM_FAR, input::TOGGLE_FRUSTUM_LEFT, input::TOGGLE_FRUSTUM_RIGHT, input::TOGGLE_FRUSTUM_BOTTOM, input::TOGGLE_FRUSTUM_TOP };
                        platform::StrBuilder frustumStr;
                        frustumStr.append("%sFrustum planes applied: ", debug::force_cut_frustum ? "[forced cut] " : "");
                        for (s32 i = 0; i < COUNT_OF(debug::frustum_planes_off) && !frustumStr.full(); i++) {
                            if (!debug::frustum_planes_off[i]) { frustumStr.append("%s ", debug::frustum_planes_names[i]); }
                            if (platform.input.keyboard.pressed(::input::keyboard::Keys::Enum(keys[i]))) {
                                platform::format(debug::eventLabel.text, sizeof(debug::eventLabel.text), debug::frustum_planes_off[i] ? "%s frustum plane off" : "%s frustum plane on", debug::frustum_planes_names[i]);
                                debug::eventLabel.time = platform.time.now;
                            }
                        };
                        renderer::im::text2d(imCtx, textParamsLeft, frustumStr.str);
                        textParamsLeft.pos.y -= lineheight;
                    }

                    if (debug::overlaymode == debug::OverlayMode::All || debug::overlaymode == debug::OverlayMode::HelpOnly) {
                        textParamsLeft.color = platform.input.mouse.down(::input::mouse::Keys::BUTTON_LEFT) ? activeCol : defaultCol;
                        renderer::im::text2d(imCtx, textParamsLeft, "Mouse (%.3f,%.3f)", platform.input.mouse.x, platform.input.mouse.y);
                        textParamsLeft.pos.y -= lineheight;
                        textParamsLeft.color = platform.input.mouse.down(::input::mouse::Keys::BUTTON_LEFT) ? activeCol : defaultCol;
                        renderer::im::text2d(imCtx, textParamsLeft, "   left click and drag to orbit");
                        textParamsLeft.pos.y -= lineheight;
                        textParamsLeft.color = platform.input.mouse.down(::input::mouse::Keys::BUTTON_RIGHT) ? activeCol : defaultCol;
                        renderer::im::text2d(imCtx, textParamsLeft, "   right click and drag to pan");
                        textParamsLeft.pos.y -= lineheight;
                        textParamsLeft.color = platform.input.mouse.scrolldy != 0 ? activeCol : defaultCol;
                        renderer::im::text2d(imCtx, textParamsLeft, "   mouse wheel to scale");
                        textParamsLeft.pos.y -= lineheight;
                        textParamsLeft.color = defaultCol;
                        {
                            float3 eulers_deg = math::scale(game.cameraMgr.orbitCamera.eulers, math::r2d_f);
                            renderer::im::text2d(imCtx, textParamsLeft, "Camera eulers: " float3_FORMAT("% .3f"), float3_PARAMS(eulers_deg));
                            textParamsLeft.pos.y -= lineheight;
                        }
                        renderer::im::text2d(imCtx, textParamsLeft, "mouse wheel to scale");
                        textParamsLeft.pos.y -= lineheight;
                        for (u32 i = 0; i < platform.input.padCount; i++)
                        {
                            const ::input::gamepad::State& pad = platform.input.pads[i];
                            renderer::im::text2d(imCtx, textParamsLeft, "Pad: %s", pad.name);
                            textParamsLeft.pos.y -= lineheight;
                            renderer::im::text2d(imCtx, textParamsLeft
                                , "Pad: L:(%.3f,%.3f) R:(%.3f,%.3f) L2:%.3f R2:%.3f"
                                , pad.sliders[::input::gamepad::Sliders::AXIS_X_LEFT], pad.sliders[::input::gamepad::Sliders::AXIS_Y_LEFT]
                                , pad.sliders[::input::gamepad::Sliders::AXIS_X_RIGHT], pad.sliders[::input::gamepad::Sliders::AXIS_Y_RIGHT]
                                , pad.sliders[::input::gamepad::Sliders::TRIGGER_LEFT], pad.sliders[::input::gamepad::Sliders::TRIGGER_RIGHT]
                            );

                            {
                                platform::StrBuilder padStr;
                                textParamsLeft.pos.y -= lineheight;
                                padStr.append("Keys: ");
                                const char* key_names[] = {
                                    "BUTTON_N", "BUTTON_S", "BUTTON_W", "BUTTON_E"
                                  , "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT"
                                  , "START", "SELECT"
                                  , "L1", "R1", "L2", "R2"
                                  , "LEFT_THUMB", "RIGHT_THUMB"
                                };
                                for (u32 key = 0; key < ::input::gamepad::KeyMask::COUNT && !padStr.full(); key++) {
                                    if (pad.down((::input::gamepad::KeyMask::Enum)(1 << key))) {
                                        padStr.append("%s ", key_names[key]);
                                    }
                                }
                                renderer::im::text2d(imCtx, textParamsLeft, padStr.str);
                                textParamsLeft.pos.y -= lineheight;
                            }
                        }

                        textParamsRight.color = defaultCol;
                        textParamsRight.pos.x -= 30.f * textscale;
                        renderer::im::text2d(imCtx, textParamsRight, "%s", platform::name);
                        textParamsRight.pos.y -= lineheight;
                        renderer::im::text2d(imCtx, textParamsRight, "%.3lf fps", 1. / debug::frameAvg);
                        textParamsRight.pos.y -= lineheight;
                    }

                    if (debug::overlaymode == debug::OverlayMode::All || debug::overlaymode == debug::OverlayMode::ArenaOnly)
                    {
                        auto renderArena = [](renderer::im::Context& im, renderer::im::TextParams& textCfg, u8* arenaEnd, u8* arenaStart, uintptr_t arenaHighmark, const char* arenaName, const Color32 defaultCol, const Color32 baseCol, const Color32 highmarkCol, const f32 lineheight, const f32 textscale) {
                            const f32 barwidth = 150.f * textscale;
                            const f32 barheight = 10.f * textscale;

                            const ptrdiff_t arenaTotal = (ptrdiff_t)arenaEnd - (ptrdiff_t)arenaStart;
                            const ptrdiff_t arenaHighmarkBytes = (ptrdiff_t)arenaHighmark - (ptrdiff_t)arenaStart;
                            const f32 arenaHighmark_barwidth = barwidth * (arenaHighmarkBytes / (f32)arenaTotal);
                            textCfg.color = highmarkCol;
                            renderer::im::text2d(im, textCfg, "%s highmark: %lu bytes", arenaName, arenaHighmarkBytes);
                            textCfg.pos.y -= lineheight;
                            textCfg.color = defaultCol;

                            renderer::im::box_2d(im
                                , float2(textCfg.pos.x, textCfg.pos.y - barheight)
                                , float2(textCfg.pos.x + barwidth, textCfg.pos.y)
                                , baseCol);
                            if (arenaHighmarkBytes) {
                                renderer::im::box_2d(im
                                                          , float2(textCfg.pos.x, textCfg.pos.y - barheight)
                                                          , float2(textCfg.pos.x + arenaHighmark_barwidth, textCfg.pos.y)
                                                          , highmarkCol);
                            }
                            textCfg.pos.y -= barheight + 5.f * textscale;
                        };

                        const f32 barwidth = 150.f * textscale;
                        const f32 barheight = 10.f * textscale;
                        textParamsCenter.pos.x -= barwidth / 2.f;
                        {
                            const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                            renderArena(imCtx, textParamsCenter, game.memory.frameArena.end, game.memory.frameArenaBuffer, game.memory.frameArenaHighmark
                                , "Frame arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                            renderArena(imCtx, textParamsCenter, platform.memory.scratchArenaRoot.end, platform.memory.scratchArenaRoot.curr, platform.memory.scratchArenaHighmark
                                , "Platform scratch arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                            renderArena(imCtx, textParamsCenter, game.memory.persistentArena.end, game.memory.persistentArenaBuffer, (ptrdiff_t)game.memory.persistentArena.curr
                                , "Game persistent arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            auto& im = imCtx;
                            const Color32 baseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Color32 used3dCol(0.95f, 0.35f, 0.8f, 1.f);
                            const Color32 used2dCol(0.35f, 0.95f, 0.8f, 1.f);
                            const Color32 used2didxCol(0.8f, 0.95f, 0.8f, 1.f);

                            const ptrdiff_t memory_size = (ptrdiff_t)game.memory.imDebugArena.curr - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_3d_start = (ptrdiff_t)im.vertices_3d - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_3d_size = (ptrdiff_t)im.vertices_3d_head * sizeof(im::Vertex3D);
                            const ptrdiff_t vertices_2d_start = (ptrdiff_t)im.vertices_2d - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_2d_size = (ptrdiff_t)im.vertices_2d_head * sizeof(im::Vertex2D);
                            const ptrdiff_t indices_2d_start = (ptrdiff_t)im.indices_2d - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t indices_2d_size = (ptrdiff_t)(im.vertices_2d_head * 3 / 2) * sizeof(u32);
                            const f32 v3d_barstart = barwidth * vertices_3d_start / (f32)memory_size;
                            const f32 v3d_barwidth = barwidth * vertices_3d_size / (f32)memory_size;
                            const f32 v2d_barstart = barwidth * vertices_2d_start / (f32)memory_size;
                            const f32 v2d_barwidth = barwidth * vertices_2d_size / (f32)memory_size;
                            const f32 i2d_barstart = barwidth * indices_2d_start / (f32)memory_size;
                            const f32 i2d_barwidth = barwidth * indices_2d_size / (f32)memory_size;

                            textParamsCenter.color = used3dCol;
                            renderer::im::text2d(imCtx, textParamsCenter, "im 3d: %lu bytes", vertices_3d_size);
                            textParamsCenter.pos.y -= lineheight;
                            textParamsCenter.color = used2dCol;
                            renderer::im::text2d(imCtx, textParamsCenter, "im 2d: %lu bytes", vertices_2d_size);
                            textParamsCenter.pos.y -= lineheight;
                            textParamsCenter.color = used2didxCol;
                            renderer::im::text2d(imCtx, textParamsCenter, "im 2d indices: %lu bytes", indices_2d_size);
                            textParamsCenter.pos.y -= lineheight;
                            textParamsCenter.color = defaultCol;

                            renderer::im::box_2d(imCtx
                                , float2(textParamsCenter.pos.x, textParamsCenter.pos.y - barheight)
                                , float2(textParamsCenter.pos.x + barwidth, textParamsCenter.pos.y)
                                , baseCol);
                            renderer::im::box_2d(imCtx
                                , float2(textParamsCenter.pos.x + v3d_barstart, textParamsCenter.pos.y - barheight)
                                , float2(textParamsCenter.pos.x + v3d_barwidth, textParamsCenter.pos.y)
                                , used3dCol);
                            renderer::im::box_2d(imCtx
                                , float2(textParamsCenter.pos.x + v2d_barstart, textParamsCenter.pos.y - barheight)
                                , float2(textParamsCenter.pos.x + v2d_barstart + v2d_barwidth, textParamsCenter.pos.y)
                                , used2dCol);
                            renderer::im::box_2d(imCtx
                                , float2(textParamsCenter.pos.x + i2d_barstart, textParamsCenter.pos.y - barheight)
                                , float2(textParamsCenter.pos.x + i2d_barstart + i2d_barwidth, textParamsCenter.pos.y)
                                , used2didxCol);
                        }
                    }

                    // event label
                    if (debug::eventLabel.time != 0.f) {
                        f32 timeRunning = f32(platform.time.now - debug::eventLabel.time);
                        f32 t = timeRunning / 2.f;
                        if (t > 1.f) { debug::eventLabel.time = 0.f; }
                        else {
                            u8 oldScale = textParamsCenter.scale;
                            textParamsCenter.color = Color32(1.0f, 0.2f, 0.1f, 1.f - t);
                            textParamsCenter.scale *= 2;
                            textParamsCenter.pos.y = math::min(textParamsCenter.pos.y, 180.f);
                            renderer::im::text2d(imCtx, textParamsCenter, debug::eventLabel.text);
                            textParamsCenter.pos.y = -lineheight;
                            textParamsCenter.color = defaultCol;
                            textParamsCenter.scale = oldScale;
                        }
                    }
                }
            }
            #endif

            // copy backbuffer to window (upscale from game resolution to window resolution)
            {
                driver::Marker_t marker;
                driver::set_marker_name(marker, "UPSCALE TO WINDOW");
                renderer::driver::start_event(marker);
                {
                    {
                        renderer::driver::ViewportParams vpParams;
                        vpParams.topLeftX = 0;
                        vpParams.topLeftY = 0;
                        vpParams.width = (f32)platform.screen.window_width;
                        vpParams.height = (f32)platform.screen.window_height;
                        renderer::driver::set_VP(vpParams);
                    }

                    renderer::driver::bind_blend_state(store.blendStateBlendOff);
                    renderer::driver::bind_DS(store.depthStateOff);
                    renderer::driver::bind_RS(store.rasterizerStateFillFrontfaces);
                    renderer::driver::bind_main_RT(store.windowRT);

                    driver::bind_shader(store.shaders[renderer::ShaderType::FullscreenBlit]);
                    driver::bind_textures(&store.gameRT.textures[0], 1);
                    driver::draw_fullscreen();
                    renderer::driver::RscTexture nullTex = {};
                    driver::bind_textures(&nullTex, 1); // unbind gameRT
                }
                renderer::driver::end_event();
            }
            // Batched 2d debug (clear cpu buffers onto the screen)
            #if __DEBUG
            {
                renderer::driver::bind_blend_state(store.blendStateOn);
                renderer::driver::bind_main_RT(store.windowRT);
                im::commit2d(mgr.imCtx);
                im::present2d(mgr.imCtx, mgr.windowProjection.matrix);
            }
            #endif

        }
    }
}

#endif // __WASTELADNS_GAME_H__
