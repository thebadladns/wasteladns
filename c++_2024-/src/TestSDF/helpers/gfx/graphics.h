#ifndef __WASTELADNS_GFX_H__
#define __WASTELADNS_GFX_H__

#include "render_hardware_interface.h"

// shaders
namespace gfx {
namespace shaders {
struct VS_src {
    const char* name;
    const char* src;
};
struct PS_src {
    const char* name;
    const char* src;
};
} // shaders
} // gfx
#if __DX11
#include "shader_src_dx11/shader_output_dx11.h"
#define POPULATE_VSSHADER_PARAMS(vs_params, shader) \
        vs_params.shader_name = shader::name; \
        vs_params.shader_src = shader::g_VS; \
        vs_params.shader_length = countof(shader::g_VS); \
        __DEBUGDEF(vs_params.srcFile = shader::srcFile;) \
        __DEBUGDEF(vs_params.binFile = shader::binFile;)
#define POPULATE_PSSHADER_PARAMS(ps_params, shader) \
        ps_params.shader_name = shader::name; \
        ps_params.shader_src = shader::g_PS; \
        ps_params.shader_length = countof(shader::g_PS); \
        __DEBUGDEF(ps_params.srcFile = shader::srcFile;) \
        __DEBUGDEF(ps_params.binFile = shader::binFile;)
#elif __GL33
#include "shader_src_gl33/shader_output_gl33.h"
#define POPULATE_SHADER_PARAMS(params, shader) \
        params.shader_name = shader::name; \
        params.shader_src = shader::src; \
        params.shader_length = (u32)strlen(shader::src); \
        __DEBUGDEF(params.srcFile = shader::srcFile;) \
        __DEBUGDEF(params.binFile = shader::binFile;)
#define POPULATE_VSSHADER_PARAMS(vs_params, shader) POPULATE_SHADER_PARAMS(vs_params, shader)
#define POPULATE_PSSHADER_PARAMS(ps_params, shader) POPULATE_SHADER_PARAMS(ps_params, shader)
#endif


