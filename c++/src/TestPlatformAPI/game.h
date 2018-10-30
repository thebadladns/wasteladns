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
        if (controlChange.x != 0.f && currentControl.x != controlChange.x) {
            localInput.x = controlChange.x;
        } else if (pad.down(Input::RIGHT)) {
            localInput.x = 1.f;
        } else if (pad.down(Input::LEFT)) {
            localInput.x = -1.f;
        } else {
            localInput.x = 0.f;
        }
        if (controlChange.y != 0.f && currentControl.y != controlChange.y) {
            localInput.y = controlChange.y;
        } else if (pad.down(Input::UP)) {
            localInput.y = 1.f;
        } else if (pad.down(Input::DOWN)) {
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
    
    struct Player
    {
        WorldData worldData;
        Control control;
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
            frustum.fov = 75.0;
            frustum.aspect = 1.0;
            frustum.near = 1.0;
            frustum.far = 1500.0;
            generateMatrix(mgr.perspProjection.matrix, frustum);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_CULL_FACE);
        }
        
        game.cameraMgr = {};
        {
            CameraManager& mgr = game.cameraMgr;
            using namespace CameraSystem;
            
            Camera& cam = mgr.cameras[CameraManager::FlyCam];
            Math::identity4x4(cam.transform);
            cam.transform.pos = Vec3(100.f, -115.f, 210.f);
            Vec3 lookAt(0.f, 0.f, 0.f);
            Vec3 lookAtDir = Math::subtract(lookAt, cam.transform.pos);
            Math::fromFront(cam.transform, lookAtDir);
            generateModelViewMatrix(cam.modelviewMatrix, cam.transform);
            
            mgr.activeCam = &cam;
        }
        
        game.player = {};
        {
            Math::identity4x4(game.player.worldData.transform);
            game.player.worldData.transform.pos = Vec3(0.f, 0.f, 0.f);
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
        }
            
        // Render update
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        {
            Renderer::Instance& mgr = game.renderMgr;
            using namespace Renderer;
            
            // PERSPECTIVE
            {
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixd(mgr.perspProjection.matrix.dataCM);
                
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                {
                    glLoadMatrixf(game.cameraMgr.activeCam->modelviewMatrix.dataCM);
                    
                    f32 pw = 5.f;
                    f32 pz = 500.f;
                    Vec3 pillarQuads[] = {
                          { -pw, -pw, 0.f }
                        , { pw, -pw, 0.f}
                        , { pw, -pw, pz}
                        , { -pw, -pw, pz}
                        
                        , { pw, -pw, 0.f }
                        , { pw, pw, 0.f }
                        , { pw, pw, pz }
                        , { pw, -pw, pz }
                        
                        , { pw, pw, 0.f }
                        , { -pw, pw, 0.f }
                        , { -pw, pw, pz }
                        , { pw, pw, pz }
                        
                        , { -pw, pw, 0.f }
                        , { -pw, -pw, 0.f }
                        , { -pw, -pw, pz }
                        , { -pw, pw, pz }
                    };
                    const u32 pillarQuadCount = sizeof(pillarQuads) / sizeof(pillarQuads[0]);
                    
                    Vec2 pillarPos[] = {
                          { -80.f, -20.f }
                        , { 80.f, -20.f }
                        , { -160.f, -20.f }
                        , { 160.f, -20.f }
                        , { -240.f, -20.f }
                        , { 240.f, -20.f }
                        , { -300.f, -20.f }
                        , { 300.f, -20.f }
                        , { -80.f, -80.f }
                        , { 80.f, -80.f }
                        , { -160.f, -80.f }
                        , { 160.f, -80.f }
                        , { -240.f, -80.f }
                        , { 240.f, -80.f }
                        , { -300.f, -80.f }
                        , { 300.f, -80.f }
                        , { -20.f, 180.f }
                        , { 20.f, 180.f }
                        , { -100.f, 180.f }
                        , { 100.f, 180.f }
                        , { -200.f, 180.f }
                        , { 200.f, 180.f }
                        , { -300.f, 180.f }
                        , { 300.f, 180.f }
                    };
                    
                    glEnable(GL_CULL_FACE);
                    glEnableClientState(GL_VERTEX_ARRAY);
                    {
                        const Col pillarColor(1.0f, 1.0f, 1.0f, 0.5f);
                        glColor4f(RGBA_PARAMS(pillarColor));
                        glVertexPointer(3, GL_FLOAT, 0, pillarQuads);
                        
                        for (Vec2& pos : pillarPos) {
                            glPushMatrix();
                            glTranslatef(pos.x, pos.y, 0.f);
                            glDrawArrays(GL_QUADS, 0, pillarQuadCount);
                            glPopMatrix();
                        }
                    }
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glDisable(GL_CULL_FACE);
                    
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
                        const f32 axisSize = 300;
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
                        textParams.text = "Text\ntest";
                        textParams.color = Col(1.0f, 1.0f, 0.0f, 1.0f);
                        Renderer::Immediate::text2d(game.renderMgr.immediateBuffer, textParams);
                        
                        Renderer::Immediate::sphere(game.renderMgr.immediateBuffer, Vec3(-170.f, 40.f, 0.f), 100.f, Col(1.0f, 1.0f, 1.0f, 0.4f));
                    }
                    
                    // Player
                    glPushMatrix();
                    {
                        f32 w = 4.f;
                        f32 wf = w;
                        f32 wb = w;
                        f32 h = 5.f;
                        f32 z = 30.f;
                        f32 yoff = -2.f;
                        f32 zoff = 0.f;
                        // pyramid sides
                        const Vec3 playerTriangles[] = {
                              { 0.f, yoff, -zoff}, { -wb, -h, z}, { -wf, h, z}
                            , { 0.f, yoff, -zoff}, { -wf, h, z}, { wf, h, z}
                            , { 0.f, yoff, -zoff}, { wf, h, z}, { wb, -h, z}
                            , { 0.f, yoff, -zoff}, { wb, -h, z}, { -wb, -h, z}
                        };
                        // pyramid top
                        const Vec3 playerQuads[] = {
                            { -wb, -h, z}, { wb, -h, z}, { wf, h, z}, { -wf, h, z}
                        };
                        const u32 playerTriangleCount = sizeof(playerTriangles) / sizeof(playerTriangles[0]);
                        const u32 playerQuadCount = sizeof(playerQuads) / sizeof(playerQuads[0]);
                        const Col playerOutlineColor(1.0f, 1.0f, 1.0f, 1.0f);
                        const Col playerSolidColor(0.0f, 0.0f, 0.0f, 1.0f);

                        glMultMatrixf(game.player.worldData.transform.matrix.dataCM);
                        
                        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                        glEnableClientState(GL_VERTEX_ARRAY);
                        {
                            glColor4f(RGBA_PARAMS(playerOutlineColor));
                            glVertexPointer(3, GL_FLOAT, 0, playerTriangles);
                            glDrawArrays(GL_TRIANGLES, 0, playerTriangleCount);
                            glVertexPointer(3, GL_FLOAT, 0, playerQuads);
                            glDrawArrays(GL_QUADS, 0, playerQuadCount);
                        }
                        glDisableClientState(GL_VERTEX_ARRAY);
                        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                    }
                    glPopMatrix();
                }
                glPopMatrix();
            } // perspective
            
            // Batched debug
            {
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixd(mgr.perspProjection.matrix.dataCM);
                Immediate::present3d(mgr.immediateBuffer, *game.cameraMgr.activeCam);
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf(mgr.orthoProjection.matrix.dataCM);
                Immediate::present2d(mgr.immediateBuffer);
                Immediate::clear(mgr.immediateBuffer);
            }
        }
    }
}

#endif // __WASTELADNS_GAME_H__
