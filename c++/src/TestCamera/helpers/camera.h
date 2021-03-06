#ifndef __WASTELADNS_CAMERA_H__
#define __WASTELADNS_CAMERA_H__

namespace Spring {
    
    struct State_linearf32 {
        f32 value;
        f32 velocity;
        
        typedef f32 Value;
        static f32 delta(f32 current, f32 target) {
            return current - target;
        }
        static f32 value_add(f32 targetValue, f32 delta) {
            return targetValue + delta;
        }
        static f32 delta_add(f32 a, f32 b) {
            return a + b;
        }
        static f32 delta_subtract(f32 a, f32 b) {
            return a - b;
        }
        static f32 delta_scale(f32 v, f32 s) {
            return v * s;
        }
        static f32 delta_mag(f32 v) {
            return v;
        }
        static f32 delta_min(f32 v, f32 max) {
            return Math::min(v, max);
        }
    };
    struct State_angf32 {
        f32 value;
        f32 velocity;
        
        typedef f32 Value;
        static f32 delta(f32 current, f32 target) {
            return Angle::subtractShort(current, target);
        }
        static f32 value_add(f32 targetValue, f32 delta) {
            return Angle::wrap(targetValue + delta);
        }
        static f32 delta_add(f32 a, f32 b) {
            return a + b;
        }
        static f32 delta_subtract(f32 a, f32 b) {
            return a - b;
        }
        static f32 delta_scale(f32 v, f32 s) {
            return v * s;
        }
        static f32 delta_mag(f32 v) {
            return v;
        }
        static f32 delta_min(f32 v, f32 max) {
            return Math::min(v, max);
        }
    };
    struct State_linearVec3 {
        Vec3 value;
        Vec3 velocity;
        
        typedef Vec3 Value;
        static Vec3 delta(const Vec3& current, const Vec3& target) {
            return Vec::subtract(current, target);
        }
        static Vec3 value_add(const Vec3& targetValue, const Vec3& delta) {
            return Vec::add(targetValue, delta);
        }
        static Vec3 delta_add(const Vec3& a, const Vec3& b) {
            return Vec::add(a, b);
        }
        static Vec3 delta_subtract(const Vec3& a, const Vec3& b) {
            return Vec::subtract(a, b);
        }
        static Vec3 delta_scale(const Vec3& v, f32 s) {
            return Vec::scale(v, s);
        }
        static f32 delta_mag(const Vec3& v) {
            return Vec::mag(v);
        }
        static Vec3 delta_min(const Vec3& v, const Vec3& max) {
            const f32 max_x = Math::abs(max.x);
            const f32 max_y = Math::abs(max.y);
            const f32 max_z = Math::abs(max.z);
            Vec3 copy(
                  Math::clamp(v.x, -max_x, max_x)
                , Math::clamp(v.y, -max_y, max_y)
                , Math::clamp(v.z, -max_z, max_z)
            );
            return copy;
        }
    };
    
    struct DampedConfig {
        f32 angularFrequency;
        f32 dampingRatio;
    };
    
    template <typename _State>
    struct DampedInstance {
        DampedConfig config;
        _State state;
    };
    
    template <typename _State>
    void spring_0(_State& spring, const typename _State::Value value_0, const typename _State::Value velocity_0) {
        spring.value = value_0;
        spring.velocity = velocity_0;
    }
    
