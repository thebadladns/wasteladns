#include "GLFW/glfw3.h"
#define __WASTELADNS_GLFW3_H__

// C libs
#include <stdlib.h>
#define __WASTELADNS_C_STDLIB_H__
#include <math.h>
#define __WASTELADNS_C_MATH_H__
#include <stdio.h>
#define __WASTELADNS_C_STDIO_H__
#include <limits>
#define __WASTELADNS_C_LIMITS_H__
#include <cstring>
#define __WASTELADNS_C_CSTRING_H__

// Core
#include "helpers/Types.h"
#define __WASTELADNS_MATH_IMPL__
#include "helpers/Math.h"
#define __WASTELADNS_ANGLE_IMPL__
#include "helpers/Angle.h"
#define __WASTELADNS_VEC_IMPL__
#include "helpers/Vec.h"
#define __WASTELADNS_COLOR_IMPL__
#include "helpers/Color.h"

// Rest
#define __WASTELADNS_DEBUGDRAW_IMPL__
#define __WASTELADNS_DEBUGDRAW_TEXT__
#include "helpers/debugdraw.h"

namespace Resources {
    
    struct Texture {
        u32 handle;
        u32 width;
        u32 height;
    };
    
    enum TextureId : u8 {
        Dot,
        TextureMaxCount,
    };
    
    struct Manager {
        Texture textures[TextureMaxCount];
    };
    
    Texture& get(Manager& manager, const TextureId id) {
        return manager.textures[id];
    }
    
    Texture& load(Manager& manager, const TextureId id, const u8* data, const u8 w, const u8 h) {
        
        Texture& texture = manager.textures[id];
        
        glGenTextures(1, &texture.handle);
        glBindTexture(GL_TEXTURE_2D, texture.handle);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
        
        return texture;
    };
}

namespace Input {
    
    namespace DebugSet {
        enum Keys : u8 {
            ESC = 1 << 0
            , EXIT = ESC
            , COUNT = 2
            , CLEAR = 0
        };
        typedef s32 Mapping[Keys::COUNT];
        extern const Mapping mapping = {
            GLFW_KEY_ESCAPE
        };
    };
    
    template<typename _Set, const s32* _mapping>
    struct DigitalState {
        bool down(_Set key) const {
            return (current & key) != 0;
        }
        bool up(_Set key) const {
            return (current & key) == 0;
        }
        bool released(_Set key) const {
            return (current & key) == 0 && (last & key) != 0;
        }
        bool pressed(_Set key) const {
            return (current & key) != 0 && (last & key) == 0;
        }
        void pollState(GLFWwindow* window) {
            last = current;
            current = _Set::CLEAR;
            for (int i = 0; i < _Set::COUNT; i++) {
                s32 keyState = glfwGetKey(window, _mapping[i]);
                s32 keyOn = (keyState == GLFW_PRESS) || (keyState == GLFW_REPEAT);
                current = (_Set) (current | (keyOn << i));
            }
        }
        
        _Set last;
        _Set current;
    };
    
    struct KeyboardState {
        bool down(s32 key) {
            return current[key] != 0;
        }
        bool up(s32 key) {
            return current[key] == 0;
        }
        bool released(s32 key) {
            return current[key] == 0 && last[key] != 0;
        }
        bool pressed(s32 key) {
            return current[key] != 0 && last[key] == 0;
        }
        void pollState(GLFWwindow* window) {
            memcpy(last, current, GLFW_KEY_LAST);
            memset(current, 0, GLFW_KEY_LAST);
            for (int i = 0; i < GLFW_KEY_LAST; i++) {
                current[i] = glfwGetKey(window, i);
            }
        }
        
        u8 last[GLFW_KEY_LAST];
        u8 current[GLFW_KEY_LAST];
    };
};

namespace Math3D {
    typedef f32 Transform32[16];
    typedef f64 Transform64[16];
}

namespace Camera {
    
    // frustum.fov = 60.0;
    // frustum.aspect = 1.0;
    // frustum.near = 1.0;
    // frustum.far = 200.0;
    struct FrustumParams {
        f32 fov;
        f32 aspect;
        f32 near;
        f32 far;
    };
    
