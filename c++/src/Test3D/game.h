#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

#include "gameplay.h"

struct Scene_ModelLoadData {
    const char* path;
    Vec3 origin;
    float scale;
};
// from blender: export fbx -> Apply Scalings: FBX All -> Forward: the one in Blender -> Use Space Transform: yes 
const Scene_ModelLoadData assets[] = {
      { "assets/meshes/boar.fbx", {5.f, 10.f, 2.30885f}, 1.f }
    , { "assets/meshes/bird.fbx", {1.f, 3.f, 2.23879f}, 1.f }
};

namespace Renderer {

    struct Drawlist {

        using DL_opaque_3d_color = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_untextured_unlit
            , Layout_Vec3Color4B
            , Layout_CBuffer_3DScene
            , Layout_CNone
            , Shaders::VSDrawType::Standard
        >;
        using DL_opaque_3d_textured = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_textured_lit_normalmapped
            , Layout_TexturedVec3
            , Layout_CBuffer_3DScene
            , Layout_CBuffer_3DScene
            , Shaders::VSDrawType::Standard
        >;
        using DL_opaque_3d_unlit = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_untextured_unlit
            , Layout_Vec3
            , Layout_CBuffer_3DScene
            , Layout_CNone
            , Shaders::VSDrawType::Standard
        >;
        using DL_opaque_3d_unlit_instanced = Drawlist_TypeContext <
              Shaders::VSTechnique::forward_base, Shaders::PSTechnique::forward_untextured_unlit
            , Layout_Vec3
            , Layout_CBuffer_3DScene
            , Layout_CNone
            , Shaders::VSDrawType::Instanced
        >;

