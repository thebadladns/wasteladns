#ifndef __WASTELADNS_RENDERER_H__
#define __WASTELADNS_RENDERER_H__

#ifndef UNITYBUILD
#include <cstring>
#include "renderer_debug.h"
#endif

namespace Renderer {

void generate_matrix_ortho_zneg1to1(float4x4& matrix, const WindowProjection::Config& config) {
    f32* matrixCM = matrix.dataCM;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = 2.f / (config.right - config.left);
    matrixCM[5] = 2.f / (config.top - config.bottom);
    matrixCM[10] = -2.f / (config.far - config.near);
    matrixCM[12] = -(config.right + config.left) / (config.right - config.left);
    matrixCM[13] = -(config.top + config.bottom) / (config.top - config.bottom);
    matrixCM[14] = -(config.far + config.near) / (config.far - config.near);
    matrixCM[15] = 1.f;
}
void generate_matrix_ortho_z0to1(float4x4& matrix, const WindowProjection::Config& config) {
    f32* matrixCM = matrix.dataCM;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = 2.f / (config.right - config.left);
    matrixCM[5] = 2.f / (config.top - config.bottom);
    matrixCM[10] = -1.f / (config.far - config.near);
    matrixCM[12] = -(config.right + config.left) / (config.right - config.left);
    matrixCM[13] = -(config.top + config.bottom) / (config.top - config.bottom);
    matrixCM[14] = -config.near / (config.far - config.near);
    matrixCM[15] = 1.f;
}

// Expects right-handed view matrix, z-coordinate point towards the viewer
void generate_matrix_persp_zneg1to1(float4x4& matrixRHwithYup, const PerspProjection::Config& config) {
    const f32 h = 1.f / Math::tan(config.fov * 0.5f * Math::d2r_f);
    const f32 w = h / config.aspect;
    
    // maps each xyz axis to [-1,1] (left handed, y points up z moves away from the viewer)
    f32 (&matrixCM)[16] = matrixRHwithYup.dataCM;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = w;
    matrixCM[5] = h;
    matrixCM[10] = -(config.far + config.near) / (config.far - config.near);
    matrixCM[11] = -1.f;
    matrixCM[14] = -(2.f * config.far * config.near) / (config.far - config.near);
}
void generate_matrix_persp_z0to1(float4x4& matrixRHwithYup, const PerspProjection::Config& config) {
    const f32 h = 1.f / Math::tan(config.fov * 0.5f * Math::d2r_f);
    const f32 w = h / config.aspect;
    // maps each xyz axis to [0,1] (left handed, y points up z moves away from the viewer)
    f32(&matrixCM)[16] = matrixRHwithYup.dataCM;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = w;
    matrixCM[5] = h;
    matrixCM[10] = config.far / (config.near - config.far);
    matrixCM[11] = -1.f;
    matrixCM[14] = config.far * config.near / (config.near - config.far);
}

void generate_MV_matrix(float4x4& modelview, const Transform& t) {
    
    const float4x4 mRHwithYUp = Math::toEyeSpace(t);

    // Simplified inverse
    // https://www.gamedev.net/forums/topic/647149-d3dxmatrixlookatlh-internally/?tab=comments#comment-5089654

    const f32 tx = -Math::dot(mRHwithYUp.col3, mRHwithYUp.col0);
    const f32 ty = -Math::dot(mRHwithYUp.col3, mRHwithYUp.col1);
    const f32 tz = -Math::dot(mRHwithYUp.col3, mRHwithYUp.col2);

    modelview.dataCM[0] = mRHwithYUp.col0.x;
    modelview.dataCM[4] = mRHwithYUp.col0.y;
    modelview.dataCM[8] = mRHwithYUp.col0.z;

    modelview.dataCM[1] = mRHwithYUp.col1.x;
    modelview.dataCM[5] = mRHwithYUp.col1.y;
    modelview.dataCM[9] = mRHwithYUp.col1.z;

    modelview.dataCM[2] = mRHwithYUp.col2.x;
    modelview.dataCM[6] = mRHwithYUp.col2.y;
    modelview.dataCM[10] = mRHwithYUp.col2.z;

    modelview.dataCM[12] = tx;
    modelview.dataCM[13] = ty;
    modelview.dataCM[14] = tz;

    modelview.dataCM[3] = mRHwithYUp.col0.w;
    modelview.dataCM[7] = mRHwithYUp.col1.w;;
    modelview.dataCM[11] = mRHwithYUp.col2.w;
    modelview.dataCM[15] = mRHwithYUp.col3.w;
}

void calculate_tangents(float3& t, float3& b, const float3& vbl, const float3& vtr, const float3& vtl) {
    const float2 bl = { 0.f, 0.f }, tr = { 1.f, 1.f }, tl = { 0.f, 1.f };
    float3 worldH = Math::subtract(vbl, vtl);
    float3 worldV = Math::subtract(vtr, vtl);
    float2 uvV = Math::subtract(bl, tl);
    float2 uvH = Math::subtract(tr, tl);
    f32 invdet = 1.0f / (uvV.x * uvH.y - uvH.x * uvV.y);
    t.x = invdet * (uvH.y * worldH.x - uvV.y * worldV.x);
    t.y = invdet * (uvH.y * worldH.y - uvV.y * worldV.y);
    t.z = invdet * (uvH.y * worldH.z - uvV.y * worldV.z);
    t = Math::normalize(t);
    b.x = invdet * (-uvH.x * worldH.x + uvV.x * worldV.x);
    b.y = invdet * (-uvH.x * worldH.y + uvV.x * worldV.y);
    b.z = invdet * (-uvH.x * worldH.z + uvV.x * worldV.z);
    b = Math::normalize(b);
}
struct CubeCreateParams {
    f32 width;
    f32 height;
    f32 depth;
};
void create_textured_cube_coords(TexturedCube& c, const CubeCreateParams& params) {

    enum Normals { Y, NY, X, NX, Z, NZ };
    const float3 normals[] = {
          { 0.f, 1.f, 0.f }
        , { 0.f, -1.f, 0.f }
        , { 1.f, 0.f, 0.f }
        , { -1.f, 0.f, 0.f }
        , { 0.f, 0.f, 1.f }
        , { 0.f, 0.f, -1.f }
    };
    const float2
          bl = { 0.f, 0.f }
        , br = { 1.f, 0.f }
        , tr = { 1.f, 1.f }
        , tl = { 0.f, 1.f }
    ;
    f32 w = params.width;
    f32 h = params.height;
    f32 d = params.depth;
    TexturedCube::Vertex* v = c.vertices;

    // vertex and texture coords
    v[0] = { { w, d, -h } }; v[1] = { { -w, d, -h } }; v[2] = { { -w, d, h } }; v[3] = { { w, d, h } };           // +y quad
    v[4] = { { -w, -d, -h } }; v[5] = { { w, -d, -h } }; v[6] = { { w, -d, h } }; v[7] = { { -w, -d, h } };       // -y quad

    v[8] = { { w, -d, -h} }; v[9] = { { w, d, -h } }; v[10] = { { w, d, h } }; v[11] = { { w, -d, h } };          // +x quad
    v[12] = { { -w, d, -h } }; v[13] = { { -w, -d, -h } }; v[14] = { { -w, -d, h } }; v[15] = { { -w, d, h } };   // -x quad

    v[16] = { { -w, -d, h } }; v[17] = { { w, -d, h } }; v[18] = { { w, d, h } }; v[19] = { { -w, d, h } };         // +z quad
    v[20] = { { w, d, -h } }; v[21] = { { w, -d, -h } }; v[22] = { { -w, -d, -h } }; v[23] = { { -w, d, -h } }; // -z quad

    // normals, tangents and bitangents
    float3 n, t, b;
    n = normals[Y]; t = normals[NX]; b = normals[NZ];
    // calculate_tangents(t, b, v[3].pos, v[1].pos, v[0].pos );
    v[0].normal = n; v[1].normal = n; v[2].normal = n; v[3].normal = n;     // +y quad
    v[0].tangent = t; v[1].tangent = t; v[2].tangent = t; v[3].tangent = t;
    v[0].bitangent = b; v[1].bitangent = b; v[2].bitangent = b; v[3].bitangent = b;
    v[0].uv = tl; v[1].uv = tr; v[2].uv = br; v[3].uv = bl;
    n = normals[NY]; t = normals[X]; b = normals[NZ];
    // calculate_tangents(t, b, v[7].pos, v[5].pos, v[4].pos );
    v[4].normal = n; v[5].normal = n; v[6].normal = n; v[7].normal = n;     // -y quad
    v[4].tangent = t; v[5].tangent = t; v[6].tangent = t; v[7].tangent = t;
    v[4].bitangent = b; v[5].bitangent = b; v[6].bitangent = b; v[7].bitangent = b;
    v[4].uv = tl; v[5].uv = tr; v[6].uv = br; v[7].uv = bl;
    n = normals[X]; t = normals[Y]; b = normals[NZ];
    // calculate_tangents(t, b, v[11].pos, v[9].pos, v[8].pos );
    v[8].normal = n; v[9].normal = n; v[10].normal = n; v[11].normal = n;   // +x quad
    v[8].tangent = t; v[9].tangent = t; v[10].tangent = t; v[11].tangent = t;
    v[8].bitangent = b; v[9].bitangent = b; v[10].bitangent = b; v[11].bitangent = b;
    v[8].uv = tl; v[9].uv = tr; v[10].uv = br; v[11].uv = bl;
    n = normals[NX]; t = normals[NY]; b = normals[NZ];
    // calculate_tangents(t, b, v[15].pos, v[13].pos, v[12].pos );
    v[12].normal = n; v[13].normal = n; v[14].normal = n; v[15].normal = n; // -x quad
    v[12].tangent = t; v[13].tangent = t; v[14].tangent = t; v[15].tangent = t;
    v[12].bitangent = b; v[13].bitangent = b; v[14].bitangent = b; v[15].bitangent = b;
    v[12].uv = tl; v[13].uv = tr; v[14].uv = br; v[15].uv = bl;
    n = normals[Z]; t = normals[X]; b = normals[NY];
    // calculate_tangents(t, b, v[19].pos, v[17].pos, v[16].pos );
    v[16].normal = n; v[17].normal = n; v[18].normal = n; v[19].normal = n; // +z quad
    v[16].tangent = t; v[17].tangent = t; v[18].tangent = t; v[19].tangent = t;
    v[16].bitangent = b; v[17].bitangent = b; v[18].bitangent = b; v[19].bitangent = b;
    v[16].uv = tl; v[17].uv = tr; v[18].uv = br; v[19].uv = bl;
    n = normals[NZ]; t = normals[NX]; b = normals[NY];
    // calculate_tangents(t, b, v[20].pos, v[22].pos, v[21].pos );
    v[20].normal = n; v[21].normal = n; v[22].normal = n; v[23].normal = n; // -z quad
    v[20].tangent = t; v[21].tangent = t; v[22].tangent = t; v[23].tangent = t;
    v[20].bitangent = b; v[21].bitangent = b; v[22].bitangent = b; v[23].bitangent = b;
    v[20].uv = bl; v[21].uv = tl; v[22].uv = tr; v[23].uv = br;

    u16* i = c.indices;

    i[0] = 2; i[1] = 1; i[2] = 0; i[3] = 2; i[4] = 0; i[5] = 3;             // +y tris
    i[6] = 6; i[7] = 5; i[8] = 4; i[9] = 6; i[10] = 4; i[11] = 7;           // -y tris

    i[12] = 10; i[13] = 9; i[14] = 8; i[15] = 10; i[16] = 8; i[17] = 11;    // +x tris
    i[18] = 14; i[19] = 13; i[20] = 12; i[21] = 14; i[22] = 12; i[23] = 15; // -x tris

    i[24] = 18; i[25] = 17; i[26] = 16; i[27] = 18; i[28] = 16; i[29] = 19; // +z tris
    i[30] = 22; i[31] = 21; i[32] = 20; i[33] = 22; i[34] = 20; i[35] = 23; // -z tris
}
void create_untextured_cube_coords(UntexturedCube& c, const CubeCreateParams& params) {

    f32 w = params.width;
    f32 h = params.height;
    f32 d = params.depth;
    float3* v = c.vertices;

    // vertex and texture coords
    v[0] = { w, d, -h }; v[1] = { -w, d, -h }; v[2] = { -w, d, h }; v[3] = { w, d, h };           // +y quad
    v[4] = { -w, -d, -h }; v[5] = { w, -d, -h }; v[6] = { w, -d, h }; v[7] = { -w, -d, h };       // -y quad

    v[8] = { w, -d, -h }; v[9] = { w, d, -h }; v[10] = { w, d, h }; v[11] = { w, -d, h };          // +x quad
    v[12] = { -w, d, -h }; v[13] = { -w, -d, -h };  v[14] = { -w, -d, h }; v[15] = { -w, d, h };   // -x quad

    v[16] = { -w, -d, h }; v[17] = { w, -d, h }; v[18] = { w, d, h }; v[19] = { -w, d, h };         // +z quad
    v[20] = { w, d, -h }; v[21] = { w, -d, -h }; v[22] = { -w, -d, -h }; v[23] = { -w, d, -h }; // -z quad

    u16* i = c.indices;

    i[0] = 2; i[1] = 1; i[2] = 0; i[3] = 2; i[4] = 0; i[5] = 3;             // +y tris
    i[6] = 6; i[7] = 5; i[8] = 4; i[9] = 6; i[10] = 4; i[11] = 7;           // -y tris

    i[12] = 10; i[13] = 9; i[14] = 8; i[15] = 10; i[16] = 8; i[17] = 11;    // +x tris
    i[18] = 14; i[19] = 13; i[20] = 12; i[21] = 14; i[22] = 12; i[23] = 15; // -x tris

    i[24] = 18; i[25] = 17; i[26] = 16; i[27] = 18; i[28] = 16; i[29] = 19; // +z tris
    i[30] = 22; i[31] = 21; i[32] = 20; i[33] = 22; i[34] = 20; i[35] = 23; // -z tris
}
};