    void computeProjectionMatrix(const FrustumParams& params, Math3D::Transform64& matrixCM) {
        const f64 xMax = params.near * tan(params.fov * 2.f * Angle<f64>::d2r);
        const f64 xMin = -xMax;
        const f64 yMin = xMin / params.aspect;
        const f64 yMax = xMax / params.aspect;
        
        memset(matrixCM, 0, sizeof(matrixCM));
        matrixCM[0] = -(2.0 * params.near) / (xMax - xMin);
        matrixCM[5] = -(2.0 * params.near) / (yMax - yMin);
        matrixCM[10] = -(params.far + params.near) / (params.far - params.near);
        matrixCM[8] = -(xMax + xMin) / (xMax - xMin);
        matrixCM[9] = -(yMax + yMin) / (yMax - yMin);
        matrixCM[11] = -1.f;
        matrixCM[14] = -(2.0 * params.far * params.near) / (params.far - params.near);
    }
    
    // ortho.right = app.mainWindow.width * 0.5f;
    // ortho.top = app.mainWindow.height * 0.5f;
    // ortho.left = -ortho.right;
    // ortho.bottom = -ortho.top;
    // ortho.near = -1.f;
    // ortho.far = 200.f;
    struct OrthoParams {
        f32 left;
        f32 right;
        f32 top;
        f32 bottom;
        f32 near;
        f32 far;
    };
    
    void computeProjectionMatrix(const OrthoParams& params, Math3D::Transform32& matrixCM) {
        memset(matrixCM, 0, sizeof(matrixCM));
        matrixCM[0] = 2.f / (params.right - params.left);
        matrixCM[5] = 2.f / (params.top - params.bottom);
        matrixCM[10] = -2.f / (params.far - params.near);
        matrixCM[12] = -(params.right + params.left) / (params.right - params.left);
        matrixCM[13] = -(params.top + params.bottom) / (params.top - params.bottom);
        matrixCM[14] = -(params.far + params.near) / (params.far - params.near);
        matrixCM[15] = 1.f;
    }
};

namespace App {
    struct Time {
        f64 running;
        f64 start;
        f64 now;
    };
    
    struct Window {
        GLFWwindow* handle;
        f32 desiredRatio;
        u32 width;
        u32 height;
        bool fullscreen = false;
    };
    
    struct Input {
        ::Input::KeyboardState keyboardSet;
    };
    
    struct Instance {
        Window mainWindow;
        Time time;
        Input input;
    };
};
App::Instance app;

namespace Game {
    
    struct Time {
        struct Config {
            f64 targetFramerate;
            f64 maxFrameLength;
        };
        
        Config config;
        
        f64 lastFrame;
        f64 lastFrameDelta;
        f64 nextFrame;
        
        s64 frame;
    };
    
    struct View {
        Camera::OrthoParams orthoParams;
        Camera::FrustumParams frustumParams;
        Math3D::Transform32 orthoTransformCM;
        Math3D::Transform64 frustumTransformCM;
        bool perspective;
    };
    
    namespace PlayerInputSet {
        enum Keys : u8 {
              LEFT = 1 << 0
            , RIGHT = 1 << 1
            , SPACE = 1 << 2
            , COUNT = 3
            , CLEAR = 0
        };
        typedef s32 Mapping[Keys::COUNT];
        extern const Mapping mapping = {
              GLFW_KEY_A
            , GLFW_KEY_D
            , GLFW_KEY_SPACE
        };
    };
    typedef ::Input::DigitalState<PlayerInputSet::Keys, PlayerInputSet::mapping> PlayerInput;
 
    namespace Cage {
        
        struct Bounds {
            f32 left;
            f32 right;
            f32 top;
            f32 bottom;
        };
        
        f32 intersectH(const Vec2& pos, const Vec2& dir, const f32 y) {
            if (Math<f32>::abs(dir.y) > Math<f32>::eps) {
                return (y - pos.y) / dir.y;
            } else {
                return std::numeric_limits<f32>::max();
            }
        }
        
        f32 intersectV(const Vec2& pos, const Vec2& dir, const f32 x) {
            if (Math<f32>::abs(dir.x) > Math<f32>::eps) {
                return (x - pos.x) / dir.x;
            } else {
                return std::numeric_limits<f32>::max();
            }
        }
        
