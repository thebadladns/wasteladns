#ifndef __WASTELADNS_PHYSICS_H__
#define __WASTELADNS_PHYSICS_H__


namespace physics {
	
struct StaticObject_Sphere {
    float3 pos;
    f32 radius;
    float3 vel;
};
struct DynamicObject_Sphere {
    float3 pos;
    f32 radius;
    float3 vel;
    f32 mass;
};

struct Scene {
    float3 gravity;
    f32 dt;
    float2 bounds;
	f32 restitution;
	u32 obstacle_count;
	StaticObject_Sphere obstacles[8];
	DynamicObject_Sphere balls[8];
};

void updatePhysics(Scene& scene, f32 game_dt)
{
    const f32 substep_length = 1 / 60.f; // todo: understepping?
    scene.dt += game_dt;
    u32 steps = (u32)math::ceil(scene.dt / substep_length);
    for (u32 s = 0; s < steps; s++) {
		f32 dt;
		if (scene.dt < substep_length * 1.01f) { dt = scene.dt; s++; scene.dt = 0.f; }
		else { dt = substep_length; scene.dt -= substep_length; }
		for (u32 i = 0; i < countof(scene.balls); i++) {
			physics::DynamicObject_Sphere& b1 = scene.balls[i];

			b1.vel = math::add(b1.vel, math::scale(scene.gravity, dt));
			b1.pos = math::add(b1.pos, math::scale(b1.vel, dt));

			for (u32 j = i + 1; j < countof(scene.balls); j++) {
				physics::DynamicObject_Sphere& b2 = scene.balls[j];

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

			for (u32 j = 0; j < scene.obstacle_count; j++) {
				physics::StaticObject_Sphere& o = scene.obstacles[j];

				float3 collisionDir = math::subtract(o.pos, b1.pos);
				f32 dist = math::mag(collisionDir);
				if (dist < math::eps32 || dist > b1.radius + o.radius) continue;

				collisionDir = math::invScale(collisionDir, dist);
				f32 correction = (b1.radius + o.radius - dist) / 2.f;
				b1.pos = math::add(b1.pos, math::scale(collisionDir, -correction));
				f32 v1 = math::dot(b1.vel, collisionDir);
				f32 v2 = math::dot(o.vel, collisionDir);
				f32 m1 = b1.mass;
				b1.vel = math::add(b1.vel, math::scale(collisionDir, v2 - v1));
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
}

#endif // __WASTELADNS_PHYSICS_H__
