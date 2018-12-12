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
    
    struct Window {
        GLFWwindow* handle;
        f32 desiredRatio;
        u32 width;
        u32 height;
        bool fullscreen = false;
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
        ;
    };
    
    struct WorldData {
        Transform transform;
    };
    
    struct Control {
        Vec2 localInput;
        f32 mag;
    };
    
    void process(Control& control, const ::Input::Gamepad::State& pad, const f32 dt) {
        
        const Vec2 prevLocalInput = control.localInput;
        
        Vec2 currentControl;
        currentControl.x = roundf(prevLocalInput.x);
        currentControl.y = roundf(prevLocalInput.y);
        
        Vec2 controlChange;
        controlChange.x = pad.pressed(Input::RIGHT) * 1.f + pad.pressed(Input::LEFT) * -1.f;
        controlChange.y = pad.pressed(Input::UP) * 1.f + pad.pressed(Input::DOWN) * -1.f;
        
        Vec2 localInput;
        const bool downr = pad.down(Input::RIGHT), downl = pad.down(Input::LEFT);
        if (controlChange.x != 0.f && currentControl.x != controlChange.x) {
            localInput.x = controlChange.x;
        } else if (downr && (currentControl.x > 0.f || !downl)) {
            localInput.x = 1.f;
        } else if (downl && (currentControl.x < 0.f || !downr)) {
            localInput.x = -1.f;
        } else {
            localInput.x = 0.f;
        }
        const bool downu = pad.down(Input::UP), downd = pad.down(Input::DOWN);
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
    
    void process(WorldData& world, const Control& control, const Transform& camera, const f32 dt) {
        
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

    typedef u32 VertexShaderHandle;
    typedef u32 PixelShaderHandle;
    typedef u32 ShaderHandle;
    typedef u32 BufferHandle;
    struct InputLayout {
        u32 mvp;
        u32 perScene;
        u32 perRenderGroup;
        u32 perRenderInstance;
    };
    
    struct RenderInstance {
        Transform transform;
    };
    struct RenderDescription {
        Col color;
        u32 vertexBuffer;
        u32 layoutBuffer;
        u32 indexBuffer;
        u32 indexCount;
        bool fill;
    };
    struct RenderGroup {
        RenderInstance instanceBuffer[128];
        RenderDescription desc;
        u32 instanceCount;
        RenderInstance* begin() { return &instanceBuffer[0]; }
        RenderInstance* end() { return &instanceBuffer[instanceCount]; }
    };
    struct CBuffer {
        enum { PerScene, PerRenderGroup, PerRenderInstance, Count };
        struct SceneData {
            Mat4 projectionMatrix;
            Mat4 viewMatrix;
        };
        struct RenderGroupData {
            Vec4 color;
        };
        struct RenderInstanceData {
            Mat4 worldMatrix;
        };
    };
    struct RenderScene {
        RenderGroup groupBuffer[16];
        BufferHandle cbuffers[CBuffer::Count];
        u32 groupCount;
        ShaderHandle shader;
        InputLayout layout;
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
        Camera* activeCam;
    };
    
    struct Instance {
        Time time;
        CameraManager cameraMgr;
        Renderer::Instance renderMgr;
        RenderScene renderScene;
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
            Renderer::Instance& mgr = game.renderMgr;
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
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_CULL_FACE);
            //glFrontFace(GL_CW);
        }
        
        ShaderHandle shader = {};
        InputLayout layout = {};
        {
            VertexShaderHandle vertexShader;
            PixelShaderHandle pixelShader;

            s32 compiled, infoLogLength;

            vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vertexShaderStr, nullptr);
            glCompileShader(vertexShader);
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
            if (compiled) {
                pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(pixelShader, 1, &pixelShaderStr, nullptr);
                glCompileShader(pixelShader);
                glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &compiled);
                if (compiled) {
                    shader = glCreateProgram();
                    glAttachShader(shader, vertexShader);
                    glAttachShader(shader, pixelShader);
                    glLinkProgram(shader);

                    glGetProgramiv(shader, GL_LINK_STATUS, &compiled);
                    if (compiled) {
                        layout.perScene = glGetUniformBlockIndex(shader, "PerScene");
                        layout.perRenderGroup = glGetUniformBlockIndex(shader, "PerRenderGroup");
                        layout.perRenderInstance = glGetUniformBlockIndex(shader, "PerRenderInstance");

                        glUniformBlockBinding(shader, layout.perScene, CBuffer::PerScene);
                        glUniformBlockBinding(shader, layout.perRenderGroup, CBuffer::PerRenderGroup);
                        glUniformBlockBinding(shader, layout.perRenderInstance, CBuffer::PerRenderInstance);
                    } else {
                        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
                        if (infoLogLength) {
                            char error[128];
                            glGetProgramInfoLog(shader, Math::min(infoLogLength, 128), nullptr, &error[0]);
                            Platform::printf("link: %s", error);
                        }
                    }
                    glDetachShader(shader, vertexShader);
                    glDetachShader(shader, pixelShader);
                } else {
                    glGetShaderiv(pixelShader, GL_INFO_LOG_LENGTH, &infoLogLength);
                    if (infoLogLength) {
                        char error[128];
                        glGetShaderInfoLog(pixelShader, Math::min(infoLogLength, 128), nullptr, &error[0]);
                        Platform::printf("PS: %s", error);
                    }
                }
                glDeleteShader(pixelShader);
            } else {
                glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
                if (infoLogLength) {
                    char error[128];
                    glGetShaderInfoLog(vertexShader, Math::min(infoLogLength, 128), nullptr, &error[0]);
                    Platform::printf("VS: %s", error);
                }
            }
            glDeleteShader(vertexShader);
        }

        BufferHandle cbuffers[CBuffer::Count];
        {
            glGenBuffers(CBuffer::Count, cbuffers);
            glBindBuffer(GL_UNIFORM_BUFFER, cbuffers[CBuffer::PerScene]);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(CBuffer::SceneData), nullptr, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, CBuffer::PerScene, cbuffers[CBuffer::PerScene]);

            glBindBuffer(GL_UNIFORM_BUFFER, cbuffers[CBuffer::PerRenderGroup]);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(CBuffer::RenderGroupData), nullptr, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, CBuffer::PerRenderGroup, cbuffers[CBuffer::PerRenderGroup]);

            glBindBuffer(GL_UNIFORM_BUFFER, cbuffers[CBuffer::PerRenderInstance]);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(CBuffer::RenderInstanceData), nullptr, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, CBuffer::PerRenderInstance, cbuffers[CBuffer::PerRenderInstance]);
            
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        Renderer::Immediate::load(game.renderMgr.immediateBuffer);

        RenderInstanceHandle playerRenderInst;
        game.renderScene = {};
        game.renderScene.shader = shader;
        game.renderScene.layout = layout;
        {
            {
                u32 count = 0;
                for (BufferHandle handle : cbuffers) {
                    game.renderScene.cbuffers[count++] = handle;
                }
            }

            {
                f32 pw = 5.f;
                f32 pz = 500.f;
                Vec3 pillarVertices[] = {
                      { -pw, -pw, 0.f }, { pw, -pw, 0.f}, { pw, -pw, pz}, { -pw, -pw, pz} // +y quad
                    , { pw, pw, 0.f }, { -pw, pw, 0.f }, { -pw, pw, pz }, { pw, pw, pz } // -y quad
                };
                u16 pillarIndexes[] = {
                    0, 1, 2, 3, 0, 2, // +y tris
                    4, 5, 6, 7, 4, 6, // -y tris
                    1, 4, 7, 2, 1, 7, // +x tris
                    5, 0, 3, 6, 5, 3, // -x tris
                };
                const u32 pillarIndexCount = sizeof(pillarIndexes) / sizeof(pillarIndexes[0]);
                const Col pillarColor(1.0f, 1.0f, 1.0f, 0.5f);

                Vec2 pillarPos[] = {
                      { -80.f, -20.f }, { 80.f, -20.f }, { -160.f, -20.f }, { 160.f, -20.f }
                    , { -240.f, -20.f }, { 240.f, -20.f }, { -300.f, -20.f }, { 300.f, -20.f }, { -80.f, -80.f }
                    , { 80.f, -80.f }, { -160.f, -80.f }, { 160.f, -80.f }, { -240.f, -80.f }, { 240.f, -80.f }
                    , { -300.f, -80.f }, { 300.f, -80.f }, { -20.f, 180.f }, { 20.f, 180.f }, { -100.f, 180.f }
                    , { 100.f, 180.f }, { -200.f, 180.f }, { 200.f, 180.f }, { -300.f, 180.f }, { 300.f, 180.f }
                };

                u32 vertexBuffer, indexBuffer, layoutBuffer;

                // Vertex buffer binding is not part of the VAO state
                glGenBuffers(1, &vertexBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(pillarVertices), pillarVertices, GL_STATIC_DRAW);
                glGenVertexArrays(1, &layoutBuffer);
                glBindVertexArray(layoutBuffer);
                {
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
                    glEnableVertexAttribArray(0);

                    glGenBuffers(1, &indexBuffer);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pillarIndexes), pillarIndexes, GL_STATIC_DRAW);
                }
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                RenderGroup& r = game.renderScene.groupBuffer[game.renderScene.groupCount++];
                r.desc.vertexBuffer = vertexBuffer;
                r.desc.layoutBuffer = layoutBuffer;
                r.desc.indexBuffer = indexBuffer;
                r.desc.indexCount = pillarIndexCount;
                r.desc.color = pillarColor;
                r.desc.fill = true;
                for (Vec2& pos : pillarPos) {
                    RenderInstance& i = r.instanceBuffer[r.instanceCount++];
                    Math::identity4x4(i.transform);
                    i.transform.pos = Vec3(pos, 0.f);
                }
            }

            {
                f32 w = 4.f;
                f32 wf = w;
                f32 wb = w;
                f32 h = 5.f;
                f32 z = 30.f;
                f32 yoff = -2.f;
                f32 zoff = 0.f;
                // pyramid sides
                const Vec3 playerVertices[] = {
                      { 0.f, yoff, -zoff} // pyramid tip
                    , { -wb, -h, z}, { -wf, h, z}, { wf, h, z}, { wb, -h, z} // pyramid sides
                };
                const u16 playerIndices[] = {
                      0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1 // side tris
                    , 1, 4, 3, 2, 1, 3 // top tris
                };
                const u32 playerIndexCount = sizeof(playerIndices) / sizeof(playerIndices[0]);
                const Col playerOutlineColor(1.0f, 1.0f, 1.0f, 1.0f);

                u32 vertexBuffer, indexBuffer, layoutBuffer;

                // Vertex buffer binding is not part of the VAO state
                glGenBuffers(1, &vertexBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(playerVertices), playerVertices, GL_STATIC_DRAW);
                glGenVertexArrays(1, &layoutBuffer);
                glBindVertexArray(layoutBuffer);
                {
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
                    glEnableVertexAttribArray(0);

                    glGenBuffers(1, &indexBuffer);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(playerIndices), playerIndices, GL_STATIC_DRAW);
                }
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                playerRenderInst.desc = game.renderScene.groupCount;
                RenderGroup& r = game.renderScene.groupBuffer[game.renderScene.groupCount++];
                r.desc.vertexBuffer = vertexBuffer;
                r.desc.layoutBuffer = layoutBuffer;
                r.desc.indexBuffer = indexBuffer;
                r.desc.indexCount = playerIndexCount;
                r.desc.color = playerOutlineColor;
                r.desc.fill = false;

                playerRenderInst.inst = r.instanceCount;
                RenderInstance& i = r.instanceBuffer[r.instanceCount++];
                i.transform = game.player.worldData.transform;
            }
        }
        
        game.cameraMgr = {};
        {
            CameraManager& mgr = game.cameraMgr;
            using namespace CameraSystem;
            
            Camera& cam = mgr.cameras[CameraManager::FlyCam];
            cam.transform.pos = Vec3(100.f, -115.f, 210.f);
            Vec3 lookAt = { 0.f, 0.f, 0.f };
            Vec3 lookAtDir = Math::subtract(lookAt, cam.transform.pos);
            Math::fromFront(cam.transform, lookAtDir);
            Renderer::generateModelViewMatrix(cam.viewMatrix, cam.transform);
            
            mgr.activeCam = &cam;
        }
        
        game.player = {};
        {
            Math::identity4x4(game.player.worldData.transform);
            game.player.worldData.transform.pos = Vec3(50.f, 0.f, 0.f);
            game.player.renderInst = playerRenderInst;
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
            if (keyboard.released(Input::PAUSE)) {
                game.time.paused = !game.time.paused;
            }
            step = !game.time.paused || keyboard.pressed(Input::STEP);
        }

        if (step)
        {
            f32 dt = (f32) game.time.lastFrameDelta;
            
            using namespace Game;
            
            const Camera* activeCam = game.cameraMgr.activeCam;
            process(game.player.control, pad, dt);
            process(game.player.worldData, game.player.control, activeCam->transform, dt);
            RenderInstance& i = game.renderScene.groupBuffer[game.player.renderInst.desc].instanceBuffer[game.player.renderInst.inst];
            i.transform = game.player.worldData.transform;
        }
            
        // Render update
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        {
            Renderer::Instance& mgr = game.renderMgr;
            using namespace Renderer;


            // PERSPECTIVE
            {
                {
                    glUseProgram(game.renderScene.shader);
   
                    CBuffer::SceneData cbufferPerScene;
                    cbufferPerScene.projectionMatrix = mgr.perspProjection.matrix;
                    cbufferPerScene.viewMatrix = game.cameraMgr.activeCam->viewMatrix;
                    glBindBuffer(GL_UNIFORM_BUFFER, game.renderScene.cbuffers[CBuffer::PerScene]);
                    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CBuffer::SceneData), &cbufferPerScene);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    for (RenderGroup& r : game.renderScene) {
                        const RenderDescription& d = r.desc;

                        CBuffer::RenderGroupData cbufferPerRenderGroup = {};
                        cbufferPerRenderGroup.color = d.color.RGBAv4();
                        glBindBuffer(GL_UNIFORM_BUFFER, game.renderScene.cbuffers[CBuffer::PerRenderGroup]);
                        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CBuffer::RenderGroupData), &cbufferPerRenderGroup);
                        glBindBuffer(GL_UNIFORM_BUFFER, 0);

                        glPolygonMode( GL_FRONT_AND_BACK, d.fill ? GL_FILL : GL_LINE );
                        glBindVertexArray(d.layoutBuffer);
                        for (RenderInstance& i : r) {

                            CBuffer::RenderInstanceData cbufferPerRenderInstance;
                            cbufferPerRenderInstance.worldMatrix = i.transform.matrix;
                            glBindBuffer(GL_UNIFORM_BUFFER, game.renderScene.cbuffers[CBuffer::PerRenderInstance]);
                            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CBuffer::RenderInstanceData), &cbufferPerRenderInstance);
                            glBindBuffer(GL_UNIFORM_BUFFER, 0);
                            glDrawElements(GL_TRIANGLES, d.indexCount, GL_UNSIGNED_SHORT, nullptr);
                        }
                        glBindVertexArray(0);
                    }
                    glUseProgram(0);

                    // Tiled floor
                    const f32 l = -500.;
                    const f32 r = -l;
                    const f32 b = -500.;
                    const f32 t = -b;
                    const f32 z = 0.f;
                    const f32 separation = 20.f;
                    const Col gridColor(1.0f, 1.0f, 1.0f, 0.25f);
                    for (f32 x = l; x < r + 0.001; x += separation) {
                        Immediate::line(mgr.immediateBuffer, Vec3(x, (b+t)*0.5f, z), Vec3(0.f, 1.f, 0.f), gridColor);
                    }
                    for (f32 y = b; y < t + 0.001; y += separation) {
                        Immediate::line(mgr.immediateBuffer, Vec3((l+r)*0.5f, y, z), Vec3(1.f, 0.f, 0.f), gridColor);
                    }
                    for (f32 z = l; z < r + 0.001; z += separation) {
                        Immediate::segment(mgr.immediateBuffer, Vec3(l, b, z), Vec3(l, t, z), gridColor);
                        Immediate::segment(mgr.immediateBuffer, Vec3(r, b, z), Vec3(r, t, z), gridColor);
                        Immediate::segment(mgr.immediateBuffer, Vec3(l, b, z), Vec3(r, b, z), gridColor);
                        Immediate::segment(mgr.immediateBuffer, Vec3(l, t, z), Vec3(r, t, z), gridColor);
                    }

                    // World axis
                    {
                        const Col axisX(0.8f, 0.15f, 0.25f, 0.7f);
                        const Col axisY(0.25f, 0.8f, 0.15f, 0.7f);
                        const Col axisZ(0.15f, 0.25f, 0.8f, 0.7f);
                        const f32 axisSize = 300.f;
                        const Vec3 pos = Vec3(0.f, 0.f, 0.f);

                        Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(1.f,0.f,0.f), axisSize)), axisX);
                        Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(0.f,1.f,0.f), axisSize)), axisY);
                        Immediate::segment(mgr.immediateBuffer, pos, Math::add(pos, Math::scale(Vec3(0.f,0.f,1.f), axisSize)), axisZ);
                    }

                    // Some debug decoration
                    {
                        Renderer::Immediate::TextParams textParams;
                        textParams.scale = 2;
                        textParams.pos = Vec3(game.renderMgr.orthoProjection.config.left + 10.f, game.renderMgr.orthoProjection.config.top - 10.f, -50);
                        textParams.color = Col(1.0f, 1.0f, 0.0f, 1.0f);
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "API tests\n=========\n%s", Platform::name);

                        Renderer::Immediate::sphere(game.renderMgr.immediateBuffer, Vec3(-170.f, 40.f, 0.f), 100.f, Col(1.0f, 1.0f, 1.0f, 0.4f));
                    }
                }
            } // perspective
            
            // Batched debug
            {
                Immediate::present3d(mgr.immediateBuffer, mgr.perspProjection.matrix, game.cameraMgr.activeCam->viewMatrix, game.renderScene.cbuffers);
                Immediate::present2d(mgr.immediateBuffer, mgr.orthoProjection.matrix, game.renderScene.cbuffers);
                Immediate::clear(mgr.immediateBuffer);
            }
        }
    }
}

#endif // __WASTELADNS_GAME_H__