    /*  Linear, non-templated f32 version, for reference
     *  See http://www.ryanjuckett.com/programming/damped-springs/
    void damped_t(DampedInstance& spring, const f32 targetValue, const f32 t) {

        if (spring.config.angularFrequency < Math::eps<f32>) return;
        if (spring.config.dampingRatio < 1.f) return;

        const f32 delta = spring.state.value - targetValue;

        if (spring.config.dampingRatio > 1.f + Math::eps<f32>) {
            const f32 za = -spring.config.angularFrequency * spring.config.dampingRatio;
            const f32 zb = spring.config.angularFrequency * Math::sqrt(Math::square(spring.config.dampingRatio) - 1.f);
            const f32 z1 = za - zb;
            const f32 z2 = za + zb;
            const f32 expTerm1 = Math::exp_taylor(z1 * t);
            const f32 expTerm2 = Math::exp_taylor(z2 * t);
            const f32 c1 = (spring.state.velocity - delta * z2) / (-2.f * zb);
            const f32 c2 = delta - c1;

            spring.state.value = targetValue + c1 * expTerm1 + c2 * expTerm2;
            spring.state.velocity = c1 * z1 * expTerm1 + c2 * z2 * expTerm2;
        } else if (spring.config.dampingRatio <= 1.f + Math::eps<f32>) {
            const f32 expTerm = Math::exp_taylor( - spring.config.angularFrequency * t);
            const f32 c1 = spring.state.velocity + spring.config.angularFrequency * delta;
            const f32 c2 = delta;
            const f32 c3 = (c1 * t + c2) * expTerm;

            spring.state.value = targetValue + c3;
            spring.state.velocity = c1 * expTerm - c3 * spring.config.angularFrequency;
        }
    }
     */
    template <typename _State>
    void damped_t(DampedInstance<_State>& spring, const typename _State::Value targetValue, const f32 t) {
        
        if (spring.config.angularFrequency < Math::eps<f32>) return;
        if (spring.config.dampingRatio < 1.f) return;
        
        const typename _State::Value delta = _State::delta(spring.state.value, targetValue);
        
        if (spring.config.dampingRatio > 1.f + Math::eps<f32>) {
            const f32 za = -spring.config.angularFrequency * spring.config.dampingRatio;
            const f32 zb = spring.config.angularFrequency * Math::sqrt(Math::square(spring.config.dampingRatio) - 1.f);
            const f32 z1 = za - zb;
            const f32 z2 = za + zb;
            const f32 expTerm1 = Math::exp_taylor(z1 * t);
            const f32 expTerm2 = Math::exp_taylor(z2 * t);
            const typename _State::Value c1 = _State::delta_scale(_State::delta_subtract(spring.state.velocity, _State::delta_scale(delta, z2)), 1.f / (-2.f * zb));
            const typename _State::Value c2 = _State::delta_subtract(delta, c1);
            
            spring.state.value = _State::value_add(targetValue, _State::delta_add(_State::delta_scale(c1, expTerm1), _State::delta_scale(c2, expTerm2)));
            spring.state.velocity = _State::delta_add(_State::delta_scale(c1, z1 * expTerm1), _State::delta_scale(c2, z2 * expTerm2));
        } else if (spring.config.dampingRatio <= 1.f + Math::eps<f32>) {
            const f32 expTerm = Math::exp_taylor(-spring.config.angularFrequency * t);
            const typename _State::Value c1 = _State::delta_add(spring.state.velocity, _State::delta_scale(delta, spring.config.angularFrequency));
            const typename _State::Value c2 = delta;
            const typename _State::Value c3 = _State::delta_scale(( _State::delta_add(_State::delta_scale(c1, t), c2)), expTerm);
            
            spring.state.value = _State::value_add(targetValue, c3);
            spring.state.velocity = _State::delta_subtract(_State::delta_scale(c1, expTerm), _State::delta_scale(c3, spring.config.angularFrequency));
        }
    }
    
    template <typename _State>
    void dampedNumericalVelocity_t(DampedInstance<_State>& spring, const typename _State::Value targetValue, const f32 t) {
        f32 w = spring.config.angularFrequency;
        typename _State::Value& velocity = spring.state.velocity;
        typename _State::Value pos = spring.state.value;
        typename _State::Value delta = _State::delta(pos, targetValue);
        typename _State::Value maxVelocity = _State::delta_scale(delta, 1.f / t);
        typename _State::Value acc = _State::delta_add(_State::delta_scale(delta, -w * w), _State::delta_scale(velocity, -2.f * w));
        velocity = _State::delta_add(velocity, _State::delta_scale(acc, t));
        velocity = _State::delta_min(velocity, maxVelocity);
    }
}

namespace Camera {
    
    void transformToRender(Mat4& renderMatrix, const Vec::Transform& t) {
        // We are left-handed, negate front for opengl
        renderMatrix.col0 = t.right;
        renderMatrix.col1 = t.up;
        renderMatrix.col2 = Vec::negate(t.front);
        renderMatrix.col3 = t.pos;
        renderMatrix.col0_w = 0.f;
        renderMatrix.col1_w = 0.f;
        renderMatrix.col2_w = 0.f;
        renderMatrix.col3_w = 1.f;
    }

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