namespace Renderer {

namespace Driver {
    
    struct MainRenderTargetParams {
        u32 width;
        u32 height;
        bool depth;
    };
    void create_main_RT(RscMainRenderTarget&, const MainRenderTargetParams&);
    void bind_main_RT(RscMainRenderTarget& rt);
    
    struct RenderTargetParams {
        u32 width;
        u32 height;
        TextureFormat::Enum textureFormat;
        InternalTextureFormat::Enum textureInternalFormat;
        Type::Enum textureFormatType;
        u32 count;
        bool depth;
    };
    void create_RT(RscRenderTarget& rt, const RenderTargetParams& params);
    void bind_RT(const RscRenderTarget& rt);
    void clear_RT(const RscRenderTarget& rt);
    void clear_RT(const RscRenderTarget& rt, Color32 color);
    struct RenderTargetCopyParams {
        // for now
        bool depth;
    };
    // ONLY WORKS WITH SAME RESOLUTION ON DX
    void copy_RT_to_main_RT(RscMainRenderTarget& dst, const RscRenderTarget& src, const RenderTargetCopyParams& params);

    struct ViewportParams {
        f32 topLeftX;
        f32 topLeftY;
        f32 width;
        f32 height;
        f32 minDepth;
        f32 maxDepth;
    };
    void set_VP(const ViewportParams&);

