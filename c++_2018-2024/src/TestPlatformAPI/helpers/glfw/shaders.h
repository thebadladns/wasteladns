#ifndef __WASTELADNS_SHADERS_GLFW_H__
#define __WASTELADNS_SHADERS_GLFW_H__

const char * vertexShaderStr = R"(
    #version 330 core
    layout(location = 0) in vec3 vertexPosition_modelspace;
    layout (std140) uniform PerScene {
        mat4 projectionMatrix;
        mat4 viewMatrix;
    };
    layout (std140) uniform PerGroup {
        mat4 worldView[64];
        vec4 bgcolor;
    };
    out vec4 fragmentColor;
    void main() {
        mat4 mvp = projectionMatrix * viewMatrix * worldView[gl_InstanceID];
        gl_Position = mvp * vec4(vertexPosition_modelspace,1.f);
        fragmentColor = bgcolor;
    }
)";

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

const char * pixelShaderStr = R"(
    #version 330 core
    layout(location = 0) out vec4 diffuseColor;
    in vec4 fragmentColor;
    void main(){
        diffuseColor = fragmentColor;
    }
)";

#endif // __WASTELADNS_SHADERS_GLFW_H__
