#ifndef __WASTELADNS_PHYSICS_H__
#define __WASTELADNS_PHYSICS_H__


namespace physics {

struct Ball {
    float3 pos;
    f32 radius;
    float3 vel;
    f32 mass;
};

struct Scene {
    float3 gravity;
    //f32 dt;// todo: substep when necessary?
    float2 bounds;
    f32 restitution;
    Ball balls[8];
};

void updatePhysics(Scene& scene, f32 dt)
{
    const u32 numballs = countof(scene.balls);
    for (u32 i = 0; i < numballs; i++) {
        physics::Ball& b1 = scene.balls[i];

        b1.vel = math::add(b1.vel, math::scale(scene.gravity, dt));
        b1.pos = math::add(b1.pos, math::scale(b1.vel, dt));

        for (u32 j = i + 1; j < numballs; j++) {
            physics::Ball& b2 = scene.balls[j];

            float3 collisionDir = math::subtract(b2.pos, b1.pos);
            f32 dist = math::mag(collisionDir);
            if (dist < math::eps32 || dist > b1.radius + b2.radius) continue;

            collisionDir = math::invScale(collisionDir, dist);
            f32 correction = (b1.radius + b2.radius - dist) / 2.f;
            b1.pos = math::add(b1.pos, math::scale(collisionDir, -correction));
            b2.pos = math::add(b2.pos, math::scale(collisionDir, correction));
            f32 v1 = math::dot(b1.vel, collisionDir);
            f32 v2 = math::dot(b2.vel, collisionDir);
            f32 m1 = b1.mass;
            f32 m2 = b2.mass;
            f32 nextv1 = (m1 * v1 + m2 * v2 - m2 * (v1 - v2) * scene.restitution) / (m1 + m2);
            f32 nextv2 = (m1 * v1 + m2 * v2 - m1 * (v2 - v1) * scene.restitution) / (m1 + m2);
            b1.vel = math::add(b1.vel, math::scale(collisionDir, nextv1 - v1));
            b2.vel = math::add(b2.vel, math::scale(collisionDir, nextv2 - v2));
        }

        if ((b1.pos.x + b1.radius) > scene.bounds.x) {
            b1.pos.x = scene.bounds.x - b1.radius;
            b1.vel.x = -b1.vel.x;
        }
        if ((b1.pos.x - b1.radius) < -scene.bounds.x) {
            b1.pos.x = -scene.bounds.x + b1.radius;
            b1.vel.x = -b1.vel.x;
        }
        if ((b1.pos.y + b1.radius) > scene.bounds.y) {
            b1.pos.y = scene.bounds.y - b1.radius;
            b1.vel.y = -b1.vel.y;
        }
        if ((b1.pos.y - b1.radius) < -scene.bounds.y) {
            b1.pos.y = -scene.bounds.y + b1.radius;
            b1.vel.y = -b1.vel.y;
        }
    }
}
}

#endif // __WASTELADNS_PHYSICS_H__