        Renderer::Driver::RscCBuffer cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::Count];
        DL_opaque_3d_color::type dl_opaque_3d_color;
        DL_opaque_3d_textured::type dl_opaque_3d_textured;
        DL_opaque_3d_unlit::type dl_opaque_3d_unlit;
        DL_opaque_3d_unlit_instanced::type dl_opaque_3d_unlit_instanced;\
    };
}

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
        Renderer::Drawlist drawlist;
        Renderer::Driver::RscRasterizerState rasterizerStateFill, rasterizerStateLine;
        Renderer::Driver::RscDepthStencilState depthStateOn;
        Renderer::Driver::RscBlendState blendStateOn;
        Renderer::Driver::RscBlendState blendStateOff;
        Renderer::Driver::RscMainRenderTarget mainRenderTarget;
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
        Renderer::OrthoProjection orthoProjection;
        Renderer::PerspProjection perspProjection;
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
        config.window_width = 640 * 2;
        config.window_height = 480 * 2;
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

            OrthoProjection::Config& ortho = mgr.orthoProjection.config;
            ortho.right = platform.screen.width * 0.5f;
            ortho.top = platform.screen.height * 0.5f;
            ortho.left = -ortho.right;
            ortho.bottom = -ortho.top;
            ortho.near = 0.f;
            ortho.far = 1000.f;
            generate_matrix_ortho(mgr.orthoProjection.matrix, ortho);

            // Todo: consider whether precision needs special handling
            PerspProjection::Config& frustum = mgr.perspProjection.config;
            frustum.fov = 45.f;
            frustum.aspect = platform.screen.width / (f32)platform.screen.height;
            frustum.near = 1.f;
            frustum.far = 1000.f;
            generate_matrix_persp(mgr.perspProjection.matrix, frustum);
        }

        game.renderMgr.renderScene = {};
        {
            RenderScene& rscene = game.renderMgr.renderScene;

            Renderer::Driver::MainRenderTargetParams renderTargetParams = {};
            renderTargetParams.depth = true;
            renderTargetParams.width = platform.screen.width;
            renderTargetParams.height = platform.screen.height;
            Renderer::Driver::create_main_RT(rscene.mainRenderTarget, renderTargetParams);

            // rasterizer states
            Renderer::Driver::create_blend_state(rscene.blendStateOn, { true });
            Renderer::Driver::create_RS(rscene.rasterizerStateFill, { Renderer::Driver::RasterizerFillMode::Fill, Renderer::Driver::RasterizerCullMode::CullBack });
            Renderer::Driver::create_RS(rscene.rasterizerStateLine, { Renderer::Driver::RasterizerFillMode::Line, Renderer::Driver::RasterizerCullMode::CullNone });
            Renderer::Driver::create_DS(rscene.depthStateOn, { true, Renderer::Driver::DepthFunc::Less });

            {
                auto& dl = game.renderMgr.renderScene.drawlist;
                dl = {};
                Renderer::create_cbuffers_3DScene(dl.cbuffers);
                {
                    dl.dl_opaque_3d_color.rasterizerState = &rscene.rasterizerStateFill;
                    Renderer::Drawlist::DL_opaque_3d_color::load_dl_technique(dl.dl_opaque_3d_color, dl.cbuffers);
                }
                {
                    Renderer::Drawlist::DL_opaque_3d_textured::load_dl_technique(dl.dl_opaque_3d_textured, dl.cbuffers);
                }
                {
                    dl.dl_opaque_3d_unlit.rasterizerState = &rscene.rasterizerStateFill;
                    Renderer::Drawlist::DL_opaque_3d_unlit::load_dl_technique(dl.dl_opaque_3d_unlit, dl.cbuffers);
                }
                {
                    dl.dl_opaque_3d_unlit_instanced.rasterizerState = &rscene.rasterizerStateFill;
                    Renderer::Drawlist::DL_opaque_3d_unlit_instanced::load_dl_technique(dl.dl_opaque_3d_unlit_instanced, dl.cbuffers);
                }
            }

            // meshes in the scene
            {
                for (const Scene_ModelLoadData& asset : assets) {
                    Transform transform;
                    Transform localTransform;
                    Math::identity4x4(transform);
                    Math::identity4x4(localTransform);
                    localTransform.matrix.col0.x *= asset.scale;
                    localTransform.matrix.col1.y *= asset.scale;
                    localTransform.matrix.col2.z *= asset.scale;
                    transform.pos = asset.origin;

                    Renderer::Drawlist::DL_opaque_3d_color::type::DL_VertexBuffer buffer = {};
                    buffer.groupData.worldMatrix = Math::mult(transform.matrix, localTransform.matrix);
                    buffer.groupData.groupColor = Col(0.3f, 0.3f, 0.3f, 1.f).RGBAv4();
                    Renderer::FBX::load(buffer.buffer, asset.path, game.memory.scratchArenaRoot);

                    rscene.drawlist.dl_opaque_3d_color.dl_perVertexBuffer.push_back(buffer);
                }
                // unit cubes
                {
                    Renderer::Drawlist::DL_opaque_3d_unlit_instanced::type::DL_VertexBuffer buffer = {};
                    Transform t; Math::identity4x4(t);
                    buffer.groupData.worldMatrix = t.matrix;
                    buffer.groupData.groupColor = Col(0.9f, 0.1f, 0.8f, 1.f).RGBAv4();
                    buffer.instancedData.resize(4);
                    Renderer::create_indexed_vertex_buffer_from_untextured_cube(buffer.buffer, { 1.f, 1.f, 1.f });

                    rscene.drawlist.dl_opaque_3d_unlit_instanced.dl_perVertexBuffer.push_back(buffer);
                }
                {
                    using DL_VertexBuffer = Renderer::Drawlist::DL_opaque_3d_textured::type::DL_VertexBuffer;
                    using DL_Material = Renderer::Drawlist::DL_opaque_3d_textured::type::DL_Material;
                    DL_VertexBuffer buffer = {};
                    Transform t; Math::identity4x4(t);
                    buffer.groupData.worldMatrix = t.matrix;
                    buffer.groupData.groupColor = Col(0.f, 0.f, 0.f, 0.f).RGBAv4();
                    Renderer::create_indexed_vertex_buffer_from_textured_cube(buffer.buffer, { 1.f, 1.f, 1.f });

                    DL_Material material = {};
                    material.rasterizerState = &rscene.rasterizerStateFill;
                    Renderer::Driver::create_texture_from_file(material.textures[DL_Material::TextureTypes::Albedo], { "assets/pbr/material04-albedo.png" });
                    Renderer::Driver::create_texture_from_file(material.textures[DL_Material::TextureTypes::NormalMap], { "assets/pbr/material04-normal.png" });
                    Renderer::Driver::create_texture_from_file(material.textures[DL_Material::TextureTypes::DepthMap], { "assets/pbr/material04-depth.png" });
                    material.dl_perVertexBuffer.push_back(buffer);

                    rscene.drawlist.dl_opaque_3d_textured.dl_perMaterial.push_back(material);
                }
            }

            __DEBUGEXP(Renderer::Immediate::load(game.renderMgr.immediateBuffer, game.memory.imDebugArena));
        }

        game.player = {};
        {
            Math::identity4x4(game.player.transform);
            game.player.transform.pos = assets[0].origin;
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
                Gameplay::Movement::process(game.player.control, keyboard, { Input::UP, Input::DOWN, Input::LEFT, Input::RIGHT }, dt);
                Gameplay::Movement::process_cameraRelative(game.player.transform, game.player.movementController, game.player.control, activeCam->transform, dt);

                // TODO: ABSOLUTE HACK
                // update the drawlist directly, until we have a system that connects gameplay and render data
                Transform localTransform;
                const float playerScale = assets[0].scale;
                Math::identity4x4(localTransform);
                localTransform.matrix.col0.x *= playerScale;
                localTransform.matrix.col1.y *= playerScale;
                localTransform.matrix.col2.z *= playerScale;
                auto& colorBuffer = game.renderMgr.renderScene.drawlist.dl_opaque_3d_color.dl_perVertexBuffer[0];
                colorBuffer.groupData.worldMatrix = Math::mult(game.player.transform.matrix, localTransform.matrix);
                auto& instancedBuffer = game.renderMgr.renderScene.drawlist.dl_opaque_3d_unlit_instanced.dl_perVertexBuffer[0];
                u32 instanceSize = (u32)instancedBuffer.instancedData.size();
                for (u32 i = 0; i < instanceSize; i++) {
                    Transform t;
                    Math::identity4x4(t);
                    t.pos = Math::add(game.player.transform.pos, Math::scale(game.player.transform.front, -5.f * i));
                    t.matrix.col0.x = 0.2f;
                    t.matrix.col1.y = 0.2f;
                    t.matrix.col2.z = 0.2f;
                    instancedBuffer.instancedData[i] = t.matrix;
                }
            }

            // camera update
            {
                Gameplay::Orbit::process(game.cameraMgr.orbitCamera, platform.input.mouse);
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
                Renderer::Driver::update_cbuffer(rscene.drawlist.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], cbufferPerScene);
            }

            {
                Renderer::Driver::bind_blend_state(rscene.blendStateOn);
                Renderer::Driver::bind_DS(rscene.depthStateOn);
                Renderer::Driver::bind_main_RT(rscene.mainRenderTarget);
                Renderer::Driver::clear_main_RT(rscene.mainRenderTarget, Col(0.f, 0.f, 0.f, 1.f));
            
                auto& dl = rscene.drawlist;

                Renderer::dl_drawPerShader(dl.dl_opaque_3d_color, dl.cbuffers);
                Renderer::dl_drawPerShader(dl.dl_opaque_3d_textured, dl.cbuffers);
                Renderer::dl_drawPerShader(dl.dl_opaque_3d_unlit, dl.cbuffers);
                Renderer::dl_drawPerShader(dl.dl_opaque_3d_unlit_instanced, dl.cbuffers);
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

                    Renderer::Immediate::TextParams textParamsLeft, textParamsRight, textParamsCenter;
                    textParamsLeft.scale = 1;
                    textParamsLeft.pos = Vec3(game.renderMgr.orthoProjection.config.left + 10.f, game.renderMgr.orthoProjection.config.top - 15.f, -50);
                    textParamsLeft.color = defaultCol;
                    textParamsRight.scale = 1;
                    textParamsRight.pos = Vec3(game.renderMgr.orthoProjection.config.right - 60.f, game.renderMgr.orthoProjection.config.top - 15.f, -50);
                    textParamsRight.color = defaultCol;
                    textParamsCenter.scale = 1;
                    textParamsCenter.pos = Vec3(0.f, game.renderMgr.orthoProjection.config.top - 15.f, -50);
                    textParamsCenter.color = defaultCol;

                    const char* overlaynames[] = { "All", "Help Only", "Arenas only", "None" };
                    textParamsLeft.color = keyboard.pressed(Input::TOGGLE_OVERLAY) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "H to toggle overlays: %s", overlaynames[game.debugVis.overlaymode]);
                    textParamsLeft.pos.y -= 15.f;

                    if (game.debugVis.overlaymode == DebugVis::OverlayMode::All || game.debugVis.overlaymode == DebugVis::OverlayMode::HelpOnly) {
                        textParamsLeft.color = platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_LEFT) ? activeCol : defaultCol;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "left mouse click and drag to orbit");
                        textParamsLeft.pos.y -= 15.f;
                        textParamsLeft.color = platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_RIGHT) ? activeCol : defaultCol;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "right mouse click and drag to pan");
                        textParamsLeft.pos.y -= 15.f;
                        textParamsLeft.color = platform.input.mouse.scrolldy != 0 ? activeCol : defaultCol;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "mouse wheel to scale");
                        textParamsLeft.pos.y -= 15.f;
                        {
                            Vec3 eulers_deg = Math::scale(game.cameraMgr.orbitCamera.eulers, Math::r2d<f32>);
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsLeft, "Camera eulers: " VEC3_FORMAT("% .3f"), VEC3_PARAMS(eulers_deg));
                            textParamsLeft.pos.y -= 15.f;
                        }

                        textParamsRight.color = defaultCol;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsRight, "%s", Platform::name);
                        textParamsRight.pos.y -= 15.f;
                        textParamsRight.pos.x -= 30.f;
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsRight, "%.3lf fps", 1. / game.debugVis.frameAvg);
                        textParamsRight.pos.y -= 15.f;
                    }

                    if (game.debugVis.overlaymode == DebugVis::OverlayMode::All || game.debugVis.overlaymode == DebugVis::OverlayMode::ArenaOnly)
                    {
                        const f32 barwidth = 150.f;
                        const f32 barheight = 10.f;
                        textParamsCenter.pos.x -= barwidth / 2.f;
                        {
                            const Col arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Col arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);

                            const ptrdiff_t arenaTotal = (ptrdiff_t)Allocator::frameArena.end - (ptrdiff_t)game.memory.frameArenaBuffer;
                            const ptrdiff_t arenaHighmark = (ptrdiff_t)game.memory.frameArenaHighmark - (ptrdiff_t)game.memory.frameArenaBuffer;
                            const f32 arenaHighmark_barwidth = barwidth * (arenaHighmark / (f32)arenaTotal);

                            textParamsCenter.color = arenahighmarkCol;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "Frame arena highmark: %lu bytes", arenaHighmark);
                            textParamsCenter.pos.y -= 15.f;
                            textParamsCenter.color = defaultCol;

                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , Vec2(textParamsCenter.pos.x, textParamsCenter.pos.y - barheight)
                                , Vec2(textParamsCenter.pos.x + barwidth, textParamsCenter.pos.y)
                                , arenabaseCol);
                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , Vec2(textParamsCenter.pos.x, textParamsCenter.pos.y - barheight)
                                , Vec2(textParamsCenter.pos.x + arenaHighmark_barwidth, textParamsCenter.pos.y)
                                , arenahighmarkCol);
                            textParamsCenter.pos.y -= barheight + 5.f;
                        }
                        {
                            const Col arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                            const Col arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);

                            const ptrdiff_t arenaTotal = (ptrdiff_t)game.memory.scratchArenaRoot.end - (ptrdiff_t)game.memory.scratchArenaRoot.curr;
                            const ptrdiff_t arenaHighmark = (ptrdiff_t)game.memory.scratchArenaHighmark - (ptrdiff_t)game.memory.scratchArenaRoot.curr;
                            const f32 arenaHighmark_barwidth = barwidth * (arenaHighmark / (f32)arenaTotal);

                            textParamsCenter.color = arenahighmarkCol;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "Scratch arena highmark: %lu bytes", arenaHighmark);
                            textParamsCenter.pos.y -= 15.f;
                            textParamsCenter.color = defaultCol;

                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , Vec2(textParamsCenter.pos.x, textParamsCenter.pos.y - barheight)
                                , Vec2(textParamsCenter.pos.x + barwidth, textParamsCenter.pos.y)
                                , arenabaseCol);
                            Renderer::Immediate::box_2d(game.renderMgr.immediateBuffer
                                , Vec2(textParamsCenter.pos.x, textParamsCenter.pos.y - barheight)
                                , Vec2(textParamsCenter.pos.x + arenaHighmark_barwidth, textParamsCenter.pos.y)
                                , arenahighmarkCol);
                            textParamsCenter.pos.y -= barheight + 5.f;
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
                            textParamsCenter.pos.y -= 15.f;
                            textParamsCenter.color = used2dCol;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "im 3d: %lu bytes", vertices_2d_size);
                            textParamsCenter.pos.y -= 15.f;
                            textParamsCenter.color = used2didxCol;
                            Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParamsCenter, "im 3d: %lu bytes", indices_2d_size);
                            textParamsCenter.pos.y -= 15.f;
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
            // Batched debug (clear cpu buffers onto the screen)
            #if __DEBUG
            {
                Immediate::present3d(mgr.immediateBuffer, mgr.perspProjection.matrix, game.cameraMgr.activeCam->viewMatrix);
                Immediate::present2d(mgr.immediateBuffer, mgr.orthoProjection.matrix);
                Immediate::clear(mgr.immediateBuffer);
            }
            #endif
        }
    }
}

#endif // __WASTELADNS_GAME_H__
