#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

namespace Game
{
    const char* inputMeshPath = "assets/meshes/mesh_dense.obj";
    const char* inputPoints = "assets/meshes/pts_1k.txt";
    const char* outputFilteredPoints = "assets/meshes/pts_1k_inside.txt";
    const char* outputFilteredProjectedPoints = "assets/meshes/pts_1k_projected.txt";

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
        Renderer::Immediate::Buffer immediateBuffer;
        Renderer::OrthoProjection orthoProjection;
        Renderer::PerspProjection perspProjection;
        Mat4 viewMatrix;
    };
    
    struct MeshQuery {
        BVH::Tree bvh;
        std::vector<Vec3> input;
        std::vector<Vec3> inputProjected;
        std::vector<Vec3> outputInside;
        std::vector<Vec3> outputInsideProjected;
    };

    struct DebugVis {
        struct PointMode { enum Enum { AllInput, AllProjected, Inside, InsideProjected, Count }; };

        f64 frameHistory[60];
        u32 frameHistoryIdx = 0;
        s32 visBVHlod = -1;
        PointMode::Enum vismode = PointMode::Enum::InsideProjected;
        bool help = true;
    };


    struct Instance {
        Time time;
        CameraManager cameraMgr;
        RenderManager renderMgr;
        Scene scene;
        MeshQuery meshQuery;
        DebugVis debugVis;
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
            MeshQuery& query = game.meshQuery;
            
            // input mesh
            {
                FILE* f;
                f = fopen(inputMeshPath, "r");

                char c;
                do {
                    c = fgetc(f);
                    if (c == 'v')
                    {
                        f32 x, y, z;
                        s32 r = fscanf(f, "%f%f%f", &x, &y, &z);
                        if (r == 3) {
                            vertices.push_back(x);
                            vertices.push_back(y);
                            vertices.push_back(z);
                        }
                    }
                    else if (c == 'f')
                    {
                        int a, b, c;
                        s32 r = fscanf(f, "%d%d%d", &a, &b, &c);
                        if (r == 3) {
                            indices.push_back(a - 1);
                            indices.push_back(b - 1);
                            indices.push_back(c - 1);
                        }
                    }
                } while (c > 0);
                fclose(f);
            }

            // query points
            {
                FILE* f;
                f = fopen(inputPoints, "r");

                int r = 0;
                do {
                    Vec3 v;
                    r = fscanf(f, "%f%f%f", &v.x, &v.y, &v.z);
                    query.input.push_back(v);
                } while (r > 0);
                fclose(f);
            }

            // perform query
            if (vertices.size() > 0 && indices.size() > 0)
            {
                BVH::buildTree(query.bvh, &vertices[0], &indices[0], indices.size());
                meshCentroid = {};
                if (query.bvh.nodes.size() > 0) {
                    for (u32 i = 0; i < query.input.size(); i++) {
                        {
                            Vec3 closest;
                            bool filterOutsidePoints = true;
                            if (BVH::findClosestPoint(closest, query.bvh, query.input[i], filterOutsidePoints, &vertices[0], &indices[0], indices.size())) {
                                query.outputInsideProjected.push_back(closest);
                                query.outputInside.push_back(query.input[i]);
                            }
                        }
                        {
                            Vec3 closest;
                            bool filterOutsidePoints = false;
                            if (BVH::findClosestPoint(closest, query.bvh, query.input[i], filterOutsidePoints, &vertices[0], &indices[0], indices.size())) {
                                query.inputProjected.push_back(closest);
                            }
                        }
                    }
                }
                // get mesh centroid so we can center the camera
                meshCentroid = Math::scale(Math::add(query.bvh.nodes[0].max, query.bvh.nodes[0].min), 0.5f);
            }

            // optional: read results instead
            //{
            //    query.outputInsideProjected.clear();

            //    FILE* f;
            //    f = fopen(outputFilteredProjectedPoints, "r");

            //    int r = 0;
            //    do {
            //        Vec3 v;;
            //        r = fscanf(f, "%f%f%f", &v.x, &v.y, &v.z);
            //        query.outputInsideProjected.push_back(v);
            //    } while (r > 0);
            //    fclose(f);
            //}

            // output results
            {
                FILE* f;
                f = fopen(outputFilteredPoints, "w");
                for (unsigned i = 0; i < query.outputInside.size(); i++) {
                    fprintf(f, "%f %f %f\n", query.outputInside[i].x, query.outputInside[i].y, query.outputInside[i].z);
                }
                fclose(f);
            }
            {
                FILE* f;
                f = fopen(outputFilteredProjectedPoints, "w");
                for (unsigned i = 0; i < query.outputInsideProjected.size(); i++) {
                    fprintf(f, "%f %f %f\n", query.outputInsideProjected[i].x, query.outputInsideProjected[i].y, query.outputInsideProjected[i].z);
                }
                fclose(f);
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
                    Platform::printf("link: %s", pixelResult.error);
                }
				vertexResult = Renderer::Driver::create(rscVS, { defaultVertexShaderStr, (u32)strlen(defaultVertexShaderStr) });
                if (!vertexResult.compiled) {
                    Platform::printf("PS: %s", vertexResult.error);
                }
                Renderer::Driver::RscShaderSet<Renderer::Layout_Vec3, Renderer::Layout_CBuffer_3DScene::Buffers> shaderSet;
                if (pixelResult.compiled && vertexResult.compiled) {
                    Renderer::Driver::ShaderResult result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, rscene.cbuffers });
					if (result.compiled) {
						rscene.sceneShader = shaderSet;
					}
					else {
						Platform::printf("link: %s", result.error);
					}
				}

                // Todo: RscShaderSet deletes pixel shader after creating, need to recompile it for reuse, not great
                pixelResult = Renderer::Driver::create(rscPS, { defaultPixelShaderStr, (u32)strlen(defaultPixelShaderStr) });
                if (!pixelResult.compiled) {
                    Platform::printf("link: %s", pixelResult.error);
                }
                vertexResult = Renderer::Driver::create(rscVS, { defaultInstancedVertexShaderStr, (u32)strlen(defaultInstancedVertexShaderStr) });
                if (!vertexResult.compiled) {
                    Platform::printf("PS: %s", vertexResult.error);
                }
                if (pixelResult.compiled && vertexResult.compiled) {
                    Renderer::Driver::ShaderResult result = Renderer::Driver::create(shaderSet, { rscVS, rscPS, rscene.cbuffers });
                    if (result.compiled) {
                        rscene.sceneInstancedShader = shaderSet;
                    }
                    else {
                        Platform::printf("link: %s", result.error);
                    }
                }
			}

            // input mesh vertex buffer
            if (vertices.size() > 0 && indices.size() > 0)
			{
				Renderer::Driver::RscIndexedBuffer<Renderer::Layout_Vec3> rscBuffer;
				Renderer::Driver::IndexedBufferParams bufferParams;
				bufferParams.vertexData = &vertices[0];
				bufferParams.vertexSize = vertices.size() * sizeof(vertices[0]);
				bufferParams.indexData = &indices[0];
				bufferParams.indexSize = indices.size() * sizeof(indices[0]);
				bufferParams.indexCount = indices.size();
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

			Renderer::Immediate::load(game.renderMgr.immediateBuffer);
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
        
        f64 raw_dt = platform.time.now - game.time.lastFrame;
        game.time.lastFrameDelta = Math::min(raw_dt, game.time.config.maxFrameLength);
        game.time.lastFrame = platform.time.now;
        config.nextFrame = platform.time.now + game.time.config.targetFramerate;

        u32 frameHistoryCount = (sizeof(game.debugVis.frameHistory) / sizeof(game.debugVis.frameHistory[0]));
        game.debugVis.frameHistory[game.debugVis.frameHistoryIdx] = raw_dt;
        game.debugVis.frameHistoryIdx = (game.debugVis.frameHistoryIdx + 1) % frameHistoryCount;
        f64 frameAvg = 0.;
        for (f64 dt : game.debugVis.frameHistory) {
            frameAvg += dt;
        }
        // miscalculated until the queue is full, but that's just the first second
        frameAvg /= frameHistoryCount;
        
        const ::Input::Gamepad::State& pad = platform.input.pads[0];
        const ::Input::Keyboard::State& keyboard = platform.input.keyboard;

        bool step = true;
        {
            if (keyboard.released(Input::EXIT)) {
                config.quit = true;
            }
            if (keyboard.pressed(Input::INCREASE_LOD)) {
                game.debugVis.visBVHlod++;
            }
            if (keyboard.pressed(Input::DECREASE_LOD) && game.debugVis.visBVHlod > -1) {
                game.debugVis.visBVHlod--;
            }
            if (keyboard.pressed(Input::TOGGLE_VIS)) {
                game.debugVis.vismode = (DebugVis::PointMode::Enum)((game.debugVis.vismode + 1) % DebugVis::PointMode::Count);
            }
            if (keyboard.pressed(Input::TOGGLE_HELP)) {
                game.debugVis.help = !game.debugVis.help;
            }
            step = !game.time.paused;
        }

        if (step)
        {
            f32 dt = (f32) (game.time.lastFrameDelta * (1.0 + game.time.updateOverspeed));
            
            using namespace Game;
            
            Camera* activeCam = game.cameraMgr.activeCam;

            // camera update
			{
				if (platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_LEFT))
				{
					f32 rotationSpeed = 0.01;
                    f32 speedx = platform.input.mouse.dx * rotationSpeed;
                    f32 speedy = platform.input.mouse.dy * rotationSpeed;
                    if (Math::abs(speedx) > 0.01 || Math::abs(speedy) > 0.01)
                    {
                        game.scene.rotationEulers.x = Math::wrap(game.scene.rotationEulers.x + speedx);
                        game.scene.rotationEulers.z = Math::wrap(game.scene.rotationEulers.z - speedy);
                    }
				}
                if (platform.input.mouse.down(::Input::Mouse::Keys::BUTTON_RIGHT))
                {
                    f32 rotationSpeed = 0.1;
                    f32 speedx = platform.input.mouse.dx * rotationSpeed;
                    f32 speedy = platform.input.mouse.dy * rotationSpeed;
                    if (Math::abs(speedx) > 0.01 || Math::abs(speedy) > 0.01)
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

                // input and output points (instanced unit cubes scaled down and translated to the points' positions)
                {
                    auto pointpass = [&rscene](Vec3* p, u32 count, Col c) {

                        RenderDescription& di = rscene.instancedCubeGroupBuffer.desc;

                        Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                        buffer.worldMatrix = rscene.instancedCubeGroupBuffer.instanceBuffer.transform.matrix;
                        Vec4 materialColor = di.material.groupColor.RGBAv4();
                        buffer.groupColor = Math::scale(materialColor, c.RGBAv4());
                        Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);

                        Renderer::Driver::bind(rscene.sceneInstancedShader);
                        Renderer::Driver::bind(di.rasterizerState);
                        Renderer::Driver::bind(di.buffer);
                        Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count, { true, false });

                        // round up, the last block will have less than or equal max elems
                        u32 blockCount = (count + Layout_CBuffer_3DScene::max_instances - 1) / Layout_CBuffer_3DScene::max_instances;
                        for (u32 blockId = 0; blockId < blockCount; blockId++) {
                            u32 begin = blockId * Layout_CBuffer_3DScene::max_instances;
                            u32 end = Math::min(begin + Layout_CBuffer_3DScene::max_instances, count);

                            // offset every cube to desired point location and scale it down
                            Renderer::Layout_CBuffer_3DScene::InstanceData instancedBuffer;
                            u32 bufferId = 0;
                            for (u32 i = begin; i < end; i++) {
                                Transform t;
                                Math::identity4x4(t);
                                t.pos = p[i];
                                t.matrix.col0.x = 0.2f;
                                t.matrix.col1.y = 0.2f;
                                t.matrix.col2.z = 0.2f;
                                instancedBuffer.instanceMatrices[bufferId++] = t.matrix;
                            }
                            Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::InstanceData], instancedBuffer);
                            drawInstances(di.buffer, end - begin);
                        }
                    };

                    Col inputCol(1.0f, 1.0f, 0.4f, 1.f);
                    Col projectedCol(0.4f, 1.f, 0.4f, 1.f);
                    if (game.meshQuery.input.size() > 0 && (game.debugVis.vismode == DebugVis::PointMode::AllInput || game.debugVis.vismode == DebugVis::PointMode::AllProjected)) {
                        pointpass(&game.meshQuery.input[0], game.meshQuery.input.size(), inputCol);
                    }
                    if (game.meshQuery.inputProjected.size() > 0 && game.debugVis.vismode == DebugVis::PointMode::AllProjected) {
                        pointpass(&game.meshQuery.inputProjected[0], game.meshQuery.inputProjected.size(), projectedCol);
                    }
                    if (game.meshQuery.outputInside.size() > 0 && (game.debugVis.vismode == DebugVis::PointMode::Inside || game.debugVis.vismode == DebugVis::PointMode::InsideProjected)) {
                        pointpass(&game.meshQuery.outputInside[0], game.meshQuery.outputInside.size(), inputCol);
                    }
                    if (game.meshQuery.outputInsideProjected.size() > 0 && game.debugVis.vismode == DebugVis::PointMode::InsideProjected) {
                        pointpass(&game.meshQuery.outputInsideProjected[0], game.meshQuery.outputInsideProjected.size(), projectedCol);
                    }
                }

                // render bounding boxes (instanced unit cubes scaled and translated by the bbs extents)
                {
                    if (game.meshQuery.bvh.nodes.size() > 0 && game.debugVis.visBVHlod >= 0) {

                        std::vector<Vec3> centers;
                        std::vector<Vec3> scales;
                        const Col boxColor(0.1f, 0.8f, 0.8f, 0.7f);
                        struct DrawNode {
                            u32 nodeid;
                            u32 depth;
                        };
                        std::queue<DrawNode> drawNodes;
                        drawNodes.push(DrawNode{ 0, 0 });
                        while (drawNodes.size() > 0) {
                            DrawNode n = drawNodes.front();
                            if (!game.meshQuery.bvh.nodes[n.nodeid].isLeaf) {
                                if (n.depth >= game.debugVis.visBVHlod) {
                                    Vec3 center = Math::scale(Math::add(game.meshQuery.bvh.nodes[n.nodeid].min, game.meshQuery.bvh.nodes[n.nodeid].max), 0.5f);
                                    Vec3 scale = Math::scale(Math::subtract(game.meshQuery.bvh.nodes[n.nodeid].max, game.meshQuery.bvh.nodes[n.nodeid].min), 0.5f);
                                    centers.push_back(center);
                                    scales.push_back(scale);
                                }
                                else {
                                    drawNodes.push(DrawNode{ game.meshQuery.bvh.nodes[n.nodeid].lchildId, n.depth + 1 });
                                    drawNodes.push(DrawNode{ game.meshQuery.bvh.nodes[n.nodeid].lchildId + 1, n.depth + 1 });
                                }
                            }
                            drawNodes.pop();
                        }

                        RenderDescription& di = rscene.instancedCubeGroupBuffer.desc;

                        Renderer::Layout_CBuffer_3DScene::GroupData buffer;
                        buffer.worldMatrix = rscene.instancedCubeGroupBuffer.instanceBuffer.transform.matrix;
                        buffer.groupColor = boxColor.RGBAv4();
                        Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], buffer);

                        Renderer::Driver::bind(rscene.sceneInstancedShader);
                        Renderer::Driver::bind(di.rasterizerState);
                        Renderer::Driver::bind(di.buffer);
                        Renderer::Driver::bind(rscene.cbuffers, Renderer::Layout_CBuffer_3DScene::Buffers::Count, { true, false });

                        // round up, the last block will have less than or equal max elems
                        u32 blockCount = (centers.size() + Layout_CBuffer_3DScene::max_instances - 1) / Layout_CBuffer_3DScene::max_instances;
                        for (u32 blockId = 0; blockId < blockCount; blockId++) {
                            u32 begin = blockId * Layout_CBuffer_3DScene::max_instances;
                            u32 end = Math::min(begin + Layout_CBuffer_3DScene::max_instances, (u32)centers.size());

                            Renderer::Layout_CBuffer_3DScene::InstanceData instancedBuffer;
                            u32 bufferId = 0;
                            for (u32 i = begin; i < end; i++) {
                                Transform t;
                                Math::identity4x4(t);
                                t.pos = centers[i];
                                t.matrix.col0.x = scales[i].x;
                                t.matrix.col1.y = scales[i].y;
                                t.matrix.col2.z = scales[i].z;
                                instancedBuffer.instanceMatrices[bufferId++] = t.matrix;
                            }
                            Renderer::Driver::update(rscene.cbuffers[Renderer::Layout_CBuffer_3DScene::Buffers::InstanceData], instancedBuffer);
                            drawInstances(di.buffer, end - begin);
                        }
                    }
                }
            }

            // Immediate-mode debug. Can be moved out of the render update, it only pushes data to cpu buffers
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
                    const char* pointvisnames[] = {"All", "All + Projected", "Inside Points", "Inside Points + Projected"};
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
                    textParams.color = keyboard.pressed(Input::TOGGLE_VIS) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "V to toggle through point visualization: %s", pointvisnames[game.debugVis.vismode]);
                    textParams.pos.y -= 15.f;
                    textParams.color = (keyboard.pressed(Input::INCREASE_LOD) || keyboard.pressed(Input::DECREASE_LOD)) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "UP and DOWN arrows to visualize bounding boxes. Depth: %d", game.debugVis.visBVHlod);
                    textParams.pos.y -= 15.f;
                    textParams.color = keyboard.pressed(Input::TOGGLE_HELP) ? activeCol : defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "H to hide");
                    textParams.pos.y -= 15.f;

                    textParams.pos = Vec3(game.renderMgr.orthoProjection.config.right -60.f, game.renderMgr.orthoProjection.config.top - 15.f, -50);
                    textParams.color = defaultCol;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "%s", Platform::name);
                    textParams.pos.y -= 15.f;
                    textParams.pos.x -= 30.f;
                    Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams, "%.3lf fps", 1. / frameAvg);
                }
            }

            // Batched debug (clear cpu buffers onto the screen)
            {
                Immediate::present3d(mgr.immediateBuffer, mgr.perspProjection.matrix, game.cameraMgr.activeCam->viewMatrix);
                Immediate::present2d(mgr.immediateBuffer, mgr.orthoProjection.matrix);
                Immediate::clear(mgr.immediateBuffer);
            }
        }
    }
}

#endif // __WASTELADNS_GAME_H__
