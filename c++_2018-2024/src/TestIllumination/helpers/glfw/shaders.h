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

const char * pixelShaderStr = R"(
    #version 330 core
    layout(location = 0) out vec4 diffuseColor;
    in vec4 fragmentColor;
    void main(){
        diffuseColor = fragmentColor;
    }
)";

const char * geometryPassVShaderStr = R"(
#version 330 core
layout(location = 0) in vec3 posMS;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normalMS;
layout(location = 3) in vec3 tangentMS;
layout(location = 4) in vec3 bitangentMS;
layout (std140) uniform PerScene {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
};
layout (std140) uniform PerGroup {
    mat4 modelMatrices[64];
    float depthTS;
};
out FragOut {
    vec3 posWS;
    vec3 normalWS;
    vec3 viewPosTS;
    vec3 posTS;
    vec2 uv;
    mat3 tbnMatrix;
} fragOut;

void main() {
    mat4 modelMatrix = modelMatrices[gl_InstanceID];
    mat4 vp = projectionMatrix * viewMatrix;
    vec4 posWS = modelMatrix * vec4(posMS, 1.f);
    vec3 normalWS = normalize(modelMatrix * vec4(normalMS, 0.f)).xyz;
    vec4 tangentWS = normalize(modelMatrix * vec4(tangentMS, 0.f));
    vec4 bitangentWS = normalize(modelMatrix * vec4(bitangentMS, 0.f));
    fragOut.tbnMatrix = mat3(tangentWS.xyz, bitangentWS.xyz, normalWS.xyz);
    mat3 invtbn = transpose(fragOut.tbnMatrix);
    fragOut.viewPosTS = invtbn * viewPosWS;
    fragOut.posTS = invtbn * posWS.xyz;
    fragOut.posWS = posWS.xyz;
    fragOut.normalWS = normalWS.xyz;
    fragOut.uv = uv;

    gl_Position = vp * posWS;
}
)";

const char * geometryPassPShaderStr = R"(
#version 330 core
layout(location = 0) out vec3 gPos;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gDiffuse;
layout (std140) uniform PerGroup {
    mat4 modelMatrices[64];
    float depthTS;
};
in FragOut {
    vec3 posWS;
    vec3 normalWS;
    vec3 viewPosTS;
    vec3 posTS;
    vec2 uv;
    mat3 tbnMatrix;
} fragIn;

uniform sampler2D texDiffuse;
uniform sampler2D texNormal;
uniform sampler2D texDepth;


vec2 binaryPOM(vec2 uv, vec2 uvextents, float mindepth, float maxdepth) {
    vec2 curruv = uv;
    for (int sampleCount = 0; sampleCount < 16; sampleCount++) {
        float depth = (mindepth + maxdepth) * 0.5f;
        curruv = uv - depth * uvextents;
        float currsampleddepth = texture(texDepth, curruv).r;

        if (currsampleddepth < depth) {
            maxdepth = depth;
        } else {
            mindepth = depth;
        }
    }

    return curruv;
}

vec2 POM(vec2 uv, vec3 viewDir) {
    float layers = mix(32.f, 8.f, abs(dot(vec3(0.f, 0.f, 1.f), viewDir)));
    float depthstep = 1.f / layers;
    vec2 uvextents = viewDir.xy * depthTS / viewDir.z;
    vec2 uvstep = uvextents * depthstep;

    vec2 curruv = uv;
    vec2 nextuv = uv, prevuv = uv;
    float currdepth = 0.f;
    float nextdepth = currdepth, prevdepth = currdepth;
    float currsampleddepth = 0.1f;
    float prevsampleddepth = 0.f;
    do {
        prevuv = curruv;
        prevdepth = currdepth;
        prevsampleddepth = currsampleddepth;
        currdepth = nextdepth;
        curruv = nextuv;
        currsampleddepth = texture(texDepth, curruv).r;
        nextuv -= uvstep;
        nextdepth += depthstep;

    } while (currdepth < currsampleddepth);

    return binaryPOM(uv, uvextents, prevdepth, currdepth);
//    float prevdepthdelta = prevsampleddepth - prevdepth;
//    float currdepthdelta = currsampleddepth - currdepth;
//    float t = prevdepthdelta / (currdepthdelta - prevdepthdelta);
//    vec2 outuv = prevuv * t + curruv * (1 - t);
//    return outuv;
}

void main() {

    vec3 viewDir = normalize(fragIn.viewPosTS - fragIn.posTS);
    vec2 uv = fragIn.uv;
    uv = POM(fragIn.uv,  viewDir);
    
    if(uv.x > 1.f || uv.y > 1.f || uv.x < 0.f || uv.y < 0.f) {
        discard;
    }
    vec4 albedo = texture(texDiffuse, uv).rgba;
    if (albedo.a < 0.9) {
        discard;
    }

    gDiffuse = albedo.xyz;
    gPos = fragIn.posWS;
    vec3 normal = texture(texNormal, uv).rgb;
    normal = normal * 2.f - 1.f;
    normal = normalize(fragIn.tbnMatrix * normal);
    gNormal = normal;
}
)";

const char * quadVShaderStr = R"(
#version 330 core
layout(location = 0) in vec2 in_posCS;
layout(location = 1) in vec2 in_uv;
out vec2 p_uv;

void main() {
    gl_Position = vec4(in_posCS, 0.f, 1.f);
    p_uv = in_uv;
}
)";

const char * quadPShaderStr = R"(
#version 330 core
in vec2 p_uv;
layout(location = 0) out vec4 FragColor;
layout (std140) uniform PerScene {
    vec4 viewPosWS;
    vec4 lightPosWS;
};

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;

void main() {
    vec3 lightWS = lightPosWS.xyz;
    vec3 albedo = texture(gDiffuse, p_uv).rgb;
    vec3 posWS = texture(gPos, p_uv).rgb;
    vec3 lightDir = normalize(lightWS - posWS);
    vec3 normalWS = texture(gNormal, p_uv).rgb;
    
    // Aggressive gooch-like shading
    vec3 viewWS = viewPosWS.xyz;
    vec3 viewDirWS = normalize(viewWS - posWS);
    float diff = dot(lightDir, normalWS);
    float t = ( diff + 1.f ) * 0.5f;
    vec3 r = 2.f * diff * normalWS - lightDir;
    float s = clamp(100.f * dot(r, viewDirWS) - 97.f, 0.f, 0.8f);
    vec3 ccool = vec3(0.f, 0.f, 0.3f) * albedo;
    vec3 cwarm = clamp(vec3(1.2f, 1.2f, 1.f) * albedo, 0.f, 1.f);
    vec3 diffuse = 0.7f * diff * albedo;
    vec3 ambient = 0.3f * albedo;

    FragColor = vec4(s * vec3(1.f,1.f,1.f) + (1 - s)*(t*cwarm + (1 - t)*ccool), 1.f);
}
)";


#endif // __WASTELADNS_SHADERS_GLFW_H__