    struct TextureFromFileParams {
        const char* path;
    };
    void create_texture_from_file(RscTexture& t, const TextureFromFileParams& params);
    struct TextureRenderTargetCreateParams {
        s32 width;
        s32 height;
        TextureFormat::Enum format;
        InternalTextureFormat::Enum internalFormat;
        Type::Enum type;
    };
    void create_texture_empty(RscTexture& t, const TextureRenderTargetCreateParams& params);
    void bind_textures(const RscTexture* textures, const u32 count);

    struct ShaderResult {
        char error[128];
        bool compiled;
    };
    struct VertexShaderRuntimeCompileParams {
        const char* shader_str;
        const VertexAttribDesc* attribs;
        u32 shader_length;
		u32 attrib_count;
    };
    ShaderResult create_shader_vs(RscVertexShader&, const VertexShaderRuntimeCompileParams&);
    struct PixelShaderRuntimeCompileParams {
        const char* shader_str;
        u32 shader_length;
    };
    ShaderResult create_shader_ps(RscPixelShader&, const PixelShaderRuntimeCompileParams&);
    struct CBufferBindingDesc {
        Driver::RscCBuffer cbuffer;
        const char* name;
        u32 stageMask;
    };
    struct TextureBindingDesc {
        //Driver::RscTexture texture; TODO???
        const char* name;
    };
    struct ShaderSetRuntimeCompileParams {
        RscVertexShader& vs;
        RscPixelShader& ps;
        const CBufferBindingDesc* cbufferBindings;
        const TextureBindingDesc* textureBindings;
        u32 cbuffer_count;
        u32 texture_count;
    };
    ShaderResult create_shader_set(RscShaderSet&, const ShaderSetRuntimeCompileParams&);
    void bind_shader(const RscShaderSet& ss);

