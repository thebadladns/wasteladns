#ifndef __WASTELADNS_PHYSICS_H__
#define __WASTELADNS_PHYSICS_H__


namespace physics {

struct StaticObject_Wall {
	float4 plane;
};
struct StaticObject_Sphere {
    float3 pos;
    f32 radius;
    float3 vel;
};
struct StaticObject_Tri {
	float3 a;
	float3 b;
	float3 c;
};
struct DynamicObject_Sphere {
    float3 pos;
    f32 radius;
    float3 vel;
    f32 mass;
};
struct ObjectType { enum Enum {
	StaticWall, StaticSphere, StaticTri, DynamicSphere, 
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
	u32 tri_count;
	StaticObject_Wall walls[8];
	StaticObject_Sphere obstacles[8];
	DynamicObject_Sphere balls[8];
	StaticObject_Tri tris[1024];
};
Scene scene;

force_inline Handle handleFromObject(StaticObject_Wall& w) {
	return (u32(&w - scene.walls) << ObjectType::Bits) | ObjectType::StaticWall;
}
force_inline Handle handleFromObject(StaticObject_Sphere& o) {
	return (u32(&o - scene.obstacles) << ObjectType::Bits) | ObjectType::StaticSphere;
}
force_inline Handle handleFromObject(StaticObject_Tri& t) {
	return (u32(&t - scene.tris) << ObjectType::Bits) | ObjectType::DynamicSphere;
}
force_inline Handle handleFromObject(DynamicObject_Sphere& b) {
	return (u32(&b - scene.balls) << ObjectType::Bits) | ObjectType::DynamicSphere;
}
void updatePositionFromHandle(Handle& handle, const float3 pos, const f32 dt) {
	u32 type = handle & ObjectType::BitMask;
	if (type == ObjectType::StaticWall) {
		u32 index = handle >> ObjectType::Bits;
		StaticObject_Wall& wall = scene.walls[index];
		wall.plane.w = -math::dot(pos, wall.plane.xyz);
	} else if (type == ObjectType::StaticSphere) {
		u32 index = handle >> ObjectType::Bits;
		StaticObject_Sphere& o = scene.obstacles[index];
		o.vel = math::invScale(math::subtract(pos, o.pos), dt);
		o.pos = pos;
	} else if (type == ObjectType::DynamicSphere) {
		u32 index = handle >> ObjectType::Bits;
		DynamicObject_Sphere& b = scene.balls[index];
		b.vel = math::invScale(math::subtract(pos, b.pos), dt);
		b.pos = pos;
	}
}

void updatePhysics(f32 game_dt)
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
				physics::StaticObject_Wall& w = scene.walls[j];

				// distance to wall
				f32 dist = w.plane.w + math::dot(b1.pos, w.plane.xyz);
				float3 normal = w.plane.xyz;
				normal.z = 0.f;

