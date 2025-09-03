#ifndef __WASTELADNS_PHYSICS_H__
#define __WASTELADNS_PHYSICS_H__


namespace physics {

struct StaticObject_Line {
	float3 start;
	float3 end;
};
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
struct ObjectType { enum Enum {
	StaticLine, StaticSphere, DynamicSphere,
	Count, Bits = math::ceillog2(Count), BitMask = (1 << Bits) - 1  }; };
static_assert(ObjectType::Bits < 16, "check");
typedef u32 Handle;

struct Scene {
    float3 gravity;
    f32 dt;
    float2 bounds;
	f32 restitution;
	u32 wall_count;
	u32 obstacle_count;
	u32 ball_count;
	StaticObject_Line walls[8];
	StaticObject_Sphere obstacles[8];
	DynamicObject_Sphere balls[8];
};
force_inline Handle handleFromObject(StaticObject_Line& w, Scene& scene) {
	return (u32(&w - scene.walls) << ObjectType::Bits) | ObjectType::StaticLine;
}
force_inline Handle handleFromObject(StaticObject_Sphere& o, Scene& scene) {
	return (u32(&o - scene.obstacles) << ObjectType::Bits) | ObjectType::StaticSphere;
}
force_inline Handle handleFromObject(DynamicObject_Sphere& b, Scene& scene) {
	return (u32(&b - scene.balls) << ObjectType::Bits) | ObjectType::DynamicSphere;
}
void updatePositionFromHandle(Scene& scene, Handle& handle, const float3 pos, const f32 dt) {
	u32 type = handle & ObjectType::BitMask;
	if (type == ObjectType::StaticLine) {
		u32 index = handle >> ObjectType::Bits;
		StaticObject_Line& line = scene.walls[index];
		line.start = math::add(line.start, pos);
		line.end = math::add(line.end, pos);
	} else if (type == ObjectType::StaticSphere) {
		u32 index = handle >> ObjectType::Bits;
		StaticObject_Sphere& o = scene.obstacles[index];
		o.vel = math::invScale(math::subtract(pos, o.pos), dt);
		o.pos = pos;
	}
	else if (type == ObjectType::DynamicSphere) {
		u32 index = handle >> ObjectType::Bits;
		DynamicObject_Sphere& b = scene.balls[index];
		b.vel = math::invScale(math::subtract(pos, b.pos), dt);
		b.pos = pos;
	}
}

void updatePhysics(Scene& scene, f32 game_dt)
{
    const f32 substep_length = 1 / 60.f; // todo: understepping?
    scene.dt += game_dt;
    u32 steps = (u32)math::ceil(scene.dt / substep_length);
    for (u32 s = 0; s < steps; s++) {
		f32 dt;
		if (scene.dt < substep_length * 1.01f) { dt = scene.dt; s++; scene.dt = 0.f; }
		else { dt = substep_length; scene.dt -= substep_length; }
		for (u32 i = 0; i < scene.ball_count; i++) {
			physics::DynamicObject_Sphere& b1 = scene.balls[i];

			b1.vel = math::add(b1.vel, math::scale(scene.gravity, dt));
			b1.pos = math::add(b1.pos, math::scale(b1.vel, dt));

			for (u32 j = i + 1; j < scene.ball_count; j++) {
				physics::DynamicObject_Sphere& b2 = scene.balls[j];

				float3 collisionDir = math::subtract(b2.pos, b1.pos);
				collisionDir.z = 0.f;
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

				float3 collisionDir = math::subtract(b1.pos, o.pos);
				collisionDir.z = 0.f;
				f32 dist = math::mag(collisionDir);
				if (dist < math::eps32 || dist > b1.radius + o.radius) continue;

				collisionDir = math::invScale(collisionDir, dist);
				f32 correction = b1.radius + o.radius - dist;
				b1.pos = math::add(b1.pos, math::scale(collisionDir, correction));
				f32 v1 = math::dot(b1.vel, collisionDir);
				f32 v2 = math::dot(o.vel, collisionDir);
				b1.vel = math::add(b1.vel, math::scale(collisionDir, (v2 - v1) * scene.restitution));
			}

			for (u32 j = 0; j < scene.wall_count; j++) {
				physics::StaticObject_Line& w = scene.walls[j];

				// distance to wall
				float3 ab = math::subtract(w.end, w.start);
				f32 t = math::max(0.f, 
					math::min(1.f,
						(math::dot(math::subtract(b1.pos, w.start), ab)) / math::dot(ab, ab)));
				float3 col = math::add(w.start, math::scale(ab, t));
				float3 d = math::subtract(b1.pos, col);
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
					if (dist > b1.radius) continue;
					b1.pos = math::add(b1.pos, math::scale(d, b1.radius - dist));
				} else { // inside the wall
					b1.pos = math::add(b1.pos, math::scale(d, -(b1.radius + dist)));
				}

				// update velocity
				f32 v = math::dot(b1.vel, d);
				f32 vnew = math::abs(v) * scene.restitution;
				b1.vel = math::add(b1.vel, math::scale(d, vnew - v));
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
