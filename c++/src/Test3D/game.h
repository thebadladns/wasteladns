#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

namespace Game
{
    //const char* inputMeshPath = "assets/meshes/PolySphere1.fbx";
    const char* inputMeshPath = "assets/meshes/mesh_ona_quads.obj";

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
            , INCREASE_LOD = ::Input::Keyboard::Keys::UP
            , DECREASE_LOD = ::Input::Keyboard::Keys::DOWN
            , TOGGLE_VIS = ::Input::Keyboard::Keys::V
            , TOGGLE_HELP = ::Input::Keyboard::Keys::H
            ;
    };

    struct RenderInstance {
        Transform transform;
    };
    struct Material {
        Col groupColor;
    };
    struct RenderDescription {
        Renderer::Driver::RscIndexedBuffer<Renderer::Layout_Vec3> buffer;
        Renderer::Driver::RscRasterizerState rasterizerState;
        Material material;
        bool fill;
    };
    struct RenderGroup {
        RenderInstance instanceBuffer;
        RenderDescription desc;
    };
    struct RenderScene {
        RenderGroup inputMeshGroupBuffer;
        RenderGroup instancedCubeGroupBuffer;
        Renderer::Driver::RscCBuffer cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::Count];
        Renderer::Driver::RscRasterizerState rasterizerStateFill, rasterizerStateLine;
        Renderer::Driver::RscBlendState blendStateOn;
        Renderer::Driver::RscBlendState blendStateOff;
        Renderer::Driver::RscMainRenderTarget mainRenderTarget;
        Renderer::Driver::RscShaderSet<Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> sceneShader;
        Renderer::Driver::RscShaderSet<Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> sceneInstancedShader;
    };
    struct RenderInstanceHandle {
        u32 desc;
        u32 inst;
    };

    struct Scene {
        Vec3 offset;
        Vec3 rotationEulers;
        Vec3 orbitPosition;
        f32 localScale;
    };

    struct CameraManager {
        enum { FlyCam, CamCount };
        Camera cameras[CamCount];
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
        Scene scene;
        __DEBUGDEF(DebugVis debugVis;)
    };
    void start(Instance& game, Platform::GameConfig& config, const Platform::State& platform) {

        game.time = {};
        game.time.config.maxFrameLength = 0.1;
        game.time.config.targetFramerate = 1.0 / 60.0;
        game.time.lastFrame = platform.time.now;

        config = {};
        config.nextFrame = platform.time.now;
        u32 requestFlags = Platform::RequestFlags::PollKeyboard | Platform::RequestFlags::PollMouse;
        config.requestFlags = (Platform::RequestFlags::Enum)requestFlags;

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

        // mesh analysis
        Vec3 meshCentroid = {};
        std::vector<f32> vertices;
        std::vector<u16> indices;
        {
             //input mesh
            ufbx_load_opts opts = { 0 };
            ufbx_error error;
            ufbx_scene* scene = ufbx_load_file(inputMeshPath, &opts, &error);
            if (scene) {
                for (size_t i = 0; i < scene->nodes.count; i++) {
                    ufbx_node* node = scene->nodes.data[i];
                    if (node->is_root) continue;

                    Platform::debuglog("Object: %s\n", node->name.data);
                    if (node->mesh) {
                        Platform::debuglog("-> mesh with %zu faces\n", node->mesh->faces.count);

                        //vertices.reserve(node->mesh->num_vertices*3);
                        //for (size_t i = 0; i < node->mesh->num_vertices; i++) {
                        //    auto v = node->mesh->vertices[i];
                        //    vertices.push_back(v.x*10.f);
                        //    vertices.push_back(v.y*10.f);
                        //    vertices.push_back(v.z*10.f);
                        //}
                        vertices.resize(node->mesh->num_vertices * 3);
                        memcpy(&vertices[0], &node->mesh->vertices[0], node->mesh->num_vertices * 3 * sizeof(f32));
                        for (size_t i = 0; i < node->mesh->num_faces; i++) {
                            if (node->mesh->faces[i].num_indices > 3) {
                                const u16 a = node->mesh->vertex_indices[node->mesh->faces[i].index_begin];
                                const u16 b = node->mesh->vertex_indices[node->mesh->faces[i].index_begin + 1];
                                const u16 c = node->mesh->vertex_indices[node->mesh->faces[i].index_begin + 2];
                                const u16 d = node->mesh->vertex_indices[node->mesh->faces[i].index_begin + 3];
                                indices.push_back(a);
                                indices.push_back(b);
                                indices.push_back(c);
                                indices.push_back(a);
                                indices.push_back(c);
                                indices.push_back(d);
                            }
                            else {
                                const u16 a = node->mesh->vertex_indices[node->mesh->faces[i].index_begin];
                                const u16 b = node->mesh->vertex_indices[node->mesh->faces[i].index_begin + 1];
                                const u16 c = node->mesh->vertex_indices[node->mesh->faces[i].index_begin + 2];
                                indices.push_back(a);
                                indices.push_back(b);
                                indices.push_back(c);
                            }
                        }
                    }
                }
            }
            else {
                Platform::debuglog("failed to load: %s\n", error.description.data);
                FILE* f;
                if (Platform::fopen(&f, inputMeshPath, "r") == 0) {
                    char c;
                    do {
                        c = Platform::fgetc(f);
                        if (c == 'v')
                        {
                            f32 x, y, z;
                            s32 r = Platform::fscanf(f, "%f%f%f", &x, &y, &z);
                            if (r == 3) {
                                vertices.push_back(x);
                                vertices.push_back(y);
                                vertices.push_back(z);
                            }
                        }
                        else if (c == 'f')
                        {
                            int a, b, c;
                            s32 r = Platform::fscanf(f, "%d%d%d", &a, &b, &c);
                            if (r == 3) {
                                int d;
                                r = Platform::fscanf(f, "%d", &d);
                                if (r <= 0) {
                                    // read three values: triangles
                                    indices.push_back(a - 1);
                                    indices.push_back(b - 1);
                                    indices.push_back(c - 1);
                                }
                                else {
                                    // read four values: divide quad in two triangles
                                    indices.push_back(a - 1);
                                    indices.push_back(b - 1);
                                    indices.push_back(c - 1);
                                    indices.push_back(a - 1);
                                    indices.push_back(c - 1);
                                    indices.push_back(d - 1);
                                }
                            }
                        }
                    } while (c > 0);
                    Platform::fclose(f);
                }
            }
        }

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
            Renderer::Driver::create(rscene.rasterizerStateFill, { Renderer::RasterizerFillMode::Fill, true });
            Renderer::Driver::create(rscene.rasterizerStateLine, { Renderer::RasterizerFillMode::Line, true });

            // cbuffers
            Renderer::Driver::create<Renderer::Layout_CBuffer_3DScene::SceneData>(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], {});
            Renderer::Driver::create<Renderer::Layout_CBuffer_3DScene::GroupData>(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], {});
            Renderer::Driver::create<Renderer::Layout_CBuffer_3DScene::InstanceData>(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::InstanceData], {});

            {
                Renderer::Driver::RscVertexShader<Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> rscVS;
                Renderer::Driver::RscPixelShader rscPS;
                Renderer::Driver::ShaderResult pixelResult;
                Renderer::Driver::ShaderResult vertexResult;
                pixelResult = Renderer::Driver::create(rscPS, { defaultPixelShaderStr, (u32)strlen(defaultPixelShaderStr) });
                if (!pixelResult.compiled) {
                    Platform::debuglog("link: %s", pixelResult.error);
                }
                vertexResult = Renderer::Driver::create(rscVS, { defaultVertexShaderStr, (u32)strlen(defaultVertexShaderStr) });
                if (!vertexResult.compiled) {
                    Platform::debuglog("PS: %s", vertexResult.error);
                }
                Renderer::Driver::RscShaderSet<Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> shaderSet;
                if (pixelResult.compiled && vertexResult.compiled) {
                    Renderer::Driver::ShaderResult result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, rscene.cbuffers });
                    if (result.compiled) {
                        rscene.sceneShader = shaderSet;
                    }
                    else {
                        Platform::debuglog("link: %s", result.error);
                    }
                }

                // Todo: RscShaderSet deletes pixel shader after creating, need to recompile it for reuse, not great
                pixelResult = Renderer::Driver::create(rscPS, { defaultPixelShaderStr, (u32)strlen(defaultPixelShaderStr) });
                if (!pixelResult.compiled) {
                    Platform::debuglog("link: %s", pixelResult.error);
                }
                vertexResult = Renderer::Driver::create(rscVS, { defaultInstancedVertexShaderStr, (u32)strlen(defaultInstancedVertexShaderStr) });
                if (!vertexResult.compiled) {
                    Platform::debuglog("PS: %s", vertexResult.error);
                }
                if (pixelResult.compiled && vertexResult.compiled) {
                    Renderer::Driver::ShaderResult result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, rscene.cbuffers });
                    if (result.compiled) {
                        rscene.sceneInstancedShader = shaderSet;
                    }
                    else {
                        Platform::debuglog("link: %s", result.error);
                    }
                }
            }

            // input mesh vertex buffer
            if (vertices.size() > 0 && indices.size() > 0)
            {
                Renderer::Driver::RscIndexedBuffer<Renderer::Layout_Vec3> rscBuffer;
                Renderer::Driver::IndexedBufferParams bufferParams;
                bufferParams.vertexData = &vertices[0];
                bufferParams.vertexSize = (u32)vertices.size() * sizeof(vertices[0]);
                bufferParams.indexData = &indices[0];
                bufferParams.indexSize = (u32)indices.size() * sizeof(indices[0]);
                bufferParams.indexCount = (u32)indices.size();
                bufferParams.memoryMode = Renderer::BufferMemoryMode::GPU;
                bufferParams.indexType = Renderer::BufferItemType::U16;
                bufferParams.type = Renderer::BufferTopologyType::Triangles;
                Renderer::Driver::create(rscBuffer, bufferParams);

                RenderGroup& r = rscene.inputMeshGroupBuffer;
                r.desc.buffer = rscBuffer;
                r.desc.rasterizerState = rscene.rasterizerStateLine;
                Material& material = r.desc.material;
                material.groupColor = Col(0.3f, 0.3f, 0.3f, 1.f);
                Math::identity4x4(r.instanceBuffer.transform);
            }

            // unit cube vertex buffer
            {
                Renderer::UntexturedCube cube;
                Renderer::create(cube, { 1.f, 1.f, 1.f });

                Renderer::Driver::RscIndexedBuffer<Renderer::Layout_Vec3> rscBuffer;
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

                RenderGroup& r = rscene.instancedCubeGroupBuffer;
                r.desc.buffer = rscBuffer;
                r.desc.rasterizerState = rscene.rasterizerStateLine;
                Material& material = r.desc.material;
                material.groupColor = Col(1.f, 1.f, 1.f, 1.f);
                Math::identity4x4(r.instanceBuffer.transform);
            }

            __DEBUGEXP(Renderer::Immediate::load(game.renderMgr.immediateBuffer));
        }

        game.scene = {};
        {
            game.scene.offset = Vec3(0.f, -100.f, 0.f);
            game.scene.rotationEulers = Vec3(0.f, 0.f, 0.f);
            game.scene.localScale = 1.f;
            game.scene.orbitPosition = Math::negate(meshCentroid);
        }

        game.cameraMgr = {};
        {
            CameraManager& mgr = game.cameraMgr;
            Camera& cam = mgr.cameras[CameraManager::FlyCam];
            mgr.activeCam = &cam;
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

            // camera update
            {
                if (platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_LEFT))
                {
                    constexpr f32 rotationSpeed = 0.01f;
                    constexpr f32 rotationEps = 0.01f;
                    f32 speedx = platform.input.mouse.dx * rotationSpeed;
                    f32 speedy = platform.input.mouse.dy * rotationSpeed;
                    if (Math::abs(speedx) > rotationEps || Math::abs(speedy) > rotationEps)
                    {
                        game.scene.rotationEulers.x = Math::wrap(game.scene.rotationEulers.x + speedx);
                        game.scene.rotationEulers.z = Math::wrap(game.scene.rotationEulers.z - speedy);
                    }
                }
                if (platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_RIGHT))
                {
                    constexpr f32 panSpeed = 0.1f;
                    constexpr f32 panEps = 0.01f;
                    f32 speedx = platform.input.mouse.dx * panSpeed;
                    f32 speedy = platform.input.mouse.dy * panSpeed;
                    if (Math::abs(speedx) > panEps || Math::abs(speedy) > panEps)
                    {
                        game.scene.offset.x -= speedx;
                        game.scene.offset.z += speedy;
                    }

                }
                if (platform.input.mouse.scrolldy != 0)
                {
                    const f32 scrollSpeed = 0.1f;
                    game.scene.localScale -= platform.input.mouse.scrolldy * scrollSpeed;
                }
            }

            // send camera data to render manager
            {
                // orbit around mesh
                Transform rotationAndScale = Math::fromPositionScaleAndRotationEulers(Math::negate(game.scene.orbitPosition), game.scene.localScale, game.scene.rotationEulers);

                // translate to centroid 
                Transform cameraTranslation;
                Math::identity4x4(cameraTranslation);
                cameraTranslation.pos = game.scene.offset;

                Transform result;
                result.matrix = Math::mult(rotationAndScale.matrix, cameraTranslation.matrix);
                activeCam->transform = result;

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
                Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], cbufferPerScene);
            }

            {
                Renderer::Driver::bind(rscene.blendStateOn);
                Renderer::Driver::bind(rscene.mainRenderTarget);
                Renderer::Driver::clear(rscene.mainRenderTarget, Col(0.f, 0.f, 0.f, 1.f));

                // mesh
                {
                    RenderDescription& d = rscene.inputMeshGroupBuffer.desc;

                    Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                    buffer.worldMatrix = rscene.inputMeshGroupBuffer.instanceBuffer.transform.matrix;
                    buffer.groupColor = Vec4(d.material.groupColor.getRf(), d.material.groupColor.getGf(), d.material.groupColor.getBf(), d.material.groupColor.getAf());
                    Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);

                    Renderer::Driver::bind(rscene.sceneShader);
                    Renderer::Driver::bind(d.rasterizerState);
                    Renderer::Driver::bind(d.buffer);
                    Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count, { true, false });
                    draw(d.buffer);
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