				// push out
				float3 d = normal;
				if (dist >= 0.f) { // outside the wall
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

struct RaycastResult {
    float3 hitpos;
    float3 hitnormal;
    bool valid;
};

RaycastResult raycast_planes(const float3& p0, const float3& p1) {

	RaycastResult result;
	f32 closest_t = FLT_MAX;
    u32 hit_idx = physics::scene.wall_count;

    // simply loop over all planes
    float3 p1p0 = math::subtract(p1, p0);
    for (u32 i = 0; i < physics::scene.wall_count; i++) {
		const StaticObject_Wall& p = physics::scene.walls[i];

		if (math::dot((p.plane.xyz), p1p0) > 0.f) { continue; }

		f32 t = -(p.plane.w +math::dot(p.plane.xyz, p0)) / math::dot(p.plane.xyz, p1p0);
        if (t > 0. && t <= 1. && t < closest_t) {
            closest_t = t;
			result.hitpos = math::add(p0, math::scale(p1p0, closest_t));
			result.hitnormal = p.plane.xyz;
			hit_idx = i;
        }
    }

    result.valid = hit_idx < physics::scene.wall_count;
    return result;
}

bool raycast_sphere( f32& t,
					 float3& hitpos,
					 float3& hitnormal,
					 const float3& p0,
					 const float3& p1,
					 const float3& center,
					 const f32 radius) {

	float3 p1p0 = math::subtract(p1, p0);
    f32 segment_length_sq = math::magSq(p1p0);

    // ignore zero segments
    if (segment_length_sq < 0.0001f) { return false; }

    f32 segment_length = math::sqrt(segment_length_sq);
    float3 d = math::invScale(p1p0, segment_length);

    // solve mag(p(t)-center) = r with p(t) = p0 + t * d
    float3 p0center = math::subtract(p0, center);
    f32 b = math::dot(p0center, d);
	f32 c = math::dot(p0center, p0center) - radius * radius;

    // away from the radius and point outwards, ignore result
    if (c > 0.f && b > 0.f) return false;

	f32 discriminant = b * b - c;

	// negative discriminant means there is no intersection
	if (discriminant < 0.f) return false;

    // choose negative root, since at this point we now it's the first intersection point
    t = -b - math::sqrt(discriminant);

    // if it's behind us, the segment is inside the sphere, choose the start of the hit position
    if (t < 0.f) t = 0.f;

	hitpos = math::add(p0, math::scale(d , t));
	hitnormal = math::normalize(math::subtract(hitpos, center));

    // t needs to be scaled from 0 to 1
    t /= segment_length;

    return true;
}
RaycastResult raycast_obstacles(const float3& p0, const float3& p1) {
	f32 closest_t = FLT_MAX;
	RaycastResult result;
    for (u32 i = 0; i < physics::scene.ball_count; i++) {
        physics::DynamicObject_Sphere& ball = physics::scene.balls[i];

        f32 t;
		float3 hitpos, hitnormal;
        if (raycast_sphere(t, hitpos, hitnormal, p0, p1, ball.pos, ball.radius)) {
            if (t > 0. && t <= 1. && t < closest_t) {
                closest_t = t;
				result.hitpos = hitpos;
				result.hitnormal = hitnormal;
            }
        }
    }
    for (u32 i = 0; i < physics::scene.obstacle_count; i++) {
		physics::StaticObject_Sphere& obstacle = physics::scene.obstacles[i];

        f32 t;
		float3 hitpos, hitnormal;
        if (raycast_sphere(t, hitpos, hitnormal, p0, p1, obstacle.pos, obstacle.radius)) {
            if (t > 0. && t <= 1. && t < closest_t) {
                closest_t = t;
				result.hitpos = hitpos;
				result.hitnormal = hitnormal;
            }
        }
    }

    result.valid = closest_t < FLT_MAX;
    return result;
}

RaycastResult raycast_tris(const float3& p0, const float3& p1) {
    f32 closest_t = FLT_MAX;
	u32 hit_idx = physics::scene.tri_count;
	RaycastResult result;
	float3 p1p0 = math::subtract(p1, p0);
    for (u32 i = 0; i < physics::scene.tri_count; i++) {

		physics::StaticObject_Tri& tri = physics::scene.tris[i];

        float3 pa = math::subtract(tri.a, p0);
        float3 pb = math::subtract(tri.b, p0);
        float3 pc = math::subtract(tri.c, p0);

        // check if p1p0 is inside the edges bc, ca and ab
        f32 u = math::dot(p1p0, math::cross(pc, pb));
        if (u < 0.f) continue;
		f32 v = math::dot(p1p0, math::cross(pa, pc));
        if (v < 0.f) continue;
		f32 w = math::dot(p1p0, math::cross(pb, pa));
        if (w < 0.f) continue;

		f32 denom = 1.f / (u + v + w);
        u = u * denom;
        v = v * denom;
        w = w * denom;

		float3 hit = math::add(math::scale(tri.a, u), math::add(math::scale(tri.b, v), math::scale(tri.c, w)));
		f32 t = math::dot(math::subtract(hit, p0), p1p0) / math::dot(p1p0, p1p0);

        if (t > 0. && t <= 1. && t < closest_t) {
			result.hitpos = hit;
			result.hitnormal = math::normalize(math::cross(math::subtract(tri.b, tri.a), math::subtract(tri.c, tri.a)));
            closest_t = t;
            hit_idx = i;
        }
    }

    result.valid = hit_idx < physics::scene.tri_count;
    return result;
}

RaycastResult raycast_world(const float3& p0, const float3& p1) {

	RaycastResult result;
    result = raycast_tris(p0, p1);

	RaycastResult obstacles_result;
    obstacles_result = raycast_obstacles(p0, p1);
    if (obstacles_result.valid &&
        (!result.valid || math::magSq(math::subtract(obstacles_result.hitpos, p0)) < math::magSq(math::subtract(result.hitpos, p0)))) {
        result = obstacles_result;
    }

	RaycastResult planes_result;
    planes_result = raycast_planes(p0, p1);
    if (planes_result.valid &&
		(!result.valid || math::magSq(math::subtract(planes_result.hitpos, p0)) < math::magSq(math::subtract(result.hitpos, p0)))) {
        result = planes_result;
    }

    return result;
}

RaycastResult raycast(const float3& p0, const float3& p1) {
    return raycast_world(p0, p1);
}


}

#endif // __WASTELADNS_PHYSICS_H__