// other helpers
namespace gfx {

// Convenience shapes
struct UntexturedCube {
    float3 vertices[24];
    u16 indices[36];
};
struct UntexturedSphere {
    float3 vertices[42];
    u16 indices[240];
};
struct ColoredVertex {
    float3 pos;
    u32 color;
};
struct ColoredCube {
    ColoredVertex vertices[24];
    u16 indices[36];
};
struct ColoredSphere {
    ColoredVertex vertices[42];
    u16 indices[240];
};

void calculate_tangents(float3& t, float3& b, const float3& vbl, const float3& vtr, const float3& vtl) {
    const float2 bl = { 0.f, 0.f }, tr = { 1.f, 1.f }, tl = { 0.f, 1.f };
    float3 worldH = math::subtract(vbl, vtl);
    float3 worldV = math::subtract(vtr, vtl);
    float2 uvV = math::subtract(bl, tl);
    float2 uvH = math::subtract(tr, tl);
    f32 invdet = 1.0f / (uvV.x * uvH.y - uvH.x * uvV.y);
    t.x = invdet * (uvH.y * worldH.x - uvV.y * worldV.x);
    t.y = invdet * (uvH.y * worldH.y - uvV.y * worldV.y);
    t.z = invdet * (uvH.y * worldH.z - uvV.y * worldV.z);
    t = math::normalize(t);
    b.x = invdet * (-uvH.x * worldH.x + uvV.x * worldV.x);
    b.y = invdet * (-uvH.x * worldH.y + uvV.x * worldV.y);
    b.z = invdet * (-uvH.x * worldH.z + uvV.x * worldV.z);
    b = math::normalize(b);
}

void create_cube_coords(
uintptr_t v, ptrdiff_t stride, u16* i, const float3 scale, const float3 offset) {
    const f32 pos_x = scale.x + offset.x; const f32 neg_x = -scale.x + offset.x;
    const f32 pos_y = scale.y + offset.y; const f32 neg_y = -scale.y + offset.y;
    const f32 pos_z = scale.z + offset.z; const f32 neg_z = -scale.z + offset.z;
    // +y quad
    *(float3*)(v + stride * 0) = { pos_x, pos_y, neg_z };
    *(float3*)(v + stride * 1) = { neg_x, pos_y, neg_z };
    *(float3*)(v + stride * 2) = { neg_x, pos_y, pos_z };
    *(float3*)(v + stride * 3) = { pos_x, pos_y, pos_z };
    // -y quad
    *(float3*)(v + stride * 4) = { neg_x, neg_y, neg_z };
    *(float3*)(v + stride * 5) = { pos_x, neg_y, neg_z };
    *(float3*)(v + stride * 6) = { pos_x, neg_y, pos_z };
    *(float3*)(v + stride * 7) = { neg_x, neg_y, pos_z };
    // +x quad
    *(float3*)(v + stride * 8) = { pos_x, neg_y, neg_z };
    *(float3*)(v + stride * 9) = { pos_x, pos_y, neg_z };
    *(float3*)(v + stride * 10) = { pos_x, pos_y, pos_z };
    *(float3*)(v + stride * 11) = { pos_x, neg_y, pos_z };
    // -x quad
    *(float3*)(v + stride * 12) = { neg_x, pos_y, neg_z };
    *(float3*)(v + stride * 13) = { neg_x, neg_y, neg_z };
    *(float3*)(v + stride * 14) = { neg_x, neg_y, pos_z };
    *(float3*)(v + stride * 15) = { neg_x, pos_y, pos_z };
    // +z quad
    *(float3*)(v + stride * 16) = { neg_x, neg_y, pos_z };
    *(float3*)(v + stride * 17) = { pos_x, neg_y, pos_z };
    *(float3*)(v + stride * 18) = { pos_x, pos_y, pos_z };
    *(float3*)(v + stride * 19) = { neg_x, pos_y, pos_z };
    // -z quad
    *(float3*)(v + stride * 20) = { pos_x, pos_y, neg_z };
    *(float3*)(v + stride * 21) = { pos_x, neg_y, neg_z };
    *(float3*)(v + stride * 22) = { neg_x, neg_y, neg_z };
    *(float3*)(v + stride * 23) = { neg_x, pos_y, neg_z };

    i[0] = 2;   i[1] = 1;   i[2] = 0;   i[3] = 2;   i[4] = 0;   i[5] = 3;   // +y tris
    i[6] = 6;   i[7] = 5;   i[8] = 4;   i[9] = 6;   i[10] = 4;  i[11] = 7;  // -y tris
    i[12] = 10; i[13] = 9;  i[14] = 8;  i[15] = 10; i[16] = 8;  i[17] = 11; // +x tris
    i[18] = 14; i[19] = 13; i[20] = 12; i[21] = 14; i[22] = 12; i[23] = 15; // -x tris
    i[24] = 18; i[25] = 17; i[26] = 16; i[27] = 18; i[28] = 16; i[29] = 19; // +z tris
    i[30] = 22; i[31] = 21; i[32] = 20; i[33] = 22; i[34] = 20; i[35] = 23; // -z tris
}
void create_colored_cube_coords(ColoredCube& cube, const Color32 color, const float3 scale, const float3 offset) {
    create_cube_coords((uintptr_t)cube.vertices, sizeof(cube.vertices[0]), cube.indices, scale, offset);
    const u32 c = color.ABGR();
    for (u32 i = 0; i < 24; i++) { cube.vertices[i].color = c; }
}
void create_sphere_coords(uintptr_t v, ptrdiff_t stride, u16* i, const f32 radius) {
    *(float3*)(v + stride * 0) =   float3( 0.0000f,   -1.0000f,    0.0000f);
    *(float3*)(v + stride * 1) =   float3( 0.7227f,   -0.4453f,   -0.5254f);
    *(float3*)(v + stride * 2) =   float3(-0.2754f,   -0.4453f,   -0.8496f);
    *(float3*)(v + stride * 3) =   float3(-0.8926f,   -0.4453f,    0.0000f);
    *(float3*)(v + stride * 4) =   float3(-0.2754f,   -0.4453f,    0.8496f);
    *(float3*)(v + stride * 5) =   float3( 0.7227f,   -0.4453f,    0.5254f);
    *(float3*)(v + stride * 6) =   float3( 0.2754f,    0.4453f,   -0.8496f);
    *(float3*)(v + stride * 7) =   float3(-0.7227f,    0.4453f,   -0.5254f);
    *(float3*)(v + stride * 8) =   float3(-0.7227f,    0.4453f,    0.5254f);
    *(float3*)(v + stride * 9) =   float3( 0.2754f,    0.4453f,    0.8496f);
    *(float3*)(v + stride * 10) =  float3( 0.8926f,    0.4453f,    0.0000f);
    *(float3*)(v + stride * 11) =  float3( 0.0000f,    1.0000f,    0.0000f);
    *(float3*)(v + stride * 12) =  float3(-0.1621f,   -0.8496f,   -0.4980f);
    *(float3*)(v + stride * 13) =  float3( 0.4238f,   -0.8496f,   -0.3086f);
    *(float3*)(v + stride * 14) =  float3( 0.2617f,   -0.5254f,   -0.8086f);
    *(float3*)(v + stride * 15) =  float3( 0.8496f,   -0.5254f,    0.0000f);
    *(float3*)(v + stride * 16) =  float3( 0.4238f,   -0.8496f,    0.3086f);
    *(float3*)(v + stride * 17) =  float3(-0.5254f,   -0.8496f,    0.0000f);
    *(float3*)(v + stride * 18) =  float3(-0.6875f,   -0.5254f,   -0.4980f);
    *(float3*)(v + stride * 19) =  float3(-0.1621f,   -0.8496f,    0.4980f);
    *(float3*)(v + stride * 20) =  float3(-0.6875f,   -0.5254f,    0.4980f);
    *(float3*)(v + stride * 21) =  float3( 0.2617f,   -0.5254f,    0.8086f);
    *(float3*)(v + stride * 22) =  float3( 0.9492f,    0.0000f,   -0.3086f);
    *(float3*)(v + stride * 23) =  float3( 0.9492f,    0.0000f,    0.3086f);
    *(float3*)(v + stride * 24) =  float3( 0.0000f,    0.0000f,   -1.0000f);
    *(float3*)(v + stride * 25) =  float3( 0.5859f,    0.0000f,   -0.8086f);
    *(float3*)(v + stride * 26) =  float3(-0.9492f,    0.0000f,   -0.3086f);
    *(float3*)(v + stride * 27) =  float3(-0.5859f,    0.0000f,   -0.8086f);
    *(float3*)(v + stride * 28) =  float3(-0.5859f,    0.0000f,    0.8086f);
    *(float3*)(v + stride * 29) =  float3(-0.9492f,    0.0000f,    0.3086f);
    *(float3*)(v + stride * 30) =  float3( 0.5859f,    0.0000f,    0.8086f);
    *(float3*)(v + stride * 31) =  float3( 0.0000f,    0.0000f,    1.0000f);
    *(float3*)(v + stride * 32) =  float3( 0.6875f,    0.5254f,   -0.4980f);
    *(float3*)(v + stride * 33) =  float3(-0.2617f,    0.5254f,   -0.8086f);
    *(float3*)(v + stride * 34) =  float3(-0.8496f,    0.5254f,    0.0000f);
    *(float3*)(v + stride * 35) =  float3(-0.2617f,    0.5254f,    0.8086f);
    *(float3*)(v + stride * 36) =  float3( 0.6875f,    0.5254f,    0.4980f);
    *(float3*)(v + stride * 37) =  float3( 0.1621f,    0.8496f,   -0.4980f);
    *(float3*)(v + stride * 38) =  float3( 0.5254f,    0.8496f,    0.0000f);
    *(float3*)(v + stride * 39) =  float3(-0.4238f,    0.8496f,   -0.3086f);
    *(float3*)(v + stride * 40) =  float3(-0.4238f,    0.8496f,    0.3086f);
    *(float3*)(v + stride * 41) =  float3( 0.1621f,    0.8496f,    0.4980f);
    for (u32 i = 0; i < 42; i++) {
        float3* currv = (float3*)(v + stride * i);
        *currv = math::scale(*currv, radius);
    }
    
    i[ 0] =   0;   i[1] =   13; i[2] =    12;
    i[ 3] =   1;   i[4] =   13; i[5] =    15;
    i[ 6] =   0;   i[7] =   12; i[8] =    17;
    i[ 9] =   0;   i[10] =  17; i[11] =   19;
    i[ 12] =  0;   i[13] =  19; i[14] =   16;
    i[ 15] =  1;   i[16] =  15; i[17] =   22;
    i[ 18] =  2;   i[19] =  14; i[20] =   24;
    i[ 21] =  3;   i[22] =  18; i[23] =   26;
    i[ 24] =  4;   i[25] =  20; i[26] =   28;
    i[ 27] =  5;   i[28] =  21; i[29] =   30;
    i[ 30] =  1;   i[31] =  22; i[32] =   25;
    i[ 33] =  2;   i[34] =  24; i[35] =   27;
    i[ 36] =  3;   i[37] =  26; i[38] =   29;
    i[ 39] =  4;   i[40] =  28; i[41] =   31;
    i[ 42] =  5;   i[43] =  30; i[44] =   23;
    i[ 45] =  6;   i[46] =  32; i[47] =   37;
    i[ 48] =  7;   i[49] =  33; i[50] =   39;
    i[ 51] =  8;   i[52] =  34; i[53] =   40;
    i[ 54] =  9;   i[55] =  35; i[56] =   41;
    i[ 57] =  10;  i[58] =  36; i[59] =   38;
    i[ 60] =  38;  i[61] =  41; i[62] =   11;
    i[ 63] =  38;  i[64] =  36; i[65] =   41;
    i[ 66] =  36;  i[67] =  9;  i[68] =   41;
    i[ 69] =  41;  i[70] =  40; i[71] =   11;
    i[ 72] =  41;  i[73] =  35; i[74] =   40;
    i[ 75] =  35;  i[76] =  8;  i[77] =   40;
    i[ 78] =  40;  i[79] =  39; i[80] =   11;
    i[ 81] =  40;  i[82] =  34; i[83] =   39;
    i[ 84] =  34;  i[85] =  7;  i[86] =   39;
    i[ 87] =  39;  i[88] =  37; i[89] =   11;
    i[ 90] =  39;  i[91] =  33; i[92] =   37;
    i[ 93] =  33;  i[94] =  6;  i[95] =   37;
    i[ 96] =  37;  i[97] =  38; i[98] =   11;
    i[ 99] =  37;  i[100] = 32; i[101] =  38;
    i[ 102] = 32;  i[103] = 10; i[104] =  38;
    i[ 105] = 23;  i[106] = 36; i[107] =  10;
    i[ 108] = 23;  i[109] = 30; i[110] =  36;
    i[ 111] = 30;  i[112] = 9;  i[113] =  36;
    i[ 114] = 31;  i[115] = 35; i[116] =  9;
    i[ 117] = 31;  i[118] = 28; i[119] =  35;
    i[ 120] = 28;  i[121] = 8;  i[122] =  35;
    i[ 123] = 29;  i[124] = 34; i[125] =  8;
    i[ 126] = 29;  i[127] = 26; i[128] =  34;
    i[ 129] = 26;  i[130] = 7;  i[131] =  34;
    i[ 132] = 27;  i[133] = 33; i[134] =  7;
    i[ 135] = 27;  i[136] = 24; i[137] =  33;
    i[ 138] = 24;  i[139] = 6;  i[140] =  33;
    i[ 141] = 25;  i[142] = 32; i[143] =  6;
    i[ 144] = 25;  i[145] = 22; i[146] =  32;
    i[ 147] = 22;  i[148] = 10; i[149] =  32;
    i[ 150] = 30;  i[151] = 31; i[152] =  9;
    i[ 153] = 30;  i[154] = 21; i[155] =  31;
    i[ 156] = 21;  i[157] = 4;  i[158] =  31;
    i[ 159] = 28;  i[160] = 29; i[161] =  8;
    i[ 162] = 28;  i[163] = 20; i[164] =  29;
    i[ 165] = 20;  i[166] = 3;  i[167] =  29;
    i[ 168] = 26;  i[169] = 27; i[170] =  7;
    i[ 171] = 26;  i[172] = 18; i[173] =  27;
    i[ 174] = 18;  i[175] = 2;  i[176] =  27;
    i[ 177] = 24;  i[178] = 25; i[179] =  6;
    i[ 180] = 24;  i[181] = 14; i[182] =  25;
    i[ 183] = 14;  i[184] = 1;  i[185] =  25;
    i[ 186] = 22;  i[187] = 23; i[188] =  10;
    i[ 189] = 22;  i[190] = 15; i[191] =  23;
    i[ 192] = 15;  i[193] = 5;  i[194] =  23;
    i[ 195] = 16;  i[196] = 21; i[197] =  5;
    i[ 198] = 16;  i[199] = 19; i[200] =  21;
    i[ 201] = 19;  i[202] = 4;  i[203] =  21;
    i[ 204] = 19;  i[205] = 20; i[206] =  4;
    i[ 207] = 19;  i[208] = 17; i[209] =  20;
    i[ 210] = 17;  i[211] = 3;  i[212] =  20;
    i[ 213] = 17;  i[214] = 18; i[215] =  3;
    i[ 216] = 17;  i[217] = 12; i[218] =  18;
    i[ 219] = 12;  i[220] = 2;  i[221] =  18;
    i[ 222] = 15;  i[223] = 16; i[224] =  5;
    i[ 225] = 15;  i[226] = 13; i[227] =  16;
    i[ 228] = 13;  i[229] = 0;  i[230] =  16;
    i[ 231] = 12;  i[232] = 14; i[233] =  2;
    i[ 234] = 12;  i[235] = 13; i[236] =  14;
    i[ 237] = 13;  i[238] = 1;  i[239] =  14;
}

void create_colored_sphere_coords(ColoredSphere& s, const f32 radius, Color32 color) {
    create_sphere_coords((uintptr_t)s.vertices, sizeof(s.vertices[0]), s.indices, radius);
    const u32 c = color.ABGR();
    for (u32 i = 0; i < 42; i++) { s.vertices[i].color = c; }
}

void create_indexed_vertex_buffer_from_untextured_mesh(
gfx::rhi::RscIndexedVertexBuffer& buffer, float3* vertices, u32 vertices_count,
u16* indices, u32 indices_count) {
    gfx::rhi::IndexedVertexBufferDesc bufferParams;
    bufferParams.vertexData = vertices;
    bufferParams.indexData = indices;
    bufferParams.vertexSize = sizeof(f32) * 3 * vertices_count;
    bufferParams.vertexCount = vertices_count;
    bufferParams.indexSize = sizeof(u16) * indices_count;
    bufferParams.indexCount = indices_count;
    bufferParams.memoryUsage = gfx::rhi::BufferMemoryUsage::GPU;
    bufferParams.accessType = gfx::rhi::BufferAccessType::GPU;
    bufferParams.indexType = gfx::rhi::BufferItemType::U16;
    bufferParams.type = gfx::rhi::BufferTopologyType::Triangles;
    rhi::VertexAttribDesc attribs[] = {
        rhi::make_vertexAttribDesc(
            "POSITION", 0, sizeof(float3), rhi::BufferAttributeFormat::R32G32B32_FLOAT)
    };
    gfx::rhi::create_indexed_vertex_buffer(buffer, bufferParams, attribs, countof(attribs));
}
void create_indexed_vertex_buffer_from_colored_mesh(
gfx::rhi::RscIndexedVertexBuffer& buffer, ColoredVertex* vertices, u32 vertices_count,
u16* indices, u32 indices_count) {
    gfx::rhi::IndexedVertexBufferDesc bufferParams;
    bufferParams.vertexData = vertices;
    bufferParams.indexData = indices;
    bufferParams.vertexSize = sizeof(ColoredVertex) * vertices_count;
    bufferParams.vertexCount = vertices_count;
    bufferParams.indexSize = sizeof(u16) * indices_count;
    bufferParams.indexCount = indices_count;
    bufferParams.memoryUsage = gfx::rhi::BufferMemoryUsage::GPU;
    bufferParams.accessType = gfx::rhi::BufferAccessType::GPU;
    bufferParams.indexType = gfx::rhi::BufferItemType::U16;
    bufferParams.type = gfx::rhi::BufferTopologyType::Triangles;
    rhi::VertexAttribDesc attribs[] = {
        rhi::make_vertexAttribDesc(
            "POSITION", offsetof(ColoredVertex, pos), sizeof(ColoredVertex),
            rhi::BufferAttributeFormat::R32G32B32_FLOAT),
        rhi::make_vertexAttribDesc(
            "COLOR", offsetof(ColoredVertex, color), sizeof(ColoredVertex),
            rhi::BufferAttributeFormat::R8G8B8A8_UNORM)
    };
    gfx::rhi::create_indexed_vertex_buffer(buffer, bufferParams, attribs, countof(attribs));
}

struct ShaderDesc {
    rhi::VertexShaderRuntimeCompileParams vs_params;
    rhi::PixelShaderRuntimeCompileParams ps_params;
    const rhi::CBufferBindingDesc* bufferBindings;
    const rhi::TextureBindingDesc* textureBindings;
    u32 bufferBinding_count;
    u32 textureBinding_count;
};

void compile_shader(rhi::RscShaderSet& shader, const ShaderDesc& desc) {
    gfx::rhi::RscVertexShader vs;
    gfx::rhi::RscPixelShader ps;
    gfx::rhi::ShaderResult pixelResult;
    gfx::rhi::ShaderResult vertexResult;
    vertexResult = gfx::rhi::create_shader_vs(vs, desc.vs_params);
    if (!vertexResult.compiled) {
        io::debuglog("%s: %s\n", desc.vs_params.shader_name, vertexResult.error);
        return;
    }
    pixelResult = gfx::rhi::create_shader_ps(ps, desc.ps_params);
    if (!pixelResult.compiled) {
        io::debuglog("%s: %s\n", desc.ps_params.shader_name, pixelResult.error);
        return;
    }
    gfx::rhi::ShaderResult result =
        gfx::rhi::create_shader_set(
            shader,
            { vs, ps, desc.bufferBindings, desc.textureBindings,
              desc.bufferBinding_count, desc.textureBinding_count });
    if (!result.compiled) {
        io::debuglog("Linking %s & %s: %s\n",
                     desc.vs_params.shader_name, desc.ps_params.shader_name, result.error);
    }
}

} // gfx

#endif // __WASTELADNS_GFX_H__
