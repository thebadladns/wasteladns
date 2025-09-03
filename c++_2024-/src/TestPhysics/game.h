#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

const size_t persistentArenaSize = 1 * 1024 * 1024;
const size_t sceneArenaSize = 256 * 1024 * 1024;
const size_t frameArenaSize = 4 * 1024 * 1024;
const size_t scratchArenaSize = 4 * 1024 * 1024;

#if __DEBUG
struct CameraNode;
namespace debug {
struct DebugMenus { enum Enum { Main, Arenas, Count }; };
struct VisualizationModes { enum Enum { Physics, Culling, BVH, Count }; };
bool visualizationModes[VisualizationModes::Count] = {};
bool debugMenus[DebugMenus::Count] = { true, false };
struct EventText { char text[256]; f64 time; };

f64 frameHistory[60];
f64 frameAvg = 0;
u32 frameHistoryIdx = 0;
u32 debugCameraStage = 0;
u32 bvhDepth = 0;
CameraNode* capturedCameras = nullptr;
EventText eventLabel = {};
bool capture_cameras_next_frame = false;
im::Pane debugPane;
im::Pane arenasPane;

}
#endif

#include "gameplay.h"
#include "shaders.h"
#include "renderer.h"
#include "animation.h"
#include "physics.h"
#include "scene.h"

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
    u64 frameCount;
    bool pausedRender;
};

namespace input
{
    constexpr ::input::keyboard::Keys::Enum
          UP = ::input::keyboard::Keys::W
        , DOWN = ::input::keyboard::Keys::S
        , LEFT = ::input::keyboard::Keys::A
        , RIGHT = ::input::keyboard::Keys::D
        , TOGGLE_PAUSE_SCENE_RENDER = ::input::keyboard::Keys::SPACE
        ;
    constexpr ::input::keyboard::Keys::Enum
          EXIT = ::input::keyboard::Keys::ESCAPE
        #if __DEBUG
        , TOGGLE_DEBUG = ::input::keyboard::Keys::H
        #endif
        ;
};

struct Memory {
    allocator::PagedArena persistentArena;
    allocator::PagedArena sceneArena;
    __DEBUGDEF(allocator::PagedArena debugArena;)
    allocator::PagedArena scratchArenaRoot; // to be passed by copy, so it works as a scoped stack allocator
    allocator::PagedArena frameArena;
    u8* frameArenaBuffer; // used to reset allocator::frameArena every frame
    u8* sceneArenaBuffer; // used to reset allocator::sceneArena upon scene switches
    // used for debugging visualization
    __DEBUGDEF(u8* persistentArenaBuffer;)
    // to track largest allocation
    __DEBUGDEF(uintptr_t scratchArenaHighmark;)
    __DEBUGDEF(uintptr_t frameArenaHighmark;)
};

struct Instance {
    Time time;
    Memory memory;
    Scene scene;
    u32 roomId;
    Resources resources;
};

void loadLaunchConfig(platform::LaunchConfig& config) {
    // hardcoded for now
    config.window_width = 320 * 3;
    config.window_height = 240 * 3;
    config.game_width = 320 * 1;
    config.game_height = 240 * 1;
    config.fullscreen = false;
    config.title = "Physics Test";
}
void start(Instance& game, platform::GameConfig& config) {

    game.time = {};
    game.time.config.maxFrameLength = 0.1;
    game.time.config.targetFramerate = 1.0 / 60.0;
    game.time.lastFrame = platform::state.time.now;

    config = {};
    config.nextFrame = platform::state.time.now;

    {
        allocator::init_arena(game.memory.persistentArena, persistentArenaSize);
        __DEBUGDEF(game.memory.persistentArenaBuffer = game.memory.persistentArena.curr;)
        allocator::init_arena(game.memory.sceneArena, sceneArenaSize);
        game.memory.sceneArenaBuffer = game.memory.sceneArena.curr;
        allocator::init_arena(game.memory.scratchArenaRoot, scratchArenaSize);
        __DEBUGDEF(
            game.memory.scratchArenaHighmark =
                (uintptr_t)game.memory.scratchArenaRoot.curr;
            game.memory.scratchArenaRoot.highmark = &game.memory.scratchArenaHighmark;)
        allocator::init_arena(
            game.memory.frameArena, frameArenaSize);
        game.memory.frameArenaBuffer = game.memory.frameArena.curr;
        __DEBUGDEF(game.memory.frameArenaHighmark =
                (uintptr_t)game.memory.frameArena.curr;
            game.memory.frameArena.highmark = &game.memory.frameArenaHighmark;)
        __DEBUGDEF(allocator::init_arena(game.memory.debugArena, im::arena_size);)
    }
    {
        game.scene = {};
        game.roomId = 0;
        SceneMemory arenas = {
              game.memory.persistentArena
            , game.memory.scratchArenaRoot
            __DEBUGDEF(, game.memory.debugArena)
        };
        load_coreResources(game.resources, arenas, platform::state.screen);
        spawn_scene_mirrorRoom(
            game.scene, game.memory.sceneArena, game.memory.scratchArenaRoot,
            game.resources, platform::state.screen,
            roomDefinitions[game.roomId]);

    }

#if __DEBUG
    im::ui.scale = (u8)platform::state.screen.window_scale;
    im::make_pane(
        debug::debugPane, "DEBUG MENU",
        float2(game.resources.renderCore.windowProjection.config.left + im::ui.scale * 20,
               game.resources.renderCore.windowProjection.config.top - im::ui.scale * 35));
    im::make_pane(
        debug::arenasPane, "ARENAS MENU",
        float2(0.f, game.resources.renderCore.windowProjection.config.top - im::ui.scale * 35));
#endif
}