        struct IntersectParams {
            const Vec2* pos;
            const Vec2* dir;
            const Bounds* cage;
            Vec2 bounceDir; // out
            f32 radius;
            f32 collisionT; // inout
            bool collision; // out
        };
        void intersect(IntersectParams& params) {
            
            params.collision = false;
            
            f32 leftT = intersectV(*params.pos, *params.dir, params.cage->left + params.radius);
            if (leftT > 0.f && leftT < params.collisionT) {
                params.bounceDir = Vec::normalize(*params.dir);
                params.bounceDir.x = -params.bounceDir.x;
                params.collisionT = leftT;
                params.collision = true;
            }
            f32 rightT = intersectV(*params.pos, *params.dir, params.cage->right - params.radius);
            if (rightT > 0.f && rightT < params.collisionT) {
                params.bounceDir = Vec::normalize(*params.dir);
                params.bounceDir.x = -params.bounceDir.x;
                params.collisionT = rightT;
                params.collision = true;
            }
            f32 topT = intersectH(*params.pos, *params.dir, params.cage->top - params.radius);
            if (topT > 0.f && topT < params.collisionT) {
                params.bounceDir = Vec::normalize(*params.dir);
                params.bounceDir.y = -params.bounceDir.y;
                params.collisionT = topT;
                params.collision = true;
            }
            f32 bottomT = intersectH(*params.pos, *params.dir, params.cage->bottom + params.radius);
            if (bottomT > 0.f && bottomT < params.collisionT) {
                params.bounceDir = Vec::normalize(*params.dir);
                params.bounceDir.y = -params.bounceDir.y;
                params.collisionT = bottomT;
                params.collision = true;
            }
        }
    }
    
    struct Player {
        
        struct Config {
            f32 maxImpulse;
            f32 bounceLoss;
            f32 friction;
            f32 radius;
        };
        
        Config config;
        Vec2 pos;
        f32 speed;
        f32 accumulatedImpulse;
        f32 heading;
    };
    
    struct Trajectory {
      
        struct Config {
            f32 baseLength;
            f32 maxLength;
            f32 baseDotStride;
            f32 minDotStride;
            f32 initialOffset;
            f32 offsetSpeed;
        };
        
        Config config;
        f32 offset;
        f32 dotStride;
    };
    
    struct Instance {
        Resources::Manager resourceManager;
        
        Time time;
        
        View view;
        
        PlayerInput playerInput;
        Cage::Bounds cage;
        Player player;
        Trajectory trajectory;
    };
}
Game::Instance game;

