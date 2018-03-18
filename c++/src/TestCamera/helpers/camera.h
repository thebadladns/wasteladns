#ifndef __WASTELADNS_CAMERA_H__
#define __WASTELADNS_CAMERA_H__

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
    
    struct Instance {
        Vec::Transform transform;
        Mat4 renderMatrix;
    };
    
    struct UpdateParams {
        struct MovementType {
            enum Enum { manual, lookat, orbit };
        };
        Instance* instance;
        const DirControl* dirControl;
        const Vec::Transform* playerTransform;
        Vec3 lookat;
        Vec3 orbit_center, orbit_offsetLS;
        MovementType::Enum movementType = MovementType::manual;
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
            Vec3 orbit_offsetWS = Vec::transform3x3(*params.playerTransform, params.orbit_offsetLS);
            camera.transform.pos = Vec::add(params.orbit_center, orbit_offsetWS);
            Vec3 lookatDir = Vec::subtract(params.orbit_center, camera.transform.pos);
            transformFromFront(camera.transform, lookatDir);
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
