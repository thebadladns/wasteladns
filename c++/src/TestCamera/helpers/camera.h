#ifndef __WASTELADNS_CAMERA_H__
#define __WASTELADNS_CAMERA_H__

namespace Math3D {
	typedef f32 Transform32[16];
	typedef f64 Transform64[16];
}

namespace Camera {
    
    // Left-handed (right is to the right of front)
    struct Transform {
        Transform() {}
        Transform(const Transform& t) {
            matrix = t.matrix;
        }
        
        union {
            struct {
                Vec3 right; f32 right_w;
                Vec3 front; f32 front_w;
                Vec3 up; f32 up_w;
                Vec3 pos; f32 pos_w;
            };
            struct {
                Vec3 x; f32 x_w;
                Vec3 y; f32 y_w;
                Vec3 z; f32 z_w;
            };
            f32 dataCM[16];
            Mat4 matrix;
        };
    };
    
    Vec3 RIGHT_AXIS(1.f, 0.f, 0.f);
    Vec3 FRONT_AXIS(0.f, 1.f, 0.f);
    Vec3 UP_AXIS(0.f, 0.f, 1.f);
    
    void transformToRender(Mat4& renderMatrix, const Transform& t) {
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
    
    void transformFromFront(Transform& t, const Vec3& v) {
        Vec3 tmpUp = UP_AXIS;
        Vec3 right = Vec::cross(v, tmpUp);
        if (!Vec::normalizeSafe(right)) {
            right = RIGHT_AXIS;
        }
        Vec3 up = Vec::normalize(Vec::cross(right, v));
        Vec3 front = Vec::cross(up, right);
        t.front = front;
        t.right = right;
        t.up = up;
    }

    void identity3x3(Transform& t) {
        t.right = RIGHT_AXIS;
        t.front = FRONT_AXIS;
        t.up = UP_AXIS;
    }
    
    void identity4x4(Transform& t) {
        identity3x3(t);
        t.pos = {};
        t.right_w = t.front_w = t.up_w = 0.f;
        t.pos_w = 1.f;
    }
    
    Vec2 transform3x3(const Transform& t, const Vec2& v) {
        Vec2 out(
              t.x.x * v.x + t.y.x * v.y
            , t.x.y * v.x + t.y.y * v.y
        );
        return out;
    }
    Vec3 transform3x3(const Transform& t, const Vec3& v) {
        Vec3 out(
              t.x.x * v.x + t.y.x * v.y + t.z.x * v.z
            , t.x.y * v.x + t.y.y * v.y + t.z.y * v.z
            , t.x.z * v.x + t.y.z * v.y + t.z.z * v.z
        );
        return out;
    }
    // TODO: probably will need to revise these when used
    Vec2 untransform3x3(const Transform& t, const Vec2& v) {
        Vec2 out = Vec::add(
              Vec::scale(t.x.xy, Vec::dot(t.x.xy, v))
            , Vec::scale(t.y.xy, Vec::dot(t.y.xy, v))
        );
        return out;
    }
    Vec3 untransform3x3(const Transform& t, const Vec3& v) {
        Vec3 out = Vec::scale(t.x, Vec::dot(t.x, v));
        out = Vec::add(out, Vec::scale(t.y, Vec::dot(t.y, v)));
        out = Vec::add(out, Vec::scale(t.z, Vec::dot(t.z, v)));
        return out;
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
    
    struct Instance {
        Transform transform;
        Mat4 renderMatrix;
    };
    
    struct UpdateCameraParams {
        Instance* instance;
        DirControl* dirControl;
        bool lookat;
    };
    void UpdateCamera(UpdateCameraParams& params) {
        Instance& camera = *params.instance;
        Vec3& posv = camera.transform.pos;
        
        Vec4 movement = params.dirControl->inputDir;
        if (Vec::normalizeSafe(movement.xyz)) {
            f32 speed = 4.f;
            Vec3 translation = Vec::scale(movement.xyz, speed);
            
            Vec3 front = camera.transform.front;
            Vec3 right = camera.transform.right;
            
            posv = Vec::add(posv, Vec::scale(right, translation.x));
            posv = Vec::add(posv, Vec::scale(Vec3(front.xy, 0.f), translation.y));
            posv = Vec::add(posv, Vec::scale(UP_AXIS, translation.z));
        }

        if (params.lookat) {
            Vec3 lookAt(0.f, 0.f, 0.f);
            Vec3 toLookAt = Vec::subtract(lookAt, posv);
            transformFromFront(camera.transform, toLookAt);
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