	void computeProjectionMatrix(const FrustumParams& params, Matrix44<f64>& matrix) {
		const f64 xMax = params.near * tan(params.fov * 2.f * Angle::d2r<f64>);
		const f64 xMin = -xMax;
		const f64 yMin = xMin / params.aspect;
		const f64 yMax = xMax / params.aspect;

        f64* matrixCM = matrix.dataCM;
		memset(matrixCM, 0, sizeof(f64) * 16);
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

	void computeProjectionMatrix(const OrthoParams& params, Mat4& matrix) {
        f32* matrixCM = matrix.dataCM;
		memset(matrixCM, 0, sizeof(f32) * 16);
		matrixCM[0] = 2.f / (params.right - params.left);
		matrixCM[5] = 2.f / (params.top - params.bottom);
		matrixCM[10] = -2.f / (params.far - params.near);
		matrixCM[12] = -(params.right + params.left) / (params.right - params.left);
		matrixCM[13] = -(params.top + params.bottom) / (params.top - params.bottom);
		matrixCM[14] = -(params.far + params.near) / (params.far - params.near);
		matrixCM[15] = 1.f;
	}
    
    struct ProjectionConfig {
        Camera::OrthoParams orthoParams;
        Camera::FrustumParams frustumParams;
        Mat4 orthoMatrix;
        Matrix44<f64> frustumMatrix;
    };
    
    struct DirControl {
        Vec4 inputDir = {};
    };
    
    struct OrbitParams {

        struct Config {
            f32 yawLS = 0.f;
            f32 pitchLS = 50 * Angle::d2r<f32>;
            f32 distance = 200.f;
            f32 maxLagXY = 25.f;
            f32 zoffset = 80.f;
        };
        
        Config config;
        Vec::Transform targetTransform;
        Spring::DampedConfig position_springConfigAtMinSpeed;
        Spring::DampedConfig position_springConfigAtMaxSpeed;
        Spring::DampedInstance<Spring::State_linearVec3> position_spring;
        Vec3 targetOffset_linear, targetPos;
        f32 targetSpeed;
        
        bool active = false;
    };
    
    struct Instance {
        Vec::Transform transform;
        Mat4 renderMatrix;
        OrbitParams orbit;
    };
    
    struct UpdateParams {
        struct MovementType {
            enum Enum { manual, lookat, orbit };
        };
        Instance* instance;
        const DirControl* dirControl;
        const Vec::Transform* orbitTransform;
        Vec3 lookat;
        f32 playerSpeedNormalized;
        f32 dt;
        MovementType::Enum movementType = MovementType::manual;
        bool debugBreak = false;
        bool playerPushing;
        bool springClosedForm;
    };
    void update(UpdateParams& params) {
        Instance& camera = *params.instance;
        Vec3& pos = camera.transform.pos;
        
        Vec4 movement = params.dirControl->inputDir;
        if (Vec::normalizeSafe(movement.xyz)) {
            f32 speed = 4.f;
            Vec3 translation = Vec::scale(movement.xyz, speed);
            
            Vec3 front = Vec3(camera.transform.front.xy, 0.f);
            f32 front_mag = Vec::mag(front);
            if (front_mag < 0.1f) {
                front = Vec::normalize(Vec3(camera.transform.up.xy, 0.f));
                translation.y = Math::min(translation.y, 0.f);
            } else {
                front = Vec::scale(front, 1 / front_mag);
            }
            
            Vec3 right = camera.transform.right;
            
            pos = Vec::add(pos, Vec::scale(right, translation.x));
            pos = Vec::add(pos, Vec::scale(front, translation.y));
            pos = Vec::add(pos, Vec::scale(Vec::UP_AXIS, translation.z));
        }

        // Rotation
        if (params.movementType == UpdateParams::MovementType::lookat) {
            Vec3 lookatDir = Vec::subtract(params.lookat, pos);
            transformFromFront(camera.transform, lookatDir);
        } else if (params.movementType == UpdateParams::MovementType::orbit) {

            OrbitParams& orbit = camera.orbit;
            const OrbitParams::Config& config = orbit.config;

            if (params.debugBreak) {
                printf("");
            }
            
            Vec3 currentOffset = Vec::normalize(Vec::subtract(camera.transform.pos, params.orbitTransform->pos));
            if (!orbit.active) {
                // Init
                Spring::spring_0(orbit.position_spring.state, camera.transform.pos, Vec3(0.f, 0.f, 0.f));
                orbit.position_springConfigAtMinSpeed.dampingRatio = 1.f;
                orbit.position_springConfigAtMinSpeed.angularFrequency = 5.f;
                orbit.position_springConfigAtMaxSpeed.dampingRatio = 1.f;
                orbit.position_springConfigAtMaxSpeed.angularFrequency = 10.f;
                orbit.targetOffset_linear = currentOffset;
                orbit.targetTransform = *params.orbitTransform;
                orbit.targetSpeed = 0.f;
                orbit.active = true;
            }

            orbit.position_spring.config.angularFrequency = Math::lerp(params.playerSpeedNormalized, orbit.position_springConfigAtMinSpeed.angularFrequency, orbit.position_springConfigAtMaxSpeed.angularFrequency);
            orbit.position_spring.config.dampingRatio = Math::lerp(params.playerSpeedNormalized, orbit.position_springConfigAtMinSpeed.dampingRatio, orbit.position_springConfigAtMaxSpeed.dampingRatio);
            
            Vec3 centerVelocity = Vec::scale(Vec::subtract(params.orbitTransform->pos, orbit.targetTransform.pos), 1.f / params.dt);
            orbit.targetSpeed = Vec::mag(centerVelocity);
            Vec3 prevPos = orbit.position_spring.state.value;
            
            f32 currentYaw = Angle::orientation(currentOffset.xy);
            f32 targetYaw = currentYaw;
            f32 targetPitch = config.pitchLS;
            f32 targetDistance = config.distance;
            f32 effectivePitch = targetPitch;
            f32 effectivePitchSin = Angle::sin(effectivePitch);
            orbit.targetOffset_linear.xy = Angle::direction(targetYaw);
            orbit.targetOffset_linear.x = targetDistance * orbit.targetOffset_linear.x * effectivePitchSin;
            orbit.targetOffset_linear.y = targetDistance * orbit.targetOffset_linear.y * effectivePitchSin;
            orbit.targetOffset_linear.z = targetDistance * Angle::cos(effectivePitch);
            
            Vec3 lookatPos = Vec::add(params.orbitTransform->pos, Vec3(0.f, 0.f, config.zoffset));
            Vec3 targetPos = Vec::add(lookatPos, orbit.targetOffset_linear);
            orbit.targetPos = targetPos;
            
            {
                Spring::dampedNumericalVelocity_t(orbit.position_spring, targetPos, params.dt);
                Vec3& pos = orbit.position_spring.state.value;
                pos = Vec::add(pos, Vec::scale(orbit.position_spring.state.velocity, params.dt));
                orbit.position_spring.state.value = pos;
            }
            
            // ensure min max dist
            Vec3 fromPos = Vec::subtract(orbit.position_spring.state.value, targetPos);
            f32 mag = Vec::mag(fromPos.xy);
            if (mag > Math::eps<f32>) {
                f32 clampedmag = Math::min(mag, config.maxLagXY);
                fromPos.xy = Vec::scale(fromPos.xy, clampedmag / mag);
                orbit.position_spring.state.value = Vec::add(targetPos, fromPos);
                orbit.position_spring.state.velocity = Vec::scale(Vec::subtract(orbit.position_spring.state.value, prevPos), 1 / params.dt);
            }
            
            orbit.targetTransform = *params.orbitTransform;

            // Build camera transform
            Vec3 cameraPos = orbit.position_spring.state.value;
            Vec3 lookatDir = Vec::subtract(lookatPos, cameraPos);
            Vec::transformFromFront(camera.transform, lookatDir);
            camera.transform.pos = cameraPos;
            
        } else if (Math::abs(movement.w) > Math::eps<f32>){
            Vec3 front = camera.transform.front;
            const f32 angleIncrement = movement.w * 0.01f;
            const f32 frontXYMag = Vec::mag(front.xy);
            f32 frontXYOrientation = Angle::orientation(front.xy);
            frontXYOrientation = Angle::wrap(frontXYOrientation + angleIncrement);
            front.xy = Vec::scale(Angle::direction(frontXYOrientation), frontXYMag);
            transformFromFront(camera.transform, front);
        }
        
        transformToRender(camera.renderMatrix, camera.transform);
        // Store the inverse modelview matrix for the renderer
        if (!Vec::inverse(camera.renderMatrix)) {
            // Reset if something went wrong
            identity4x4(camera.transform);
            transformToRender(camera.renderMatrix, camera.transform);
            Vec::inverse(camera.renderMatrix);
        }
    }
};

#endif // __WASTELADNS_CAMERA_H__
