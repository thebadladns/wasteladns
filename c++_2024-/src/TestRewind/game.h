#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

#include "gameplay.h"
#include "shaders.h"
#include "game_renderer.h"
;
const u32 numCubes = 4;

namespace Game
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

    namespace Input
    {
        constexpr ::Input::Keyboard::Keys::Enum
              UP = ::Input::Keyboard::Keys::W
            , DOWN = ::Input::Keyboard::Keys::S
            , LEFT = ::Input::Keyboard::Keys::A
            , RIGHT = ::Input::Keyboard::Keys::D
            ;
        constexpr ::Input::Keyboard::Keys::Enum
              EXIT = ::Input::Keyboard::Keys::ESCAPE
            , TOGGLE_OVERLAY = ::Input::Keyboard::Keys::H
            , TOGGLE_DEBUG3D = ::Input::Keyboard::Keys::V
            , CAPTURE_CAMERAS = ::Input::Keyboard::Keys::P
            , TOGGLE_CAPTURED_CAMERA = ::Input::Keyboard::Keys::C
            ;
    };

    struct CameraManager {
        enum { FlyCam, CamCount };
        Camera cameras[CamCount];
        Gameplay::Orbit::State orbitCamera;
        Camera* activeCam;
    };

    struct RenderManager {
        Renderer::Store store;
        __DEBUGDEF(Renderer::Immediate::Buffer immediateBuffer;)
        Renderer::WindowProjection windowProjection;
        Renderer::PerspProjection perspProjection;
    };

    #if __DEBUG
    struct DebugVis {
        struct OverlayMode { enum Enum { All, HelpOnly, ArenaOnly, None, Count }; };
        struct Debug3DView { enum Enum { All, Culling, None, Count }; };
        struct DebugCameraStage { enum Enum { Scene, SceneMirror, SceneMirrorClipped, Count }; };

        f64 frameHistory[60];
        f64 frameAvg = 0;
        u64 frameHistoryIdx = 0;
        OverlayMode::Enum overlaymode = OverlayMode::Enum::All;
        Debug3DView::Enum debug3Dmode = Debug3DView::Enum::None;
        DebugCameraStage::Enum debugCameraStage = DebugCameraStage::Enum::Scene;
        float4x4 capturedCameras[DebugCameraStage::Count] = {};
    };
    #endif

    struct Memory {
        Allocator::Arena persistentArena;
        __DEBUGDEF(Allocator::Arena imDebugArena;)
        Allocator::Arena frameArena;
        u8* frameArenaBuffer; // used to reset Allocator::frameArena every frame
        __DEBUGDEF(u8* persistentArenaBuffer;) // used for debugging visualization
        __DEBUGDEF(uintptr_t frameArenaHighmark;)
    };

    struct Instance {
        Time time;
        CameraManager cameraMgr;
        RenderManager renderMgr;
        Gameplay::Movement::State player;
        Memory memory;
        __DEBUGDEF(DebugVis debugVis;)
    };

    void loadLaunchConfig(Platform::LaunchConfig& config) {
        // hardcoded for now
        config.window_width = 320 * 3;
        config.window_height = 240 * 3;
        config.game_width = 320 * 1;
        config.game_height = 240 * 1;
        config.fullscreen = false;
        config.title = "3D Test";
		config.scratchArena_size = 1 << 20; // 1MB
    }

    void start(Instance& game, Platform::GameConfig& config, const Platform::State& platform) {

        game.time = {};
        game.time.config.maxFrameLength = 0.1;
        game.time.config.targetFramerate = 1.0 / 60.0;
        game.time.lastFrame = platform.time.now;

        config = {};
        config.nextFrame = platform.time.now;

        {
            __DEBUGEXP(Allocator::init_arena(Allocator::emergencyArena, 1 << 20)); // 1MB
            Allocator::init_arena(game.memory.persistentArena, 1 << 20); // 1MB
            __DEBUGEXP(game.memory.persistentArenaBuffer = game.memory.persistentArena.curr);
            Allocator::init_arena(game.memory.frameArena, 1 << 20); // 1MB
            game.memory.frameArenaBuffer = game.memory.frameArena.curr;
            __DEBUGEXP(game.memory.frameArenaHighmark = (uintptr_t)game.memory.frameArena.curr; game.memory.frameArena.highmark = &game.memory.frameArenaHighmark);
            __DEBUGEXP(Allocator::init_arena(game.memory.imDebugArena, Renderer::Immediate::arena_size));
        }


#if READ_SHADERCACHE
        { // shader cache
            FILE* f;
            if (Platform::fopen(&f, "shaderCache.bin", "rb") == 0) {
                u64 count;
                fread(&count, sizeof(u64), 1, f);
                Renderer::Driver::shaderCache.shaderBytecode = (Renderer::Driver::ShaderBytecode*)malloc(sizeof(Renderer::Driver::ShaderBytecode) * count);
                for (u64 i = 0; i < count; i++) {
                    u64 size;
                    fread(&size, sizeof(u64), 1, f);
                    Renderer::Driver::ShaderBytecode& shader = Renderer::Driver::shaderCache.shaderBytecode[i];
                    shader.data = (u8*)malloc(sizeof(u8) * size);
                    shader.size = size;
                    fread(shader.data, sizeof(u8), size, f);
                }
                Platform::fclose(f);
            }
        }
#endif

        game.renderMgr = {};
        {
            RenderManager& mgr = game.renderMgr;
            using namespace Renderer;

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
            Renderer::init_pipelines(game.renderMgr.store, platform.memory.scratchArenaRoot, platform.screen);
            __DEBUGEXP(Renderer::Immediate::init(game.renderMgr.immediateBuffer, game.memory.imDebugArena));
        }

        game.player = {};
        {
            Math::identity4x4(game.player.transform);
            Renderer::NodeData& nodeData = Renderer::get_draw_node_core(game.renderMgr.store, game.renderMgr.store.playerDrawNodeHandle).nodeData;
            game.player.transform.pos = nodeData.worldMatrix.col3.xyz;
        }

        game.cameraMgr = {};
        {
            CameraManager& mgr = game.cameraMgr;
            Camera& cam = mgr.cameras[CameraManager::FlyCam];
            mgr.activeCam = &cam;

            mgr.orbitCamera.offset = float3(0.f, -100.f, 0.f);
            mgr.orbitCamera.eulers = float3(45.f * Math::d2r_f, 0.f, -25.f * Math::d2r_f);
            mgr.orbitCamera.scale = 1.f;
            mgr.orbitCamera.origin = float3(0.f, 0.f, 0.f);
        }

#if WRITE_SHADERCACHE
        { // shader cache
            FILE* f;
            if (Platform::fopen(&f, "shaderCache.bin", "wb") == 0) {
                fwrite(&Renderer::Driver::shaderCache.shaderBytecodeCount, sizeof(u64), 1, f);
                for (size_t i = 0; i < Renderer::Driver::shaderCache.shaderBytecodeCount; i++) {
                    fwrite(&Renderer::Driver::shaderCache.shaderBytecode[i].size, sizeof(u64), 1, f);
                    fwrite(Renderer::Driver::shaderCache.shaderBytecode[i].data, sizeof(u8), Renderer::Driver::shaderCache.shaderBytecode[i].size, f);
                }
                Platform::fclose(f);
            }
        }
#endif
#if WRITE_SHADERCACHE || READ_SHADERCACHE
		{ // shader cache cleanup
			for (u64 i = 0; i < Renderer::Driver::shaderCache.shaderBytecodeCount; i++) {
				free(Renderer::Driver::shaderCache.shaderBytecode[i].data);
			}
			free(Renderer::Driver::shaderCache.shaderBytecode);
        }
#endif

    }

    void update(Instance& game, Platform::GameConfig& config, Platform::State& platform) {

        // frame arena reset
        game.memory.frameArena.curr = game.memory.frameArenaBuffer;

        // frame timing calculations
        {
            f64 raw_dt = platform.time.now - game.time.lastFrame;
            game.time.lastFrameDelta = Math::min(raw_dt, game.time.config.maxFrameLength);
            game.time.lastFrame = platform.time.now;
            config.nextFrame = platform.time.now + game.time.config.targetFramerate;

            #if __DEBUG
            {
                f64 frameAvg = 0.;
                constexpr u64 frameHistoryCount = (sizeof(game.debugVis.frameHistory) / sizeof(game.debugVis.frameHistory[0]));
                game.debugVis.frameHistory[game.debugVis.frameHistoryIdx] = raw_dt;
                game.debugVis.frameHistoryIdx = (game.debugVis.frameHistoryIdx + 1) % frameHistoryCount;
                for (f64 dt : game.debugVis.frameHistory) {
                    frameAvg += dt;
                }
                // miscalculated until the queue is full, but that's just the first second
                frameAvg /= frameHistoryCount;
                game.debugVis.frameAvg = frameAvg;
            }
            #endif
        }

        // meta input checks
        const ::Input::Keyboard::State& keyboard = platform.input.keyboard;
        bool step = true;
        bool captureCameras = false;
        {
            if (keyboard.released(Input::EXIT)) {
                config.quit = true;
            }
            #if __DEBUG
            if (keyboard.pressed(Input::TOGGLE_OVERLAY)) { game.debugVis.overlaymode = (DebugVis::OverlayMode::Enum)((game.debugVis.overlaymode + 1) % DebugVis::OverlayMode::Count); }
            if (keyboard.pressed(Input::TOGGLE_DEBUG3D)) { game.debugVis.debug3Dmode = (DebugVis::Debug3DView::Enum)((game.debugVis.debug3Dmode + 1) % DebugVis::Debug3DView::Count); }
            if (keyboard.pressed(Input::TOGGLE_CAPTURED_CAMERA)) { game.debugVis.debugCameraStage = (DebugVis::DebugCameraStage::Enum)((game.debugVis.debugCameraStage + 1) % DebugVis::DebugCameraStage::Count); }
            if (keyboard.pressed(Input::CAPTURE_CAMERAS)) { captureCameras = true; }
            #endif      
            step = !game.time.paused;
        }

        if (step)
        {
            f32 dt = (f32)(game.time.lastFrameDelta * (1.0 + game.time.updateOverspeed));

            using namespace Game;

            Camera* activeCam = game.cameraMgr.activeCam;

            // player movement update
            {
                if (platform.input.padCount > 0) {
                    Gameplay::Movement::process(game.player.control, platform.input.pads[0]);
                }
                else {
                    Gameplay::Movement::process(game.player.control, keyboard, { Input::UP, Input::DOWN, Input::LEFT, Input::RIGHT });
                }
                Gameplay::Movement::process_cameraRelative(game.player.transform, game.player.movementController, game.player.control, activeCam->transform, dt);

                // update player render
                Renderer::NodeData& nodeData = Renderer::get_draw_node_core(game.renderMgr.store, game.renderMgr.store.playerDrawNodeHandle).nodeData;
                nodeData.worldMatrix = game.player.transform.matrix;

                { // animation hack
                    Animation::AnimatedNode& animatedData = Renderer::get_animatedNode(game.renderMgr.store, game.renderMgr.store.playerAnimatedNodeHandle);

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
                        const f32 scale = 1.f + Math::lerp(run_t, jerkyRunMinScale, jerkyRunMaxScale);
                        particle_scaley = Math::lerp(run_t, jerkyRunParticleMinScaley, jerkyRunParticleMaxScaley);
                        particle_scalez = Math::lerp(run_t, jerkyRunParticleMinScalez, jerkyRunParticleMaxScalez);
                        particle_offsety = 0.3f;
                        animatedData.state.scale = scale;
                        if (animatedData.state.animIndex != 1) {
                            animatedData.state.animIndex = 1;
                            animatedData.state.time = 0.f;
                        }
                    } else if (game.player.movementController.speed > 0.2f) {
                        const f32 run_t = (game.player.movementController.speed - runSpeedStart) / (jerkyRunSpeedStart - runSpeedStart);
                        const f32 scale = 1.f + Math::lerp(run_t, runMinScale, runMaxScale);
                        particle_scaley = Math::lerp(run_t, runParticleMinScaley, runParticleMaxScaley);
                        particle_scalez = Math::lerp(run_t, runParticleMinScalez, runParticleMaxScalez);
                        particle_offsetz = Math::lerp(run_t, runParticleMinOffsetz, runParticleMaxOffsetz);
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
                    Renderer::Matrices64* instance_matrices;
                    u32* instance_count;
                    Renderer::get_draw_instanced_data(instance_matrices, instance_count, game.renderMgr.store, game.renderMgr.store.particlesDrawHandle);
                    for (u32 i = 0; i < *instance_count; i++) {

                        const f32 scaley = particle_scaley;
                        const f32 scalez = particle_scalez;
                        const f32 offsety = particle_offsety;
                        const f32 offsetz = particle_offsetz;
                        const f32 scale = particle_totalscale;

                        Transform t;
                        Math::identity4x4(t);
                        t.pos = Math::add(game.player.transform.pos, Math::scale(game.player.transform.front, offsety + scaley * (i + 2)));
                        t.pos.z = t.pos.z + offsetz + scalez * (Math::cos((f32)platform.time.now * 10.f + i * 2.f) - 1.5f);
                        t.matrix.col0.x = 0.2f;
                        t.matrix.col1.y = 0.2f;
                        t.matrix.col2.z = 0.2f;
                        t.matrix.col0 = Math::scale(t.matrix.col0, scale);
                        t.matrix.col1 = Math::scale(t.matrix.col1, scale);
                        t.matrix.col2 = Math::scale(t.matrix.col2, scale);

                        instance_matrices->data[i] = t.matrix;
                    }
                }

                // anim update
                {
                    Renderer::Store& store = game.renderMgr.store;
                    for (u32 n = 0, count = 0; n < store.animatedNodes.cap && count < store.animatedNodes.count; n++) {
                        if (store.animatedNodes.data[n].alive == 0) { continue; }
                        count++;

                        Animation::AnimatedNode& animatedData = store.animatedNodes.data[n].state.live;
                        Renderer::Matrices32& skinningData = Renderer::get_draw_skinning_data(game.renderMgr.store, animatedData.drawNodeHandle );
                        Animation::updateAnimation(skinningData.data, animatedData.state, animatedData.clips[animatedData.state.animIndex], animatedData.skeleton, dt);
                    }
                }
            }

            // camera update
            {
                if (platform.input.padCount > 0) {
                    Gameplay::Orbit::process(game.cameraMgr.orbitCamera, platform.input.pads[0]);
                } else {
                    Gameplay::Orbit::process(game.cameraMgr.orbitCamera, platform.input.mouse);
                }
                Gameplay::Orbit::process(activeCam->transform, game.cameraMgr.orbitCamera);
                // send camera data to render manager
                Renderer::generate_MV_matrix(activeCam->viewMatrix, activeCam->transform);
            }
        }

        //Render update
        Renderer::VisibleNodes* visibleNodes, * mirrorVisibleNodes; // todo: move these two somewhere?
        {
            RenderManager& mgr = game.renderMgr;
            Renderer::Store& store = mgr.store;
            using namespace Renderer;

            Camera* activeCam = game.cameraMgr.activeCam;
            float4x4 viewMatrix = activeCam->viewMatrix;
            float4x4 mirrorViewMatrix, mirrorProjectionMatrix, mirrorVPMatrix;

            // perspective cam set up
            float4x4 vpMatrix;
            Driver::RscCBuffer& scene_cbuffer = store.cbuffers[store.cbuffer_scene];
            Renderer::SceneData cbufferPerScene;
            {
                cbufferPerScene.projectionMatrix = mgr.perspProjection.matrix;
                cbufferPerScene.viewMatrix = viewMatrix;
                cbufferPerScene.viewPos = viewMatrix.col3.xyz;
                cbufferPerScene.lightPos = float3(3.f, 8.f, 15.f);
                Renderer::Driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);
                vpMatrix = Math::mult(mgr.perspProjection.matrix, viewMatrix);
                __DEBUGEXP(if (captureCameras) { game.debugVis.capturedCameras[DebugVis::DebugCameraStage::Scene] = vpMatrix; });
            }

            {
                {
                    Driver::ViewportParams vpParams;
                    vpParams.topLeftX = 0;
                    vpParams.topLeftY = 0;
                    vpParams.width = (f32)platform.screen.width;
                    vpParams.height = (f32)platform.screen.height;
                    vpParams.minDepth = 0.f;
                    vpParams.maxDepth = 1.f;
                    Driver::set_VP(vpParams);
                }
            
                // cull nodes
                {
                    {
                        visibleNodes = (VisibleNodes*)Allocator::alloc_arena(game.memory.frameArena, sizeof(VisibleNodes), alignof(VisibleNodes));
                        memset(visibleNodes, 0, sizeof(VisibleNodes));
                        computeVisibility(*visibleNodes, vpMatrix, store);
                    }

                    // set up mirror scene if mirror is visible
                    mirrorVisibleNodes = (VisibleNodes*)Allocator::alloc_arena(game.memory.frameArena, sizeof(VisibleNodes), alignof(VisibleNodes));
                    memset(mirrorVisibleNodes, 0, sizeof(VisibleNodes));
                    for (u32 i = 0; i < visibleNodes->visible_nodes_count; i++) {
                        u32 n = visibleNodes->visible_nodes[i];
                        DrawNode& node = store.drawNodes.data[n].state.live;

                        if (store.mirror.drawHandle != handle_from_node(store, node)) { // todo: improve?

                            // todo: investigate reflection at y = y
                            auto reflectionMatrix = [](float4 p) -> float4x4 { // todo: understand properly
                                float4x4 o;
                                o.col0.x = 1 - 2.f * p.x * p.x; o.col1.x = -2.f * p.x * p.y;    o.col2.x = -2.f * p.x * p.z;        o.col3.x = -2.f * p.x * p.w;
                                o.col0.y = -2.f * p.y * p.x;    o.col1.y = 1 - 2.f * p.y * p.y; o.col2.y = -2.f * p.y * p.z;        o.col3.y = -2.f * p.y * p.w;
                                o.col0.z = -2.f * p.z * p.x;    o.col1.z = -2.f * p.z * p.y;    o.col2.z = 1.f - 2.f * p.z * p.z;   o.col3.z = -2.f * p.z * p.w;
                                o.col0.w = 0.f;                 o.col1.w = 0.f;                 o.col2.w = 0.f;                     o.col3.w = 1.f;
                                return o;
                            };
                            
                            float4 plane { store.mirror.normal.x, store.mirror.normal.y, store.mirror.normal.z, -Math::dot(store.mirror.pos, store.mirror.normal) };
                            float4x4 reflect = reflectionMatrix(plane);
                            mirrorViewMatrix = Math::mult(viewMatrix, reflect);
                            mirrorProjectionMatrix = mgr.perspProjection.matrix;
                            __DEBUGEXP(if (captureCameras) { game.debugVis.capturedCameras[DebugVis::DebugCameraStage::SceneMirror] = Math::mult(mirrorProjectionMatrix, mirrorViewMatrix); });
                            float3 normalCameraSpace = Math::mult(mirrorViewMatrix, float4(store.mirror.normal, 0.f)).xyz;
                            float3 posCameraSpace = Math::mult(mirrorViewMatrix, float4(store.mirror.pos, 1.f)).xyz;
                            float4 pCameraSpace{ normalCameraSpace.x, normalCameraSpace.y, normalCameraSpace.z, -Math::dot(posCameraSpace, normalCameraSpace) };
                            add_oblique_plane_to_persp(mirrorProjectionMatrix, pCameraSpace);
                            mirrorVPMatrix = Math::mult(mirrorProjectionMatrix, mirrorViewMatrix);
                            __DEBUGEXP(if (captureCameras) { game.debugVis.capturedCameras[DebugVis::DebugCameraStage::SceneMirrorClipped] = mirrorVPMatrix; });
                            computeVisibility(*mirrorVisibleNodes, mirrorVPMatrix, store);

                            break;
                        }
                    }
                }
                // update visible nodes
                {
                    // figure out which nodes are visible among all of the visibility lists
                    Allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                    bool* nodesToUpdate = (bool*)Allocator::alloc_arena(scratchArena, store.drawNodes.count * sizeof(bool), alignof(bool));
                    memset(nodesToUpdate, 0, store.drawNodes.count * sizeof(bool));
                    for (u32 i = 0; i < visibleNodes->visible_nodes_count; i++) {
                        u32 n = visibleNodes->visible_nodes[i];
                        u32 index = Allocator::get_pool_index(store.drawNodes, store.drawNodes.data[n].state.live);
                        nodesToUpdate[index] = true;
                    }
                    bool* nodesSkinnedToUpdate = (bool*)Allocator::alloc_arena(scratchArena, store.drawNodesSkinned.count * sizeof(bool), alignof(bool));
                    memset(nodesSkinnedToUpdate, 0, store.drawNodesSkinned.count * sizeof(bool));
                    for (u32 i = 0; i < visibleNodes->visible_nodes_skinned_count; i++) {
                        u32 n = visibleNodes->visible_nodes_skinned[i];
                        u32 index = Allocator::get_pool_index(store.drawNodesSkinned, store.drawNodesSkinned.data[n].state.live);
                        nodesSkinnedToUpdate[index] = true;
                    }
                    for (u32 i = 0; i < mirrorVisibleNodes->visible_nodes_count; i++) {
                        u32 n = mirrorVisibleNodes->visible_nodes[i];
                        u32 index = Allocator::get_pool_index(store.drawNodes, store.drawNodes.data[n].state.live);
                        nodesToUpdate[index] = true;
                    }
                    for (u32 i = 0; i < mirrorVisibleNodes->visible_nodes_skinned_count; i++) {
                        u32 n = mirrorVisibleNodes->visible_nodes_skinned[i];
                        u32 index = Allocator::get_pool_index(store.drawNodesSkinned, store.drawNodesSkinned.data[n].state.live);
                        nodesSkinnedToUpdate[index] = true;
                    }

                    // update cbuffers for all visible nodes
                    for (u32 n = 0, count = 0; n < store.drawNodes.cap && count < store.drawNodes.count; n++) {
                        if (store.drawNodes.data[n].alive == 0) { continue; }
                        count++;
                        const DrawNode& node = store.drawNodes.data[n].state.live;
                        if (!nodesToUpdate[n]) continue;
                        Driver::update_cbuffer(store.cbuffers[node.cbuffer_node], &node.nodeData);
                    }
                    for (u32 n = 0, count = 0; n < store.drawNodesSkinned.cap && count < store.drawNodesSkinned.count; n++) {
                        if (store.drawNodesSkinned.data[n].alive == 0) { continue; }
                        count++;
                        const DrawNodeSkinned& node = store.drawNodesSkinned.data[n].state.live;
                        if (!nodesSkinnedToUpdate[n]) continue;
                        Driver::update_cbuffer(store.cbuffers[node.core.cbuffer_node], &node.core.nodeData);
                        Driver::update_cbuffer(store.cbuffers[node.cbuffer_skinning], &node.skinningMatrixPalette);
                    }
                    for (u32 n = 0, count = 0; n < store.drawNodesInstanced.cap && count < store.drawNodesInstanced.count; n++) {
                        if (store.drawNodesInstanced.data[n].alive == 0) { continue; }
                        count++;
                        const DrawNodeInstanced& node = store.drawNodesInstanced.data[n].state.live;
                        Driver::update_cbuffer(store.cbuffers[node.core.cbuffer_node], &node.core.nodeData);
                        Driver::update_cbuffer(store.cbuffers[node.cbuffer_instances], &node.instanceMatrices);
                    }
                }

                Driver::Marker_t marker;
                Driver::set_marker_name(marker, "BASE SCENE"); Driver::start_event(marker);
                {
                    Driver::bind_RT(store.gameRT);

                    Driver::clear_RT(store.gameRT, Driver::RenderTargetClearFlags::Color | Driver::RenderTargetClearFlags::Depth | Driver::RenderTargetClearFlags::Stencil);
                    Driver::bind_RS(store.rasterizerStateFillFrontfaces);

                    Driver::set_marker_name(marker, "SKY"); Driver::start_event(marker); // todo: quad?
                    {
                        Driver::bind_DS(store.depthStateOff);
                        Driver::bind_shader(store.shaders[ShaderType::Color3D]);
                        Driver::bind_blend_state(store.blendStateBlendOff);
                        for (u32 i = 0; i < COUNT_OF(store.sky.buffers); i++) {
                            Driver::bind_indexed_vertex_buffer(store.sky.buffers[i]);
                            Driver::RscCBuffer buffers[] = { scene_cbuffer, store.sky.cbuffer };
                            Driver::bind_cbuffers(store.shaders[ShaderType::Color3D], buffers, 2);
                            Driver::draw_indexed_vertex_buffer(store.sky.buffers[i]);
                        }
                    }
                    Driver::end_event();

                    Driver::set_marker_name(marker, "OPAQUE"); Driver::start_event(marker);
                    {
                        Driver::bind_DS(store.depthStateOn);
                        Allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                        Drawlist dl = {};
                        u32 maxDrawCallsThisFrame = (visibleNodes->visible_nodes_count + visibleNodes->visible_nodes_skinned_count + (u32)store.drawNodesInstanced.count) * DrawlistStreams::Count;
                        dl.items = (DrawCall_Item*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                        dl.keys = (Key*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));
                        addNodesToDrawlistSorted(dl, *visibleNodes, game.cameraMgr.activeCam->transform.pos, store, 0, Renderer::DrawlistFilter::Alpha | Renderer::DrawlistFilter::Mirror, Renderer::SortParams::Type::Default);
                        Drawlist_Context ctx = {};
                        Drawlist_Overrides overrides = {};
                        ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                        draw_drawlist(dl, ctx, overrides);
                    }
                    Driver::end_event();

                    Driver::set_marker_name(marker, "ALPHA"); Driver::start_event(marker);
                    {
                        Driver::bind_DS(store.depthStateReadOnly);

                        Allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                        Drawlist dl = {};
                        u32 maxDrawCallsThisFrame = (visibleNodes->visible_nodes_count + visibleNodes->visible_nodes_skinned_count + (u32)store.drawNodesInstanced.count) * DrawlistStreams::Count;
                        dl.items = (DrawCall_Item*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                        dl.keys = (Key*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));
                        Driver::bind_blend_state(store.blendStateOn);
                        addNodesToDrawlistSorted(dl, *visibleNodes, game.cameraMgr.activeCam->transform.pos, store, Renderer::DrawlistFilter::Alpha, Renderer::DrawlistFilter::Mirror, Renderer::SortParams::Type::BackToFront);
                        Drawlist_Context ctx = {};
                        Drawlist_Overrides overrides = {};
                        overrides.forced_blendState = true;
                        ctx.blendState = &store.blendStateOn;
                        ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                        draw_drawlist(dl, ctx, overrides);
                    }
                    Driver::end_event();
                }
                Driver::end_event();
                
                // Mirror passes
                if (mirrorVisibleNodes->visible_nodes_count + mirrorVisibleNodes->visible_nodes_skinned_count) {

                    Driver::set_marker_name(marker, "MARK MIRROR"); Driver::start_event(marker);
                    {
                        Allocator::Arena scratchArena = platform.memory.scratchArenaRoot;

                        Driver::bind_DS(store.depthStateMarkMirrors);
                        Driver::bind_RS(store.rasterizerStateFillFrontfaces);
                        Driver::bind_blend_state(store.blendStateOff);
                        Drawlist_Context ctx = {};
                        Drawlist_Overrides overrides = {};
                        overrides.forced_blendState = true;
                        ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                        ctx.blendState = &store.blendStateOff;

                        Drawlist dl = {};
                        u32 maxDrawCallsThisFrame = DrawlistStreams::Count; // support for one single mirror
                        dl.items = (DrawCall_Item*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                        dl.keys = (Key*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));

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
                            item.name = Renderer::shaderNames[mesh.type];
                        }
                        draw_drawlist(dl, ctx, overrides);
                    }
                    Driver::end_event();

                    Driver::set_marker_name(marker, "REFLECTION SCENE"); Driver::start_event(marker);
                    {
                        Driver::RscCBuffer& scene_cbuffer = store.cbuffers[store.cbuffer_scene];
                        {
                            Renderer::SceneData cbufferPerSceneMirrored = cbufferPerScene;
                            cbufferPerSceneMirrored.projectionMatrix = mirrorProjectionMatrix;
                            cbufferPerSceneMirrored.viewMatrix = mirrorViewMatrix;
                            cbufferPerSceneMirrored.viewPos = mirrorViewMatrix.col3.xyz;
                            Renderer::Driver::update_cbuffer(scene_cbuffer, &cbufferPerSceneMirrored);
                        }

                        Driver::clear_RT(store.gameRT, Driver::RenderTargetClearFlags::Depth); // todo: can we get away without doing this for all the depth buffer?

                        Driver::bind_RS(store.rasterizerStateFillBackfaces);

                        Driver::set_marker_name(marker, "SKY"); Driver::start_event(marker);
                        {
                            Driver::bind_DS(store.depthStateMirrorReflectionsDepthReadOnly);
                            Driver::bind_shader(store.shaders[ShaderType::Color3D]);
                            Driver::bind_blend_state(store.blendStateBlendOff);
                            for (u32 i = 0; i < COUNT_OF(store.sky.buffers); i++) {
                                Driver::bind_indexed_vertex_buffer(store.sky.buffers[i]);
                                Driver::RscCBuffer buffers[] = { scene_cbuffer, store.sky.cbuffer };
                                Driver::bind_cbuffers(store.shaders[ShaderType::Color3D], buffers, 2);
                                Driver::draw_indexed_vertex_buffer(store.sky.buffers[i]);
                            }
                        }
                        Driver::end_event();

                        Driver::set_marker_name(marker, "OPAQUE"); Driver::start_event(marker);
                        {
                            Driver::bind_DS(store.depthStateMirrorReflections);
                            Allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                            Drawlist dl = {};
                            u32 maxDrawCallsThisFrame = (mirrorVisibleNodes->visible_nodes_count + mirrorVisibleNodes->visible_nodes_skinned_count + (u32)store.drawNodesInstanced.count) * DrawlistStreams::Count;
                            dl.items = (DrawCall_Item*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                            dl.keys = (Key*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));
                            addNodesToDrawlistSorted(dl, *mirrorVisibleNodes, game.cameraMgr.activeCam->transform.pos, store, 0, Renderer::DrawlistFilter::Alpha, Renderer::SortParams::Type::Default);
                            Drawlist_Context ctx = {};
                            Drawlist_Overrides overrides = {};
                            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                            draw_drawlist(dl, ctx, overrides);
                        }
                        Driver::end_event();

                        Driver::set_marker_name(marker, "ALPHA"); Driver::start_event(marker);
                        {
                            Driver::bind_DS(store.depthStateMirrorReflectionsDepthReadOnly);
                            Driver::bind_blend_state(store.blendStateOn);

                            Allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                            Drawlist dl = {};
                            u32 maxDrawCallsThisFrame = (mirrorVisibleNodes->visible_nodes_count + mirrorVisibleNodes->visible_nodes_skinned_count + (u32)store.drawNodesInstanced.count) * DrawlistStreams::Count;
                            dl.items = (DrawCall_Item*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                            dl.keys = (Key*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));
                            addNodesToDrawlistSorted(dl, *mirrorVisibleNodes, game.cameraMgr.activeCam->transform.pos, store, Renderer::DrawlistFilter::Alpha, 0, Renderer::SortParams::Type::BackToFront);
                            Drawlist_Context ctx = {};
                            Drawlist_Overrides overrides = {};
                            overrides.forced_blendState = true;
                            ctx.blendState = &store.blendStateOn;
                            ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                            draw_drawlist(dl, ctx, overrides);
                        }
                        Driver::end_event();
                    }
                    Driver::end_event();

                    Driver::set_marker_name(marker, "MIRROR IN SCENE"); Driver::start_event(marker);
                    {
                        Driver::RscCBuffer& scene_cbuffer = store.cbuffers[store.cbuffer_scene];
                        {
                            Renderer::SceneData cbufferPerScene;
                            cbufferPerScene.projectionMatrix = mgr.perspProjection.matrix;
                            cbufferPerScene.viewMatrix = viewMatrix;
                            cbufferPerScene.viewPos = viewMatrix.col3.xyz;
                            cbufferPerScene.lightPos = float3(3.f, 8.f, 15.f); // todo: save off somewhere?
                            Renderer::Driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);
                        }

                        Driver::bind_DS(store.depthStateOn); // todo: this is a hack so opengl can clear depth
                        Driver::clear_RT(store.gameRT, Driver::RenderTargetClearFlags::Depth); // todo: could we just clear the mirror?

                        Driver::bind_DS(store.depthStateMirrorReflectionsDepthReadOnly);
                        Driver::bind_RS(store.rasterizerStateFillFrontfaces);
                        Driver::bind_blend_state(store.blendStateOn);
                        Drawlist_Context ctx = {};
                        Drawlist_Overrides overrides = {};
                        overrides.forced_blendState = true;
                        ctx.cbuffers[overrides.forced_cbuffer_count++] = scene_cbuffer;
                        ctx.blendState = &store.blendStateOn;

                        Allocator::Arena scratchArena = platform.memory.scratchArenaRoot;
                        Drawlist dl = {};
                        u32 maxDrawCallsThisFrame = DrawlistStreams::Count; // support for one single mirror
                        dl.items = (DrawCall_Item*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(DrawCall_Item), alignof(DrawCall_Item));
                        dl.keys = (Key*)Allocator::alloc_arena(scratchArena, maxDrawCallsThisFrame * sizeof(Key), alignof(Key));

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
                            item.name = Renderer::shaderNames[mesh.type];
                        }
                        draw_drawlist(dl, ctx, overrides);
                    }
                    Driver::end_event();
                }
            }

            // Immediate-mode debug. Can be moved out of the render update, it only pushes data to cpu buffers
            #if __DEBUG
            {
                // World axis
                //{
                //    const Color32 axisX(0.8f, 0.15f, 0.25f, 0.7f); // x is red
                //    const Color32 axisY(0.25f, 0.8f, 0.15f, 0.7f); // y is green
                //    const Color32 axisZ(0.15f, 0.25f, 0.8f, 0.7f); // z is blue
                //    const f32 axisSize = 30.f;
                //    const float3 pos(0.f, 0.f, 0.1f);

                //    Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(float3(1.f, 0.f, 0.f), axisSize)), axisX);
                //    Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(float3(0.f, 1.f, 0.f), axisSize)), axisY);
                //    Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(float3(0.f, 0.f, 1.f), axisSize)), axisZ);

                //    const f32 thickness = 0.1f;
                //    const u32 steps = 10;
                //    const f32 spacing = 1.f / (f32)steps;
                //    for (int i = 1; i <= steps; i++) {
                //        Immediate::segment(mgr.immediateBuffer,
                //              Math::add(pos, float3(spacing * i, 0.f, -thickness))
                //            , Math::add(pos, float3(spacing * i, 0.f, thickness))
                //            , axisX);
                //        Immediate::segment(mgr.immediateBuffer,
                //              Math::add(pos, float3(0.f, spacing * i, -thickness))
                //            , Math::add(pos, float3(0.f, spacing * i, thickness))
                //            , axisY);
                //        Immediate::segment(mgr.immediateBuffer,
                //              Math::add(pos, float3(-thickness, thickness, spacing * i))
                //            , Math::add(pos, float3(thickness, -thickness, spacing * i))
                //            , axisZ);
                //    }
                //}
                {
                    if (game.debugVis.debug3Dmode == DebugVis::Debug3DView::All || game.debugVis.debug3Dmode == DebugVis::Debug3DView::Culling) {
                        const Color32 bbColor(0.8f, 0.15f, 0.25f, 0.7f);

                        Renderer::VisibleNodes visibleNodesDebug = {};
                        float4x4 matrix = game.debugVis.capturedCameras[game.debugVis.debugCameraStage];
                        if (!Math::isZeroAll(matrix)) {
                            Renderer::computeVisibility(visibleNodesDebug, matrix, store);
                            for (u32 i = 0; i < visibleNodesDebug.visible_nodes_count; i++) {
                                u32 n = visibleNodesDebug.visible_nodes[i];
                                const DrawNode& node = store.drawNodes.data[n].state.live;
                                Immediate::obb(mgr.immediateBuffer, node.nodeData.worldMatrix, node.min, node.max, bbColor);
                            }
                            for (u32 i = 0; i < visibleNodesDebug.visible_nodes_skinned_count; i++) {
                                u32 n = visibleNodesDebug.visible_nodes_skinned[i];
                                const DrawNodeSkinned& node = store.drawNodesSkinned.data[n].state.live;
                                Immediate::obb(mgr.immediateBuffer, node.core.nodeData.worldMatrix, node.core.min, node.core.max, bbColor);
                            }

                            const Color32 frustum_color(0.25f, 0.8f, 0.15f, 0.7f);
                            Math::inverse(matrix);
                            Immediate::obb(mgr.immediateBuffer, matrix, float3(-1.f, -1.f, Renderer::min_z), float3(1.f, 1.f, 1.f), frustum_color);
                        }
                    }
                }
                // 2d debug info
                if (game.debugVis.overlaymode != DebugVis::OverlayMode::None)
                {
                    Color32 defaultCol(0.7f, 0.8f, 0.15f, 1.0f);
                    Color32 activeCol(1.0f, 0.2f, 0.1f, 1.0f);
                    
                    f32 textscale = platform.screen.text_scale;
                    f32 lineheight = 15.f * textscale;

                    Renderer::Immediate::TextParams textParamsLeft, textParamsRight, textParamsCenter;
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
                    textParamsLeft.color = keyboard.pressed(Input::TOGGLE_OVERLAY) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "H to toggle overlays: %s", overlaynames[game.debugVis.overlaymode]);
                    textParamsLeft.pos.y -= lineheight;
                    const char* visualizationModes[] = { "All", "Culling", "None" };
                    textParamsLeft.color = keyboard.pressed(Input::TOGGLE_DEBUG3D) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "V to toggle 3D visualization modes: %s", visualizationModes[game.debugVis.debug3Dmode]);
                    textParamsLeft.pos.y -= lineheight;
                    const char* cameraStages[] = { "Scene", "Scene Mirror", "Scene Mirror Clipped" };
                    textParamsLeft.color = keyboard.pressed(Input::TOGGLE_CAPTURED_CAMERA) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "C to toggle camera capture stages: %s %s", cameraStages[game.debugVis.debugCameraStage], Math::isZeroAll(game.debugVis.capturedCameras[game.debugVis.debugCameraStage]) ? "Camera not captured" : "Camera Captured");
                    textParamsLeft.pos.y -= lineheight;

                    if (game.debugVis.overlaymode == DebugVis::OverlayMode::All || game.debugVis.overlaymode == DebugVis::OverlayMode::HelpOnly) {
                        textParamsLeft.color = platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_LEFT) ? activeCol : defaultCol;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "Mouse (%.3f,%.3f)", platform.input.mouse.x, platform.input.mouse.y);
                        textParamsLeft.pos.y -= lineheight;
                        textParamsLeft.color = platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_LEFT) ? activeCol : defaultCol;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "   left click and drag to orbit");
                        textParamsLeft.pos.y -= lineheight;
                        textParamsLeft.color = platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_RIGHT) ? activeCol : defaultCol;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "   right click and drag to pan");
                        textParamsLeft.pos.y -= lineheight;
                        textParamsLeft.color = platform.input.mouse.scrolldy != 0 ? activeCol : defaultCol;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "   mouse wheel to scale");
                        textParamsLeft.pos.y -= lineheight;
                        textParamsLeft.color = defaultCol;
                        {
                            float3 eulers_deg = Math::scale(game.cameraMgr.orbitCamera.eulers, Math::r2d_f);
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "Camera eulers: " float3_FORMAT("% .3f"), float3_PARAMS(eulers_deg));
                            textParamsLeft.pos.y -= lineheight;
                        }
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "mouse wheel to scale");
                        textParamsLeft.pos.y -= lineheight;
                        for (u32 i = 0; i < platform.input.padCount; i++)
                        {
                            namespace Pad = ::Input::Gamepad;
                            const Pad::State& pad = platform.input.pads[i];
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "Pad: %s", pad.name);
                            textParamsLeft.pos.y -= lineheight;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft
                                , "Pad: L:(%.3f,%.3f) R:(%.3f,%.3f) L2:%.3f R2:%.3f"
                                , pad.sliders[Pad::Sliders::AXIS_X_LEFT], pad.sliders[Pad::Sliders::AXIS_Y_LEFT]
                                , pad.sliders[Pad::Sliders::AXIS_X_RIGHT], pad.sliders[Pad::Sliders::AXIS_Y_RIGHT]
                                , pad.sliders[Pad::Sliders::TRIGGER_LEFT], pad.sliders[Pad::Sliders::TRIGGER_RIGHT]
                            );
                            char keys_str[128];
                            textParamsLeft.pos.y -= lineheight;
                            char* curr = keys_str;
                            char* last = keys_str + sizeof(keys_str);
                            curr += Platform::format(keys_str, (int)(last-curr), "Keys: ");
                            const char* key_names[] = {
                                "BUTTON_N", "BUTTON_S", "BUTTON_W", "BUTTON_E"
                              , "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT"
                              , "START", "SELECT"
                              , "L1", "R1", "L2", "R2"
                              , "LEFT_THUMB", "RIGHT_THUMB"
                            };
                            for (u32 key = 0; key < Pad::KeyMask::COUNT && curr < last; key++) {
                                if (pad.down((Pad::KeyMask::Enum)(1<<key))) {
                                    curr += Platform::format(curr, (int)(last - curr), "%s ", key_names[key]);
                                } else {
                                    curr += Platform::format(curr, (int)(last - curr), "");
                                }
                            }
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, keys_str);
                            textParamsLeft.pos.y -= lineheight;
                        }

                        textParamsRight.color = defaultCol;
                        textParamsRight.pos.x -= 30.f * textscale;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsRight, "%s", Platform::name);
                        textParamsRight.pos.y -= lineheight;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsRight, "%.3lf fps", 1. / game.debugVis.frameAvg);
                        textParamsRight.pos.y -= lineheight;
                    }

                    if (game.debugVis.overlaymode == DebugVis::OverlayMode::All || game.debugVis.overlaymode == DebugVis::OverlayMode::ArenaOnly)
                    {
                        auto renderArena = [](Renderer::Immediate::Buffer& im, Renderer::Immediate::TextParams& textCfg, u8* arenaEnd, u8* arenaStart, uintptr_t arenaHighmark, const char* arenaName, const Color32 defaultCol, const Color32 baseCol, const Color32 highmarkCol, const f32 lineheight, const f32 textscale) {
                            const f32 barwidth = 150.f * textscale;
                            const f32 barheight = 10.f * textscale;

                            const ptrdiff_t arenaTotal = (ptrdiff_t)arenaEnd - (ptrdiff_t)arenaStart;
                            const ptrdiff_t arenaHighmarkBytes = (ptrdiff_t)arenaHighmark - (ptrdiff_t)arenaStart;
                            const f32 arenaHighmark_barwidth = barwidth * (arenaHighmarkBytes / (f32)arenaTotal);
                            textCfg.color = highmarkCol;
                            Renderer::Immediate::text2d(im, textCfg, "%s highmark: %lu bytes", arenaName, arenaHighmarkBytes);
                            textCfg.pos.y -= lineheight;
                            textCfg.color = defaultCol;

                            Renderer::Immediate::box_2d(im
                                , float2(textCfg.pos.x, textCfg.pos.y - barheight)
                                , float2(textCfg.pos.x + barwidth, textCfg.pos.y)
                                , baseCol);
                            if (arenaHighmarkBytes) {
                                Renderer::Immediate::box_2d(im
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
                            renderArena(game.renderMgr.immediateBuffer, textParamsCenter, game.memory.frameArena.end, game.memory.frameArenaBuffer, game.memory.frameArenaHighmark
                                , "Frame arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                            renderArena(game.renderMgr.immediateBuffer, textParamsCenter, platform.memory.scratchArenaRoot.end, platform.memory.scratchArenaRoot.curr, platform.memory.scratchArenaHighmark
                                , "Platform scratch arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                            renderArena(game.renderMgr.immediateBuffer, textParamsCenter, game.memory.persistentArena.end, game.memory.persistentArenaBuffer, (ptrdiff_t)game.memory.persistentArena.curr
                                , "Game persistent arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            auto& im = game.renderMgr.immediateBuffer;
                            const Color32 baseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Color32 used3dCol(0.95f, 0.35f, 0.8f, 1.f);
                            const Color32 used2dCol(0.35f, 0.95f, 0.8f, 1.f);
                            const Color32 used2didxCol(0.8f, 0.95f, 0.8f, 1.f);

                            const ptrdiff_t memory_size = (ptrdiff_t)game.memory.imDebugArena.curr - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_3d_start = (ptrdiff_t)im.vertices_3d - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_3d_size = (ptrdiff_t)im.vertices_3d_head * sizeof(Immediate::Vertex3D);
                            const ptrdiff_t vertices_2d_start = (ptrdiff_t)im.vertices_2d - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_2d_size = (ptrdiff_t)im.vertices_2d_head * sizeof(Immediate::Vertex2D);
                            const ptrdiff_t indices_2d_start = (ptrdiff_t)im.indices_2d - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t indices_2d_size = (ptrdiff_t)(im.vertices_2d_head * 3 / 2) * sizeof(u32);
                            const f32 v3d_barstart = barwidth * vertices_3d_start / (f32)memory_size;
                            const f32 v3d_barwidth = barwidth * vertices_3d_size / (f32)memory_size;
                            const f32 v2d_barstart = barwidth * vertices_2d_start / (f32)memory_size;
                            const f32 v2d_barwidth = barwidth * vertices_2d_size / (f32)memory_size;
                            const f32 i2d_barstart = barwidth * indices_2d_start / (f32)memory_size;
                            const f32 i2d_barwidth = barwidth * indices_2d_size / (f32)memory_size;

                            textParamsCenter.color = used3dCol;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "im 3d: %lu bytes", vertices_3d_size);
                            textParamsCenter.pos.y -= lineheight;
                            textParamsCenter.color = used2dCol;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "im 2d: %lu bytes", vertices_2d_size);
                            textParamsCenter.pos.y -= lineheight;
                            textParamsCenter.color = used2didxCol;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "im 2d indices: %lu bytes", indices_2d_size);
                            textParamsCenter.pos.y -= lineheight;
                            textParamsCenter.color = defaultCol;

                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , float2(textParamsCenter.pos.x, textParamsCenter.pos.y - barheight)
                                , float2(textParamsCenter.pos.x + barwidth, textParamsCenter.pos.y)
                                , baseCol);
                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , float2(textParamsCenter.pos.x + v3d_barstart, textParamsCenter.pos.y - barheight)
                                , float2(textParamsCenter.pos.x + v3d_barwidth, textParamsCenter.pos.y)
                                , used3dCol);
                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , float2(textParamsCenter.pos.x + v2d_barstart, textParamsCenter.pos.y - barheight)
                                , float2(textParamsCenter.pos.x + v2d_barstart + v2d_barwidth, textParamsCenter.pos.y)
                                , used2dCol);
                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , float2(textParamsCenter.pos.x + i2d_barstart, textParamsCenter.pos.y - barheight)
                                , float2(textParamsCenter.pos.x + i2d_barstart + i2d_barwidth, textParamsCenter.pos.y)
                                , used2didxCol);
                        }
                    }
                }
            }
            #endif
            
            //  Batched 3D debug (clear cpu buffers onto the screen)
            #if __DEBUG
            {
                Immediate::present3d(mgr.immediateBuffer, mgr.perspProjection.matrix, viewMatrix);
                Immediate::clear3d(mgr.immediateBuffer);
            }
            #endif

            // copy backbuffer to window (upscale from game resolution to window resolution)
            {
                Driver::Marker_t marker;
                Driver::set_marker_name(marker, "UPSCALE TO WINDOW");
                Renderer::Driver::start_event(marker);
                {
                    {
                        Renderer::Driver::ViewportParams vpParams;
                        vpParams.topLeftX = 0;
                        vpParams.topLeftY = 0;
                        vpParams.width = (f32)platform.screen.window_width;
                        vpParams.height = (f32)platform.screen.window_height;
                        Renderer::Driver::set_VP(vpParams);
                    }

                    Renderer::Driver::bind_blend_state(store.blendStateBlendOff);
                    Renderer::Driver::bind_DS(store.depthStateOff);
                    Renderer::Driver::bind_RS(store.rasterizerStateFillFrontfaces);
                    Renderer::Driver::bind_main_RT(store.windowRT);

                    Driver::bind_shader(store.shaders[Renderer::ShaderType::FullscreenBlit]);
                    Driver::bind_textures(&store.gameRT.textures[0], 1);
                    Driver::draw_fullscreen();
                    Renderer::Driver::RscTexture nullTex = {};
                    Driver::bind_textures(&nullTex, 1); // unbind gameRT
                }
                Renderer::Driver::end_event();
            }
            // Batched 2d debug (clear cpu buffers onto the screen)
            #if __DEBUG
            {
                Renderer::Driver::bind_blend_state(store.blendStateBlendOff);
                Renderer::Driver::bind_main_RT(store.windowRT);
                Immediate::present2d(mgr.immediateBuffer, mgr.windowProjection.matrix);
                Immediate::clear2d(mgr.immediateBuffer);
            }
            #endif

        }
    }
}

#endif // __WASTELADNS_GAME_H__