    struct BlendStateParams {
        bool enable;
    };
    void create_blend_state(RscBlendState&, const BlendStateParams&);
    void bind_blend_state(const RscBlendState& bs);

    struct RasterizerStateParams {
        RasterizerFillMode::Enum fill;
        RasterizerCullMode::Enum cull;
    };
    void create_RS(RscRasterizerState&, const RasterizerStateParams&);
    void bind_RS(const RscRasterizerState& rs);

    struct DepthStencilStateParams {
        bool enable;
        DepthFunc::Enum func;
        DepthWriteMask::Enum writemask;
    };
    void create_DS(RscDepthStencilState&, const DepthStencilStateParams&);
    void bind_DS(const RscDepthStencilState& ds);

    struct VertexBufferDesc {
        void* vertexData;
        u32 vertexSize;
        u32 vertexCount;
        BufferTopologyType::Enum type;
        BufferMemoryUsage::Enum memoryUsage;
        BufferAccessType::Enum accessType;
    };
    void create_vertex_buffer(RscVertexBuffer&, const VertexBufferDesc&, const VertexAttribDesc*, const u32);
    struct BufferUpdateParams {
        void* vertexData;
        u32 vertexSize;
        u32 vertexCount;
    };
    void update_vertex_buffer(RscVertexBuffer&, const BufferUpdateParams&);
    void bind_vertex_buffer(const RscVertexBuffer&);
    void draw_vertex_buffer(const RscVertexBuffer&);
    