int main(int argc, char** argv) {
    
    if (glfwInit())
    {
        App::Window& window = app.mainWindow;
        
        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
        window.width = 640;
        window.height = 480;
        window.desiredRatio = window.width / (f32) window.height;
        window.fullscreen = false;
        window.handle = glfwCreateWindow(
                                         window.width
                                         , window.height
                                         , "smooth motion test"
                                         , nullptr /*monitor*/
                                         , nullptr /*share*/
                                         );
        if (window.handle)
        {
            // Input setup
            glfwSetInputMode(app.mainWindow.handle, GLFW_STICKY_KEYS, 1);
            
            // Render setup
            glfwMakeContextCurrent(window.handle);
            glfwSwapInterval(1);
            glMatrixMode(GL_PROJECTION);
            {
                glLoadIdentity();
                Math3D::Transform32& projectionTransformCM = game.view.orthoTransformCM;
                Camera::OrthoParams& ortho = game.view.orthoParams;
                ortho.right = app.mainWindow.width * 0.5f;
                ortho.top = app.mainWindow.height * 0.5f;
                ortho.left = -ortho.right;
                ortho.bottom = -ortho.top;
                ortho.near = -1.f;
                ortho.far = 200.f;
                Camera::computeProjectionMatrix(ortho, projectionTransformCM);
                glMultMatrixf(projectionTransformCM);

                Math3D::Transform64& frustumTransformCM = game.view.frustumTransformCM;
                Camera::FrustumParams& frustum = game.view.frustumParams;
                frustum.fov = 60.0;
                frustum.aspect = 1.0;
                frustum.near = 1.0;
                frustum.far = 200.0;
                Camera::computeProjectionMatrix(frustum, frustumTransformCM);
                
                game.view.perspective = false;
            }
            
            // Logic setup
            {
                game.cage.left = -80.f;
                game.cage.right = 80.f;
                game.cage.top = 80.f;
                game.cage.bottom = -80.f;
                
                game.player.pos = Vec2(0.f, 0.f);
                game.player.speed = 0.f;
                game.player.accumulatedImpulse = 0.f;
                game.player.config.maxImpulse = 600.f;
                game.player.config.friction = 400.f;
                game.player.config.bounceLoss = 0.8f;
                game.player.config.radius = 11.f;
                
                game.trajectory.config.baseLength = 80.f;
                game.trajectory.config.maxLength = 320.f;
                game.trajectory.config.baseDotStride = 8.f;
                game.trajectory.config.minDotStride = 6.f;
                game.trajectory.config.initialOffset = game.player.config.radius + 2.f;
                game.trajectory.config.offsetSpeed = 20.f;
                game.trajectory.offset = 0.f;
                game.trajectory.dotStride = game.trajectory.config.baseDotStride;
                
                const u8 dotTextureSize = 8;
                const unsigned char dotTexture[ 8 * 8 ] = {
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
                    0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
                    0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
                    0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };
                Resources::load(game.resourceManager, Resources::TextureId::Dot, dotTexture, dotTextureSize, dotTextureSize);
            }
            
            game.time.config.maxFrameLength = 0.1;
            game.time.config.targetFramerate = 1.0 / 60.0;
            app.time.now = app.time.start = glfwGetTime();
            game.time.lastFrame = game.time.nextFrame = app.time.now;
            game.time.frame = 0;
            do {
                if (app.time.now >= game.time.nextFrame) {
                    game.time.lastFrameDelta = Math<f64>::min(app.time.now - game.time.lastFrame, game.time.config.maxFrameLength);
                    game.time.lastFrame = app.time.now;
                    game.time.nextFrame = app.time.now + game.time.config.targetFramerate;
                    
                    {
                        f32 timeDelta = (f32) game.time.config.targetFramerate;
                        
                        using namespace Game;
                        
                        glfwPollEvents();
                        app.input.keyboardSet.pollState(window.handle);
                        game.playerInput.pollState(window.handle);
                        
                        // Logic
                        {
                            if (app.input.keyboardSet.released(GLFW_KEY_ESCAPE)) {
                                glfwSetWindowShouldClose(app.mainWindow.handle, 1);
                            }
                            if (app.input.keyboardSet.pressed(GLFW_KEY_TAB)) {
                                game.view.perspective = !game.view.perspective;
                            }
                            
                            Game::Player& player = game.player;
                            
                            if (player.speed < Math<f32>::eps) {
                                if (game.playerInput.down(Game::PlayerInputSet::LEFT)) {
                                   player.heading -= 0.035f;
                                }
                                if (game.playerInput.down(Game::PlayerInputSet::RIGHT)) {
                                   player.heading += 0.035f;
                                }
                                if (game.playerInput.down(Game::PlayerInputSet::SPACE)) {
                                    game.player.accumulatedImpulse += 600.f * timeDelta;
                                    game.player.accumulatedImpulse = Math<f32>::min(game.player.accumulatedImpulse, game.player.config.maxImpulse);
                                } else if (game.playerInput.released(Game::PlayerInputSet::SPACE)) {
                                    game.player.speed = game.player.accumulatedImpulse;
                                    game.player.accumulatedImpulse = 0.f;
                                }
                                
                                game.trajectory.dotStride = Math<f32>::max(game.trajectory.config.baseDotStride / Math<f32>::max(game.player.accumulatedImpulse / 400.f, 1.f), game.trajectory.config.minDotStride);
                                if (game.player.accumulatedImpulse > Math<f32>::eps) {
                                    game.trajectory.offset = fmod(game.trajectory.offset + game.trajectory.config.offsetSpeed * timeDelta, game.trajectory.dotStride);
                                }
                            }

                            player.speed = Math<f32>::max(player.speed - player.config.friction * timeDelta, 0.f);
                            
                            if (player.speed > Math<f32>::eps) {
                                Vec2 dir = Angle<f32>::direction(player.heading);
                                Vec2 vel = Vec::scale(dir, player.speed * timeDelta);
                                
                                f32 currentT = 0.f;
                                
                                Cage::IntersectParams params;
                                params.cage = &game.cage;
                                params.radius = player.config.radius;
                                do {
                                    vel = Vec::scale(dir, player.speed * timeDelta * (1.f - currentT));
                                    
                                    params.pos = &player.pos;
                                    params.dir = &vel;
                                    params.collisionT = 1.f;
                                    Cage::intersect(params);
                                    
                                    player.pos = Vec::add(*params.pos, Vec::scale(vel, params.collisionT));
                                    
                                    if (params.collision) {
                                        player.speed = Math<f32>::max(player.speed * player.config.bounceLoss, 0.f);
                                        currentT += params.collisionT;
                                        dir = params.bounceDir;
                                        player.pos = Vec::add(player.pos, Vec::scale(dir, 0.1f));
                                    }
                                    
                                } while(params.collision);
                                
                                player.heading = Angle<f32>::heading(vel);
                                
                            }
                        }
                        
                        { // Render update
                            glClearColor(0.f, 0.f, 0.f, 1.f);
                            glClear(GL_COLOR_BUFFER_BIT);
                            
                            // Debug
                            {
                                glMatrixMode(GL_PROJECTION);
                                glLoadIdentity();
                                glMultMatrixf(game.view.orthoTransformCM);
                                
                                Vec3 debugPos = Vec3(game.view.orthoParams.left + 10.f, game.view.orthoParams.top - 10.f, -50);
                                const Col textColor(1.0f, 1.0f, 1.0f, 1.0f);
                                
                                DebugDraw::TextParams textParams;
                                textParams.scale = 2.f;
                                char buffer[128];
                                
                                snprintf(buffer, sizeof(buffer), "A & D to turn, SPACE to fire, TAB toggle perp %s", game.view.perspective ? "OFF" : "ON");
                                textParams.pos = debugPos;
                                textParams.text = buffer;
                                DebugDraw::text(textParams);
                                debugPos.y -= 15.f * textParams.scale;
                             
                                snprintf(buffer, sizeof(buffer), "player impulse:%.3f speed:%.3f", game.player.accumulatedImpulse, game.player.speed);
                                textParams.pos = debugPos;
                                textParams.text = buffer;
                                DebugDraw::text(textParams);
                            }
                            
                            // Scene
                            {
                                glMatrixMode(GL_MODELVIEW);
                                glPushMatrix();
                                
                                if (game.view.perspective) {
                                    glMatrixMode(GL_PROJECTION);
                                    glLoadIdentity();
                                    glMultMatrixd(game.view.frustumTransformCM);
                                    
                                    glMatrixMode(GL_MODELVIEW);
                                    glTranslatef(0.f, 40.f, 0.f);
                                    glRotatef(15.f, -1.f, 0.f, 0.f);
                                }
                                    
                                f32 z = -150.f;
                                
                                // Player
                                glMatrixMode(GL_MODELVIEW);
                                glPushMatrix();
                                {
                                    Game::Player& player = game.player;
    
                                    glTranslatef(player.pos.x, player.pos.y, 0.f);
                                    glRotatef(player.heading * Angle<f32>::r2d, 0.f, 0.f, -1.f);
                                    
                                    f32 w = 5.f;
                                    f32 h = 2.5f;
                                    Vec2 renderPos(0.f, 5.f);
                                    f32 x = renderPos.x;
                                    f32 y = renderPos.y;
                                    
                                    const Col playerColor(1.0f, 1.0f, 1.0f, 1.0f);
                                    DebugDraw::circle(Vec3(0.f, 0.f, z), Vec3(0.f, 0.f, -1.f), game.player.config.radius, playerColor);
                                    DebugDraw::segment(Vec3(x - w, y - h, z), Vec3(x, y + h, z), playerColor);
                                    DebugDraw::segment(Vec3(x, y + h, z), Vec3(x + w, y - h, z), playerColor);
                                    DebugDraw::segment(Vec3(x + w, y - h, z), Vec3(x - w, y - h, z), playerColor);
                                }
                                glMatrixMode(GL_MODELVIEW);
                                glPopMatrix();
                                
                                // Trajectory
                                if ( game.player.speed < Math<f32>::eps )
                                {
                                    Game::Trajectory& trajectory = game.trajectory;
                                    Resources::Texture& texture = Resources::get(game.resourceManager, Resources::TextureId::Dot);
                                    
                                    glEnable(GL_BLEND);
                                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                                    glDepthMask(GL_FALSE);
                                    glEnable(GL_TEXTURE_2D);
                                    glBindTexture(GL_TEXTURE_2D, texture.handle);
                                    
                                    glBegin(GL_QUADS);
                                    const Col color(0.88f, 0.83f, 0.73f, 1.0f);
                                    glColor4f(RGBA_PARAMS(color));
                                    
                                    Vec2 pos = game.player.pos;
                                    Vec2 dir = Angle<f32>::direction(game.player.heading);
                                    Vec2 renderPos = Vec::add(pos, Vec::scale(dir, game.player.config.radius));
                                    
                                    // Reduce stride when impulse is high
                                    const f32 trajectoryLength = Math<f32>::min(trajectory.config.baseLength * Math<f32>::max(game.player.accumulatedImpulse / 300.f , 1.f), trajectory.config.maxLength);
                                    
                                    Vec2 vel = Vec::scale(dir, trajectoryLength);
                                    f32 offset = trajectory.offset + trajectory.config.initialOffset;
                                    
                                    Cage::IntersectParams params;
                                    params.cage = &game.cage;
                                    params.radius = game.player.config.radius;
                                    do {
                                        params.pos = &pos;
                                        params.dir = &vel;
                                        params.collisionT = 1.f;
                                        Cage::intersect(params);
                                        
                                        Vec2 nextPos = Vec::add(*params.pos, Vec::scale(vel, params.collisionT));
                                        Vec2 resultDir = Vec::subtract(nextPos, pos);
                                        f32 resultLength = Vec::mag(resultDir);
                                        resultDir = Vec::invScale(resultDir, resultLength);
                                        f32 currentDistance = offset;
                                        while (currentDistance < resultLength) {
                                            const f32 dotSize = 2.f;
                                            const Vec2 dotPos = Vec::add(pos, Vec::scale(resultDir, currentDistance));
                                            
                                            glTexCoord2f(0.0, 1.0); glVertex3f(dotPos.x - dotSize,dotPos.y - dotSize, z);
                                            glTexCoord2f(1.0, 1.0); glVertex3f(dotPos.x + dotSize,dotPos.y - dotSize, z);
                                            glTexCoord2f(1.0, 0.0); glVertex3f(dotPos.x + dotSize,dotPos.y + dotSize, z);
                                            glTexCoord2f(0.0, 0.0); glVertex3f(dotPos.x - dotSize,dotPos.y + dotSize, z);
                                            
                                            currentDistance += trajectory.dotStride;
                                        }
                                        
                                        if (params.collision) {
                                            vel = Vec::scale(params.bounceDir, Vec::mag(vel) * (1.f - params.collisionT));
                                            pos = Vec::add(nextPos, Vec::scale(Vec::normalize(vel), 0.1f));
                                            renderPos = pos;
                                            offset = currentDistance - resultLength;
                                        }
                                        
                                    } while(params.collision);
                                    
                                    
                                    glEnd();
                                    glBindTexture(GL_TEXTURE_2D, 0);
                                    glDisable(GL_TEXTURE_2D);
                                    glDisable(GL_BLEND);
                                    glDepthMask(GL_TRUE);
                                }
                                
                                // Cage
                                
                                Game::Cage::Bounds& cage = game.cage;
                                
                                const Col gridColor(1.0f, 1.0f, 1.0f, 1.0f);
                                DebugDraw::segment(Vec3(cage.left, cage.bottom, z), Vec3(cage.left, cage.top, z), gridColor);
                                DebugDraw::segment(Vec3(cage.left, cage.top, z), Vec3(cage.right, cage.top, z), gridColor);
                                DebugDraw::segment(Vec3(cage.right, cage.top, z), Vec3(cage.right, cage.bottom, z), gridColor);
                                DebugDraw::segment(Vec3(cage.right, cage.bottom, z), Vec3(cage.left, cage.bottom, z), gridColor);
                                
                                glMatrixMode(GL_MODELVIEW);
                                glPopMatrix();
                            }

                            // Needed since GLFW_DOUBLEBUFFER is GL_TRUE
                            glfwSwapBuffers(app.mainWindow.handle);
                        }
                    }
                }
                
                app.time.now = glfwGetTime();
                app.time.running = app.time.now - app.time.start;
            }
            while (!glfwWindowShouldClose(app.mainWindow.handle));
            glfwDestroyWindow(app.mainWindow.handle);
        }
        
        glfwTerminate();
    }
    
    return 0;
}
