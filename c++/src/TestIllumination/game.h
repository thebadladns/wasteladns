#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

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
          EXIT = ::Input::Keyboard::Keys::ESCAPE
        , TIMESWITCH = ::Input::Keyboard::Keys::SPACE
        , TIMEMOD = ::Input::Keyboard::Keys::RIGHT
        ;
        constexpr ::Input::Gamepad::Keys::Enum
          UP = ::Input::Gamepad::Keys::B_U
        , DOWN = ::Input::Gamepad::Keys::B_D
        , LEFT = ::Input::Gamepad::Keys::B_L
        , RIGHT = ::Input::Gamepad::Keys::B_R
        , C_UP = ::Input::Gamepad::Keys::D_U
        , C_DOWN = ::Input::Gamepad::Keys::D_D
        , C_LEFT = ::Input::Gamepad::Keys::D_L
        , C_RIGHT = ::Input::Gamepad::Keys::D_R
        ;
    };
    
    struct WorldData {
        Transform transform;
    };
    
    struct Control {
        Vec2 localInput;
        f32 mag;
    };

    struct MovementController {
        f32 speed;
    };
    
    
    struct ControlButtons {
        const ::Input::Gamepad::Keys::Enum up, down, left, right;
    };
    void process(Control& control, const ::Input::Gamepad::State& pad, const ControlButtons& buttons, const f32 dt) {
        
        const Vec2 prevLocalInput = control.localInput;
        
        Vec2 currentControl;
        currentControl.x = roundf(prevLocalInput.x);
        currentControl.y = roundf(prevLocalInput.y);
        
        Vec2 controlChange;
        controlChange.x = pad.pressed(buttons.right) * 1.f + pad.pressed(buttons.left) * -1.f;
        controlChange.y = pad.pressed(buttons.up) * 1.f + pad.pressed(buttons.down) * -1.f;
        
        Vec2 localInput;
        const bool downr = pad.down(buttons.right), downl = pad.down(buttons.left);
        if (controlChange.x != 0.f && currentControl.x != controlChange.x) {
            localInput.x = controlChange.x;
        } else if (downr && (currentControl.x > 0.f || !downl)) {
            localInput.x = 1.f;
        } else if (downl && (currentControl.x < 0.f || !downr)) {
            localInput.x = -1.f;
        } else {
            localInput.x = 0.f;
        }
        const bool downu = pad.down(buttons.up), downd = pad.down(buttons.down);
        if (controlChange.y != 0.f && currentControl.y != controlChange.y) {
            localInput.y = controlChange.y;
        } else if (downu && (currentControl.y > 0.f || !downd)) {
            localInput.y = 1.f;
        } else if (downd && (currentControl.y < 0.f || !downu)) {
            localInput.y = -1.f;
        } else {
            localInput.y = 0.f;
        }
        
        f32 mag = Math::mag(localInput);
        if (mag > Math::eps<f32>) {
            localInput = Math::invScale(localInput, mag);
            mag = Math::min(mag, 1.f);
        }
        
        control.localInput = localInput;
        control.mag = mag;
    }

    void process_absolute(Transform& t, const Control& control, const f32 dt) {

        f32 speed = 150.f;

        const Vec3& up = t.up;
        const Vec3& right = t.right;

        const f32 translation = control.mag * speed * dt;
        if (translation > Math::eps<f32>) {
            const Vec3 cameraRelativeInput(control.localInput, 0.f);
            const Vec3 worldInput = Math::add(Math::scale(up, cameraRelativeInput.y), Math::scale(right, cameraRelativeInput.x));
            const Vec3 worldVelocity = Math::scale(worldInput, translation);

            Vec3 pos = t.pos;
            pos = Math::add(pos, worldVelocity);
            t.pos = pos;
        }
    }
    
    void process_cameraRelative(Transform& transform, MovementController& controller, const Control& control, const Transform& camera, const f32 dt) {
        
        Transform33 movementTransform = Math::fromUpTowardsFront(transform.up, camera.front);
        const Vec3& front = movementTransform.front;
        const Vec3& right = movementTransform.right;
        
        // Interpolate input angles
        float effectiveLocalInputRad = 0.f;
        const float camCurrentRad = Math::orientation(camera.front.xy);
        const float playerCurrentRad = Math::orientation(transform.front.xy);
        const float camPlayerCurrentRad = Math::subtractShort(playerCurrentRad, camCurrentRad);
        const float camInputRad = Math::orientation(control.localInput);
        const float localInputRad = Math::subtractShort(camInputRad, camPlayerCurrentRad);
        effectiveLocalInputRad = Math::eappr(effectiveLocalInputRad, localInputRad, 0.14f, dt);
        const float effectiveCamInputRad = Math::wrap(camPlayerCurrentRad + effectiveLocalInputRad);
        const Vec2 effectiveLocalInput = Math::direction(effectiveCamInputRad);

        // Default to idle decceleration
        f32 speedtimehorizon = 0.1f;
        f32 targetspeed = 0.f;
        if (control.mag > Math::eps<f32>) {
            const f32 absLocalInputRad = Math::abs(localInputRad);
            const f32 minturndeltatomove = Math::pi<f32> / 2.f;
            if (absLocalInputRad < minturndeltatomove) {
                // Some friction on acceleration based on turn strength
                targetspeed = 150.f * control.mag;
                f32 turnfrictionfactor = Math::clamp(Math::abs(localInputRad) / minturndeltatomove, 0.f, 1.f);
                speedtimehorizon = Math::lerp(turnfrictionfactor, 0.014f, 0.3f);
            } else {
                // Stop on tight turns
                speedtimehorizon = -1.f;
                targetspeed = 0.f;
            }
        }
        if (speedtimehorizon > 0.f) {
            controller.speed = Math::eappr(controller.speed, targetspeed, speedtimehorizon, dt);
        } else {
            controller.speed = 0.f;
        }

        // Facing update
        if (control.mag > Math::eps<f32>) {
            const Vec3 cameraRelativeFacingInput(effectiveLocalInput, 0.f);
            const Vec3 worldFacingInput = Math::add(Math::scale(front, cameraRelativeFacingInput.y), Math::scale(right, cameraRelativeFacingInput.x));
            Transform33 t = Math::fromUpTowardsFront(transform.up, worldFacingInput);
            transform.front = t.front;
            transform.right = t.right;
            transform.up = t.up;
        }

        // Movement update
        if (controller.speed > Math::eps<f32>) {
            Vec3 cameraRelativeMovementInput;
            if (control.mag > Math::eps<f32>) {
                cameraRelativeMovementInput = Vec3(control.localInput, 0.f);
            } else {
                cameraRelativeMovementInput = Vec3(Math::direction(camPlayerCurrentRad), 0.f);
            }
            const f32 translation = controller.speed * dt;
            const Vec3 worldMovementInput = Math::add(Math::scale(front, cameraRelativeMovementInput.y), Math::scale(right, cameraRelativeMovementInput.x));
            const Vec3 worldVelocity = Math::scale(worldMovementInput, translation);
            Vec3 pos = transform.pos;
            pos = Math::add(pos, worldVelocity);
            transform.pos = pos;
        }
    }

    struct RenderInstance {
        Transform transform;
    };
    struct Material {
        Renderer::Driver::RscTexture albedo;
        Renderer::Driver::RscTexture normal;
        Renderer::Driver::RscTexture depth;
    };
    struct RenderDescription {
        Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec3> buffer;
        Renderer::Driver::RscRasterizerState rasterizerState;
        Material material;
        bool fill;
    };
    struct RenderGroup {
        RenderInstance instanceBuffer[64];
        RenderDescription desc;
        u32 instanceCount;
    };
    struct RenderScene {
        RenderGroup groupBuffer[16];
        u32 groupCount;
        Renderer::Driver::RscCBuffer cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::Count];
        Renderer::Driver::RscRasterizerState rasterizerStateFill, rasterizerStateLine;
        Renderer::Driver::RscBlendState blendStateOn;
        Renderer::Driver::RscBlendState blendStateOff;
        Renderer::Driver::RscMainRenderTarget mainRenderTarget;
        Renderer::Driver::RscShaderSet<Renderer::Layout_TexturedVec3, Renderer::Layout_CBuffer_3DScene::Buffers> geometryPassShader;
        Renderer::Driver::RscShaderSet<Renderer::Layout_TexturedVec2, Renderer::Layout_CNone::Buffers> quadShader;
        Renderer::Driver::RscTexture gPos;
        Renderer::Driver::RscTexture gNormal;
        Renderer::Driver::RscTexture gDiffuse;
        Renderer::Driver::RscRenderTarget gTarget;
        Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec2> quadBuffer;
        RenderGroup* begin() { return &groupBuffer[0]; }
        RenderGroup* end() { return &groupBuffer[groupCount]; }
    };
    struct RenderInstanceHandle {
        u32 desc;
        u32 inst;
    };

    struct Player {
        WorldData worldData;
        Control control;
        MovementController movementController;
        RenderInstanceHandle renderInst;
    };
    
    struct CameraManager {
        enum { FlyCam, CamCount };
        Camera cameras[CamCount];
        Control control;
        Camera* activeCam;
    };
    
    struct RenderManager {
        RenderScene renderScene;
        Renderer::Immediate::Buffer immediateBuffer;
        Renderer::OrthoProjection orthoProjection;
        Renderer::PerspProjection perspProjection;
        Mat4 viewMatrix;
    };
    
    struct Instance {
        Time time;
        CameraManager cameraMgr;
        RenderManager renderMgr;
        Player player;
    };
    
    void start(Instance& game, Platform::GameConfig& config, const Platform::State& platform) {

        game.time = {};
        game.time.config.maxFrameLength = 0.1;
        game.time.config.targetFramerate = 1.0 / 60.0;
        game.time.lastFrame = platform.time.now;
        
        config = {};
        config.nextFrame = platform.time.now;
        config.requestFlags = Platform::RequestFlags::PollKeyboard;
        
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

        RenderInstanceHandle playerRenderInst;
        game.renderMgr.renderScene = {};
        {
            RenderScene& rscene = game.renderMgr.renderScene;
            
            Renderer::Driver::MainRenderTargetParams renderTargetParams = {};
            renderTargetParams.depth = true;
            renderTargetParams.width = platform.screen.width;
            renderTargetParams.height = platform.screen.height;
            Renderer::Driver::create(rscene.mainRenderTarget, renderTargetParams);
            
            Renderer::Driver::create(rscene.blendStateOn, { true });
            Renderer::Driver::create(rscene.blendStateOff, { false });
            
            Renderer::Driver::create(rscene.rasterizerStateFill, { Renderer::RasterizerFillMode::Fill, true });
            Renderer::Driver::create(rscene.rasterizerStateLine, { Renderer::RasterizerFillMode::Line, true });


            Renderer::Driver::create<Renderer::Layout_CBuffer_3DScene::SceneData>(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], {});
            Renderer::Driver::create<Renderer::Layout_CBuffer_3DScene::GroupData>(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], {});

            {
                Renderer::Driver::RscVertexShader<Renderer::Layout_TexturedVec3, Renderer::Layout_CBuffer_3DScene::Buffers> rscVS;
                Renderer::Driver::RscPixelShader rscPS;
                Renderer::Driver::RscShaderSet<Renderer::Layout_TexturedVec3, Renderer::Layout_CBuffer_3DScene::Buffers> shaderSet;
                Renderer::Driver::ShaderResult result;
                result = Renderer::Driver::create(rscVS, { geometryPassVShaderStr, (u32)strlen(geometryPassVShaderStr) });
                if (result.compiled) {
                    result = Renderer::Driver::create(rscPS, { geometryPassPShaderStr, (u32)strlen(geometryPassPShaderStr) });
                    if (result.compiled) {
                        result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, rscene.cbuffers });
                        if (result.compiled) {
                            rscene.geometryPassShader = shaderSet;
                        }
                        else {
                            Platform::printf("link: %s", result.error);
                        }
                    }
                    else {
                        Platform::printf("PS: %s", result.error);
                    }
                }
                else {
                    Platform::printf("VS: %s", result.error);
                }
            }
            {
                Renderer::Driver::RscVertexShader<Renderer::Layout_TexturedVec2, Renderer::Layout_CNone::Buffers> rscVS;
                Renderer::Driver::RscPixelShader rscPS;
                Renderer::Driver::RscShaderSet<Renderer::Layout_TexturedVec2, Renderer::Layout_CNone::Buffers> shaderSet;
                Renderer::Driver::ShaderResult result;
                result = Renderer::Driver::create(rscVS, { quadVShaderStr, (u32)strlen(quadVShaderStr) });
                if (result.compiled) {
                    result = Renderer::Driver::create(rscPS, { quadPShaderStr, (u32)strlen(quadPShaderStr) });
                    if (result.compiled) {
                        result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, nullptr });
                        if (result.compiled) {
                            rscene.quadShader = shaderSet;
                        }
                        else {
                            Platform::printf("link: %s", result.error);
                        }
                    }
                    else {
                        Platform::printf("PS: %s", result.error);
                    }
                }
                else {
                    Platform::printf("VS: %s", result.error);
                }
            }

            // Deferred shading targets
            {
                Renderer::Driver::RenderTargetParams renderTargetParams;
                renderTargetParams.depth = true;
                renderTargetParams.width = platform.screen.width;
                renderTargetParams.height = platform.screen.height;
                Renderer::Driver::create(rscene.gTarget, renderTargetParams);
                Renderer::Driver::TextureRenderTargetCreateParams params;
                params.width = platform.screen.width;
                params.height = platform.screen.height;
                params.format = Renderer::TextureFormat::V316;
                params.type = Renderer::Type::Float;
                Renderer::Driver::create(rscene.gPos, params);
                Renderer::Driver::create(rscene.gNormal, params);
                Renderer::Driver::create(rscene.gDiffuse, params);
                Renderer::Driver::RscTexture textures[] = { rscene.gPos, rscene.gNormal, rscene.gDiffuse };
                Renderer::Driver::bind(rscene.gTarget, textures, sizeof(textures) / sizeof(textures[0]));
            }

            // Shader texture binding points
            {
                const char* params[] = { "texDiffuse", "texNormal", "texDepth" };
                Renderer::Driver::bind(rscene.geometryPassShader, params, sizeof(params) / sizeof(params[0]));
            }
            {
                const char* params[] = { "gPos", "gNormal", "gDiffuse" };
                Renderer::Driver::bind(rscene.quadShader, params, sizeof(params) / sizeof(params[0]));
            }
            {
                Renderer::MappedCube cube;
                Renderer::create(cube, { 25.f, 50.f, 25.f });

                Vec2 pillarPos[] = {
                      { -80.f, -20.f }, { 80.f, -20.f }, { -160.f, -20.f }, { 160.f, -20.f }
                    , { -240.f, -20.f }, { 240.f, -20.f }, { -300.f, -20.f }, { 300.f, -20.f }, { -80.f, -80.f }
                    , { 80.f, -80.f }, { -160.f, -80.f }, { 160.f, -80.f }, { -240.f, -80.f }, { 240.f, -80.f }
                    , { -300.f, -80.f }, { 300.f, -80.f }, { -100.f, 180.f }, { -300.f, 180.f }, { 300.f, 180.f }
                    , { 100.f, 180.f }, { -200.f, 180.f }, { 200.f, 180.f }
                };

                Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec3> rscBuffer;
                Renderer::Driver::IndexedBufferParams bufferParams;
                bufferParams.vertexData = cube.vertices;
                bufferParams.indexData = cube.indices;
                bufferParams.vertexSize = sizeof(cube.vertices);
                bufferParams.indexSize = sizeof(cube.indices);
                bufferParams.indexCount = bufferParams.indexSize / sizeof(cube.indices[0]);
                bufferParams.memoryMode = Renderer::BufferMemoryMode::GPU;
                bufferParams.indexType = Renderer::BufferItemType::U16;
                bufferParams.type = Renderer::BufferTopologyType::Triangles;
                Renderer::Driver::create(rscBuffer, bufferParams);

                RenderGroup& r = rscene.groupBuffer[rscene.groupCount++];
                r.desc.buffer = rscBuffer;
                r.desc.rasterizerState = rscene.rasterizerStateFill;
                Material& material = r.desc.material;
                Renderer::Driver::create(material.albedo, { "assets/pbr/material02-albedo.png" });
                Renderer::Driver::create(material.normal, { "assets/pbr/material02-normal.png" });
                Renderer::Driver::create(material.depth, { "assets/pbr/material02-depth.png" });
                for (Vec2& pos : pillarPos) {
                    RenderInstance& i = r.instanceBuffer[r.instanceCount++];
                    Math::identity4x4(i.transform);
                    i.transform.pos = Vec3(pos, 0.f);
                }
            }

            {
                Renderer::MappedCube cube;
                Renderer::create(cube, { 15.f, 30.f, 15.f });

                Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec3> rscBuffer;
                Renderer::Driver::IndexedBufferParams bufferParams;
                bufferParams.vertexData = (void*)cube.vertices;
                bufferParams.indexData = (void*)cube.indices;
                bufferParams.vertexSize = sizeof(cube.vertices);
                bufferParams.indexSize = sizeof(cube.indices);
                bufferParams.indexCount = bufferParams.indexSize / sizeof(cube.indices[0]);
                bufferParams.memoryMode = Renderer::BufferMemoryMode::GPU;
                bufferParams.indexType = Renderer::BufferItemType::U16;
                bufferParams.type = Renderer::BufferTopologyType::Triangles;
                Renderer::Driver::create(rscBuffer, bufferParams);

                playerRenderInst.desc = rscene.groupCount;
                RenderGroup& r = rscene.groupBuffer[rscene.groupCount++];
                r.desc.buffer = rscBuffer;
                r.desc.rasterizerState = rscene.rasterizerStateFill;

                Material& material = r.desc.material;
                Renderer::Driver::create(material.albedo, { "assets/pbr/material01-albedo.png" });
                Renderer::Driver::create(material.normal, { "assets/pbr/material01-normal.png" });
                Renderer::Driver::create(material.depth, { "assets/pbr/material01-depth.png" });

                playerRenderInst.inst = r.instanceCount;
                RenderInstance& i = r.instanceBuffer[r.instanceCount++];
                i.transform = game.player.worldData.transform;
            }
            
            {
                Renderer::RenderTargetTexturedQuad quad;
                Renderer::create(quad);
                
                Renderer::Driver::IndexedBufferParams bufferParams;
                bufferParams.vertexData = quad.vertices;
                bufferParams.indexData = quad.indices;
                bufferParams.vertexSize = sizeof(quad.vertices);
                bufferParams.indexSize = sizeof(quad.indices);
                bufferParams.indexCount = bufferParams.indexSize / sizeof(quad.indices[0]);
                bufferParams.memoryMode = Renderer::BufferMemoryMode::GPU;
                bufferParams.indexType = Renderer::BufferItemType::U16;
                bufferParams.type = Renderer::BufferTopologyType::Triangles;
                Renderer::Driver::create(rscene.quadBuffer, bufferParams);
            }
            
            Renderer::Immediate::load(game.renderMgr.immediateBuffer);
        }

        game.player = {};
        {
            Math::identity4x4(game.player.worldData.transform);
            game.player.worldData.transform.pos = Vec3(50.f, 0.f, 0.f);
            game.player.renderInst = playerRenderInst;
        }
        
        game.cameraMgr = {};
        {
            CameraManager& mgr = game.cameraMgr;
            using namespace CameraSystem;
            
            Camera& cam = mgr.cameras[CameraManager::FlyCam];
            cam.transform.pos = Vec3(70.f, -75.f, 150.f);
            Vec3 lookAt = game.player.worldData.transform.pos;
            Vec3 lookAtDir = Math::subtract(lookAt, cam.transform.pos);
            Math::fromFront(cam.transform, lookAtDir);
            Renderer::generateModelViewMatrix(cam.viewMatrix, cam.transform);
            
            mgr.activeCam = &cam;
        }
    }
    
    void update(Instance& game, Platform::GameConfig& config, const Platform::State& platform) {
        
        f64 raw_dt = platform.time.now - game.time.lastFrame;
        game.time.lastFrameDelta = Math::min(raw_dt, game.time.config.maxFrameLength);
        game.time.lastFrame = platform.time.now;
        config.nextFrame = platform.time.now + game.time.config.targetFramerate;
        
        const ::Input::Gamepad::State& pad = platform.input.pads[0];
        const ::Input::Keyboard::State& keyboard = platform.input.keyboard;

        bool step = true;
        {
            if (keyboard.released(Input::EXIT)) {
                config.quit = true;
            }
            if (keyboard.released(Input::TIMESWITCH)) {
                game.time.updateOverspeed = 0.;
            }
            if (keyboard.pressed(Input::TIMEMOD)) {
                game.time.updateOverspeed -= 0.1;
                game.time.updateOverspeed = Math::max(game.time.updateOverspeed, -0.95);
            }
            step = !game.time.paused;
        }

        if (step)
        {
            f32 dt = (f32) (game.time.lastFrameDelta * (1.0 + game.time.updateOverspeed));
            
            using namespace Game;
            
            Camera* activeCam = game.cameraMgr.activeCam;

            // Player
            const ControlButtons plyButtons = { Input::UP, Input::DOWN, Input::LEFT, Input::RIGHT };
            process(game.player.control, pad, plyButtons, dt);
            process_cameraRelative(game.player.worldData.transform, game.player.movementController, game.player.control, activeCam->transform, dt);
            RenderInstance& i = game.renderMgr.renderScene.groupBuffer[game.player.renderInst.desc].instanceBuffer[game.player.renderInst.inst];
            i.transform = game.player.worldData.transform;

            // Camera
            const ControlButtons camButtons = { Input::C_UP, Input::C_DOWN, Input::C_LEFT, Input::C_RIGHT };
            process(game.cameraMgr.control, pad, camButtons, dt);
            process_absolute(activeCam->transform, game.cameraMgr.control, dt);
            Vec3 lookAt(
                  game.player.worldData.transform.pos.xy
                , game.player.worldData.transform.pos.z + 50.f);
            Vec3 lookAtDir = Math::subtract(lookAt, activeCam->transform.pos);
            Math::fromFront(activeCam->transform, lookAtDir);
            Renderer::generateModelViewMatrix(activeCam->viewMatrix, activeCam->transform);
        }
            
        // Render update
        {
            RenderManager& mgr = game.renderMgr;
            RenderScene& rscene = mgr.renderScene;
            using namespace Renderer;

            Camera* activeCam = game.cameraMgr.activeCam;
            Renderer::Layout_CBuffer_3DScene::SceneData cbufferPerScene;
            cbufferPerScene.projectionMatrix = mgr.perspProjection.matrix;
            cbufferPerScene.viewMatrix = activeCam->viewMatrix;
            cbufferPerScene.viewPos = activeCam->transform.pos;
            Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], cbufferPerScene);

            // G buffer
            {
                // No blending when writing into color attachments
                Renderer::Driver::bind(rscene.blendStateOff);
                Renderer::Driver::bind(rscene.gTarget);
                Renderer::Driver::clear(rscene.gTarget);

                Renderer::Driver::bind(rscene.geometryPassShader);

                for (RenderGroup& r : rscene) {

                    RenderDescription& d = r.desc;

                    Renderer::Driver::RscTexture textures[] = {
                          d.material.albedo, d.material.normal, d.material.depth,
                    };
                    Renderer::Driver::bind(textures, sizeof(textures) / sizeof(textures[0]));

                    Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                    for (u32 i = 0; i < r.instanceCount; i++) {
                        buffer.worldMatrix[i] = r.instanceBuffer[i].transform.matrix;
                    }
                    Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);

                    Renderer::Driver::bind(d.rasterizerState);
                    Renderer::Driver::bind(d.buffer);
                    Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count, { true, false });
                    drawInstances(d.buffer, r.instanceCount);
                }

                Renderer::Driver::RscTexture nullTex[] = { {}, {}, {} };
                Renderer::Driver::bind(nullTex, sizeof(nullTex) / sizeof(nullTex[0]));

            }
            // buffer quad
            {
                Renderer::Driver::bind(rscene.blendStateOn);

                Renderer::Driver::bind(rscene.mainRenderTarget);
                Renderer::Driver::clear(rscene.mainRenderTarget, Col(0.f, 0.f, 0.f, 1.f));
                
                Renderer::Driver::bind(rscene.quadShader);

                Renderer::Driver::RscTexture textures[] = {
                      rscene.gPos, rscene.gNormal, rscene.gDiffuse
                };
                const u32 textureCount = sizeof(textures) / sizeof(textures[0]);
                Renderer::Driver::bind(textures, textureCount);

                Renderer::Driver::bind(rscene.quadBuffer);
                Renderer::Driver::draw(rscene.quadBuffer);

                Renderer::Driver::RscTexture nullTex[] = { {}, {}, {} };
                Renderer::Driver::bind(nullTex, textureCount);

                Renderer::Driver::RenderTargetCopyParams params;
                params.depth = true;
                Renderer::Driver::copy(rscene.mainRenderTarget, rscene.gTarget, params);
            }

             //Scene
            {
                // Immediate-mode debug. Can be moved out of the render update, it only pushes data to cpu buffers
                {
                    // Tiled floor
                    const f32 l = -500.;
                    const f32 r = -l;
                    const f32 d = -500.;
                    const f32 u = -d;
                    const f32 z = 0.f;
                    const f32 b = -200.f;
                    const f32 t = 200.f;
                    const f32 separation = 20.f;
                    const Col gridColor(1.0f, 1.0f, 1.0f, 0.25f);
                    for (f32 z = b; z < t + 0.001; z += separation) {
                        Immediate::segment(mgr.immediateBuffer, Vec3(l, d, z), Vec3(l, u, z), gridColor);
                        Immediate::segment(mgr.immediateBuffer, Vec3(r, d, z), Vec3(r, u, z), gridColor);
                        Immediate::segment(mgr.immediateBuffer, Vec3(l, d, z), Vec3(r, d, z), gridColor);
                        Immediate::segment(mgr.immediateBuffer, Vec3(l, u, z), Vec3(r, u, z), gridColor);
                    }
                    for (f32 x = l; x < r + 0.001; x += separation) {
                        Immediate::line(mgr.immediateBuffer, Vec3(x, (d + u)*0.5f, z), Vec3(0.f, 1.f, 0.f), gridColor);
                    }
                    for (f32 y = d; y < u + 0.001; y += separation) {
                        Immediate::line(mgr.immediateBuffer, Vec3((l + r)*0.5f, y, z), Vec3(1.f, 0.f, 0.f), gridColor);
                    }

                    // World axis
                    {
                        const Col axisX(0.8f, 0.15f, 0.25f, 0.7f);
                        const Col axisY(0.25f, 0.8f, 0.15f, 0.7f);
                        const Col axisZ(0.15f, 0.25f, 0.8f, 0.7f);
                        const f32 axisSize = 300.f;
                        const Vec3 pos = Vec3(0.f, 0.f, 0.1f);

                        Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(1.f, 0.f, 0.f), axisSize)), axisX);
                        Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(0.f, 1.f, 0.f), axisSize)), axisY);
                        Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(0.f, 0.f, 1.f), axisSize)), axisZ);
                    }

                    // Some debug decoration
                    {
                        Renderer::Immediate::TextParams textParams;
                        textParams.scale = 2;
                        textParams.pos = Vec3(game.renderMgr.orthoProjection.config.left + 10.f, game.renderMgr.orthoProjection.config.top - 10.f, -50);
                        textParams.color = Col(1.0f, 1.0f, 0.0f, 1.0f);
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "%s x%.2f", Platform::name, 1.0 + game.time.updateOverspeed);
                        Renderer::Immediate::sphere(game.renderMgr.immediateBuffer, Vec3(0.f, 0.f, 100.f), 5.f, Col(1.f, 1.f, 1.f, 1.f));
                    }
                }
            } // perspective

            // Batched debug
            {
                Immediate::present3d(mgr.immediateBuffer, mgr.perspProjection.matrix, game.cameraMgr.activeCam->viewMatrix);
                Immediate::present2d(mgr.immediateBuffer, mgr.orthoProjection.matrix);
                Immediate::clear(mgr.immediateBuffer);
            }
        }
    }
}

#endif // __WASTELADNS_GAME_H__
