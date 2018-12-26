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

const char * texturedVShaderStr = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec2 uvCoords;
layout(location = 2) in vec3 normal;
layout (std140) uniform PerScene {
    mat4 projectionMatrix;
    mat4 viewMatrix;
};
layout (std140) uniform PerGroup {
    mat4 worldView[64];
    vec4 bgcolor;
};
out FragOut {
    vec3 worldPos;
    vec3 normal;
    vec2 uv;
} fragOut;

void main() {
    mat4 worldView = worldView[gl_InstanceID];
    vec4 worldPos = worldView * vec4(vertexPosition_modelspace,1.f);
    vec4 worldNormal = worldView * vec4(normal,0.f);
    mat4 vp = projectionMatrix * viewMatrix;
    gl_Position = vp * worldPos;
    fragOut.uv = uvCoords;
    fragOut.normal = worldNormal.xyz;
    fragOut.worldPos = worldPos.xyz;
    
    fragOut.uv = uvCoords;
}
)";

const char * texturedPShaderStr = R"(
#version 330 core
in FragOut {
    vec3 worldPos;
    vec3 normal;
    vec2 uv;
} fragIn;
layout(location = 0) out vec4 FragColor;

uniform sampler2D albedoTex;

void main(){

    vec3 lightPos = vec3(0.f, 0.f, 100.f);
    
    vec3 albedo = texture(albedoTex, fragIn.uv).rgb;
    vec3 lightDir = normalize(lightPos - fragIn.worldPos);
    vec3 normal = normalize(fragIn.normal);
    
    float diff = max(dot(lightDir, normal), 0.f);
    vec3 diffuse = diff * albedo;
    vec3 ambient = 0.5f * albedo;
    
    FragColor = vec4(diffuse + ambient, 1.f);

}
)";

#endif // __WASTELADNS_SHADERS_GLFW_H__