    struct IndexedVertexBufferDesc {
        void* vertexData;
        void* indexData;
        u32 vertexSize;
        u32 vertexCount;
        u32 indexSize;
        u32 indexCount;
        BufferItemType::Enum indexType;
        BufferTopologyType::Enum type;
        BufferMemoryUsage::Enum memoryUsage;
        BufferAccessType::Enum accessType;
    };
    void create_indexed_vertex_buffer(RscIndexedVertexBuffer&, const IndexedVertexBufferDesc&, const VertexAttribDesc*, const u32);
    struct IndexedBufferUpdateParams {
        void* vertexData;
        void* indexData;
        u32 vertexSize;
        u32 indexSize;
        u32 indexCount;
    };
    void update_indexed_vertex_buffer(RscIndexedVertexBuffer&, const IndexedBufferUpdateParams&);
    void bind_indexed_vertex_buffer(const RscIndexedVertexBuffer&);
    void draw_indexed_vertex_buffer(const RscIndexedVertexBuffer&);
    void draw_instances_indexed_vertex_buffer(const RscIndexedVertexBuffer&, const u32);

    void draw_fullscreen();

    struct CBufferCreateParams {
        u32 byteWidth;
        u32 bindingIndex;
    };
    void create_cbuffer(RscCBuffer& cb, const CBufferCreateParams& params);
    void update_cbuffer(RscCBuffer& cb, const void* data);
    struct CBufferBindParams {
        bool vertex;
        bool pixel;
    };
    void bind_cbuffers(const RscCBuffer* cb, const u32 count, const CBufferBindParams& params);

