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
        
        union {
            struct {
                Vec3 right; f32 bottomRow0;
                Vec3 up; f32 bottomRow1;
                Vec3 front; f32 bottomRow2;
                Vec3 pos; f32 bottomRow3;
            };
            f32 dataCM[16];
            Mat4 matrix;
        };
    };
    
    Vec3 UP_AXIS(0.f, 0.f, 1.f);
    Vec3 RIGHT_AXIS(1.f, 0.f, 0.f);
    Vec3 FRONT_AXIS(0.f, 1.f, 0.f);

    void identity3x3(Transform& t) {
        t.right = RIGHT_AXIS;
        t.up = UP_AXIS;
        t.front = FRONT_AXIS;
    }
    
    void identity4x4(Transform& t) {
        identity3x3(t);
        t.pos = {};
        t.bottomRow0 = t.bottomRow1 = t.bottomRow3 = 0.f;
        t.bottomRow3 = 1.f;
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
        Vec3 inputDir = {};
    };
    
    struct Instance {
        Transform transform;
        Mat4 renderMatrix;
    };
    
    struct UpdateCameraParams {
        Instance* instance;
        Vec3 inputDir;
        bool lookat;
    };
    void UpdateCamera(UpdateCameraParams& params) {
        Instance& camera = *params.instance;
        Vec3& posv = camera.transform.pos;
        
        Vec3 movement = params.inputDir;
        if (Vec::normalizeSafe(movement)) {
            f32 speed = 4.f;
            movement = Vec::scale(movement, speed);
            
            Vec3 front = camera.transform.front;
            Vec3 right = camera.transform.right;
            
            posv = Vec::add(posv, Vec::scale(right, movement.x));
            posv = Vec::add(posv, Vec::scale(front, movement.y));
            posv.z += movement.z;
        }

        if (params.lookat) {
            Vec3 lookAt(0.f, 0.f, 0.f);
            Vec3 toLookAt = Vec::subtract(lookAt, posv);
            Vec3 tmpUp = UP_AXIS;
            Vec3 right = Vec::cross(toLookAt, tmpUp);
            if (!Vec::normalizeSafe(right)) {
                right = RIGHT_AXIS;
            }
            Vec3 up = Vec::normalize(Vec::cross(right, toLookAt));
            Vec3 front = Vec::normalize(Vec::cross(up, right));
            camera.transform.front = front;
            camera.transform.right = right;
            camera.transform.up = up;

        } else {
            identity3x3(camera.transform);
        }
        
        // We are left-handed, negate front for opengl
        camera.renderMatrix.col2 = Vec::negate(camera.transform.front);
        camera.renderMatrix.col0 = camera.transform.right;
        camera.renderMatrix.col1 = camera.transform.up;
        camera.renderMatrix.col3 = camera.transform.pos;
        camera.renderMatrix.bottomRow0 = 0.f;
        camera.renderMatrix.bottomRow1 = 0.f;
        camera.renderMatrix.bottomRow2 = 0.f;
        camera.renderMatrix.bottomRow3 = 1.f;
        
        if (!Vec::inverse(camera.renderMatrix)) {
            identity4x4(camera.transform);
            // Render identity
            // right
            camera.renderMatrix.col0 = Vec3(1.f, 0.f, 0.f);
            // up
            camera.renderMatrix.col1 = Vec3(0.f, 0.f, 1.f);
            // front
            camera.renderMatrix.col2 = Vec3(0.f, -1.f, 0.f);
            // pos
            camera.renderMatrix.col3 = Vec3(0.f, 0.f, 0.f);
        }
    }
};

#endif // __WASTELADNS_CAMERA_H__
