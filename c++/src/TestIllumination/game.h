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
        s64 frameCount;
        bool paused;
    };
    
    namespace Input
    {
        constexpr ::Input::Keyboard::Keys::Enum
          EXIT = ::Input::Keyboard::Keys::ESCAPE
        , PAUSE = ::Input::Keyboard::Keys::SPACE
        , STEP = ::Input::Keyboard::Keys::RIGHT
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

    void process_absolute(WorldData& world, const Control& control, const f32 dt) {

        f32 speed = 150.f;

        const Vec3& up = world.transform.up;
        const Vec3& right = world.transform.right;

        const f32 translation = control.mag * speed * dt;
        if (translation > Math::eps<f32>) {
            const Vec3 cameraRelativeInput(control.localInput, 0.f);
            const Vec3 worldInput = Math::add(Math::scale(up, cameraRelativeInput.y), Math::scale(right, cameraRelativeInput.x));
            const Vec3 worldVelocity = Math::scale(worldInput, translation);

            Vec3 pos = world.transform.pos;
            pos = Math::add(pos, worldVelocity);
            world.transform.pos = pos;
        }
    }
    
    void process_cameraRelative(WorldData& world, const Control& control, const Transform& camera, const f32 dt) {
        
        f32 speed = 150.f;
        
        Transform33 movementTransform = Math::fromUpTowardsFront(world.transform.up, camera.front);
        const Vec3& front = movementTransform.front;
        const Vec3& right = movementTransform.right;
        
        const f32 translation = control.mag * speed * dt;
        if (translation > Math::eps<f32>) {
            const Vec3 cameraRelativeInput(control.localInput, 0.f);
            const Vec3 worldInput = Math::add(Math::scale(front, cameraRelativeInput.y), Math::scale(right, cameraRelativeInput.x));
            const Vec3 worldVelocity = Math::scale(worldInput, translation);
            
            Vec3 pos = world.transform.pos;
            pos = Math::add(pos, worldVelocity);
            
            Transform33 t = Math::fromUpTowardsFront(world.transform.up, worldInput);
            
            world.transform.front = t.front;
            world.transform.right = t.right;
            world.transform.up = t.up;
            world.transform.pos = pos;
        }
    }

    struct RenderInstance {
        Transform transform;
    };
    struct RenderDescription {
        Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec3> buffer;
        Renderer::Driver::RscRasterizerState rasterizerState;
        Col color;
        bool fill;
    };
    struct RenderGroup {
        RenderInstance instanceBuffer[64];
        RenderDescription desc;
        u32 instanceCount;
    };
    struct PointLight {
        Vec3 pos;
        Vec3 color;
    };
    struct RenderScene {
        RenderGroup groupBuffer[16];
        u32 groupCount;
        Renderer::Driver::RscCBuffer cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::Count];
        Renderer::Driver::RscRasterizerState rasterizerStateFill, rasterizerStateLine;
        Renderer::Driver::RscBlendState blendStateOn;
        Renderer::Driver::RscMainRenderTarget mainRenderTarget;
        Renderer::Driver::RscShaderSet<Renderer::Layout_TexturedVec3, Renderer::Layout_CBuffer_3DScene::Buffers> shaderSet;
        Renderer::Driver::RscTexture albedo;
        PointLight light;
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
            ortho.near = -1.f;
            ortho.far = 200.f;
            generateMatrix(mgr.orthoProjection.matrix, ortho);

            PerspProjection::Config& frustum = mgr.perspProjection.config;
            frustum.fov = 45.f;
            frustum.aspect = platform.screen.width / (f32)platform.screen.height;
            frustum.near = 1.f;
            frustum.far = 1000.f;
            generateMatrix(mgr.perspProjection.matrix, frustum);
        }

        Renderer::Immediate::load(game.renderMgr.immediateBuffer);

        RenderInstanceHandle playerRenderInst;
        game.renderMgr.renderScene = {};
        {
            RenderScene& rscene = game.renderMgr.renderScene;
            
            Renderer::Driver::create(rscene.blendStateOn, { true });
            Renderer::Driver::bind(rscene.blendStateOn);
            
            Renderer::Driver::create(rscene.rasterizerStateFill, { Renderer::RasterizerFillMode::Fill, true });
            Renderer::Driver::create(rscene.rasterizerStateLine, { Renderer::RasterizerFillMode::Line, true });

            Renderer::Driver::create<Renderer::Layout_CBuffer_3DScene::SceneData>(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], {});
            Renderer::Driver::create<Renderer::Layout_CBuffer_3DScene::GroupData>(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], {});
            
            {
                Renderer::Driver::RscVertexShader<Renderer::Layout_TexturedVec3, Renderer::Layout_CBuffer_3DScene::Buffers> rscVS;
                Renderer::Driver::RscPixelShader rscPS;
                Renderer::Driver::RscShaderSet<Renderer::Layout_TexturedVec3, Renderer::Layout_CBuffer_3DScene::Buffers> shaderSet;
                Renderer::Driver::ShaderResult result;
                result = Renderer::Driver::create(rscVS, { texturedVShaderStr, (u32)strlen(vertexShaderStr) });
                if (result.compiled) {
                    result = Renderer::Driver::create(rscPS, { texturedPShaderStr, (u32)strlen(pixelShaderStr) });
                    if (result.compiled) {
                        result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, rscene.cbuffers });
                        if (result.compiled) {
                            rscene.shaderSet = shaderSet;
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
            
            Renderer::Driver::create(rscene.mainRenderTarget, { true });
            
            {
                enum Normals { Y, NY, X, NX, Z, NZ};
                const Vec3 n[] = {
                      { 0.f, 1.f, 0.f }
                    , { 0.f, -1.f, 0.f }
                    , { 1.f, 0.f, 0.f }
                    , { -1.f, 0.f, 0.f }
                    , { 0.f, 0.f, 1.f }
                    , { 0.f, 0.f, -1.f }
                };
                const Vec2
                      bl = {0.f, 0.f}
                    , br = {1.f, 0.f}
                    , tr = {1.f, 1.f}
                    , tl = {0.f, 1.f}
                ;
                
                f32 pw = 25.f;
                f32 pz = pw * 2.f;
                Renderer::Layout_TexturedVec3 pillarVertices[] = {
                      {{ pw, pw, 0.f },br,n[Y]}, {{ -pw, pw, 0.f },bl,n[Y]}, {{ -pw, pw, pz },tl,n[Y]}, {{ pw, pw, pz },tr,n[Y]} // +y quad
                    , {{ -pw, -pw, 0.f },br,n[NY]}, {{ pw, -pw, 0.f },bl,n[NY]}, {{ pw, -pw, pz },tl,n[NY]}, {{ -pw, -pw, pz},tr,n[NY]} // -y quad
                    , {{ pw, -pw, 0.f},br,n[X]}, {{ pw, pw, 0.f },bl,n[X]}, {{ pw, pw, pz},tl,n[X]}, {{ pw, -pw, pz },tr,n[X]} // +x quad
                    , {{ -pw, pw, 0.f },br,n[NX]}, {{ -pw, -pw, 0.f },bl,n[NX]}, {{ -pw, -pw, pz },tl,n[NX]}, {{ -pw, pw, pz },tr,n[NX]} // -x quad
                    
                    , {{ -pw, -pw, pz },bl,n[Z]}, {{ pw, -pw, pz },br,n[Z]}, {{ pw, pw, pz },tr,n[Z]}, {{ -pw, pw, pz },tl,n[Z]} // +z quad
                    , {{ pw, pw, 0.f },bl,n[NZ]}, {{ pw, -pw, 0.f },br,n[NZ]}, {{ -pw, -pw, 0.f },tr,n[NZ]}, {{ -pw, pw, 0.f },tl,n[NZ]} // -z quad
                    
                };
                u16 pillarIndexes[] = {
                    0, 1, 2, 3, 0, 2,       // +y tris
                    4, 5, 6, 7, 4, 6,       // -y tris
                    
                    8, 9, 10, 11, 8, 10,    // +x tris
                    12, 13, 14, 15, 12, 14, // -x tris
                    
                    16, 17, 18, 19, 16, 18, // +z tris
                    20, 21, 22, 23, 20, 22, // -z tris
                };
                const Col pillarColor(0.956f, 0.29f, 0.18f, 1.0f);

                Vec2 pillarPos[] = {
                      { -80.f, -20.f }, { 80.f, -20.f }, { -160.f, -20.f }, { 160.f, -20.f }
                    , { -240.f, -20.f }, { 240.f, -20.f }, { -300.f, -20.f }, { 300.f, -20.f }, { -80.f, -80.f }
                    , { 80.f, -80.f }, { -160.f, -80.f }, { 160.f, -80.f }, { -240.f, -80.f }, { 240.f, -80.f }
                    , { -300.f, -80.f }, { 300.f, -80.f }, { -100.f, 180.f }, { -300.f, 180.f }, { 300.f, 180.f }
                    , { 100.f, 180.f }, { -200.f, 180.f }, { 200.f, 180.f }
                };

                Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec3> rscBuffer;
                Renderer::Driver::IndexedBufferParams bufferParams;
                bufferParams.vertexData = pillarVertices;
                bufferParams.indexData = pillarIndexes;
                bufferParams.vertexSize = sizeof(pillarVertices);
                bufferParams.indexSize = sizeof(pillarIndexes);
                bufferParams.indexCount = bufferParams.indexSize / sizeof(pillarIndexes[0]);
                bufferParams.memoryMode = Renderer::BufferMemoryMode::GPU;
                bufferParams.indexType = Renderer::BufferItemType::U16;
                bufferParams.type = Renderer::BufferTopologyType::Triangles;
                Renderer::Driver::create(rscBuffer, bufferParams);

                RenderGroup& r = rscene.groupBuffer[rscene.groupCount++];
                r.desc.buffer = rscBuffer;
                r.desc.rasterizerState = rscene.rasterizerStateFill;
                r.desc.color = pillarColor;
                for (Vec2& pos : pillarPos) {
                    RenderInstance& i = r.instanceBuffer[r.instanceCount++];
                    Math::identity4x4(i.transform);
                    i.transform.pos = Vec3(pos, 0.f);
                }
            }

            {
                const Vec2
                      bl = {0.f, 0.f}
                    , br = {1.f, 0.f}
                    , tr = {1.f, 1.f}
                    , tl = {0.f, 1.f}
                ;
                
                f32 w = 4.f;
                f32 wf = w;
                f32 wb = w;
                f32 h = 5.f;
                f32 z = 30.f;
                f32 yoff = 0.f;//-2.f;
                f32 zoff = 0.f;
                
                // pyramid sides
                const Vec3
                      nz = { 0.f, 0.f, 1.f }
                    , n = {}
                ;
                Renderer::Layout_TexturedVec3 v[] = {
                      {{ -wb, -h, z},bl,nz}, {{ -wf, h, z},tl,nz}, {{ wf, h, z},tr,nz}, {{ wb, -h, z},br,nz} // +z side
                    , {{ 0.f, yoff, -zoff},bl,n}, {{ -wf, h, z},tl,n}, {{ wf, h, z},tr,n} // y side
                    , {{ 0.f, yoff, -zoff},bl,n}, {{ wb, -h, z},tl,n}, {{ -wb, -h, z},tr,n} // -y side
                    , {{ 0.f, yoff, -zoff},bl,n}, {{ wf, h, z},tl,n}, {{ wb, -h, z},tr,n} // x side
                    , {{ 0.f, yoff, -zoff},bl,n}, {{ -wb, -h, z},tl,n}, {{ -wf, h, z},tr,n} // -x side
                };
                v[4].normal = Math::normalize(Math::cross(Math::subtract(v[4].pos,v[5].pos), Math::subtract(v[4].pos,v[6].pos)));
                v[5].normal = v[4].normal;
                v[6].normal = v[4].normal;
                v[7].normal = Math::normalize(Math::cross(Math::subtract(v[7].pos,v[8].pos), Math::subtract(v[7].pos,v[9].pos)));
                v[8].normal = v[7].normal;
                v[9].normal = v[7].normal;
                v[10].normal = Math::normalize(Math::cross(Math::subtract(v[10].pos,v[11].pos), Math::subtract(v[10].pos,v[12].pos)));
                v[11].normal = v[10].normal;
                v[12].normal = v[10].normal;
                v[13].normal = Math::normalize(Math::cross(Math::subtract(v[13].pos,v[14].pos), Math::subtract(v[13].pos,v[15].pos)));
                v[14].normal = v[13].normal;
                v[15].normal = v[13].normal;
                
                const u16 playerIndices[] = {
                      4, 5, 6, 7, 8, 9          // y faces
                    , 10, 11, 12, 13, 14, 15    // x faces
                    , 0, 3, 2, 1, 0, 2          // top tris
                };
                const Col playerOutlineColor(1.0f, 1.0f, 1.0f, 1.0f);

                Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec3> rscBuffer;
                Renderer::Driver::IndexedBufferParams bufferParams;
                bufferParams.vertexData = (void*)v;
                bufferParams.indexData = (void*)playerIndices;
                bufferParams.vertexSize = sizeof(v);
                bufferParams.indexSize = sizeof(playerIndices);
                bufferParams.indexCount = bufferParams.indexSize / sizeof(playerIndices[0]);
                bufferParams.memoryMode = Renderer::BufferMemoryMode::GPU;
                bufferParams.indexType = Renderer::BufferItemType::U16;
                bufferParams.type = Renderer::BufferTopologyType::Triangles;
                Renderer::Driver::create(rscBuffer, bufferParams);

                playerRenderInst.desc = rscene.groupCount;
                RenderGroup& r = rscene.groupBuffer[rscene.groupCount++];
                r.desc.buffer = rscBuffer;
                r.desc.rasterizerState = rscene.rasterizerStateFill;
                r.desc.color = playerOutlineColor;

                playerRenderInst.inst = r.instanceCount;
                RenderInstance& i = r.instanceBuffer[r.instanceCount++];
                i.transform = game.player.worldData.transform;
            }
            
            Renderer::Driver::create( rscene.albedo, { "assets/pbr/material01-albedo.png" } );
            rscene.light = {};
            rscene.light.color = { 0.6f, 0.9f, 0.2f };
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
            cam.transform.pos = Vec3(100.f, -115.f, 210.f);
            Vec3 lookAt = game.player.worldData.transform.pos;
            Vec3 lookAtDir = Math::subtract(lookAt, cam.transform.pos);
            Math::fromFront(cam.transform, lookAtDir);
            Renderer::generateModelViewMatrix(cam.viewMatrix, cam.transform);
            
            mgr.activeCam = &cam;
        }
        
        // gl hacks
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
            if (keyboard.released(Input::PAUSE)) {
                game.time.paused = !game.time.paused;
            }
            step = !game.time.paused || keyboard.pressed(Input::STEP);
        }

        if (step)
        {
            f32 dt = (f32) game.time.lastFrameDelta;
            
            using namespace Game;
            
            Camera* activeCam = game.cameraMgr.activeCam;

            // Player
            const ControlButtons plyButtons = { Input::UP, Input::DOWN, Input::LEFT, Input::RIGHT };
            process(game.player.control, pad, plyButtons, dt);
            process_cameraRelative(game.player.worldData, game.player.control, activeCam->transform, dt);
            RenderInstance& i = game.renderMgr.renderScene.groupBuffer[game.player.renderInst.desc].instanceBuffer[game.player.renderInst.inst];
            i.transform = game.player.worldData.transform;

            // Camera
            const ControlButtons camButtons = { Input::C_UP, Input::C_DOWN, Input::C_LEFT, Input::C_RIGHT };
            process(game.cameraMgr.control, pad, camButtons, dt);
            WorldData& wd = *((WorldData*)&activeCam->transform);
            process_absolute(wd, game.cameraMgr.control, dt);
            Vec3 lookAt = game.player.worldData.transform.pos;
            Vec3 lookAtDir = Math::subtract(lookAt, activeCam->transform.pos);
            Math::fromFront(activeCam->transform, lookAtDir);
            Renderer::generateModelViewMatrix(activeCam->viewMatrix, activeCam->transform);
        }
            
        // Render update
        {
            RenderManager& mgr = game.renderMgr;
            RenderScene& rscene = mgr.renderScene;
            using namespace Renderer;

            Renderer::Driver::clear(rscene.mainRenderTarget, Col(0.f, 0.f, 0.f, 1.f));

            // Scene
            {
                Camera* activeCam = game.cameraMgr.activeCam;
                Renderer::Layout_CBuffer_3DScene::SceneData cbufferPerScene;
                cbufferPerScene.projectionMatrix = mgr.perspProjection.matrix;
                cbufferPerScene.viewMatrix = activeCam->viewMatrix;
                Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], cbufferPerScene);

                Renderer::Driver::bind(rscene.shaderSet);
                Renderer::Driver::RscTexture textures[] = { rscene.albedo };
                Renderer::Driver::bind(textures, 1);
                for (RenderGroup& r : rscene) {

                    RenderDescription& d = r.desc;

                    Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                    for (u32 i = 0; i < r.instanceCount; i++) {
                        buffer.worldMatrix[i] = r.instanceBuffer[i].transform.matrix;
                    }
                    buffer.color = d.color.RGBAv4();
                    Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);
                        
                    Renderer::Driver::bind(d.rasterizerState);
                    Renderer::Driver::bind(d.buffer);
                    Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count);
                    drawInstances(d.buffer, r.instanceCount);
                }

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
                        const Vec3 pos = Vec3(0.f, 0.f, 0.f);

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
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "%s", Platform::name);
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
