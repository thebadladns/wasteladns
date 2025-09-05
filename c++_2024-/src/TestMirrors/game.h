#ifndef __WASTELADNS_GAME_H__
#define __WASTELADNS_GAME_H__

const size_t persistentArenaSize = 1 * 1024 * 1024;
const size_t sceneArenaSize = 256 * 1024 * 1024;
const size_t frameArenaSize = 4 * 1024 * 1024;
const size_t scratchArenaSize = 4 * 1024 * 1024;

#if __DEBUG
struct CameraNode;
namespace debug {
struct OverlayMode { enum Enum { All, HelpOnly, ArenaOnly, None, Count }; };
const char* overlaynames[] = { "All", "Help Only", "Arenas only", "None" };
static_assert(countof(overlaynames) == debug::OverlayMode::Count, "check");
struct Debug3DView { enum Enum { Physics, Culling, BVH, All, None, Count }; };
const char* visualizationModes[] = { "Physics", "Culling", "BVH", "All", "None" };
static_assert(countof(visualizationModes) == debug::Debug3DView::Count, "check");
struct EventText { char text[256]; f64 time; };

f64 frameHistory[60];
f64 frameAvg = 0;
u64 frameHistoryIdx = 0;
OverlayMode::Enum overlaymode = OverlayMode::Enum::All;
Debug3DView::Enum debug3Dmode = Debug3DView::Enum::None;
u32 debugCameraStage = 0;
u32 bvhDepth = 0;
CameraNode* capturedCameras = nullptr;
EventText eventLabel = {};

}
#endif

#include "gameplay.h"
#include "shaders.h"
#include "drawlist.h"
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
        , CYCLE_ROOM = ::input::keyboard::Keys::N
        #if __DEBUG
        , TOGGLE_OVERLAY = ::input::keyboard::Keys::H
        , TOGGLE_DEBUG3D = ::input::keyboard::Keys::V
        , CAPTURE_CAMERAS = ::input::keyboard::Keys::P
        , TOGGLE_CAPTURED_CAMERA = ::input::keyboard::Keys::C
        , TOGGLE_FRUSTUM_CUT = ::input::keyboard::Keys::NUM0
        , TOGGLE_FRUSTUM_NEAR = ::input::keyboard::Keys::NUM1
        , TOGGLE_FRUSTUM_FAR = ::input::keyboard::Keys::NUM2
        , TOGGLE_FRUSTUM_LEFT = ::input::keyboard::Keys::NUM3
        , TOGGLE_FRUSTUM_RIGHT = ::input::keyboard::Keys::NUM4
        , TOGGLE_FRUSTUM_BOTTOM = ::input::keyboard::Keys::NUM5
        , TOGGLE_FRUSTUM_TOP = ::input::keyboard::Keys::NUM6
        , TOGGLE_BVH_LEVEL = ::input::keyboard::Keys::C
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
    config.title = "Mirror Test";
}
void start(Instance& game, platform::GameConfig& config, platform::State& platform) {

    game.time = {};
    game.time.config.maxFrameLength = 0.1;
    game.time.config.targetFramerate = 1.0 / 60.0;
    game.time.lastFrame = platform.time.now;

    config = {};
    config.nextFrame = platform.time.now;

    {
        allocator::init_arena(
            game.memory.persistentArena, persistentArenaSize);
        __DEBUGDEF(game.memory.persistentArenaBuffer = game.memory.persistentArena.curr;)
        allocator::init_arena(
            game.memory.sceneArena, sceneArenaSize);
        game.memory.sceneArenaBuffer = game.memory.sceneArena.curr;
        allocator::init_arena(game.memory.scratchArenaRoot, scratchArenaSize);
        __DEBUGDEF(game.memory.scratchArenaHighmark =
                (uintptr_t)game.memory.scratchArenaRoot.curr;
            game.memory.scratchArenaRoot.highmark = &game.memory.scratchArenaHighmark;)
        allocator::init_arena(
            game.memory.frameArena, frameArenaSize);
        game.memory.frameArenaBuffer = game.memory.frameArena.curr;
        __DEBUGDEF(game.memory.frameArenaHighmark =
                (uintptr_t)game.memory.frameArena.curr;
            game.memory.frameArena.highmark = &game.memory.frameArenaHighmark;)
        __DEBUGDEF(allocator::init_arena(
                game.memory.debugArena, renderer::im::arena_size);)
    }
    {
        game.scene = {};
        game.roomId = 0;
        SceneMemory arenas = {
            game.memory.persistentArena,
            game.memory.scratchArenaRoot
            __DEBUGDEF(, game.memory.debugArena)
        };
        load_coreResources(game.resources, arenas, platform.screen);
        spawn_scene_mirrorRoom(
            game.scene, game.memory.sceneArena, game.memory.scratchArenaRoot,
            game.resources, platform.screen,
            roomDefinitions[game.roomId]);

    }
}

