#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

#include "gameplay.h"
#include "game_renderer.h"

struct Scene_ModelLoadData {
    const char* path;
    Vec3 origin;
    float scale;
    bool player;
};
// from blender: export fbx -> Apply Scalings: FBX All -> Forward: the one in Blender -> Use Space Transform: yes 
const Scene_ModelLoadData assets[] = {
      { "assets/meshes/boar.fbx", {5.f, 10.f, 2.30885f}, 1.f, false }
    , { "assets/meshes/bird.fbx", {1.f, 3.f, 2.23879f}, 1.f, true }
    , { "assets/meshes/bird.fbx", {0.f, 5.f, 2.23879f}, 1.f, false }
};
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
            ;
    };

    struct RenderScene {
        Renderer::Store store;
    };

    struct CameraManager {
        enum { FlyCam, CamCount };
        Camera cameras[CamCount];
        Gameplay::Orbit::State orbitCamera;
        Camera* activeCam;
    };

    struct RenderManager {
        RenderScene renderScene;
        __DEBUGDEF(Renderer::Immediate::Buffer immediateBuffer;)
        Renderer::WindowProjection windowProjection;
        Renderer::PerspProjection perspProjection;
        Renderer::Drawlist::Drawlist_node player_DLnode;
        Renderer::Drawlist::Drawlist_game::Handle particles_DLhandle;
    };

    #if __DEBUG
    struct DebugVis {
        struct OverlayMode { enum Enum { All, HelpOnly, ArenaOnly, None, Count }; };

        f64 frameHistory[60];
        f64 frameAvg = 0;
        u64 frameHistoryIdx = 0;
        OverlayMode::Enum overlaymode = OverlayMode::Enum::All;
    };
    #endif

    struct Memory {
        Allocator::Arena scratchArenaRoot; // to be always passed by copy, so it auto-resets
        __DEBUGDEF(Allocator::Arena imDebugArena;)
        u8* frameArenaBuffer; // used to reset Allocator::frameArena every frame
        __DEBUGDEF(u8* frameArenaHighmark;)
        __DEBUGDEF(u8* scratchArenaHighmark;)
    };

    struct Instance {
        Time time;
        CameraManager cameraMgr;
        RenderManager renderMgr;
        Gameplay::Movement::State player;
        Memory memory;
        __DEBUGDEF(DebugVis debugVis;)
    };

    void loadLaunchConfig(Platform::WindowConfig& config) {
        // hardcoded for now
        config.window_width = 320 * 3;
        config.window_height = 240 * 3;
        config.game_width = 320 * 1;
        config.game_height = 240 * 1;
        config.fullscreen = false;
        config.title = "3D Test";
    }

    void start(Instance& game, Platform::GameConfig& config, const Platform::State& platform) {

        game.time = {};
        game.time.config.maxFrameLength = 0.1;
        game.time.config.targetFramerate = 1.0 / 60.0;
        game.time.lastFrame = platform.time.now;

        config = {};
        config.nextFrame = platform.time.now;

        {
            Allocator::init_arena(Allocator::frameArena, 1 << 20); // 1MB
            game.memory.frameArenaBuffer = Allocator::frameArena.curr;
            __DEBUGEXP(game.memory.frameArenaHighmark = Allocator::frameArena.curr; Allocator::frameArena.highmark = &game.memory.frameArenaHighmark);
            Allocator::init_arena(game.memory.scratchArenaRoot, 1 << 20); // 1MB
            __DEBUGEXP(game.memory.scratchArenaHighmark = game.memory.scratchArenaRoot.curr; game.memory.scratchArenaRoot.highmark = &game.memory.scratchArenaHighmark);
            __DEBUGEXP(Allocator::init_arena(game.memory.imDebugArena, Renderer::Immediate::arena_size));
        }

        // camera set up
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
            auto& store = game.renderMgr.renderScene.store;

            store = {};
            Renderer::start_store(game.renderMgr.renderScene.store, platform, game.memory.scratchArenaRoot);
            __DEBUGEXP(Renderer::Immediate::load(game.renderMgr.immediateBuffer, game.memory.imDebugArena));

            for (const Scene_ModelLoadData& asset : assets) {
                Renderer::Drawlist::Drawlist_node nodeToAdd = {};
                Math::identity4x4(nodeToAdd.localTransform);
                Math::identity4x4(nodeToAdd.worldTransform);
                nodeToAdd.localTransform.matrix.col0.x *= asset.scale;
                nodeToAdd.localTransform.matrix.col1.y *= asset.scale;
                nodeToAdd.localTransform.matrix.col2.z *= asset.scale;
                nodeToAdd.worldTransform.pos = asset.origin;

                Renderer::FBX::load_with_material(nodeToAdd, store, asset.path, game.memory.scratchArenaRoot);

                if (asset.player) {
                    game.renderMgr.player_DLnode = nodeToAdd;
                }
            }
            // unit cubes
            {
                using DL = Renderer::Drawlist::DL_unlit_instanced_t;
                using DL_id = Renderer::Drawlist::DL_unlit_instanced_id;

                Renderer::Drawlist::DL_unlit_instanced_vb buffer = {};
                Transform t; Math::identity4x4(t);
                buffer.groupData.worldMatrix = t.matrix;
                buffer.groupData.groupColor = Col(0.9f, 0.7f, 0.8f, 0.6f).RGBAv4();
                buffer.instancedData.resize(numCubes);
                Renderer::create_indexed_vertex_buffer_from_untextured_cube(buffer.buffer, { 1.f, 1.f, 1.f });

                Renderer::Drawlist::Drawlist_node nodeToAdd = {};
                nodeToAdd.handle.DL_id::id = (u32)store.drawlist.DL::dl_perVertexBuffer.size();
                nodeToAdd.worldTransform = t;
                Math::identity4x4(nodeToAdd.localTransform);
                game.renderMgr.particles_DLhandle = nodeToAdd.handle;
                store.drawlist.DL::dl_perVertexBuffer.push_back(buffer);
                store.drawlist_nodes.push_back(nodeToAdd);
            }
            {
                using DL = Renderer::Drawlist::DL_textureMapped_t;
                using DL_VertexBuffer = Renderer::Drawlist::DL_textureMapped_vb;
                using DL_Material = Renderer::Drawlist::DL_textureMapped_mat;
                using DL_id = Renderer::Drawlist::DL_textureMapped_id;
                DL_VertexBuffer buffer = {};
                Transform t; Math::identity4x4(t);
                buffer.groupData.worldMatrix = t.matrix;
                buffer.groupData.groupColor = Col(0.f, 0.f, 0.f, 0.f).RGBAv4();
                Renderer::create_indexed_vertex_buffer_from_textured_cube(buffer.buffer, { 1.f, 1.f, 1.f });

                Renderer::Drawlist::Drawlist_node nodeToAdd = {};
                nodeToAdd.worldTransform = t;
                Math::identity4x4(nodeToAdd.localTransform);
                nodeToAdd.handle.DL_id::material = (u16)store.drawlist.DL::dl_perMaterial.size();
                DL_Material material = {};
                Renderer::Driver::create_texture_from_file(material.textures[DL_Material::TextureTypes::Albedo], { "assets/pbr/material04-albedo.png" });
                Renderer::Driver::create_texture_from_file(material.textures[DL_Material::TextureTypes::NormalMap], { "assets/pbr/material04-normal.png" });
                Renderer::Driver::create_texture_from_file(material.textures[DL_Material::TextureTypes::DepthMap], { "assets/pbr/material04-depth.png" });
                nodeToAdd.handle.DL_id::buffer = (u16)material.dl_perVertexBuffer.size();
                material.dl_perVertexBuffer.push_back(buffer);

                store.drawlist.DL::dl_perMaterial.push_back(material);
                store.drawlist_nodes.push_back(nodeToAdd);
            }
        }

        game.player = {};
        {
            Math::identity4x4(game.player.transform);
            game.player.transform.pos = game.renderMgr.player_DLnode.worldTransform.pos;
        }

        game.cameraMgr = {};
        {
            CameraManager& mgr = game.cameraMgr;
            Camera& cam = mgr.cameras[CameraManager::FlyCam];
            mgr.activeCam = &cam;

            mgr.orbitCamera.offset = Vec3(0.f, -100.f, 0.f);
            mgr.orbitCamera.eulers = Vec3(45.f * Math::d2r<f32>, 0.f, -25.f * Math::d2r<f32>);
            mgr.orbitCamera.scale = 1.f;
            mgr.orbitCamera.origin = Vec3(0.f, 0.f, 0.f);
        }
    }

    void update(Instance& game, Platform::GameConfig& config, const Platform::State& platform) {

        // frame arena reset
        Allocator::frameArena.curr = game.memory.frameArenaBuffer;

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
        {
            if (keyboard.released(Input::EXIT)) {
                config.quit = true;
            }
            #if __DEBUG
            if (keyboard.pressed(Input::TOGGLE_OVERLAY)) {
                game.debugVis.overlaymode = (DebugVis::OverlayMode::Enum)((game.debugVis.overlaymode + 1) % DebugVis::OverlayMode::Count);
            }
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
                } else {
                    Gameplay::Movement::process(game.player.control, keyboard, { Input::UP, Input::DOWN, Input::LEFT, Input::RIGHT });
                }
                Gameplay::Movement::process_cameraRelative(game.player.transform, game.player.movementController, game.player.control, activeCam->transform, dt);

                // update drawlist
                game.renderMgr.player_DLnode.worldTransform.matrix = Math::mult(game.player.transform.matrix, game.renderMgr.player_DLnode.localTransform.matrix);
                Renderer::Drawlist::update_dl_node_matrix(game.renderMgr.renderScene.store.drawlist, game.renderMgr.player_DLnode);
                
                {
                    Mat4 instance_matrices[numCubes];
                    for (u32 i = 0; i < numCubes; i++) {
                        float scaley = -1.f * Math::clamp(game.player.movementController.speed, 0.f, 1.5f);
                        float scalez = 0.5f * Math::clamp(game.player.movementController.speed, 0.f, 1.f);
                        Transform t;
                        Math::identity4x4(t);
                        t.pos = Math::add(game.player.transform.pos, Math::scale(game.player.transform.front, scaley * (i + 2)));
                        t.pos.z = t.pos.z + scalez * (Math::cos((f32)platform.time.now * 10.f + i * 2.f) - 1.5f);
                        t.matrix.col0.x = 0.2f;
                        t.matrix.col1.y = 0.2f;
                        t.matrix.col2.z = 0.2f;
                        instance_matrices[i] = t.matrix;
                    }
                    Renderer::Drawlist::update_dl_node_instance_data(game.renderMgr.renderScene.store.drawlist, game.renderMgr.particles_DLhandle, instance_matrices);
                }
            }
            
            // animation
            {
                const f32 time = 0.f; // tmp hack
                for (u32 i = 0; i < game.renderMgr.renderScene.store.animated_nodes.size(); i++) {
                    Allocator::Arena scratchArena = game.memory.scratchArenaRoot;
                    const Renderer::Animation::AnimatedNode& node = game.renderMgr.renderScene.store.animated_nodes[i];
                    Mat4* world_to_joint = (Mat4*)Allocator::alloc_arena(scratchArena, node.skeleton.jointCount, 16);
                    Renderer::Animation::updateAnimation(world_to_joint, node, time);
                    
                    Renderer::Drawlist::update_dl_node_instance_data(game.renderMgr.renderScene.store.drawlist, game.renderMgr.player_DLnode.handle, world_to_joint);
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

        // Render update
        {
            RenderManager& mgr = game.renderMgr;
            RenderScene& rscene = mgr.renderScene;
            using namespace Renderer;

            // perspective cam set up
            {
                Camera* activeCam = game.cameraMgr.activeCam;
                Renderer::Layout_CBuffer_3DScene::SceneData cbufferPerScene;
                cbufferPerScene.projectionMatrix = mgr.perspProjection.matrix;
                cbufferPerScene.viewMatrix = activeCam->viewMatrix;
                cbufferPerScene.viewPos = activeCam->transform.pos;
                cbufferPerScene.lightPos = Vec3(3.f, 8.f, 15.f);
                Renderer::Driver::update_cbuffer(rscene.store.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], cbufferPerScene);
            }

            {
                auto& store = rscene.store;
                auto& dl = store.drawlist;

                Renderer::Driver::bind_blend_state(store.blendStateOn);
                Renderer::Driver::bind_DS(store.depthStateOn);
                Renderer::Driver::bind_RT(store.gameRT);
                Renderer::Driver::clear_RT(store.gameRT, Col(0.2f, 0.344f, 0.59f, 1.f));
                {
                    Renderer::Driver::ViewportParams vpParams;
                    vpParams.topLeftX = 0;
                    vpParams.topLeftY = 0;
                    vpParams.width = (f32)platform.screen.width;
                    vpParams.height = (f32)platform.screen.height;
                    vpParams.minDepth = 0.f;
                    vpParams.maxDepth = 1.f;
                    Renderer::Driver::set_VP(vpParams);
                }
            
                draw_drawlists(dl, store.cbuffers);
            }

            // Immediate-mode debug. Can be moved out of the render update, it only pushes data to cpu buffers
            #if __DEBUG
            {
                // World axis
                {
                    const Col axisX(0.8f, 0.15f, 0.25f, 0.7f);
                    const Col axisY(0.25f, 0.8f, 0.15f, 0.7f);
                    const Col axisZ(0.15f, 0.25f, 0.8f, 0.7f);
                    const f32 axisSize = 30.f;
                    const Vec3 pos = Vec3(0.f, 0.f, 0.1f);

                    Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(1.f, 0.f, 0.f), axisSize)), axisX);
                    Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(0.f, 1.f, 0.f), axisSize)), axisY);
                    Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(0.f, 0.f, 1.f), axisSize)), axisZ);

                    const f32 thickness = 0.1f;
                    const u32 steps = 10;
                    const f32 spacing = 1.f / (f32)steps;
                    for (int i = 1; i <= steps; i++) {
                        Immediate::segment(mgr.immediateBuffer,
                              Math::add(pos, Vec3(spacing * i, 0.f, -thickness))
                            , Math::add(pos, Vec3(spacing * i, 0.f, thickness))
                            , axisX);
                        Immediate::segment(mgr.immediateBuffer,
                              Math::add(pos, Vec3(0.f, spacing * i, -thickness))
                            , Math::add(pos, Vec3(0.f, spacing * i, thickness))
                            , axisY);
                        Immediate::segment(mgr.immediateBuffer,
                              Math::add(pos, Vec3(-thickness, thickness, spacing * i))
                            , Math::add(pos, Vec3(thickness, -thickness, spacing * i))
                            , axisZ);
                    }
                }
                // 2d debug info
                if (game.debugVis.overlaymode != DebugVis::OverlayMode::None)
                {
                    Col defaultCol(0.7f, 0.8f, 0.15f, 1.0f);
                    Col activeCol(1.0f, 0.2f, 0.1f, 1.0f);
                    
                    f32 textscale = platform.screen.text_scale;
                    f32 lineheight = 15.f * textscale;

                    Renderer::Immediate::TextParams textParamsLeft, textParamsRight, textParamsCenter;
                    textParamsLeft.scale = (u8)platform.screen.text_scale;
                    textParamsLeft.pos = Vec3(game.renderMgr.windowProjection.config.left + 10.f * textscale, game.renderMgr.windowProjection.config.top - 10.f * textscale, -50);
                    textParamsLeft.color = defaultCol;
                    textParamsRight.scale = (u8)platform.screen.text_scale;
                    textParamsRight.pos = Vec3(game.renderMgr.windowProjection.config.right - 60.f * textscale, game.renderMgr.windowProjection.config.top - 10.f * textscale, -50);
                    textParamsRight.color = defaultCol;
                    textParamsCenter.scale = (u8)platform.screen.text_scale;
                    textParamsCenter.pos = Vec3(0.f, game.renderMgr.windowProjection.config.top - 10.f * textscale, -50);
                    textParamsCenter.color = defaultCol;

                    const char* overlaynames[] = { "All", "Help Only", "Arenas only", "None" };
                    textParamsLeft.color = keyboard.pressed(Input::TOGGLE_OVERLAY) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "H to toggle overlays: %s", overlaynames[game.debugVis.overlaymode]);
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
                            Vec3 eulers_deg = Math::scale(game.cameraMgr.orbitCamera.eulers, Math::r2d<f32>);
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "Camera eulers: " VEC3_FORMAT("% .3f"), VEC3_PARAMS(eulers_deg));
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
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsRight, "%s", Platform::name);
                        textParamsRight.pos.y -= lineheight;
                        textParamsRight.pos.x -= 30.f * textscale;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsRight, "%.3lf fps", 1. / game.debugVis.frameAvg);
                        textParamsRight.pos.y -= lineheight;
                    }

                    if (game.debugVis.overlaymode == DebugVis::OverlayMode::All || game.debugVis.overlaymode == DebugVis::OverlayMode::ArenaOnly)
                    {
                        auto renderArena = [](Renderer::Immediate::Buffer& im, Renderer::Immediate::TextParams& textCfg, u8* arenaEnd, u8* arenaStart, u8* arenaHighmark, const char* arenaName, const Col defaultCol, const Col baseCol, const Col highmarkCol, const f32 lineheight, const f32 textscale) {
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
                                , Vec2(textCfg.pos.x, textCfg.pos.y - barheight)
                                , Vec2(textCfg.pos.x + barwidth, textCfg.pos.y)
                                , baseCol);
                            Renderer::Immediate::box_2d(im
                                , Vec2(textCfg.pos.x, textCfg.pos.y - barheight)
                                , Vec2(textCfg.pos.x + arenaHighmark_barwidth, textCfg.pos.y)
                                , highmarkCol);
                            textCfg.pos.y -= barheight + 5.f * textscale;
                        };

                        const f32 barwidth = 150.f * textscale;
                        const f32 barheight = 10.f * textscale;
                        textParamsCenter.pos.x -= barwidth / 2.f;
                        {
                            const Col arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Col arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                            renderArena(game.renderMgr.immediateBuffer, textParamsCenter, Allocator::frameArena.end, game.memory.frameArenaBuffer, game.memory.frameArenaHighmark
                                , "Frame arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            const Col arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Col arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                            renderArena(game.renderMgr.immediateBuffer, textParamsCenter, platform.memory.scratchArenaRoot.end, platform.memory.scratchArenaRoot.curr, platform.memory.scratchArenaHighmark
                                , "Platform scratch arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            const Col arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Col arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                            renderArena(game.renderMgr.immediateBuffer, textParamsCenter, game.memory.scratchArenaRoot.end, game.memory.scratchArenaRoot.curr, game.memory.scratchArenaHighmark
                                , "Game scratch arena", defaultCol, arenabaseCol, arenahighmarkCol, lineheight, textscale);
                        }
                        {
                            auto& im = game.renderMgr.immediateBuffer;
                            const Col baseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Col used3dCol(0.95f, 0.35f, 0.8f, 1.f);
                            const Col used2dCol(0.35f, 0.95f, 0.8f, 1.f);
                            const Col used2didxCol(0.8f, 0.95f, 0.8f, 1.f);

                            const ptrdiff_t memory_size = (ptrdiff_t)game.memory.imDebugArena.curr - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_3d_start = (ptrdiff_t)im.vertices_3d - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_3d_size = (ptrdiff_t)im.vertices_3d_head * sizeof(Layout_Vec3Color4B);
                            const ptrdiff_t vertices_2d_start = (ptrdiff_t)im.vertices_2d - (ptrdiff_t)im.vertices_3d;
                            const ptrdiff_t vertices_2d_size = (ptrdiff_t)im.vertices_2d_head * sizeof(Layout_Vec2Color4B);
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
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "im 3d: %lu bytes", vertices_2d_size);
                            textParamsCenter.pos.y -= lineheight;
                            textParamsCenter.color = used2didxCol;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "im 3d: %lu bytes", indices_2d_size);
                            textParamsCenter.pos.y -= lineheight;
                            textParamsCenter.color = defaultCol;

                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , Vec2(textParamsCenter.pos.x, textParamsCenter.pos.y - barheight)
                                , Vec2(textParamsCenter.pos.x + barwidth, textParamsCenter.pos.y)
                                , baseCol);
                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , Vec2(textParamsCenter.pos.x + v3d_barstart, textParamsCenter.pos.y - barheight)
                                , Vec2(textParamsCenter.pos.x + v3d_barwidth, textParamsCenter.pos.y)
                                , used3dCol);
                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , Vec2(textParamsCenter.pos.x + v2d_barstart, textParamsCenter.pos.y - barheight)
                                , Vec2(textParamsCenter.pos.x + v2d_barstart + v2d_barwidth, textParamsCenter.pos.y)
                                , used2dCol);
                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , Vec2(textParamsCenter.pos.x + i2d_barstart, textParamsCenter.pos.y - barheight)
                                , Vec2(textParamsCenter.pos.x + i2d_barstart + i2d_barwidth, textParamsCenter.pos.y)
                                , used2didxCol);
                        }
                    }
                }
            }
            #endif
            
            //  Batched 3D debug (clear cpu buffers onto the screen)
            #if __DEBUG
            {
                Immediate::present3d(mgr.immediateBuffer, mgr.perspProjection.matrix, game.cameraMgr.activeCam->viewMatrix);
                Immediate::clear3d(mgr.immediateBuffer);
            }
            #endif

            // copy backbuffer to window (upscale from game resolution to window resolution)
            {
                SET_MARKER_NAME(Driver::Marker_t marker, "UPSCALE TO WINDOW");
                Renderer::Driver::start_event(marker);
                {
                    {
                        Renderer::Driver::ViewportParams vpParams;
                        vpParams.topLeftX = 0;
                        vpParams.topLeftY = 0;
                        vpParams.width = (f32)platform.screen.window_width;
                        vpParams.height = (f32)platform.screen.window_height;
                        vpParams.minDepth = 0.f;
                        vpParams.maxDepth = 1.f;
                        Renderer::Driver::set_VP(vpParams);
                    }

                    Renderer::Driver::bind_blend_state(rscene.store.blendStateOff);
                    Renderer::Driver::bind_DS(rscene.store.depthStateOff);
                    Renderer::Driver::bind_RS(rscene.store.rasterizerStateFill);
                    Renderer::Driver::bind_main_RT(rscene.store.windowRT);

                    Driver::bind_shader(rscene.store.shader_blit);
                    Driver::bind_textures(&rscene.store.gameRT.textures[0], 1);
                    Driver::draw_fullscreen();
                }
                Renderer::Driver::end_event();
                Renderer::Driver::bind_blend_state(rscene.store.blendStateOff);
                Renderer::Driver::bind_DS(rscene.store.depthStateOff);
                Renderer::Driver::bind_main_RT(rscene.store.windowRT);
            }
            // Batched 2d debug (clear cpu buffers onto the screen)
            #if __DEBUG
            {
                Immediate::present2d(mgr.immediateBuffer, mgr.windowProjection.matrix);
                Immediate::clear2d(mgr.immediateBuffer);
            }
            #endif

        }
    }
}

#endif // __WASTELADNS_GAME_H__
