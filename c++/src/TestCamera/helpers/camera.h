#ifndef __WASTELADNS_CAMERA_H__
#define __WASTELADNS_CAMERA_H__

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
		const f64 xMax = params.near * tan(params.fov * 2.f * Angle::d2r<f64>);
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
    
    struct ProjectionConfig {
        Camera::OrthoParams orthoParams;
        Camera::FrustumParams frustumParams;
        Math3D::Transform32 orthoTransformCM;
        Math3D::Transform64 frustumTransformCM;
    };
    
    struct Instance {
        Vec2 inputDirWS = {};
        f32 matrixCM[16];
    };
    
    struct UpdateCameraParams {
        Instance* instance;
        bool input_up_down;
        bool input_up_pressed;
        bool input_up_released;
        bool input_down_down;
        bool input_down_pressed;
        bool input_down_released;
        bool input_left_down;
        bool input_left_pressed;
        bool input_left_released;
        bool input_right_down;
        bool input_right_pressed;
        bool input_right_released;
    };
    void UpdateCamera(UpdateCameraParams& params) {
        
        Instance& camera = *params.instance;
        Vec3& pos = *((Vec3*)&camera.matrixCM[12]);
        Vec2& inputDirWS = camera.inputDirWS;
        
        inputDirWS.y -= 3 * params.input_down_pressed + params.input_down_down - params.input_down_released;
        inputDirWS.y += 3 * params.input_up_pressed + params.input_up_down - params.input_up_released;
        inputDirWS.x -= 3 * params.input_left_pressed + params.input_left_down - params.input_left_released;
        inputDirWS.x += 3 * params.input_right_pressed + params.input_right_down - params.input_right_released;
        inputDirWS.x = Math::clamp(inputDirWS.x, -1.f, 1.f);
        inputDirWS.y = Math::clamp(inputDirWS.y, -1.f, 1.f);
        
        Vec2 movement = inputDirWS;
        if (Vec::normalizeSafe(movement)) {
            f32 speed = 4.f;
            movement = Vec::scale(movement, speed);
            pos.x -= movement.x;
            pos.y -= movement.y;
        }
    }
};

#endif // __WASTELADNS_CAMERA_H__
