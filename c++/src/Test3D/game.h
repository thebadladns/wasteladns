#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

#include "gameplay.h"

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
            , TOGGLE_HELP = ::Input::Keyboard::Keys::H
            ;
    };

    template<typename _vertexLayout>
    struct RenderItemModel {
        std::vector< Renderer::Driver::RscIndexedBuffer<_vertexLayout> > buffers;
        Transform transform;
        Transform localTransform;
        Renderer::Driver::RscRasterizerState rasterizerState;
        Col groupColor;
        bool fill;
    };
    struct RenderItemUntexturedGeo {
        Renderer::Driver::RscIndexedBuffer<Renderer::Layout_Vec3> buffer;
        Renderer::Driver::RscRasterizerState rasterizerState;
        Transform transform;
        Col groupColor;
        bool fill;
    };
    struct RenderItemTexturedGeo {
        Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec3> buffer;
        Renderer::Driver::RscTexture albedo;
        Renderer::Driver::RscTexture normal;
        Renderer::Driver::RscTexture depth;
        Transform transform;
    };
    struct RenderScene {
        RenderItemModel<Renderer::Layout_Vec3Color4B> inputMeshGroupBuffer;
        RenderItemModel<Renderer::Layout_Vec3> inputMeshUnlitGroupBuffer;
        RenderItemTexturedGeo texturedCubeGroupBuffer;
        RenderItemUntexturedGeo untexturedCubeGroupBuffer;
        Renderer::Driver::RscCBuffer cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::Count];
        Renderer::Driver::RscRasterizerState rasterizerStateFill, rasterizerStateLine, rasterizerStateFillCulledBack;
        Renderer::Driver::RscBlendState blendStateOn;
        Renderer::Driver::RscBlendState blendStateOff;
        Renderer::Driver::RscMainRenderTarget mainRenderTarget;
        Renderer::Driver::RscShaderSet<Renderer::Layout_Vec3Color4B, Renderer::Layout_CBuffer_3DScene::Buffers> sceneShader;
        Renderer::Driver::RscShaderSet<Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> sceneUnlitShader;
        Renderer::Driver::RscShaderSet<Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> sceneInstancedShader;
        Renderer::Driver::RscShaderSet<Renderer::Layout_TexturedVec3, Renderer::Layout_CBuffer_3DScene::Buffers> sceneTexturedShader;
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
        Mat4 viewMatrix;
    };

    #if __DEBUG
    struct DebugVis {
        f64 frameHistory[60];
        f64 frameAvg = 0;
        u64 frameHistoryIdx = 0;
        bool help = true;
    };
    #endif

    struct Instance {
        Time time;
        CameraManager cameraMgr;
        RenderManager renderMgr;
        Gameplay::Movement::State player;
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
            generateMatrix(mgr.orthoProjection.matrix, ortho);

            // Todo: consider whether precision needs special handling
            PerspProjection::Config& frustum = mgr.perspProjection.config;
            frustum.fov = 45.f;
            frustum.aspect = platform.screen.width / (f32)platform.screen.height;
            frustum.near = 1.f;
            frustum.far = 1000.f;
            generateMatrix(mgr.perspProjection.matrix, frustum);
        }

        Vec3 meshCentroid = {};

        game.renderMgr.renderScene = {};
        {
            RenderScene& rscene = game.renderMgr.renderScene;

            Renderer::Driver::MainRenderTargetParams renderTargetParams = {};
            renderTargetParams.depth = true;
            renderTargetParams.width = platform.screen.width;
            renderTargetParams.height = platform.screen.height;
            Renderer::Driver::create(rscene.mainRenderTarget, renderTargetParams);

            // rasterizer states
            Renderer::Driver::create(rscene.blendStateOn, { true });
            Renderer::Driver::create(rscene.rasterizerStateFill, { Renderer::Driver::RasterizerFillMode::Fill, Renderer::Driver::RasterizerCullMode::CullBack });
            Renderer::Driver::create(rscene.rasterizerStateFillCulledBack, { Renderer::Driver::RasterizerFillMode::Fill, Renderer::Driver::RasterizerCullMode::CullFront });
            Renderer::Driver::create(rscene.rasterizerStateLine, { Renderer::Driver::RasterizerFillMode::Line, Renderer::Driver::RasterizerCullMode::CullBack });

            // cbuffers
            Renderer::create(rscene.cbuffers);

            // shaders
            {
                Renderer::TechniqueSrcParams< Renderer::Layout_Vec3Color4B, Renderer::Layout_CBuffer_3DScene::Buffers> params;
                Renderer::create<Renderer::Shaders::VSTechnique::forward_base
                               , Renderer::Shaders::PSTechnique::forward_untextured_unlit
                               , Renderer::Shaders::VSDrawType::Standard>
                    (params, rscene.cbuffers);
                Renderer::create(rscene.sceneShader, params);
            }
            {
                Renderer::TechniqueSrcParams< Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> params;
                Renderer::create<Renderer::Shaders::VSTechnique::forward_base
                               , Renderer::Shaders::PSTechnique::forward_untextured_unlit
                               , Renderer::Shaders::VSDrawType::Standard>
                    (params, rscene.cbuffers);
                Renderer::create(rscene.sceneUnlitShader, params);
            }
            {
                Renderer::TechniqueSrcParams< Renderer::Layout_TexturedVec3, Renderer::Layout_CBuffer_3DScene::Buffers> params;
                Renderer::create<Renderer::Shaders::VSTechnique::forward_base
                               , Renderer::Shaders::PSTechnique::forward_textured_lit_normalmapped
                               , Renderer::Shaders::VSDrawType::Standard>
                    (params, rscene.cbuffers);
                Renderer::create(rscene.sceneTexturedShader, params);
            }
            {
                Renderer::TechniqueSrcParams< Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> params;
                Renderer::create<Renderer::Shaders::VSTechnique::forward_base
                               , Renderer::Shaders::PSTechnique::forward_untextured_unlit
                               , Renderer::Shaders::VSDrawType::Instanced>
                    (params, rscene.cbuffers);
                Renderer::create(rscene.sceneInstancedShader, params);
            }

            // input mesh vertex buffer
            {
                {
                    auto& r = rscene.inputMeshGroupBuffer;
                    Math::identity4x4(r.transform);
                    const float scale = 1.f;
                    Math::identity4x4(r.localTransform);
                    r.localTransform.matrix.col0.x *= scale;
                    r.localTransform.matrix.col1.y *= -scale;
                    r.localTransform.matrix.col2.z *= scale;
                    // reverse culling, since we are inverting the model's y
                    r.rasterizerState = rscene.rasterizerStateFillCulledBack;
                    r.groupColor = Col(0.3f, 0.3f, 0.3f, 1.f);
                    Renderer::FBX::load(r.buffers, "assets/meshes/boar.fbx");
                }

                {
                    auto& r = rscene.inputMeshUnlitGroupBuffer;
                    Math::identity4x4(r.transform);
                    const float scale = 1.f;
                    Math::identity4x4(r.localTransform);
                    r.localTransform.matrix.col0.x *= scale;
                    r.localTransform.matrix.col1.y *= -scale;
                    r.localTransform.matrix.col2.z *= scale;
                    // reverse culling, since we are inverting the model's y
                    r.rasterizerState = rscene.rasterizerStateLine;
                    r.groupColor = Col(1.f, 1.f, 1.f, 1.f);
                    Renderer::FBX::load(r.buffers, "assets/meshes/boar.fbx");
                }
            }

            // unit cube vertex buffers
            {
                RenderItemUntexturedGeo& untextured = rscene.untexturedCubeGroupBuffer;
                untextured.rasterizerState = rscene.rasterizerStateFill;
                untextured.groupColor = Col(0.9f, 0.1f, 0.8f, 1.f);
                Renderer::create(untextured.buffer, { 1.f, 1.f, 1.f });
                Math::identity4x4(untextured.transform);

                RenderItemTexturedGeo& textured = rscene.texturedCubeGroupBuffer;
                Renderer::Driver::create(textured.albedo, { "assets/pbr/material04-albedo.png" });
                Renderer::Driver::create(textured.normal, { "assets/pbr/material04-normal.png" });
                Renderer::Driver::create(textured.depth, { "assets/pbr/material04-depth.png" });
                Renderer::create(textured.buffer, { 1.f, 1.f, 1.f });
                Math::identity4x4(textured.transform);
            }

            __DEBUGEXP(Renderer::Immediate::load(game.renderMgr.immediateBuffer));
        }

        game.player = {};
        {
            Math::identity4x4(game.player.transform);
            game.player.transform.pos = Vec3(5.f, 10.f, 0.f);
        }

        game.cameraMgr = {};
        {
            CameraManager& mgr = game.cameraMgr;
            Camera& cam = mgr.cameras[CameraManager::FlyCam];
            mgr.activeCam = &cam;

            mgr.orbitCamera.offset = Vec3(0.f, -100.f, 0.f);
            mgr.orbitCamera.eulers = Vec3(0.f, 0.f, 0.f);
            mgr.orbitCamera.scale = 1.f;
            mgr.orbitCamera.origin = Vec3(0.f, 0.f, 0.f);
        }
    }

    void update(Instance& game, Platform::GameConfig& config, const Platform::State& platform) {

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
        const ::Input::Gamepad::State& pad = platform.input.pads[0];
        const ::Input::Keyboard::State& keyboard = platform.input.keyboard;
        bool step = true;
        {
            if (keyboard.released(Input::EXIT)) {
                config.quit = true;
            }
            #if __DEBUG
            if (keyboard.pressed(Input::TOGGLE_HELP)) {
                game.debugVis.help = !game.debugVis.help;
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
                game.renderMgr.renderScene.inputMeshGroupBuffer.transform = game.player.transform;
            }

            // camera update
            {
                Gameplay::Orbit::process(game.cameraMgr.orbitCamera, platform.input.mouse);
                Gameplay::Orbit::process(activeCam->transform, game.cameraMgr.orbitCamera);
                // send camera data to render manager
                Renderer::generateModelViewMatrix(activeCam->viewMatrix, activeCam->transform);
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
                Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], cbufferPerScene);
            }

            {
                Renderer::Driver::bind(rscene.blendStateOn);
                Renderer::Driver::bind(rscene.mainRenderTarget);
                Renderer::Driver::clear(rscene.mainRenderTarget, Col(0.f, 0.f, 0.f, 1.f));

                // mesh
                {
                    const auto& r = rscene.inputMeshGroupBuffer;
                    Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                    buffer.worldMatrix = Math::mult(r.transform.matrix, r.localTransform.matrix);
                    buffer.groupColor = r.groupColor.RGBAv4();
                    Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);

                    Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count, { true, false });
                    Renderer::Driver::bind(rscene.sceneShader);
                    Renderer::Driver::bind(r.rasterizerState);
                    for (const auto& d : r.buffers) {
                        Renderer::Driver::bind(d);
                        draw(d);
                    }
                }
                {
                    const auto& r = rscene.inputMeshUnlitGroupBuffer;
                    Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                    buffer.worldMatrix = Math::mult(r.transform.matrix, r.localTransform.matrix);
                    buffer.groupColor = r.groupColor.RGBAv4();
                    Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);

                    Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count, { true, false });
                    Renderer::Driver::bind(rscene.sceneUnlitShader);
                    Renderer::Driver::bind(r.rasterizerState);
                    for (const auto& d : r.buffers) {
                        Renderer::Driver::bind(d);
                        draw(d);
                    }
                }
                
                // unit cubes
                {
                    {
                        const RenderItemTexturedGeo& r = rscene.texturedCubeGroupBuffer;
                        Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                        buffer.worldMatrix = r.transform.matrix;
                        Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);

                        // TODO: handle visibility of constant buffers better than this (or include it in the compile data of the shader)
                        Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count, { true, true });
                        Renderer::Driver::bind(rscene.sceneTexturedShader);
                        Renderer::Driver::bind(rscene.rasterizerStateFill);
                        Renderer::Driver::RscTexture textures[] = { r.albedo, r.normal, r.depth };
                        const u32 textureCount = sizeof(textures) / sizeof(textures[0]);
                        Renderer::Driver::bind(textures, textureCount);
                        Renderer::Driver::bind(r.buffer);
                        draw(r.buffer);
                        Renderer::Driver::RscTexture nullTex[] = { {}, {}, {} };
                        Renderer::Driver::bind(nullTex, textureCount);
                    }

                    {
                        const RenderItemUntexturedGeo& r = rscene.untexturedCubeGroupBuffer;
                        Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                        buffer.worldMatrix = r.transform.matrix;
                        buffer.groupColor = r.groupColor.RGBAv4();
                        Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);

                        Renderer::Driver::bind(rscene.sceneInstancedShader);
                        Renderer::Driver::bind(r.rasterizerState);
                        Renderer::Driver::bind(r.buffer);
                        Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count, { true, false });

                        Renderer::Layout_CBuffer_3DScene::InstanceData instancedBuffer;
                        u32 bufferId = 0;
                        u32 begin = 1;
                        u32 end = 4;
                        for (u32 i = begin; i < end; i++) {
                            Transform t;
                            Math::identity4x4(t);
                            t.pos = Math::add(rscene.inputMeshGroupBuffer.transform.pos, Math::scale(rscene.inputMeshGroupBuffer.transform.front, -5.f * i));
                            t.matrix.col0.x = 0.2f;
                            t.matrix.col1.y = 0.2f;
                            t.matrix.col2.z = 0.2f;
                            instancedBuffer.instanceMatrices[bufferId++] = t.matrix;
                        }

                        Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::InstanceData], instancedBuffer);
                        drawInstances(r.buffer, end - begin);
                    }

                }
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
                }
                // Some debug decoration
                if (game.debugVis.help)
                {
                    Col defaultCol(0.7f, 0.8f, 0.15f, 1.0f);
                    Col activeCol(1.0f, 0.2f, 0.1f, 1.0f);
                    Renderer::Immediate::TextParams textParams;
                    textParams.scale = 1;
                    textParams.pos = Vec3(game.renderMgr.orthoProjection.config.left + 10.f, game.renderMgr.orthoProjection.config.top - 15.f, -50);
                    textParams.color = platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_LEFT) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "left mouse click and drag to orbit");
                    textParams.pos.y -= 15.f;
                    textParams.color = platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_RIGHT) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "right mouse click and drag to pan");
                    textParams.pos.y -= 15.f;
                    textParams.color = platform.input.mouse.scrolldy != 0 ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "mouse wheel to scale");
                    textParams.pos.y -= 15.f;
                    textParams.color = keyboard.pressed(Input::TOGGLE_HELP) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "H to hide");
                    textParams.pos.y -= 15.f;

                    textParams.pos = Vec3(game.renderMgr.orthoProjection.config.right - 60.f, game.renderMgr.orthoProjection.config.top - 15.f, -50);
                    textParams.color = defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "%s", Platform::name);
                    textParams.pos.y -= 15.f;
                    textParams.pos.x -= 30.f;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "%.3lf fps", 1. / game.debugVis.frameAvg);
                    textParams.pos.y -= 15.f;
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