void update(Instance& game, platform::GameConfig& config) {

    // frame arena reset
    game.memory.frameArena.curr = game.memory.frameArenaBuffer;

    // frame timing calculations
    {
        f64 raw_dt = platform::state.time.now - game.time.lastFrame;
        game.time.lastFrameDelta = math::min(raw_dt, game.time.config.maxFrameLength);
        game.time.lastFrame = platform::state.time.now;
        config.nextFrame = platform::state.time.now + game.time.config.targetFramerate;
        game.time.frameCount++;

        #if __DEBUG
        if (game.time.frameCount > 1) // ignore first frame, which has a frame time of 0
        {
            f64 frameAvg = 0.;
            constexpr u32 frameHistoryCount =
                u32(sizeof(debug::frameHistory) / sizeof(debug::frameHistory[0]));
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
    const ::input::keyboard::State& keyboard = platform::state.input.keyboard;
    const u32 prevRoomId = game.roomId;
    bool step = true;
    __DEBUGDEF(bool captureCameras = debug::capture_cameras_next_frame; debug::capture_cameras_next_frame = false;)
    {
        if (keyboard.released(input::EXIT)) {
            config.quit = true;
        }
        #if __DEBUG
        if (keyboard.pressed(input::TOGGLE_DEBUG)) {
            debug::debugMenus[debug::DebugMenus::Main] = !debug::debugMenus[debug::DebugMenus::Main];
        }
        #endif
    }

    if (prevRoomId != game.roomId) {
        game.memory.sceneArena.curr = game.memory.sceneArenaBuffer;
        game.scene = {};
        spawn_scene_mirrorRoom(
            game.scene, game.memory.sceneArena, game.memory.scratchArenaRoot,
            game.resources, platform::state.screen,
            roomDefinitions[game.roomId]);
    }

    if (step)
    {
        f32 dt = (f32)(game.time.lastFrameDelta * (1.0 + game.time.updateOverspeed));

        using namespace game;

        // player movement update
        {
            game::MovementInput controlpad = game.scene.player.control, controlkeyboard = game.scene.player.control;
            game::movementInputFromKeys(controlpad, keyboard, { input::UP, input::DOWN, input::LEFT, input::RIGHT });
            game::movementInputFromPad(controlkeyboard, platform::state.input.pads[0]);
            game.scene.player.control.localInput = math::add(controlpad.localInput, controlkeyboard.localInput);
            game.scene.player.control.mag = math::mag(game.scene.player.control.localInput);
            if (game.scene.player.control.mag > math::eps32) {
                game.scene.player.control.localInput = math::invScale(game.scene.player.control.localInput, game.scene.player.control.mag);
                game.scene.player.control.mag = math::min(game.scene.player.control.mag, 1.f);
            }
            const float2 effectiveLocalInput =  game::calculateMovement_cameraRelative(
                game.scene.player.transform, game.scene.player.state,
                game.scene.player.control, game.scene.camera.transform, dt);

            // enforce player movement
            {
                Transform33 movementTransform = 
                    math::fromUpTowardsFront(
                        game.scene.player.transform.up, game.scene.camera.transform.front);
                const float3& front = movementTransform.front;
                const float3& right = movementTransform.right;

                // Movement update
                float3 desiredMovement = {};
                if (game.scene.player.state.speed > math::eps32) {
                    float3 cameraRelativeMovementInput;
                    if (game.scene.player.control.mag > math::eps32) {
                        cameraRelativeMovementInput = float3(game.scene.player.control.localInput, 0.f);
                    } else {
                        const f32 camCurrentRad = math::orientation(game.scene.camera.transform.front.xy);
                        const f32 playerCurrentRad = math::orientation(game.scene.player.transform.front.xy);
                        const f32 camPlayerCurrentRad = math::subtractShort(playerCurrentRad, camCurrentRad);

                        cameraRelativeMovementInput = float3(math::direction(camPlayerCurrentRad), 0.f);
                    }
                    const f32 translation = game.scene.player.state.speed * dt;
                    const float3 worldMovementInput = math::add(math::scale(front, cameraRelativeMovementInput.y), math::scale(right, cameraRelativeMovementInput.x));
                    const float3 worldVelocity = math::scale(worldMovementInput, translation);
                    desiredMovement = worldVelocity;
                }

                float3 pos = math::add(game.scene.player.transform.pos, desiredMovement);
                u32 playerObstacleIndex = (game.scene.playerPhysicsNodeHandle) >> physics::ObjectType::Bits;
                const float radius = game.scene.physicsScene.obstacles[playerObstacleIndex].radius;

                // hack: update player movement in physics
                for (u32 i = 0; i < game.scene.physicsScene.obstacle_count; i++) {

                    if (i == playerObstacleIndex) continue;

                    physics::StaticObject_Sphere& o = game.scene.physicsScene.obstacles[i];
                    float3 collisionDir = math::subtract(pos, o.pos);
                    collisionDir.z = 0.f;
                    f32 dist = math::mag(collisionDir);
                    if (dist < math::eps32 || dist > radius + o.radius) continue;
                    collisionDir = math::invScale(collisionDir, dist);
                    f32 correction = radius + o.radius - dist;
                    pos = math::add(pos, math::scale(collisionDir, correction));
                }
                for (u32 i = 0; i < game.scene.physicsScene.wall_count; i++) {

                    physics::StaticObject_Line& w = game.scene.physicsScene.walls[i];

                    // distance to wall
                    float3 ab = math::subtract(w.end, w.start);
                    f32 t = math::max(0.f,
                        math::min(1.f,
                            (math::dot(math::subtract(pos, w.start), ab)) / math::dot(ab, ab)));
                    float3 col = math::add(w.start, math::scale(ab, t));
                    float3 d = math::subtract(pos, col);
                    d.z = 0.f;
                    f32 dist = math::mag(d);
                    float3 normal(-ab.y, ab.x, ab.z);

                    // push out
                    if (dist == 0.f) {
                        d = normal;
                        dist = math::mag(normal);
                    }
                    d = math::invScale(d, dist);
                    if (math::dot(d, normal) >= 0.f) { // outside the wall
                        if (dist > radius) continue;
                        pos = math::add(pos, math::scale(d, radius - dist));
                    }
                    else { // inside the wall
                        pos = math::add(pos, math::scale(d, -(radius + dist)));
                    }
                }

                if (game.scene.player.control.mag > math::eps32) {
                    // Facing update
                    const float3 cameraRelativeFacingInput(effectiveLocalInput, 0.f);
                    const float3 worldFacingInput = math::add(math::scale(front, cameraRelativeFacingInput.y), math::scale(right, cameraRelativeFacingInput.x));
                    Transform33 t = math::fromUpTowardsFront(game.scene.player.transform.up, worldFacingInput);
                    game.scene.player.transform.front = t.front;
                    game.scene.player.transform.right = t.right;
                    game.scene.player.transform.up = t.up;
                }
                // Positon update
                game.scene.player.transform.pos = pos;
            }


            // update player render
            renderer::NodeData& nodeData = renderer::node_from_handle(game.scene.renderScene, game.scene.playerDrawNodeHandle).nodeData;
            nodeData.worldMatrix = game.scene.player.transform.matrix;

            { // animation hack
                animation::Node& animatedData = animation::get_node(game.scene.animScene, game.scene.playerAnimatedNodeHandle);

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
                if (game.scene.player.state.speed > jerkyRunSpeedStart) {
                    const f32 run_t = (game.scene.player.state.speed - jerkyRunSpeedStart) / (maxSpeed - jerkyRunSpeedStart);
                    const f32 scale = 1.f + math::lerp(run_t, jerkyRunMinScale, jerkyRunMaxScale);
                    particle_scaley = math::lerp(run_t, jerkyRunParticleMinScaley, jerkyRunParticleMaxScaley);
                    particle_scalez = math::lerp(run_t, jerkyRunParticleMinScalez, jerkyRunParticleMaxScalez);
                    particle_offsety = 0.3f;
                    animatedData.state.scale = scale;
                    if (animatedData.state.animIndex != 1) {
                        animatedData.state.animIndex = 1;
                        animatedData.state.time = 0.f;
                    }
                } else if (game.scene.player.state.speed > 0.2f) {
                    const f32 run_t = (game.scene.player.state.speed - runSpeedStart) / (jerkyRunSpeedStart - runSpeedStart);
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
                renderer::instanced_node_from_handle(
                        instance_matrices, instance_count,
                        game.scene.renderScene,
                        game.scene.instancedNodesHandles[Scene::InstancedTypes::PlayerTrail]);
                for (u32 i = 0; i < *instance_count; i++) {

                    const f32 scaley = particle_scaley;
                    const f32 scalez = particle_scalez;
                    const f32 offsety = particle_offsety;
                    const f32 offsetz = particle_offsetz;
                    const f32 scale = particle_totalscale;

                    Transform t;
                    math::identity4x4(t);
                    t.pos = math::add(
                        game.scene.player.transform.pos,
                        math::scale(game.scene.player.transform.front, offsety + scaley * (i + 2)));
                    t.pos.z = t.pos.z
                            + offsetz
                            + scalez * (math::cos((f32)platform::state.time.now * 10.f + i * 2.f) - 1.5f);
                    t.matrix.col0.x = 0.2f;
                    t.matrix.col1.y = 0.2f;
                    t.matrix.col2.z = 0.2f;
                    t.matrix.col0 = math::scale(t.matrix.col0, scale);
                    t.matrix.col1 = math::scale(t.matrix.col1, scale);
                    t.matrix.col2 = math::scale(t.matrix.col2, scale);

                    instance_matrices->data[i] = t.matrix;
                }
            }
        }

        // physics update
        if (game.scene.instancedNodesHandles[Scene::InstancedTypes::PhysicsBalls])
        {
            physics::updatePositionFromHandle(
                game.scene.physicsScene,
                game.scene.playerPhysicsNodeHandle,
                game.scene.player.transform.pos, (f32)game.time.lastFrameDelta);

            physics::updatePhysics(game.scene.physicsScene, dt);

            // update draw positions
            renderer::Matrices64* instance_matrices; u32* instance_count;
            renderer::instanced_node_from_handle(instance_matrices, instance_count, game.scene.renderScene, game.scene.instancedNodesHandles[Scene::InstancedTypes::PhysicsBalls]);
            for (u32 i = 0; i < *instance_count; i++) {
                float4x4& m = instance_matrices->data[i];
                m.col3.xyz = game.scene.physicsScene.balls[i].pos;
                m.col0.x = game.scene.physicsScene.balls[i].radius;
                m.col1.y = game.scene.physicsScene.balls[i].radius;
                m.col2.z = game.scene.physicsScene.balls[i].radius;
            }
        }

        // anim update
        {
            animation::Scene& animScene = game.scene.animScene;
            animation::updateAnimation(animScene, dt);
        }

        // camera update
        {
            bool mousecontrols = true;
            __DEBUGDEF(mousecontrols = !im::is_mouse_captured();)
            if (mousecontrols) {
                game::orbitInputFromPad(game.scene.orbitCamera, platform::state.input.pads[0]);
                game::orbitInputFromMouse(game.scene.orbitCamera, platform::state.input.mouse);
            }
            game::updateOrbit(game.scene.camera.transform, game.scene.orbitCamera);

            // compute the camera render matrix
            camera::generate_matrix_view(game.scene.camera.viewMatrix, game.scene.camera.transform);
        }
    }

    // Render update
    CameraNode* cameraTree = nullptr;
    Camera mainCamera = {};
    if (!game.time.pausedRender)
    {
        renderer::Scene& scene = game.scene.renderScene;
        renderer::CoreResources& renderCore = game.resources.renderCore;
        using namespace renderer;

        mainCamera.viewMatrix = game.scene.camera.viewMatrix;
        mainCamera.projectionMatrix = renderCore.perspProjection.matrix;
        mainCamera.vpMatrix =
            math::mult(renderCore.perspProjection.matrix, game.scene.camera.viewMatrix);
        mainCamera.pos = game.scene.camera.transform.pos;

        {
            {
                gfx::rhi::ViewportParams vpParams;
                vpParams.topLeftX = 0;
                vpParams.topLeftY = 0;
                vpParams.width = (f32)platform::state.screen.width;
                vpParams.height = (f32)platform::state.screen.height;
                vpParams.minDepth = 0.f;
                vpParams.maxDepth = 1.f;
                gfx::rhi::set_VP(vpParams);
            }
        
            renderer::VisibleNodes* visibleNodesTree = nullptr;
            {
                allocator::PagedArena scratchArena = game.memory.scratchArenaRoot;

                // gather mirrors
                u32 numCameras = 0;
                {
                    allocator::Buffer<CameraNode> cameraTreeBuffer;
                    CameraNode mainCameraRoot = {};
                    // Initialize main camera in our camera tree format
                    mainCameraRoot.viewMatrix = mainCamera.viewMatrix;
                    mainCameraRoot.projectionMatrix = mainCamera.projectionMatrix;
                    mainCameraRoot.vpMatrix = mainCamera.vpMatrix;
                    mainCameraRoot.pos = mainCamera.pos;
                    mainCameraRoot.sourceId = mainCameraRoot.parentIndex = 0xffffffff;
                    mainCameraRoot.depth = 0;
                    __PROFILEONLY(io::format(mainCameraRoot.str, sizeof(mainCameraRoot.str), "_");)
                    gfx::extract_frustum_planes_from_vp(
                        mainCameraRoot.frustum.planes, mainCamera.vpMatrix);
                    mainCameraRoot.frustum.numPlanes = 6;
                    // the buffer will copy this data into it's allocated region as
                    // it starts adding more cameras
                    cameraTreeBuffer.data = &mainCameraRoot;
                    cameraTreeBuffer.cap = cameraTreeBuffer.len = 1;
                    GatherMirrorTreeContext gatherTreeContext =
                    { game.memory.frameArena, game.memory.scratchArenaRoot,
                      cameraTreeBuffer, game.scene.mirrors, game.scene.maxMirrorBounces };
                    numCameras = gatherMirrorTreeRecursive(gatherTreeContext, 1, mainCameraRoot);
                    cameraTree = cameraTreeBuffer.data;
                    cameraTree[0].siblingIndex = numCameras;
                }

                #if __DEBUG
                if (captureCameras) {
                    if (debug::capturedCameras) { free(debug::capturedCameras); }
                    debug::debugCameraStage = math::min(debug::debugCameraStage, (u32)numCameras - 1);
                    debug::capturedCameras = (CameraNode*)malloc(sizeof(CameraNode) * numCameras);
                    memcpy(debug::capturedCameras, cameraTree, sizeof(CameraNode) * numCameras);
                }
                #endif

                // figure out which nodes are visible among all of the visibility lists
                u32* isEachNodeVisible =
                    ALLOC_ARRAY(game.memory.frameArena, u32, scene.drawNodes.count);
                memset(isEachNodeVisible, 0, scene.drawNodes.count * sizeof(bool));
                visibleNodesTree = // todo: in function??
                    ALLOC_ARRAY(game.memory.frameArena, VisibleNodes, numCameras);
                visibleNodesTree[0].visible_nodes =
                    ALLOC_ARRAY(game.memory.frameArena, u32, scene.drawNodes.count);
                visibleNodesTree[0].visible_nodes_count = 0;
                renderer::CullEntries cullEntries = {};
                allocCullEntries(scratchArena, cullEntries, scene);
                renderer::computeVisibilityCS(
                    visibleNodesTree[0], isEachNodeVisible, mainCamera.vpMatrix, scene);
                for (u32 i = 1; i < numCameras; i++) {
                    renderer::computeVisibilityWS(
                        game.memory.frameArena, visibleNodesTree[i], isEachNodeVisible,
                        cameraTree[i].frustum, cullEntries);
                }
                
                // update cbuffers of all visible nodes
                for (u32 n = 0, count = 0; n < scene.drawNodes.cap && count < scene.drawNodes.count; n++) {
                    if (scene.drawNodes.data[n].alive == 0) { continue; }
                    count++;
                    if (!isEachNodeVisible[n]) { continue; }
                    const DrawNode& node = scene.drawNodes.data[n].state.live;
                    gfx::rhi::update_cbuffer(
                            cbuffer_from_handle(scene, node.cbuffer_node),
                            &node.nodeData);
                    if (node.cbuffer_ext) {
                        gfx::rhi::update_cbuffer(
                                cbuffer_from_handle(scene, node.cbuffer_ext),
                                node.ext_data);
                    }
                }
                for (u32 n = 0, count = 0; n < scene.instancedDrawNodes.cap && count < scene.instancedDrawNodes.count; n++) {
                    if (scene.instancedDrawNodes.data[n].alive == 0) { continue; }
                    count++;
                    const DrawNodeInstanced& node = scene.instancedDrawNodes.data[n].state.live;
                    gfx::rhi::update_cbuffer(
                            cbuffer_from_handle(scene, node.cbuffer_node),
                            &node.nodeData);
                    gfx::rhi::update_cbuffer(
                            cbuffer_from_handle(scene, node.cbuffer_instances),
                            &node.instanceMatrices);
                }
            }

            // render main camera
            gfx::rhi::bind_RT(renderCore.gameRT);
            // TODO: figure out whether this is needed
            gfx::rhi::clear_RT(renderCore.gameRT,
                gfx::rhi::RenderTargetClearFlags::Stencil);

            {
                gfx::rhi::start_event("BASE SCENE");

                RenderSceneContext renderSceneContext = {
                    cameraTree[0], visibleNodesTree[0], game.scene, renderCore,
                    renderCore.depthStateAlways,
                    renderCore.depthStateOn,
                    renderCore.depthStateReadOnly,
                    renderCore.rasterizerStateFillFrontfaces,
                    game.memory.scratchArenaRoot };
                renderBaseScene(renderSceneContext);

                gfx::rhi::end_event();
            }

            // render camera tree
            if (cameraTree[0].siblingIndex > 1) {
                renderMirrorTree(
                        cameraTree, visibleNodesTree, game.scene, renderCore,
                        game.memory.scratchArenaRoot);
            }
        }
    }
    {
        // render update wrap up
        renderer::Scene& scene = game.scene.renderScene;

        #if __DEBUG
        // Immediate mode debug, to be rendered along with the 3D scene on the next frame
        {
            if (debug::visualizationModes[debug::VisualizationModes::BVH]) {

                allocator::PagedArena scratchArena = game.memory.scratchArenaRoot; // explicit copy
                renderer::CoreResources& renderCore = game.resources.renderCore;
                bvh::Tree& bvh = game.scene.mirrors.bvh;

                gfx::rhi::start_event("BVH");
                if (bvh.nodeCount > 0) {

                    struct DrawNode {
                        u16 nodeid;
                        u16 depth;
                    };
                    DrawNode* nodeStack = ALLOC_ARRAY(scratchArena, DrawNode, bvh.nodeCount);
                    u32 stackCount = 0;
                    nodeStack[stackCount++] = { 0, 0 };
                    struct AABB {
                        float3 center;
                        float3 scale;
                    };
                    allocator::Buffer<AABB> aabbs = {};
                    const Color32 boxColor(0.1f, 0.8f, 0.8f, 0.7f);
                    while (stackCount > 0) {
                        const DrawNode n = nodeStack[--stackCount];
                        const bvh::Node& node = bvh.nodes[n.nodeid];
                        if (n.depth == debug::bvhDepth || node.isLeaf) {
                            float3 min, max;
                            bvh::ymm_to_minmax(
                                min, max, node.xcoords_256, node.ycoords_256, node.zcoords_256);
                            float3 center = math::scale(math::add(min, max), 0.5f);
                            float3 scale = math::scale(math::subtract(max, min), 0.5f);
                            allocator::push(aabbs, scratchArena) = { center, scale };
                        } else if (!node.isLeaf) {
                            nodeStack[stackCount++] =
                                DrawNode{ node.lchildId, u16(n.depth + 1u) };
                            nodeStack[stackCount++] =
                                DrawNode{ u16(node.lchildId + 1u), u16(n.depth + 1u) };
                        }
                    }

                    gfx::rhi::RscCBuffer& scene_cbuffer =
                        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
                    gfx::rhi::RscCBuffer& node_cbuffer =
                        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::NodeIdentity];
                    gfx::rhi::RscCBuffer& instanced_cbuffer =
                        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::Instances64];

                    renderer::NodeData nodeColor = {};
                    math::identity4x4(*(Transform*)&nodeColor.worldMatrix);
                    nodeColor.groupColor = Color32(0.4f, 0.54f, 1.f, 0.3f).RGBAv4();
                    gfx::rhi::update_cbuffer(node_cbuffer, &nodeColor);

                    gfx::rhi::RscCBuffer cbuffers[] =
                    { scene_cbuffer, node_cbuffer, instanced_cbuffer };
                    renderer::DrawMesh& drawMesh =
                        renderer::drawMesh_from_handle(
                            renderCore, game.resources.instancedUnitCubeMesh);

                    gfx::rhi::bind_DS(renderCore.depthStateOn);
                    gfx::rhi::bind_shader(renderCore.shaders[drawMesh.shaderTechnique]);
                    gfx::rhi::bind_RS(renderCore.rasterizerStateLine);
                    gfx::rhi::bind_indexed_vertex_buffer(drawMesh.vertexBuffer);
                    gfx::rhi::bind_cbuffers(
                        renderCore.shaders[drawMesh.shaderTechnique], cbuffers, countof(cbuffers));

                    // round up, the last block will have less than or equal max elems
                    u32 blockCount = ((u32)aabbs.len + 64 - 1) / 64;
                    for (u32 blockId = 0; blockId < blockCount; blockId++) {
                        u32 begin = blockId * 64;
                        u32 end = math::min(begin + 64, (u32)aabbs.len);

                        u32 bufferId = 0;
                        renderer::Matrices64 matrices = {};
                        for (u32 i = begin; i < end; i++) {
                            Transform t;
                            math::identity4x4(t);
                            t.pos = aabbs.data[i].center;
                            t.matrix.col0.x = aabbs.data[i].scale.x;
                            t.matrix.col1.y = aabbs.data[i].scale.y;
                            t.matrix.col2.z = aabbs.data[i].scale.z;
                            matrices.data[bufferId++] = t.matrix;
                        }
                        gfx::rhi::update_cbuffer(instanced_cbuffer, &matrices);
                        gfx::rhi::draw_instances_indexed_vertex_buffer(
                            drawMesh.vertexBuffer, end - begin);
                    }

                    // clean up identity node
                    nodeColor.groupColor = Color32(1.f, 1.f, 1.f, 1.f).RGBAv4();
                    gfx::rhi::update_cbuffer(node_cbuffer, &nodeColor);
                }
                gfx::rhi::end_event();
            }

            
            if (debug::visualizationModes[debug::VisualizationModes::Physics]) {
                physics::Scene& pScene = game.scene.physicsScene;
                for (u32 i = 0; i < pScene.wall_count; i++) {
                    im::segment(pScene.walls[i].start, pScene.walls[i].end, Color32(1.f, 1.f, 1.f, 1.f));
                }
                for (u32 i = 0; i < pScene.obstacle_count; i++) {
                    im::sphere(pScene.obstacles[i].pos, pScene.obstacles[i].radius, Color32(1.f, 1.f, 1.f, 1.f));
                    im::sphere(pScene.obstacles[i].pos, pScene.obstacles[i].radius + 1.f, Color32(1.f, 1.f, 1.f, 0.25f));
                }
            }

            if (debug::visualizationModes[debug::VisualizationModes::Culling]) {
                const Color32 bbColor(0.8f, 0.15f, 0.25f, 0.7f);
                const Color32 ceColor(0.2f, 0.85f, 0.85f, 0.7f);
                
                if (debug::capturedCameras) {
                    allocator::PagedArena scratchArena = game.memory.scratchArenaRoot; // explicit copy
                    u32* isEachNodeVisible = ALLOC_ARRAY(scratchArena, u32, scene.drawNodes.count);
                    memset(isEachNodeVisible, 0, scene.drawNodes.count * sizeof(bool));
                    renderer::VisibleNodes visibleNodesDebug = {};
                    CameraNode& cameraNode = debug::capturedCameras[debug::debugCameraStage];
                    if (debug::debugCameraStage == 0) {
                        // render culled nodes
                        visibleNodesDebug.visible_nodes = ALLOC_ARRAY(scratchArena, u32, scene.drawNodes.count);
                        visibleNodesDebug.visible_nodes_count = 0;
                        float4x4 vpMatrix =
                            math::mult(cameraNode.projectionMatrix, cameraNode.viewMatrix);
                        renderer::computeVisibilityCS(
                            visibleNodesDebug, isEachNodeVisible, vpMatrix, scene);
                        const Color32 color(0.25f, 0.8f, 0.15f, 0.7f);
                        im::frustum(vpMatrix, color);
                    } else {
                        // render mirror with normal
                        const game::Mirrors::Poly& p = game.scene.mirrors.polys[cameraNode.sourceId];
                        // normal is v2-v0xv1-v0 assuming clockwise winding and right handed coordinates
                        float3 normalWS = math::normalize(
                            math::cross(
                                math::subtract(p.v[2], p.v[0]), math::subtract(p.v[1], p.v[0])));
                        float4 planeWS(
                            normalWS.x, normalWS.y, normalWS.z,-math::dot(p.v[0], normalWS));
                        im::plane(planeWS, Color32(1.f, 1.f, 1.f, 1.f));

                        renderer::Frustum& frustum =
                            debug::capturedCameras[cameraNode.parentIndex].frustum;
                        float3 poly[8];
                        u32 poly_count = p.numPts;
                        memcpy(poly, p.v, sizeof(float3) * p.numPts);
                        clip_poly_in_frustum(
                            poly, poly_count, frustum.planes, frustum.numPlanes, countof(poly));
                        const float2 screenScale(
                            320.f * 3.f * 0.5f,
                            240.f * 3.f * 0.5f);
                        float2 polyScreen[8];
                        for (u32 i = 0; i < poly_count; i++) {
                            float4 pCS = math::mult(mainCamera.vpMatrix, float4(poly[i], 1.f));
                            polyScreen[i] = math::scale(math::invScale(pCS.xy, pCS.w), screenScale);
                            im::Text2DParams textParams;
                            textParams.scale = 1;
                            textParams.pos = polyScreen[i];
                            textParams.color = Color32(1.f, 1.f, 1.f, 1.f);
                            im::text2d_format(textParams, "%d", i);
                        }
                        im::poly2d(polyScreen, poly_count, Color32(1.f, 1.f, 1.f, 0.3f));

                        // render culled nodes
                        renderer::CullEntries cullEntries = {};
                        allocCullEntries(scratchArena, cullEntries, scene);
                        renderer::computeVisibilityWS(
                            scratchArena, visibleNodesDebug, isEachNodeVisible,
                            cameraNode.frustum, cullEntries);
                        const Color32 color(0.25f, 0.8f, 0.15f, 0.7f);
                        im::frustum(cameraNode.frustum.planes, cameraNode.frustum.numPlanes, color);
                    }
                    for (u32 i = 0; i < visibleNodesDebug.visible_nodes_count; i++) {
                        u32 n = visibleNodesDebug.visible_nodes[i];
                        const renderer::DrawNode& node = scene.drawNodes.data[n].state.live;
                        im::obb(node.nodeData.worldMatrix, node.min, node.max, bbColor);
                    }
                }
            }

            im::commit3d();
        }
        #endif
        
        // Immediate-mode debug in 2D
        // Can be moved out of the render update, it only pushes data to cpu buffers
        #if __DEBUG
        {
            Color32 defaultCol(0.7f, 0.8f, 0.15f, 1.0f);
            Color32 activeCol(1.0f, 0.2f, 0.1f, 1.0f);
                
            struct EventText {
                char text[256];
                f64 time;
            };

            if (debug::debugMenus[debug::DebugMenus::Main]) {
                im::pane_start(debug::debugPane);
                {
                    // FPS widget
                    {
                        im::label_format("%s: %.lf fps", platform::name, 1. / debug::frameAvg);

                        Color32 green(0.2f, 0.9f, 0.2f, 1.f);
                        const u32 maxHistoryCount = countof(debug::frameHistory);
                        const float barHeight = im::ui.scale * 30.f;
                        float barWidth = im::ui.scale * 4.f;
                        float2 originWS;
                        float2 extents(barWidth * maxHistoryCount, barHeight);
                        im::impl::adjust_bounds_in_parent(
                            originWS, extents, /* match parent width */ true);
                        barWidth = extents.x / maxHistoryCount;
                        im::box_2d(
                            float2(originWS.x, originWS.y - extents.y),
                            float2(originWS.x + extents.x, originWS.y), im::color_dark);
                        for (u32 i = 0; i < maxHistoryCount; i++) {
                            // skip initial frames
                            if (game.time.frameCount < (maxHistoryCount - i + 1)) { continue; }

                            u32 idx = (debug::frameHistoryIdx + i) % maxHistoryCount;
                            f32 h = barHeight * 1.f / (60.f * (f32)debug::frameHistory[idx]);
                            im::box_2d(
                                float2(originWS.x + barWidth * i, originWS.y - extents.y),
                                float2(originWS.x + barWidth * (i + 1), originWS.y - extents.y + h),
                                green);
                        }
                    }

                    float2 mouseWS = im::get_mouse_WS();
                    const float3 mousepos_WS = camera::screenPosToWorldPos(
                        platform::state.input.mouse.x, platform::state.input.mouse.y,
                        platform::state.screen.window_width, platform::state.screen.window_height,
                        game.resources.renderCore.perspProjection.config, game.scene.camera.viewMatrix);
                    
                    float3 eulers_deg = math::scale(game.scene.orbitCamera.eulers, math::r2d32);
                    im::label_format("mouse (%.3f, %.3f)"
                              "\nmouse in scene (%.3f,%.3f,%.3f)"
                              "\nCamera eulers: " FLOAT3_FORMAT("% .3f"),
                        mouseWS.x, mouseWS.y,
                        mousepos_WS.x, mousepos_WS.y, mousepos_WS.z,
                        FLOAT3_PARAMS(eulers_deg));

                        
                    if (im::checkbox("Pause render", &game.time.pausedRender)) {
                        io::format(
                            debug::eventLabel.text, sizeof(debug::eventLabel.text),
                            "Pause scene render: %s", game.time.pausedRender ? "true" : "false");
                        debug::eventLabel.time = platform::state.time.now;
                    }
                    im::checkbox(
                        "Toggle memory arenas menu", &debug::debugMenus[debug::DebugMenus::Arenas]);
                    im::checkbox(
                        "Toggle BVH debugging",
                        &debug::visualizationModes[debug::VisualizationModes::BVH]);
                    if (debug::visualizationModes[debug::VisualizationModes::BVH]) {
                        im::input_step("BVH depth", &debug::bvhDepth, 0u, 10000u);
                    }

                    im::checkbox(
                        "Toggle Culling debugging",
                        &debug::visualizationModes[debug::VisualizationModes::Culling]);
                    if (debug::visualizationModes[debug::VisualizationModes::Culling]) {
                        im::horizontal_layout_start();
                        if (im::button("Capture cameras")) {
                            debug::capture_cameras_next_frame = true;
                        }
                        u32 numCameras = 0;
                        if (debug::capturedCameras) {
                            if (im::button("Clear")) {
                                debug::capturedCameras = nullptr;
                            }
                        }
                        if (debug::capturedCameras) {
                            numCameras = debug::capturedCameras[0].siblingIndex;
                            im::label_format("%d cameras", numCameras);
                        }
                        im::horizontal_layout_end();
                        if (debug::capturedCameras) {
                            im::input_step("Toggle camera", &debug::debugCameraStage, 0u, numCameras - 1, /* wrap */ true);

                            // camera list
                            const u32 minidx = debug::debugCameraStage > 10 ? debug::debugCameraStage - 10 : 0;
                            const u32 maxidx = debug::debugCameraStage + 10 < numCameras ? debug::debugCameraStage + 10 : numCameras;
                            for (u32 i = minidx; i < maxidx; i++) {
                                Color32 color = (i == debug::debugCameraStage) ? im::color_highlight : im::color_bright;
                                im::label_format(color, "%*d: %s", debug::capturedCameras[i].depth, i, debug::capturedCameras[i].str);
                            }

                            // camera frustum debug
                            im::checkbox("force cut frustum visualization", &debug::force_cut_frustum);
                            if (debug::force_cut_frustum) {
                                im::label("Disable frustum planes:");
                                im::checkbox("near", &debug::frustum_planes_off[0]);
                                im::checkbox("far", &debug::frustum_planes_off[1]);
                                im::checkbox("left", &debug::frustum_planes_off[2]);
                                im::checkbox("right", &debug::frustum_planes_off[3]);
                                im::checkbox("bottom", &debug::frustum_planes_off[4]);
                                im::checkbox("top", &debug::frustum_planes_off[5]);
                            }
                        }
                    }
                    im::checkbox(
                        "Toggle Physics debugging",
                        &debug::visualizationModes[debug::VisualizationModes::Physics]);
                }
                im::pane_end();
            }
            im::reset_frame();

            // todo: retest with a pad
            //if (debug::overlaymode == debug::OverlayMode::All
            //    || debug::overlaymode == debug::OverlayMode::HelpOnly) {
            //    for (u32 i = 0; i < platform::state.input.padCount; i++)
            //    {
            //        const ::input::gamepad::State& pad = platform::state.input.pads[i];
            //        im::text2d(textParamsLeft, "Pad: %s", pad.name);
            //        textParamsLeft.pos.y -= lineheight;
            //        im::text2d(textParamsLeft
            //            , "Pad: L:(%.3f,%.3f) R:(%.3f,%.3f) L2:%.3f R2:%.3f"
            //            , pad.sliders[::input::gamepad::Sliders::AXIS_X_LEFT], pad.sliders[::input::gamepad::Sliders::AXIS_Y_LEFT]
            //            , pad.sliders[::input::gamepad::Sliders::AXIS_X_RIGHT], pad.sliders[::input::gamepad::Sliders::AXIS_Y_RIGHT]
            //            , pad.sliders[::input::gamepad::Sliders::TRIGGER_LEFT], pad.sliders[::input::gamepad::Sliders::TRIGGER_RIGHT]
            //        );

            //        {
            //            io::StrBuilder padStr{ appendBuff, sizeof(appendBuff) };
            //            textParamsLeft.pos.y -= lineheight;
            //            io::append(padStr.curr, padStr.last, "Keys: ");
            //            const char* key_names[] = {
            //                  "BUTTON_N", "BUTTON_S", "BUTTON_W", "BUTTON_E"
            //                , "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT"
            //                , "START", "SELECT"
            //                , "L1", "R1", "L2", "R2"
            //                , "LEFT_THUMB", "RIGHT_THUMB"
            //            };
            //            for (u32 key = 0; key < ::input::gamepad::KeyMask::COUNT && !padStr.full(); key++) {
            //                if (pad.down((::input::gamepad::KeyMask::Enum)(1 << key))) {
            //                    io::append(
            //                        padStr.curr, padStr.last, "%s ", key_names[key]);
            //                }
            //            }
            //            im::text2d(textParamsLeft, padStr.str);
            //            textParamsLeft.pos.y -= lineheight;
            //        }
            //    }
            //}

            if (debug::debugMenus[debug::DebugMenus::Arenas])
            {
                auto renderArena = [](u8* arenaEnd, u8* arenaStart, uintptr_t arenaHighmark,
                                      const char* arenaName,
                                      const Color32 baseCol, const Color32 highmarkCol) {
                    const f32 barwidth = 150.f * im::ui.scale;
                    const f32 barheight = 10.f * im::ui.scale;

                    const ptrdiff_t arenaTotal = (ptrdiff_t)arenaEnd - (ptrdiff_t)arenaStart;
                    const ptrdiff_t arenaHighmarkBytes =
                        (ptrdiff_t)arenaHighmark - (ptrdiff_t)arenaStart;
                    const f32 occupancy = arenaHighmarkBytes / (f32)arenaTotal;
                    const f32 arenaHighmark_barwidth = barwidth * occupancy;

                    if (arenaHighmarkBytes > 1024 * 1024) {
                        im::label_format(
                            highmarkCol, "%s highmark: %lu bytes (%.3fMB) / %.3f%%",
                            arenaName, arenaHighmarkBytes, arenaHighmarkBytes / (1024.f * 1024.f),
                            occupancy * 100.f);
                    } else if (arenaHighmarkBytes > 1024) {
                        im::label_format(
                            highmarkCol, "%s highmark: %lu bytes (%.3fKB) / %.3f%%",
                            arenaName, arenaHighmarkBytes, arenaHighmarkBytes / (1024.f),
                            occupancy * 100.f);
                    } else {
                        im::label_format(
                            highmarkCol, "%s highmark: %lu bytes / %.3f%%",
                            arenaName, arenaHighmarkBytes,
                            occupancy * 100.f);
                    }

                    // compute extents in pane
                    float2 extents(barwidth, barheight);
                    float2 originWS;
                    im::impl::adjust_bounds_in_parent(
                        originWS, extents, /* match parent width */ false);
                    
                    // background
                    im::box_2d(
                        float2(originWS.x, originWS.y - extents.y),
                        float2(originWS.x + extents.x, originWS.y), baseCol);
                    if (arenaHighmarkBytes) {
                        im::box_2d(
                            float2(originWS.x, originWS.y - extents.y),
                            float2(originWS.x + arenaHighmark_barwidth, originWS.y), highmarkCol);
                    }
                };

                im::pane_start(debug::arenasPane);
                {
                    const f32 barwidth = 150.f * im::ui.scale;
                    const f32 barheight = 10.f * im::ui.scale;
                    {
                        const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                        const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                        renderArena(
                            (u8*)math::max(
                                (uintptr_t)game.memory.frameArena.end, game.memory.frameArenaHighmark),
                            game.memory.frameArenaBuffer, game.memory.frameArenaHighmark,
                            "Frame arena", arenabaseCol, arenahighmarkCol);
                    }
                    {
                        const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                        const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                        renderArena(
                            (u8*)math::max(
                                (uintptr_t)game.memory.scratchArenaRoot.end, game.memory.scratchArenaHighmark),
                            game.memory.scratchArenaRoot.curr, game.memory.scratchArenaHighmark,
                            "Scratch arena", arenabaseCol, arenahighmarkCol);
                    }
                    {
                        const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                        const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                        renderArena(
                            game.memory.persistentArena.end, game.memory.persistentArenaBuffer,
                            (ptrdiff_t)game.memory.persistentArena.curr,
                            "Persistent arena", arenabaseCol, arenahighmarkCol);
                    }
                    {
                        const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                        const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                        renderArena(
                            game.memory.sceneArena.end, game.memory.sceneArenaBuffer,
                            (ptrdiff_t)game.memory.sceneArena.curr,
                            "Scene arena", arenabaseCol, arenahighmarkCol);
                    }
                    {
                        const Color32 baseCol(0.65f, 0.65f, 0.65f, 0.4f);
                        const Color32 used3dCol(0.95f, 0.35f, 0.8f, 1.f);
                        const Color32 used2dCol(0.35f, 0.95f, 0.8f, 1.f);
                        const Color32 used2didxCol(0.8f, 0.95f, 0.8f, 1.f);

                        const ptrdiff_t memory_size =
                            (ptrdiff_t)game.memory.debugArena.curr - (ptrdiff_t)im::ctx.vertices_3d;
                        const ptrdiff_t vertices_3d_start =
                            (ptrdiff_t)im::ctx.vertices_3d - (ptrdiff_t)im::ctx.vertices_3d;
                        const ptrdiff_t vertices_3d_size =
                            (ptrdiff_t)debug::vertices_3d_head_last_frame * sizeof(im::Vertex3D);
                        const ptrdiff_t vertices_2d_start =
                            (ptrdiff_t)im::ctx.vertices_2d - (ptrdiff_t)im::ctx.vertices_3d;
                        const ptrdiff_t vertices_2d_size =
                            (ptrdiff_t)debug::vertices_2d_head_last_frame * sizeof(im::Vertex2D);
                        const ptrdiff_t indices_2d_start =
                            (ptrdiff_t)im::ctx.indices_2d - (ptrdiff_t)im::ctx.vertices_3d;
                        const ptrdiff_t indices_2d_size =
                            (ptrdiff_t)(debug::vertices_2d_head_last_frame * 3 / 2) * sizeof(u32);
                        const f32 v3d_barstart = barwidth * vertices_3d_start / (f32)memory_size;
                        const f32 v3d_barwidth = barwidth * vertices_3d_size / (f32)memory_size;
                        const f32 v2d_barstart = barwidth * vertices_2d_start / (f32)memory_size;
                        const f32 v2d_barwidth = barwidth * vertices_2d_size / (f32)memory_size;
                        const f32 i2d_barstart = barwidth * indices_2d_start / (f32)memory_size;
                        const f32 i2d_barwidth = barwidth * indices_2d_size / (f32)memory_size;

                        im::label_format(used3dCol, "im 3d: %lu bytes", vertices_3d_size);
                        im::label_format(used2dCol, "im 2d: %lu bytes", vertices_2d_size);
                        im::label_format(used2didxCol, "im 2d indices: %lu bytes", indices_2d_size);

                        // compute extents in pane
                        float2 extents(barwidth, barheight);
                        float2 originWS;
                        im::impl::adjust_bounds_in_parent(
                            originWS, extents, /* match parent width */ false);

                        im::box_2d(
                            float2(originWS.x, originWS.y - extents.y),
                            float2(originWS.x + extents.x, originWS.y),
                            baseCol);
                        im::box_2d(
                            float2(originWS.x + v3d_barstart, originWS.y - extents.y),
                            float2(originWS.x + v3d_barstart + v3d_barwidth, originWS.y),
                            used3dCol);
                        im::box_2d(
                            float2(originWS.x + v2d_barstart, originWS.y - extents.y),
                            float2(originWS.x + v2d_barstart + v2d_barwidth, originWS.y),
                            used2dCol);
                        im::box_2d(
                            float2(originWS.x + i2d_barstart, originWS.y - extents.y),
                            float2(originWS.x + i2d_barstart + i2d_barwidth, originWS.y),
                            used2didxCol);
                    }
                }
                im::pane_end();
            }

            // event label
            if (debug::eventLabel.time != 0.f) {

                const f32 label_width = im::ui.scale * (f32)stb_easy_font_width(debug::eventLabel.text);
                im::Text2DParams textParams;
                textParams.scale = 2 * (u8)im::ui.scale;
                textParams.pos = float2(
                    -label_width / 2.f,
                    game.resources.renderCore.windowProjection.config.top - 10.f * im::ui.scale);

                f32 timeRunning = f32(platform::state.time.now - debug::eventLabel.time);
                f32 t = timeRunning / 2.f;
                if (t > 1.f) { debug::eventLabel.time = 0.f; }
                else { 
                    textParams.color = Color32(1.0f, 0.2f, 0.1f, 1.f - t);
                    im::text2d(debug::eventLabel.text, textParams);
                }
            }
        }
        #endif

        // copy backbuffer to window (upscale from game resolution to window resolution)
        {
            gfx::rhi::start_event("UPSCALE TO WINDOW");
            {
                {
                    gfx::rhi::ViewportParams vpParams;
                    vpParams.topLeftX = 0;
                    vpParams.topLeftY = 0;
                    vpParams.width = (f32)platform::state.screen.window_width;
                    vpParams.height = (f32)platform::state.screen.window_height;
                    gfx::rhi::set_VP(vpParams);
                }

                gfx::rhi::bind_blend_state(game.resources.renderCore.blendStateBlendOff);
                gfx::rhi::bind_DS( game.resources.renderCore.depthStateOff);
                gfx::rhi::bind_RS( game.resources.renderCore.rasterizerStateFillFrontfaces);
                gfx::rhi::bind_main_RT(game.resources.renderCore.windowRT);

                gfx::rhi::bind_shader(game.resources.renderCore.shaders[renderer::ShaderTechniques::FullscreenBlitTextured]);
                gfx::rhi::RscTexture textures[] = {
                    game.resources.renderCore.gameRT.textures[0],
                    //game.resources.renderCore.gameRT.depthStencil // todo: make work in opengl
                };
                gfx::rhi::bind_textures(textures, countof(textures));
                gfx::rhi::draw_fullscreen();
                gfx::rhi::RscTexture nullTex = {};
                gfx::rhi::bind_textures(&nullTex, 1); // unbind gameRT
            }
            gfx::rhi::end_event();
        }

        // Hires SDF scene (only available if the upscale pass copies depth too)
        //renderSDFScene(cameraTree[0], game.resources.renderCore, game.resources.renderCore.depthStateOn);

        // from now we should have
        // rhi::bind_main_RT(game.resources.renderCore.windowRT);
        // UI
        {
            gfx::rhi::bind_blend_state(game.resources.renderCore.blendStateOn);
            gfx::rhi::start_event("UI");
            {
                renderer::CoreResources& rsc = game.resources.renderCore;
                gfx::rhi::RscCBuffer& scene_cbuffer =
                    rsc.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
                gfx::rhi::RscCBuffer& uitext_cbuffer =
                    rsc.cbuffers[renderer::CoreResources::CBuffersMeta::UIText];

                renderer::SceneData cbufferPerScene;
                cbufferPerScene.vpMatrix = rsc.windowProjection.matrix;
                gfx::rhi::update_cbuffer(scene_cbuffer, &cbufferPerScene);

                gfx::rhi::bind_RS(game.resources.renderCore.rasterizerStateFillFrontfaces);
                gfx::rhi::bind_DS(game.resources.renderCore.depthStateOff);

                gfx::rhi::bind_shader(rsc.shaders[renderer::ShaderTechniques::Color2D]);
                gfx::rhi::RscCBuffer cbuffers[] = { scene_cbuffer, uitext_cbuffer };
                gfx::rhi::bind_cbuffers(rsc.shaders[renderer::ShaderTechniques::Color2D], cbuffers, 2);
                
                #if __DEBUG
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor =
                        platform::state.input.keyboard.pressed(input::TOGGLE_DEBUG) ?
                        float4(1.f, 0.f, 0.f, 1.f)
                        : float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    gfx::rhi::update_cbuffer(uitext_cbuffer, &noteUItext);

                    gfx::rhi::bind_indexed_vertex_buffer(game.resources.gpuBufferDebugText);
                    gfx::rhi::draw_indexed_vertex_buffer(game.resources.gpuBufferDebugText);
                }
                #endif
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor = float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    gfx::rhi::update_cbuffer(uitext_cbuffer, &noteUItext);

                    gfx::rhi::bind_indexed_vertex_buffer(game.resources.gpuBufferHeaderText);
                    gfx::rhi::draw_indexed_vertex_buffer(game.resources.gpuBufferHeaderText);
                }
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor =
                        (platform::state.input.mouse.scrolldy != 0) ?
                            float4(1.f, 0.f, 0.f, 1.f)
                          : float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    gfx::rhi::update_cbuffer(uitext_cbuffer, &noteUItext);

                    gfx::rhi::bind_indexed_vertex_buffer(game.resources.gpuBufferScaleText);
                    gfx::rhi::draw_indexed_vertex_buffer(game.resources.gpuBufferScaleText);
                }
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor =
                        platform::state.input.mouse.down(::input::mouse::Keys::BUTTON_RIGHT) ?
                            float4(1.f, 0.f, 0.f, 1.f)
                          : float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    gfx::rhi::update_cbuffer(uitext_cbuffer, &noteUItext);

                    gfx::rhi::bind_indexed_vertex_buffer(game.resources.gpuBufferPanText);
                    gfx::rhi::draw_indexed_vertex_buffer(game.resources.gpuBufferPanText);
                }
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor =
                        platform::state.input.mouse.down(::input::mouse::Keys::BUTTON_LEFT) ?
                            float4(1.f, 0.f, 0.f, 1.f)
                          : float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    gfx::rhi::update_cbuffer(uitext_cbuffer, &noteUItext);

                    gfx::rhi::bind_indexed_vertex_buffer(game.resources.gpuBufferOrbitText);
                    gfx::rhi::draw_indexed_vertex_buffer(game.resources.gpuBufferOrbitText);
                }
            }
            gfx::rhi::end_event();
        }
        // Batched 2d debug (clear cpu buffers onto the screen)
        #if __DEBUG
        {
            gfx::rhi::bind_blend_state(game.resources.renderCore.blendStateOn);
            im::commit2d();
            im::present2d(game.resources.renderCore.windowProjection.matrix);
        }
        #endif

    }
}
}

#endif // __WASTELADNS_GAME_H__
