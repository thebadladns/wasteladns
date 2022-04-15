#ifndef __WASTELADNS_SHADERS_GLFW_H__
#define __WASTELADNS_SHADERS_GLFW_H__

const char * coloredVertexShaderStr = R"(
    #version 330 core
    layout(location = 0) in vec3 vertexPosition_modelspace;
    layout(location = 1) in vec4 color;
    layout (std140) uniform PerGroup {
        mat4 MVP;
    };
    out vec4 fragmentColor;
    void main() {
        gl_Position = MVP * vec4(vertexPosition_modelspace.xyz, 1.f);
        fragmentColor = color.rgba;
    }
)";


const char * defaultVertexShaderStr = R"(
    #version 330 core
    layout(location = 0) in vec3 posMS;
	layout (std140) uniform PerScene {
		mat4 projectionMatrix;
		mat4 viewMatrix;
		vec3 viewPosWS;
        float padding;
	};
	layout (std140) uniform PerGroup {
		mat4 modelMatrix;
		vec4 groupColor;
	};
	layout (std140) uniform PerInstance {
		mat4 instanceMatrices[256];
	};
    out vec4 fragmentColor;
    void main() {
		mat4 vp = projectionMatrix * viewMatrix;
		vec4 posWS = modelMatrix * vec4(posMS, 1.f);
		gl_Position = vp * posWS;
		fragmentColor = groupColor;
    }
)";
const char* defaultInstancedVertexShaderStr = R"(
    #version 330 core
    layout(location = 0) in vec3 posMS;
	layout (std140) uniform PerScene {
		mat4 projectionMatrix;
		mat4 viewMatrix;
		vec3 viewPosWS;
        float padding;
	};
	layout (std140) uniform PerGroup {
		mat4 modelMatrix;
		vec4 groupColor;
	};
	layout (std140) uniform PerInstance {
		mat4 instanceMatrices[256];
	};
    out vec4 fragmentColor;
    void main() {
        mat4 instanceMatrix = instanceMatrices[gl_InstanceID];
        mat4 mm = instanceMatrix * modelMatrix;
		mat4 vp = projectionMatrix * viewMatrix;
		vec4 posWS = mm * vec4(posMS, 1.f);
		gl_Position = vp * posWS;
		fragmentColor = groupColor;
    }
)";

const char * defaultPixelShaderStr = R"(
    #version 330 core
    layout(location = 0) out vec4 diffuseColor;
    in vec4 fragmentColor;
    void main(){
        diffuseColor = fragmentColor;
    }
)";

#endif // __WASTELADNS_SHADERS_GLFW_H__