    void set_marker_name(Marker_t& wide, const char* ansi);
    void set_marker(Marker_t);
    void start_event(Marker_t);
    void end_event();

} // Driver

void create_indexed_vertex_buffer_from_untextured_cube(Renderer::Driver::RscIndexedVertexBuffer& buffer, const CubeCreateParams& params) {
    Renderer::UntexturedCube cube;
    Renderer::create_untextured_cube_coords(cube, { 1.f, 1.f, 1.f });
    Renderer::Driver::IndexedVertexBufferDesc bufferParams;
    bufferParams.vertexData = cube.vertices;
    bufferParams.indexData = cube.indices;
    bufferParams.vertexSize = sizeof(cube.vertices);
    bufferParams.vertexCount = COUNT_OF(cube.vertices);
    bufferParams.indexSize = sizeof(cube.indices);
    bufferParams.indexCount = COUNT_OF(cube.indices);
    bufferParams.memoryUsage = Renderer::Driver::BufferMemoryUsage::GPU;
    bufferParams.accessType = Renderer::Driver::BufferAccessType::GPU;
    bufferParams.indexType = Renderer::Driver::BufferItemType::U16;
    bufferParams.type = Renderer::Driver::BufferTopologyType::Triangles;
    Driver::VertexAttribDesc attribs[] = {
        Driver::make_vertexAttribDesc("POSITION", 0, sizeof(float3), Driver::BufferAttributeFormat::R32G32B32_FLOAT)
    };
    Renderer::Driver::create_indexed_vertex_buffer(buffer, bufferParams, attribs, COUNT_OF(attribs));
}
void create_indexed_vertex_buffer_from_textured_cube(Renderer::Driver::RscIndexedVertexBuffer& buffer, const CubeCreateParams& params) {
    Renderer::TexturedCube cube;
    Renderer::create_textured_cube_coords(cube, { 1.f, 1.f, 1.f });
    Renderer::Driver::IndexedVertexBufferDesc bufferParams;
    bufferParams.vertexData = cube.vertices;
    bufferParams.indexData = cube.indices;
    bufferParams.vertexSize = sizeof(cube.vertices);
    bufferParams.vertexCount = COUNT_OF(cube.vertices);
    bufferParams.indexSize = sizeof(cube.indices);
    bufferParams.indexCount = COUNT_OF(cube.indices);
    bufferParams.memoryUsage = Renderer::Driver::BufferMemoryUsage::GPU;
    bufferParams.accessType = Renderer::Driver::BufferAccessType::GPU;
    bufferParams.indexType = Renderer::Driver::BufferItemType::U16;
    bufferParams.type = Renderer::Driver::BufferTopologyType::Triangles;
    Driver::VertexAttribDesc attribs[] = {
        Driver::make_vertexAttribDesc("POSITION", OFFSET_OF(TexturedCube::Vertex, pos), sizeof(TexturedCube::Vertex), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
        Driver::make_vertexAttribDesc("TEXCOORD", OFFSET_OF(TexturedCube::Vertex, uv), sizeof(TexturedCube::Vertex), Driver::BufferAttributeFormat::R32G32_FLOAT),
        Driver::make_vertexAttribDesc("NORMAL", OFFSET_OF(TexturedCube::Vertex, normal), sizeof(TexturedCube::Vertex), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
        Driver::make_vertexAttribDesc("TANGENT", OFFSET_OF(TexturedCube::Vertex, tangent), sizeof(TexturedCube::Vertex), Driver::BufferAttributeFormat::R32G32B32_FLOAT),
        Driver::make_vertexAttribDesc("BITANGENT", OFFSET_OF(TexturedCube::Vertex, bitangent), sizeof(TexturedCube::Vertex), Driver::BufferAttributeFormat::R32G32B32_FLOAT)
    };
    Renderer::Driver::create_indexed_vertex_buffer(buffer, bufferParams, attribs, COUNT_OF(attribs));
}
struct ShaderDesc {
    const Driver::VertexAttribDesc* vertexAttrs;
    const Renderer::Driver::CBufferBindingDesc* bufferBindings;
    const Renderer::Driver::TextureBindingDesc* textureBindings;
    const char* vs_name;
    const char* ps_name;
    const char* vs_src;
    const char* ps_src;
    u32 vertexAttr_count;
    u32 bufferBinding_count;
    u32 textureBinding_count;
};

void compile_shader(Driver::RscShaderSet& shader, const ShaderDesc& desc) {
    Renderer::Driver::RscVertexShader vs;
    Renderer::Driver::RscPixelShader ps;
    Renderer::Driver::ShaderResult pixelResult;
    Renderer::Driver::ShaderResult vertexResult;
    vertexResult = Renderer::Driver::create_shader_vs(vs, { desc.vs_src, desc.vertexAttrs, (u32)strlen(desc.vs_src), desc.vertexAttr_count });
    if (!vertexResult.compiled) {
        Platform::debuglog("%s: %s\n", desc.vs_name, vertexResult.error);
        return;
    }
    pixelResult = Renderer::Driver::create_shader_ps(ps, { desc.ps_src, (u32)strlen(desc.ps_src) });
    if (!pixelResult.compiled) {
        Platform::debuglog("%s: %s\n", desc.ps_name, pixelResult.error);
        return;
    }
    Renderer::Driver::ShaderResult result = Renderer::Driver::create_shader_set(shader, { vs, ps, desc.bufferBindings, desc.textureBindings, desc.bufferBinding_count, desc.textureBinding_count });
    if (!result.compiled) {
        Platform::debuglog("Linking %s & %s: %s\n", desc.vs_name, desc.ps_name, result.error);
    }
}
} // Renderer

#endif // __WASTELADNS_RENDERER_H__