void update(Instance& game, platform::GameConfig& config, platform::State& platform) {

    // frame arena reset
    game.memory.frameArena.curr = game.memory.frameArenaBuffer;

    // frame timing calculations
    {
        f64 raw_dt = platform.time.now - game.time.lastFrame;
        game.time.lastFrameDelta = math::min(raw_dt, game.time.config.maxFrameLength);
        game.time.lastFrame = platform.time.now;
        config.nextFrame = platform.time.now + game.time.config.targetFramerate;

        #if __DEBUG
        {
            f64 frameAvg = 0.;
            constexpr u64 frameHistoryCount =
                (sizeof(debug::frameHistory) / sizeof(debug::frameHistory[0]));
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
    const ::input::keyboard::State& keyboard = platform.input.keyboard;
    const u32 prevRoomId = game.roomId;
    bool step = true;
    __DEBUGDEF(bool captureCameras = false;)
    {
        if (keyboard.released(input::EXIT)) {
            config.quit = true;
        }
        if (keyboard.pressed(input::CYCLE_ROOM)) {
            game.roomId = (game.roomId + 1) % countof(roomDefinitions);
        }
        if (keyboard.pressed(input::TOGGLE_PAUSE_SCENE_RENDER)) {
            game.time.pausedRender = !game.time.pausedRender;
        }
        #if __DEBUG
        if (keyboard.pressed(input::TOGGLE_OVERLAY)) {
            debug::overlaymode =
                (debug::OverlayMode::Enum)((debug::overlaymode + 1) % debug::OverlayMode::Count);
        }
        if (keyboard.pressed(input::TOGGLE_DEBUG3D)) {
            debug::debug3Dmode =
                (debug::Debug3DView::Enum)((debug::debug3Dmode + 1) % debug::Debug3DView::Count);
        }
        if (debug::debug3Dmode == debug::Debug3DView::Culling) {
            if (keyboard.pressed(input::TOGGLE_CAPTURED_CAMERA)) {
                if (debug::capturedCameras) {
                    if (keyboard.down(::input::keyboard::Keys::LEFT_SHIFT)) {
                        if (debug::debugCameraStage == 0) {
                            debug::debugCameraStage = debug::capturedCameras[0].siblingIndex - 1;
                        } else { debug::debugCameraStage--; }
                    } else {
                        if (debug::debugCameraStage == debug::capturedCameras[0].siblingIndex - 1) {
                            debug::debugCameraStage = 0;
                        } else { debug::debugCameraStage++; }
                    }
                }
            }
            if (keyboard.pressed(input::CAPTURE_CAMERAS)) { captureCameras = true; }
            if (keyboard.pressed(input::TOGGLE_FRUSTUM_CUT)) {
                debug::force_cut_frustum = !debug::force_cut_frustum;
            }
            if (keyboard.pressed(input::TOGGLE_FRUSTUM_NEAR)) {
                debug::frustum_planes_off[0] = !debug::frustum_planes_off[0];
            }
            if (keyboard.pressed(input::TOGGLE_FRUSTUM_FAR)) {
                debug::frustum_planes_off[1] = !debug::frustum_planes_off[1];
            }
            if (keyboard.pressed(input::TOGGLE_FRUSTUM_LEFT)) {
                debug::frustum_planes_off[2] = !debug::frustum_planes_off[2];
            }
            if (keyboard.pressed(input::TOGGLE_FRUSTUM_RIGHT)) {
                debug::frustum_planes_off[3] = !debug::frustum_planes_off[3];
            }
            if (keyboard.pressed(input::TOGGLE_FRUSTUM_BOTTOM)) {
                debug::frustum_planes_off[4] = !debug::frustum_planes_off[4];
            }
            if (keyboard.pressed(input::TOGGLE_FRUSTUM_TOP)) {
                debug::frustum_planes_off[5] = !debug::frustum_planes_off[5];
            }
        } else if (debug::debug3Dmode == debug::Debug3DView::BVH) {
            if (keyboard.pressed(input::TOGGLE_BVH_LEVEL)) {
                if (keyboard.down(::input::keyboard::Keys::LEFT_SHIFT)) {
                    if (debug::bvhDepth > 0) { debug::bvhDepth--; }
                } else { debug::bvhDepth++; }
            }
        }
        #endif
    }

    if (prevRoomId != game.roomId) {
        game.memory.sceneArena.curr = game.memory.sceneArenaBuffer;
        game.scene = {};
        spawn_scene_mirrorRoom(
            game.scene, game.memory.sceneArena, game.memory.scratchArenaRoot,
            game.resources, platform.screen,
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
            game::movementInputFromPad(controlkeyboard, platform.input.pads[0]);
            game.scene.player.control.localInput = math::add(controlpad.localInput, controlkeyboard.localInput);
            game.scene.player.control.mag = math::mag(game.scene.player.control.localInput);
            if (game.scene.player.control.mag > math::eps32) {
                game.scene.player.control.localInput = math::invScale(game.scene.player.control.localInput, game.scene.player.control.mag);
                game.scene.player.control.mag = math::min(game.scene.player.control.mag, 1.f);
            }
            const float2 effectiveLocalInput = game::calculateMovement_cameraRelative(
                game.scene.player.transform, game.scene.player.state, game.scene.player.control,
                game.scene.camera.transform, dt);

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
                            + scalez * (math::cos((f32)platform.time.now * 10.f + i * 2.f) - 1.5f);
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
            game::orbitInputFromPad(game.scene.orbitCamera, platform.input.pads[0]);
            game::orbitInputFromMouse(game.scene.orbitCamera, platform.input.mouse);
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
                driver::ViewportParams vpParams;
                vpParams.topLeftX = 0;
                vpParams.topLeftY = 0;
                vpParams.width = (f32)platform.screen.width;
                vpParams.height = (f32)platform.screen.height;
                vpParams.minDepth = 0.f;
                vpParams.maxDepth = 1.f;
                driver::set_VP(vpParams);
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
                    __PROFILEONLY(platform::format(mainCameraRoot.str, sizeof(mainCameraRoot.str), "_");)
                    renderer::extract_frustum_planes_from_vp(
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
                    (u32*)allocator::alloc_arena(
                        game.memory.frameArena,
                        scene.drawNodes.count * sizeof(u32), alignof(u32));
                memset(isEachNodeVisible, 0, scene.drawNodes.count * sizeof(bool));
                visibleNodesTree = // todo: in function??
                    (VisibleNodes*)allocator::alloc_arena(
                        game.memory.frameArena,
                        numCameras * sizeof(VisibleNodes), alignof(VisibleNodes));
                visibleNodesTree[0].visible_nodes =
                    (u32*)allocator::alloc_arena(
                            game.memory.frameArena,
                        scene.drawNodes.count * sizeof(u32), alignof(u32));
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
                    driver::update_cbuffer(
                            cbuffer_from_handle(scene, node.cbuffer_node),
                            &node.nodeData);
                    if (node.cbuffer_ext) {
                        driver::update_cbuffer(
                                cbuffer_from_handle(scene, node.cbuffer_ext),
                                node.ext_data);
                    }
                }
                for (u32 n = 0, count = 0; n < scene.instancedDrawNodes.cap && count < scene.instancedDrawNodes.count; n++) {
                    if (scene.instancedDrawNodes.data[n].alive == 0) { continue; }
                    count++;
                    const DrawNodeInstanced& node = scene.instancedDrawNodes.data[n].state.live;
                    driver::update_cbuffer(
                            cbuffer_from_handle(scene, node.cbuffer_node),
                            &node.nodeData);
                    driver::update_cbuffer(
                            cbuffer_from_handle(scene, node.cbuffer_instances),
                            &node.instanceMatrices);
                }
            }

            // render main camera
            driver::bind_RT(renderCore.gameRT);
            // TODO: figure out whether this is needed
            driver::clear_RT(renderCore.gameRT,
                driver::RenderTargetClearFlags::Stencil);

            driver::Marker_t marker;
            driver::set_marker_name(marker, "BASE SCENE"); driver::start_event(marker);
            {
                RenderSceneContext renderSceneContext = {
                    cameraTree[0], visibleNodesTree[0], game.scene, renderCore,
                    renderCore.depthStateAlways,
                    renderCore.depthStateOn,
                    renderCore.depthStateReadOnly,
                    renderCore.rasterizerStateFillFrontfaces,
                    game.memory.scratchArenaRoot };
                renderBaseScene(renderSceneContext);
            }
            driver::end_event();

            // render camera tree
            if (cameraTree[0].siblingIndex > 1) {
                renderMirrorTree(
                        cameraTree, visibleNodesTree, game.scene, renderCore,
                        game.memory.scratchArenaRoot);
            }
        }
    }
    {
        using namespace renderer;
        #if __DEBUG
        // render update wrap up
        renderer::Scene& scene = game.scene.renderScene;

        // Immediate mode debug, to be rendered along with the 3D scene on the next frame
        {
            //World axis
            const Color32 axisX(0.8f, 0.15f, 0.25f, 0.7f); // x is red
            const Color32 axisY(0.25f, 0.8f, 0.15f, 0.7f); // y is green
            const Color32 axisZ(0.15f, 0.25f, 0.8f, 0.7f); // z is blue
            const f32 axisSize = 7.f;
            const float3 pos(0.f, 0.f, 0.1f);

            im::segment(pos, math::add(pos, math::scale(float3(1.f, 0.f, 0.f), axisSize)), axisX);
            im::segment(pos, math::add(pos, math::scale(float3(0.f, 1.f, 0.f), axisSize)), axisY);
            im::segment(pos, math::add(pos, math::scale(float3(0.f, 0.f, 1.f), axisSize)), axisZ);

            const f32 thickness = 0.1f;
            const u32 steps = 10;
            const f32 spacing = 1.f / (f32)steps;
            for (int i = 1; i <= steps; i++) {
                im::segment(
                      math::add(pos, float3(spacing * i, 0.f, -thickness))
                    , math::add(pos, float3(spacing * i, 0.f, thickness))
                    , axisX);
                im::segment(
                      math::add(pos, float3(0.f, spacing * i, -thickness))
                    , math::add(pos, float3(0.f, spacing * i, thickness))
                    , axisY);
                im::segment(
                      math::add(pos, float3(-thickness, thickness, spacing * i))
                    , math::add(pos, float3(thickness, -thickness, spacing * i))
                    , axisZ);
            }


            if (debug::debug3Dmode == debug::Debug3DView::All
                || debug::debug3Dmode == debug::Debug3DView::BVH) {

                allocator::PagedArena scratchArena = game.memory.scratchArenaRoot; // explicit copy
                renderer::CoreResources& renderCore = game.resources.renderCore;
                bvh::Tree& bvh = game.scene.mirrors.bvh;

                driver::Marker_t marker;
                driver::set_marker_name(marker, "BVH"); driver::start_event(marker);
                if (bvh.nodeCount > 0) {

                    struct DrawNode {
                        u16 nodeid;
                        u16 depth;
                    };
                    DrawNode* nodeStack = (DrawNode*)allocator::alloc_arena(
                        scratchArena, bvh.nodeCount * sizeof(DrawNode), alignof(DrawNode));
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
                            #if DISABLE_INTRINSICS
                                min = node.min; max = node.max;
                            #else
                                bvh::ymm_to_minmax(
                                    min, max, node.xcoords_256, node.ycoords_256, node.zcoords_256);
                            #endif
                            float3 center = math::scale(math::add(min, max), 0.5f);
                            float3 scale = math::scale(math::subtract(max, min), 0.5f);
                            allocator::push(aabbs, scratchArena) = { center, scale };
                        }
                        else if (!node.isLeaf) {
                            nodeStack[stackCount++] =
                                DrawNode{ bvh.nodes[n.nodeid].lchildId, u16(n.depth + 1u) };
                            nodeStack[stackCount++] =
                                DrawNode{ u16(bvh.nodes[n.nodeid].lchildId + 1u), u16(n.depth + 1u) };
                        }
                    }

                    driver::RscCBuffer& scene_cbuffer =
                        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
                    driver::RscCBuffer& node_cbuffer =
                        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::NodeIdentity];
                    driver::RscCBuffer& instanced_cbuffer =
                        renderCore.cbuffers[renderer::CoreResources::CBuffersMeta::Instances64];

                    renderer::NodeData nodeColor = {};
                    math::identity4x4(*(Transform*)&nodeColor.worldMatrix);
                    nodeColor.groupColor = Color32(0.4f, 0.54f, 1.f, 0.3f).RGBAv4();
                    driver::update_cbuffer(node_cbuffer, &nodeColor);

                    driver::RscCBuffer cbuffers[] =
                    { scene_cbuffer, node_cbuffer, instanced_cbuffer };
                    renderer::DrawMesh& drawMesh =
                        renderer::drawMesh_from_handle(
                            renderCore, game.resources.instancedUnitCubeMesh);

                    renderer::driver::bind_DS(renderCore.depthStateOn);
                    renderer::driver::bind_shader(renderCore.shaders[drawMesh.shaderTechnique]);
                    renderer::driver::bind_RS(renderCore.rasterizerStateLine);
                    renderer::driver::bind_indexed_vertex_buffer(drawMesh.vertexBuffer);
                    driver::bind_cbuffers(
                        renderCore.shaders[drawMesh.shaderTechnique], cbuffers, countof(cbuffers));

                    // round up, the last block will have less than or equal max elems
                    u32 blockCount = ((u32)aabbs.len + 64 - 1) / 64;
                    for (u32 blockId = 0; blockId < blockCount; blockId++) {
                        u32 begin = blockId * 64;
                        u32 end = math::min(begin + 64, (u32)aabbs.len);

                        u32 bufferId = 0;
                        Matrices64 matrices = {};
                        for (u32 i = begin; i < end; i++) {
                            Transform t;
                            math::identity4x4(t);
                            t.pos = aabbs.data[i].center;
                            t.matrix.col0.x = aabbs.data[i].scale.x;
                            t.matrix.col1.y = aabbs.data[i].scale.y;
                            t.matrix.col2.z = aabbs.data[i].scale.z;
                            matrices.data[bufferId++] = t.matrix;
                        }
                        driver::update_cbuffer(instanced_cbuffer, &matrices);
                        driver::draw_instances_indexed_vertex_buffer(
                            drawMesh.vertexBuffer, end - begin);
                    }

                    // clean up identity node
                    nodeColor.groupColor = Color32(1.f, 1.f, 1.f, 1.f).RGBAv4();
                    driver::update_cbuffer(node_cbuffer, &nodeColor);
                }
                renderer::driver::end_event();
            }

            
            if (debug::debug3Dmode == debug::Debug3DView::All
             || debug::debug3Dmode == debug::Debug3DView::Physics) {
                physics::Scene& pScene = game.scene.physicsScene;
                for (u32 i = 0; i < pScene.wall_count; i++) {
                    renderer::im::segment(pScene.walls[i].start, pScene.walls[i].end, Color32(1.f, 1.f, 1.f, 1.f));
                }
                for (u32 i = 0; i < pScene.obstacle_count; i++) {
                    renderer::im::sphere(pScene.obstacles[i].pos, pScene.obstacles[i].radius, Color32(1.f, 1.f, 1.f, 1.f));
                }
            }

            if (debug::debug3Dmode == debug::Debug3DView::All
             || debug::debug3Dmode == debug::Debug3DView::Culling) {
                const Color32 bbColor(0.8f, 0.15f, 0.25f, 0.7f);
                const Color32 ceColor(0.2f, 0.85f, 0.85f, 0.7f);
                
                if (debug::capturedCameras) {
                    allocator::PagedArena scratchArena = game.memory.scratchArenaRoot; // explicit copy
                    u32* isEachNodeVisible =
                        (u32*)allocator::alloc_arena(
                            scratchArena,
                            scene.drawNodes.count * sizeof(u32), alignof(u32));
                    memset(isEachNodeVisible, 0, scene.drawNodes.count * sizeof(bool));
                    renderer::VisibleNodes visibleNodesDebug = {};
                    CameraNode& cameraNode = debug::capturedCameras[debug::debugCameraStage];
                    if (debug::debugCameraStage == 0) {
                        // render culled nodes
                        visibleNodesDebug.visible_nodes =
                            (u32*)allocator::alloc_arena(
                                scratchArena,
                                scene.drawNodes.count * sizeof(u32), alignof(u32));
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
                        #if DISABLE_INTRINSICS
                            clip_poly_in_frustum(
                                poly, poly_count, frustum.planes, frustum.numPlanes, countof(poly));
                        #else
                            m256_4 planes_256[renderer::Frustum::MAX_PLANE_COUNT];
                            for (u32 p = 0; p < frustum.numPlanes; p++) {
                                planes_256[p].vx = _mm256_set1_ps(frustum.planes[p].x);
                                planes_256[p].vy = _mm256_set1_ps(frustum.planes[p].y);
                                planes_256[p].vz = _mm256_set1_ps(frustum.planes[p].z);
                                planes_256[p].vw = _mm256_set1_ps(frustum.planes[p].w);
                            }
                            clip_poly_in_frustum_256(
                                poly, poly_count, planes_256, frustum.numPlanes, countof(poly));
                        #endif
                        const float2 screenScale(
                            320.f * 3.f * 0.5f,
                            240.f * 3.f * 0.5f);
                        float2 polyScreen[8];
                        for (u32 i = 0; i < poly_count; i++) {
                            float4 pCS = math::mult(mainCamera.vpMatrix, float4(poly[i], 1.f));
                            polyScreen[i] = math::scale(math::invScale(pCS.xy, pCS.w), screenScale);
                            renderer::im::Text2DParams textParams;
                            textParams.scale = 1;
                            textParams.pos = polyScreen[i];
                            textParams.color = Color32(1.f, 1.f, 1.f, 1.f);
                            renderer::im::text2d(textParams, "%d", i);
                        }
                        renderer::im::poly2d(polyScreen, poly_count, Color32(1.f, 1.f, 1.f, 0.3f));

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
                        const DrawNode& node = scene.drawNodes.data[n].state.live;
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
        if (debug::overlaymode != debug::OverlayMode::None)
        {
            Color32 defaultCol(0.7f, 0.8f, 0.15f, 1.0f);
            Color32 activeCol(1.0f, 0.2f, 0.1f, 1.0f);
                
            f32 textscale = platform.screen.window_scale;
            f32 lineheight = 15.f * textscale;

            char appendBuff[128];

            struct EventText {
                char text[256];
                f64 time;
            };
            renderer::im::Text2DParams textParamsLeft, textParamsRight, textParamsCenter;
            textParamsLeft.scale = (u8)textscale;
            textParamsLeft.pos =float2(
                game.resources.renderCore.windowProjection.config.left + 10.f * textscale,
                game.resources.renderCore.windowProjection.config.top - 10.f * textscale);
            textParamsLeft.color = defaultCol;
            textParamsRight.scale = (u8)textscale;
            textParamsRight.pos = float2(
                 game.resources.renderCore.windowProjection.config.right - 60.f * textscale,
                 game.resources.renderCore.windowProjection.config.top - 10.f * textscale);
            textParamsRight.color = defaultCol;
            textParamsCenter.scale = (u8)textscale;
            textParamsCenter.pos = float2(
                0.f, game.resources.renderCore.windowProjection.config.top - 10.f * textscale);
            textParamsCenter.color = defaultCol;

            if (keyboard.pressed(input::TOGGLE_OVERLAY)) {
                platform::format(
                    debug::eventLabel.text, sizeof(debug::eventLabel.text),
                    "Overlays: %s", debug::overlaynames[debug::overlaymode]);
                debug::eventLabel.time = platform.time.now;
                textParamsLeft.color = activeCol;
            }
            else {
                textParamsLeft.color = defaultCol;
            }

            renderer::im::text2d(
                textParamsLeft, "H to toggle overlays: %s", debug::overlaynames[debug::overlaymode]);
            textParamsLeft.pos.y -= lineheight;

            if (keyboard.pressed(input::TOGGLE_PAUSE_SCENE_RENDER)) {
                platform::format(
                    debug::eventLabel.text, sizeof(debug::eventLabel.text), "Paused scene render");
                debug::eventLabel.time = platform.time.now;
                textParamsLeft.color = activeCol;
            }
            else {
                textParamsLeft.color = defaultCol;
            }
            renderer::im::text2d(textParamsLeft, "SPACE to toggle scene rendering pause");
            textParamsLeft.pos.y -= lineheight;

            if (keyboard.pressed(input::TOGGLE_DEBUG3D)) {
                platform::format(
                    debug::eventLabel.text, sizeof(debug::eventLabel.text),
                    "Visualization mode: %s", debug::visualizationModes[debug::debug3Dmode]);
                debug::eventLabel.time = platform.time.now;
                textParamsLeft.color = activeCol;
            }
            else {
                textParamsLeft.color = defaultCol;
            }
            renderer::im::text2d(
                textParamsLeft, "V to toggle 3D visualization modes: %s",
                debug::visualizationModes[debug::debug3Dmode]);
            textParamsLeft.pos.y -= lineheight;

            if (debug::debug3Dmode == debug::Debug3DView::BVH) {

                if (keyboard.pressed(input::TOGGLE_BVH_LEVEL)) {
                    platform::format(
                        debug::eventLabel.text, sizeof(debug::eventLabel.text),
                        "BVH level: %d", debug::bvhDepth);
                    debug::eventLabel.time = platform.time.now;
                    textParamsLeft.color = activeCol;
                }
                else {
                    textParamsLeft.color = defaultCol;
                }
                renderer::im::text2d(textParamsLeft,
                    "C to increase bvh depth level (+shift to decrease): %d", debug::bvhDepth);
                textParamsLeft.color = defaultCol;
                textParamsLeft.pos.y -= lineheight;
            }

            if (debug::debug3Dmode == debug::Debug3DView::Culling) {
                if (keyboard.pressed(input::CAPTURE_CAMERAS)) {
                    platform::format(
                        debug::eventLabel.text, sizeof(debug::eventLabel.text), "Capture cameras");
                    debug::eventLabel.time = platform.time.now;
                    textParamsLeft.color = activeCol;
                }
                else {
                    textParamsLeft.color = defaultCol;
                }
                renderer::im::text2d(textParamsLeft, "P to capture cameras in scene");
                textParamsLeft.color = defaultCol;
                textParamsLeft.pos.y -= lineheight;

                if (keyboard.pressed(input::TOGGLE_CAPTURED_CAMERA)) {
                    platform::format(
                        debug::eventLabel.text,sizeof(debug::eventLabel.text), "Camera #%d/#%d", debug::debugCameraStage + 1, debug::capturedCameras ? debug::capturedCameras[0].siblingIndex : 0);
                    debug::eventLabel.time = platform.time.now;
                    textParamsLeft.color = activeCol;
                } else {
                    textParamsLeft.color = defaultCol;
                }
                renderer::im::text2d(textParamsLeft, "C to toggle camera capture stages (+shift to go back): #%d/#%d", debug::debugCameraStage + 1, debug::capturedCameras ? debug::capturedCameras[0].siblingIndex : 0);
                textParamsLeft.color = defaultCol;
                textParamsLeft.pos.y -= lineheight;

                if (debug::capturedCameras) {
                    const u32 numCameras = debug::capturedCameras[0].siblingIndex;
                    const u32 minidx = debug::debugCameraStage > 10 ? debug::debugCameraStage - 10 : 0;
                    const u32 maxidx = debug::debugCameraStage + 10 < numCameras ? debug::debugCameraStage + 10 : numCameras;
                    for (u32 i = minidx; i < maxidx; i++) {
                        if (i == debug::debugCameraStage) { textParamsLeft.color = activeCol; }
                        renderer::im::text2d(textParamsLeft, "%*d: %s", debug::capturedCameras[i].depth, i, debug::capturedCameras[i].str);
                        textParamsLeft.color = defaultCol;
                        textParamsLeft.pos.y -= lineheight;
                    }
                }
               
                {
                    if (platform.input.keyboard.pressed(input::TOGGLE_FRUSTUM_CUT)) { textParamsLeft.color = activeCol; }
                    else { textParamsLeft.color = defaultCol; }
                    platform::StrBuilder frustumStr { appendBuff, sizeof(appendBuff) };
                    constexpr ::input::keyboard::Keys::Enum keys[] = {
                        input::TOGGLE_FRUSTUM_NEAR, input::TOGGLE_FRUSTUM_FAR,
                        input::TOGGLE_FRUSTUM_LEFT, input::TOGGLE_FRUSTUM_RIGHT,
                        input::TOGGLE_FRUSTUM_BOTTOM, input::TOGGLE_FRUSTUM_TOP };
                    platform::append(
                        frustumStr.curr, frustumStr.last,
                        "%sFrustum planes: ", debug::force_cut_frustum ? "[forced cut] " : "");
                    for (s32 i = 0; i < countof(debug::frustum_planes_off) && !frustumStr.full(); i++) {
                        if (!debug::frustum_planes_off[i]) {
                            platform::append(
                                frustumStr.curr,
                                frustumStr.last, "%s ", debug::frustum_planes_names[i]);
                        }
                        if (platform.input.keyboard.pressed(::input::keyboard::Keys::Enum(keys[i]))) {
                            textParamsLeft.color = activeCol;
                            platform::format(debug::eventLabel.text, sizeof(debug::eventLabel.text), debug::frustum_planes_off[i] ? "%s frustum plane off" : "%s frustum plane on", debug::frustum_planes_names[i]);
                            debug::eventLabel.time = platform.time.now;
                        }
                    };
                    renderer::im::text2d(textParamsLeft, "[0] to force frustum cut, [1-6] to toggle frustum planes");
                    textParamsLeft.pos.y -= lineheight;
                    renderer::im::text2d(textParamsLeft, frustumStr.str);
                    textParamsLeft.color = defaultCol;
                    textParamsLeft.pos.y -= lineheight;
                }
            }

            if (debug::overlaymode == debug::OverlayMode::All
                || debug::overlaymode == debug::OverlayMode::HelpOnly) {
                textParamsLeft.color = platform.input.mouse.down(::input::mouse::Keys::BUTTON_LEFT) ? activeCol : defaultCol;
                renderer::im::text2d(textParamsLeft, "Mouse (%.3f,%.3f)", platform.input.mouse.x, platform.input.mouse.y);
                textParamsLeft.pos.y -= lineheight;
                textParamsLeft.color = defaultCol;
                const float3 mousepos_WS = camera::screenPosToWorldPos(
                    platform.input.mouse.x, platform.input.mouse.y,
                    platform.screen.window_width, platform.screen.window_height,
                    game.resources.renderCore.perspProjection.config, game.scene.camera.viewMatrix);
                renderer::im::text2d(textParamsLeft, "Mouse 3D (%.3f,%.3f,%.3f)",mousepos_WS.x, mousepos_WS.y, mousepos_WS.z);
                textParamsLeft.pos.y -= lineheight;
                {
                    float3 eulers_deg = math::scale(game.scene.orbitCamera.eulers, math::r2d32);
                    renderer::im::text2d(textParamsLeft, "Camera eulers: " FLOAT3_FORMAT("% .3f"), FLOAT3_PARAMS(eulers_deg));
                    textParamsLeft.pos.y -= lineheight;
                }
                for (u32 i = 0; i < platform.input.padCount; i++)
                {
                    const ::input::gamepad::State& pad = platform.input.pads[i];
                    renderer::im::text2d(textParamsLeft, "Pad: %s", pad.name);
                    textParamsLeft.pos.y -= lineheight;
                    renderer::im::text2d(textParamsLeft
                        , "Pad: L:(%.3f,%.3f) R:(%.3f,%.3f) L2:%.3f R2:%.3f"
                        , pad.sliders[::input::gamepad::Sliders::AXIS_X_LEFT], pad.sliders[::input::gamepad::Sliders::AXIS_Y_LEFT]
                        , pad.sliders[::input::gamepad::Sliders::AXIS_X_RIGHT], pad.sliders[::input::gamepad::Sliders::AXIS_Y_RIGHT]
                        , pad.sliders[::input::gamepad::Sliders::TRIGGER_LEFT], pad.sliders[::input::gamepad::Sliders::TRIGGER_RIGHT]
                    );

                    {
                        platform::StrBuilder padStr{ appendBuff, sizeof(appendBuff) };
                        textParamsLeft.pos.y -= lineheight;
                        platform::append(padStr.curr, padStr.last, "Keys: ");
                        const char* key_names[] = {
                              "BUTTON_N", "BUTTON_S", "BUTTON_W", "BUTTON_E"
                            , "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT"
                            , "START", "SELECT"
                            , "L1", "R1", "L2", "R2"
                            , "LEFT_THUMB", "RIGHT_THUMB"
                        };
                        for (u32 key = 0; key < ::input::gamepad::KeyMask::COUNT && !padStr.full(); key++) {
                            if (pad.down((::input::gamepad::KeyMask::Enum)(1 << key))) {
                                platform::append(
                                    padStr.curr, padStr.last, "%s ", key_names[key]);
                            }
                        }
                        renderer::im::text2d(textParamsLeft, padStr.str);
                        textParamsLeft.pos.y -= lineheight;
                    }
                }

                textParamsRight.color = defaultCol;
                textParamsRight.pos.x -= 30.f * textscale;
                renderer::im::text2d(textParamsRight, "%s", platform::name);
                textParamsRight.pos.y -= lineheight;
                renderer::im::text2d(textParamsRight, "%.3lf fps", 1. / debug::frameAvg);
                textParamsRight.pos.y -= lineheight;
            }

            if (debug::overlaymode == debug::OverlayMode::All
             || debug::overlaymode == debug::OverlayMode::ArenaOnly)
            {
                auto renderArena = [](renderer::im::Text2DParams& textCfg, u8* arenaEnd,
                                        u8* arenaStart, uintptr_t arenaHighmark,
                                        const char* arenaName, const Color32 defaultCol,
                                        const Color32 baseCol, const Color32 highmarkCol,
                                        const f32 lineheight, const f32 textscale) {
                    const f32 barwidth = 150.f * textscale;
                    const f32 barheight = 10.f * textscale;

                    const ptrdiff_t arenaTotal = (ptrdiff_t)arenaEnd - (ptrdiff_t)arenaStart;
                    const ptrdiff_t arenaHighmarkBytes =
                        (ptrdiff_t)arenaHighmark - (ptrdiff_t)arenaStart;
                    const f32 occupancy = arenaHighmarkBytes / (f32)arenaTotal;
                    const f32 arenaHighmark_barwidth = barwidth * occupancy;
                    textCfg.color = highmarkCol;
                    if (arenaHighmarkBytes > 1024 * 1024) {
                        renderer::im::text2d(
                            textCfg, "%s highmark: %lu bytes (%.3fMB) / %.3f%%",
                            arenaName, arenaHighmarkBytes, arenaHighmarkBytes / (1024.f * 1024.f),
                            occupancy * 100.f);
                    } else if (arenaHighmarkBytes > 1024) {
                        renderer::im::text2d(
                            textCfg, "%s highmark: %lu bytes (%.3fKB) / %.3f%%",
                            arenaName, arenaHighmarkBytes, arenaHighmarkBytes / (1024.f),
                            occupancy * 100.f);
                    } else {
                        renderer::im::text2d(
                            textCfg, "%s highmark: %lu bytes / %.3f%%",
                            arenaName, arenaHighmarkBytes,
                            occupancy * 100.f);
                    }
                    textCfg.pos.y -= lineheight;
                    textCfg.color = defaultCol;

                    renderer::im::box_2d(
                        float2(textCfg.pos.x, textCfg.pos.y - barheight),
                        float2(textCfg.pos.x + barwidth, textCfg.pos.y), baseCol);
                    if (arenaHighmarkBytes) {
                        renderer::im::box_2d(
                            float2(textCfg.pos.x, textCfg.pos.y - barheight),
                            float2(textCfg.pos.x + arenaHighmark_barwidth, textCfg.pos.y),
                            highmarkCol);
                    }
                    textCfg.pos.y -= barheight + 5.f * textscale;
                };

                const f32 barwidth = 150.f * textscale;
                const f32 barheight = 10.f * textscale;
                textParamsCenter.pos.x -= barwidth / 2.f;
                {
                    const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                    const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                    renderArena(
                        textParamsCenter,
                        (u8*)math::max(
                            (uintptr_t)game.memory.frameArena.end, game.memory.frameArenaHighmark),
                        game.memory.frameArenaBuffer, game.memory.frameArenaHighmark,
                        "Frame arena", defaultCol, arenabaseCol, arenahighmarkCol,
                        lineheight, textscale);
                }
                {
                    const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                    const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                    renderArena(
                        textParamsCenter,
                        (u8*)math::max(
                            (uintptr_t)game.memory.scratchArenaRoot.end, game.memory.scratchArenaHighmark),
                        game.memory.scratchArenaRoot.curr, game.memory.scratchArenaHighmark,
                        "Scratch arena", defaultCol, arenabaseCol, arenahighmarkCol,
                        lineheight, textscale);
                }
                {
                    const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                    const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                    renderArena(
                        textParamsCenter, game.memory.persistentArena.end,
                        game.memory.persistentArenaBuffer,
                        (ptrdiff_t)game.memory.persistentArena.curr,
                        "Persistent arena", defaultCol, arenabaseCol, arenahighmarkCol,
                        lineheight, textscale);
                }
                {
                    const Color32 arenabaseCol(0.65f, 0.65f, 0.65f, 0.4f);
                    const Color32 arenahighmarkCol(0.95f, 0.35f, 0.8f, 1.f);
                    renderArena(
                        textParamsCenter, game.memory.sceneArena.end,
                        game.memory.sceneArenaBuffer,
                        (ptrdiff_t)game.memory.sceneArena.curr,
                        "Scene arena", defaultCol, arenabaseCol, arenahighmarkCol,
                        lineheight, textscale);
                }
                {
                    const Color32 baseCol(0.65f, 0.65f, 0.65f, 0.4f);
                    const Color32 used3dCol(0.95f, 0.35f, 0.8f, 1.f);
                    const Color32 used2dCol(0.35f, 0.95f, 0.8f, 1.f);
                    const Color32 used2didxCol(0.8f, 0.95f, 0.8f, 1.f);

                    const ptrdiff_t memory_size =
                        (ptrdiff_t)game.memory.debugArena.curr - (ptrdiff_t)debug::ctx.vertices_3d;
                    const ptrdiff_t vertices_3d_start =
                        (ptrdiff_t)debug::ctx.vertices_3d - (ptrdiff_t)debug::ctx.vertices_3d;
                    const ptrdiff_t vertices_3d_size =
                        (ptrdiff_t)debug::vertices_3d_head_last_frame * sizeof(im::Vertex3D);
                    const ptrdiff_t vertices_2d_start =
                        (ptrdiff_t)debug::ctx.vertices_2d - (ptrdiff_t)debug::ctx.vertices_3d;
                    const ptrdiff_t vertices_2d_size =
                        (ptrdiff_t)debug::vertices_2d_head_last_frame * sizeof(im::Vertex2D);
                    const ptrdiff_t indices_2d_start =
                        (ptrdiff_t)debug::ctx.indices_2d - (ptrdiff_t)debug::ctx.vertices_3d;
                    const ptrdiff_t indices_2d_size =
                        (ptrdiff_t)(debug::ctx.vertices_2d_head * 3 / 2) * sizeof(u32);
                    const f32 v3d_barstart = barwidth * vertices_3d_start / (f32)memory_size;
                    const f32 v3d_barwidth = barwidth * vertices_3d_size / (f32)memory_size;
                    const f32 v2d_barstart = barwidth * vertices_2d_start / (f32)memory_size;
                    const f32 v2d_barwidth = barwidth * vertices_2d_size / (f32)memory_size;
                    const f32 i2d_barstart = barwidth * indices_2d_start / (f32)memory_size;
                    const f32 i2d_barwidth = barwidth * indices_2d_size / (f32)memory_size;

                    textParamsCenter.color = used3dCol;
                    renderer::im::text2d(textParamsCenter, "im 3d: %lu bytes", vertices_3d_size);
                    textParamsCenter.pos.y -= lineheight;
                    textParamsCenter.color = used2dCol;
                    renderer::im::text2d(textParamsCenter, "im 2d: %lu bytes", vertices_2d_size);
                    textParamsCenter.pos.y -= lineheight;
                    textParamsCenter.color = used2didxCol;
                    renderer::im::text2d(textParamsCenter, "im 2d indices: %lu bytes", indices_2d_size);
                    textParamsCenter.pos.y -= lineheight;
                    textParamsCenter.color = defaultCol;

                    renderer::im::box_2d(
                        float2(textParamsCenter.pos.x, textParamsCenter.pos.y - barheight),
                        float2(textParamsCenter.pos.x + barwidth, textParamsCenter.pos.y),
                        baseCol);
                    renderer::im::box_2d(
                        float2(textParamsCenter.pos.x + v3d_barstart, textParamsCenter.pos.y - barheight),
                        float2(textParamsCenter.pos.x + v3d_barwidth, textParamsCenter.pos.y),
                        used3dCol);
                    renderer::im::box_2d(
                        float2(textParamsCenter.pos.x + v2d_barstart, textParamsCenter.pos.y - barheight),
                        float2(textParamsCenter.pos.x + v2d_barstart + v2d_barwidth, textParamsCenter.pos.y),
                        used2dCol);
                    renderer::im::box_2d(
                        float2(textParamsCenter.pos.x + i2d_barstart, textParamsCenter.pos.y - barheight),
                        float2(textParamsCenter.pos.x + i2d_barstart + i2d_barwidth, textParamsCenter.pos.y),
                        used2didxCol);

                    textParamsCenter.pos.y -= barheight + 5.f * textscale;

                }
            }

            // event label
            if (debug::eventLabel.time != 0.f) {
                f32 timeRunning = f32(platform.time.now - debug::eventLabel.time);
                f32 t = timeRunning / 2.f;
                if (t > 1.f) { debug::eventLabel.time = 0.f; }
                else {
                    u8 oldScale = textParamsCenter.scale;
                    textParamsCenter.color = Color32(1.0f, 0.2f, 0.1f, 1.f - t);
                    textParamsCenter.scale *= 2;
                    renderer::im::text2d(textParamsCenter, debug::eventLabel.text);
                    textParamsCenter.pos.y -= textParamsCenter.scale * 12.f;
                    textParamsCenter.color = defaultCol;
                    textParamsCenter.scale = oldScale;
                }
            }
        }
        #endif

        // copy backbuffer to window (upscale from game resolution to window resolution)
        {
            driver::Marker_t marker;
            driver::set_marker_name(marker, "UPSCALE TO WINDOW");
            renderer::driver::start_event(marker);
            {
                {
                    renderer::driver::ViewportParams vpParams;
                    vpParams.topLeftX = 0;
                    vpParams.topLeftY = 0;
                    vpParams.width = (f32)platform.screen.window_width;
                    vpParams.height = (f32)platform.screen.window_height;
                    renderer::driver::set_VP(vpParams);
                }

                renderer::driver::bind_blend_state(game.resources.renderCore.blendStateBlendOff);
                renderer::driver::bind_DS( game.resources.renderCore.depthStateOff);
                renderer::driver::bind_RS( game.resources.renderCore.rasterizerStateFillFrontfaces);
                renderer::driver::bind_main_RT(game.resources.renderCore.windowRT);

                driver::bind_shader(game.resources.renderCore.shaders[renderer::ShaderTechniques::FullscreenBlitTextured]);
                driver::bind_textures(&game.resources.renderCore.gameRT.textures[0], 1);
                driver::draw_fullscreen();
                renderer::driver::RscTexture nullTex = {};
                driver::bind_textures(&nullTex, 1); // unbind gameRT
            }
            renderer::driver::end_event();
        }
        // from now we should have
        // driver::bind_main_RT(game.resources.renderCore.windowRT);
        // UI
        {
            driver::bind_blend_state(game.resources.renderCore.blendStateOn);
            driver::Marker_t marker;
            driver::set_marker_name(marker, "UI");
            driver::start_event(marker);
            {
                renderer::CoreResources& rsc = game.resources.renderCore;
                driver::RscCBuffer& scene_cbuffer =
                    rsc.cbuffers[renderer::CoreResources::CBuffersMeta::Scene];
                driver::RscCBuffer& uitext_cbuffer =
                    rsc.cbuffers[renderer::CoreResources::CBuffersMeta::UIText];

                renderer::SceneData cbufferPerScene;
                cbufferPerScene.vpMatrix = rsc.windowProjection.matrix;
                renderer::driver::update_cbuffer(scene_cbuffer, &cbufferPerScene);

                renderer::driver::bind_RS(game.resources.renderCore.rasterizerStateFillFrontfaces);
                renderer::driver::bind_DS(game.resources.renderCore.depthStateOff);

                driver::bind_shader(rsc.shaders[renderer::ShaderTechniques::Color2D]);
                driver::RscCBuffer cbuffers[] = { scene_cbuffer, uitext_cbuffer };
                driver::bind_cbuffers(rsc.shaders[renderer::ShaderTechniques::Color2D], cbuffers, 2);
                
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor = float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    renderer::driver::update_cbuffer(uitext_cbuffer, &noteUItext);

                    driver::bind_indexed_vertex_buffer(game.resources.gpuBufferHeaderText);
                    driver::draw_indexed_vertex_buffer(game.resources.gpuBufferHeaderText);
                }

                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor =
                        platform.input.keyboard.pressed(input::CYCLE_ROOM) ?
                            float4(1.f, 0.f, 0.f, 1.f)
                          : float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    renderer::driver::update_cbuffer(uitext_cbuffer, &noteUItext);

                    driver::bind_indexed_vertex_buffer(game.resources.gpuBufferCycleRoomText);
                    driver::draw_indexed_vertex_buffer(game.resources.gpuBufferCycleRoomText);
                }
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor =
                        (platform.input.mouse.scrolldy != 0) ?
                            float4(1.f, 0.f, 0.f, 1.f)
                          : float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    renderer::driver::update_cbuffer(uitext_cbuffer, &noteUItext);

                    driver::bind_indexed_vertex_buffer(game.resources.gpuBufferScaleText);
                    driver::draw_indexed_vertex_buffer(game.resources.gpuBufferScaleText);
                }
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor =
                        platform.input.mouse.down(::input::mouse::Keys::BUTTON_RIGHT) ?
                            float4(1.f, 0.f, 0.f, 1.f)
                          : float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    renderer::driver::update_cbuffer(uitext_cbuffer, &noteUItext);

                    driver::bind_indexed_vertex_buffer(game.resources.gpuBufferPanText);
                    driver::draw_indexed_vertex_buffer(game.resources.gpuBufferPanText);
                }
                {
                    renderer::NodeData noteUItext = {};
                    noteUItext.groupColor =
                        platform.input.mouse.down(::input::mouse::Keys::BUTTON_LEFT) ?
                            float4(1.f, 0.f, 0.f, 1.f)
                          : float4(1.f, 1.f, 1.f, 1.f);
                    math::identity4x4(*(Transform*)&noteUItext.worldMatrix);
                    renderer::driver::update_cbuffer(uitext_cbuffer, &noteUItext);

                    driver::bind_indexed_vertex_buffer(game.resources.gpuBufferOrbitText);
                    driver::draw_indexed_vertex_buffer(game.resources.gpuBufferOrbitText);
                }
            }
            driver::end_event();
        }
        // Batched 2d debug (clear cpu buffers onto the screen)
        #if __DEBUG
        {
            driver::bind_blend_state(game.resources.renderCore.blendStateOn);
            im::commit2d();
            im::present2d(game.resources.renderCore.windowProjection.matrix);
        }
        #endif

    }
}
}

#endif // __WASTELADNS_GAME_H__
