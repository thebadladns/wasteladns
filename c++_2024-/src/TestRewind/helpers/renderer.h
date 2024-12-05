#ifndef __WASTELADNS_RENDERER_H__
#define __WASTELADNS_RENDERER_H__

namespace renderer {

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

void create_cube_coords(uintptr_t v, ptrdiff_t stride, u16* i, const float3 scale, const float3 offset) {
    const f32 pos_x = scale.x + offset.x; const f32 neg_x = -scale.x + offset.x;
    const f32 pos_y = scale.y + offset.y; const f32 neg_y = -scale.y + offset.y;
    const f32 pos_z = scale.z + offset.z; const f32 neg_z = -scale.z + offset.z;
    float3* v0, * v1, * v2, * v3;
    v0 = (float3*)(v + stride * 0); v1 = (float3*)(v + stride * 1); v2 = (float3*)(v + stride * 2); v3 = (float3*)(v + stride * 3);
    *v0 = { pos_x, pos_y, neg_z }; *v1 = { neg_x, pos_y, neg_z }; *v2 = { neg_x, pos_y, pos_z }; *v3 = { pos_x, pos_y, pos_z }; // +y quad
    v0 = (float3*)(v + stride * 4); v1 = (float3*)(v + stride * 5); v2 = (float3*)(v + stride * 6); v3 = (float3*)(v + stride * 7);
    *v0 = { neg_x, neg_y, neg_z }; *v1 = { pos_x, neg_y, neg_z }; *v2 = { pos_x, neg_y, pos_z }; *v3 = { neg_x, neg_y, pos_z }; // -y quad
    v0 = (float3*)(v + stride * 8); v1 = (float3*)(v + stride * 9); v2 = (float3*)(v + stride * 10); v3 = (float3*)(v + stride * 11);
    *v0 = { pos_x, neg_y, neg_z }; *v1 = { pos_x, pos_y, neg_z }; *v2 = { pos_x, pos_y, pos_z }; *v3 = { pos_x, neg_y, pos_z }; // +x quad
    v0 = (float3*)(v + stride * 12); v1 = (float3*)(v + stride * 13); v2 = (float3*)(v + stride * 14); v3 = (float3*)(v + stride * 15);
    *v0 = { neg_x, pos_y, neg_z }; *v1 = { neg_x, neg_y, neg_z }; *v2 = { neg_x, neg_y, pos_z }; *v3 = { neg_x, pos_y, pos_z }; // -x quad
    v0 = (float3*)(v + stride * 16); v1 = (float3*)(v + stride * 17); v2 = (float3*)(v + stride * 18); v3 = (float3*)(v + stride * 19);
    *v0 = { neg_x, neg_y, pos_z }; *v1 = { pos_x, neg_y, pos_z }; *v2 = { pos_x, pos_y, pos_z }; *v3 = { neg_x, pos_y, pos_z }; // +z quad
    v0 = (float3*)(v + stride * 20); v1 = (float3*)(v + stride * 21); v2 = (float3*)(v + stride * 22); v3 = (float3*)(v + stride * 23);
    *v0 = { pos_x, pos_y, neg_z }; *v1 = { pos_x, neg_y, neg_z }; *v2 = { neg_x, neg_y, neg_z }; *v3 = { neg_x, pos_y, neg_z }; // -z quad

    i[0] = 2; i[1] = 1; i[2] = 0; i[3] = 2; i[4] = 0; i[5] = 3;             // +y tris
    i[6] = 6; i[7] = 5; i[8] = 4; i[9] = 6; i[10] = 4; i[11] = 7;           // -y tris

    i[12] = 10; i[13] = 9; i[14] = 8; i[15] = 10; i[16] = 8; i[17] = 11;    // +x tris
    i[18] = 14; i[19] = 13; i[20] = 12; i[21] = 14; i[22] = 12; i[23] = 15; // -x tris

    i[24] = 18; i[25] = 17; i[26] = 16; i[27] = 18; i[28] = 16; i[29] = 19; // +z tris
    i[30] = 22; i[31] = 21; i[32] = 20; i[33] = 22; i[34] = 20; i[35] = 23; // -z tris
}
void create_colored_cube_coords(ColoredCube& cube, const Color32 color, const float3 scale, const float3 offset) {
    create_cube_coords((uintptr_t)cube.vertices, sizeof(cube.vertices[0]), cube.indices, scale, offset);
    const u32 c = color.ABGR();
    for (u32 i = 0; i < 24; i++) { cube.vertices[i].color = c; }
}

void create_untextured_sphere_coords(UntexturedSphere& s, const f32 radius) {
    s.vertices[0] =  float3( 0.0000f,   -1.0000f,    0.0000f);
    s.vertices[1] =  float3( 0.7227f,   -0.4453f,   -0.5254f);
    s.vertices[2] =  float3(-0.2754f,   -0.4453f,   -0.8496f);
    s.vertices[3] =  float3(-0.8926f,   -0.4453f,    0.0000f);
    s.vertices[4] =  float3(-0.2754f,   -0.4453f,    0.8496f);
    s.vertices[5] =  float3( 0.7227f,   -0.4453f,    0.5254f);
    s.vertices[6] =  float3( 0.2754f,    0.4453f,   -0.8496f);
    s.vertices[7] =  float3(-0.7227f,    0.4453f,   -0.5254f);
    s.vertices[8] =  float3(-0.7227f,    0.4453f,    0.5254f);
    s.vertices[9] =  float3( 0.2754f,    0.4453f,    0.8496f);
    s.vertices[10] = float3( 0.8926f,    0.4453f,    0.0000f);
    s.vertices[11] = float3( 0.0000f,    1.0000f,    0.0000f);
    s.vertices[12] = float3(-0.1621f,   -0.8496f,   -0.4980f);
    s.vertices[13] = float3( 0.4238f,   -0.8496f,   -0.3086f);
    s.vertices[14] = float3( 0.2617f,   -0.5254f,   -0.8086f);
    s.vertices[15] = float3( 0.8496f,   -0.5254f,    0.0000f);
    s.vertices[16] = float3( 0.4238f,   -0.8496f,    0.3086f);
    s.vertices[17] = float3(-0.5254f,   -0.8496f,    0.0000f);
    s.vertices[18] = float3(-0.6875f,   -0.5254f,   -0.4980f);
    s.vertices[19] = float3(-0.1621f,   -0.8496f,    0.4980f);
    s.vertices[20] = float3(-0.6875f,   -0.5254f,    0.4980f);
    s.vertices[21] = float3( 0.2617f,   -0.5254f,    0.8086f);
    s.vertices[22] = float3( 0.9492f,    0.0000f,   -0.3086f);
    s.vertices[23] = float3( 0.9492f,    0.0000f,    0.3086f);
    s.vertices[24] = float3( 0.0000f,    0.0000f,   -1.0000f);
    s.vertices[25] = float3( 0.5859f,    0.0000f,   -0.8086f);
    s.vertices[26] = float3(-0.9492f,    0.0000f,   -0.3086f);
    s.vertices[27] = float3(-0.5859f,    0.0000f,   -0.8086f);
    s.vertices[28] = float3(-0.5859f,    0.0000f,    0.8086f);
    s.vertices[29] = float3(-0.9492f,    0.0000f,    0.3086f);
    s.vertices[30] = float3( 0.5859f,    0.0000f,    0.8086f);
    s.vertices[31] = float3( 0.0000f,    0.0000f,    1.0000f);
    s.vertices[32] = float3( 0.6875f,    0.5254f,   -0.4980f);
    s.vertices[33] = float3(-0.2617f,    0.5254f,   -0.8086f);
    s.vertices[34] = float3(-0.8496f,    0.5254f,    0.0000f);
    s.vertices[35] = float3(-0.2617f,    0.5254f,    0.8086f);
    s.vertices[36] = float3( 0.6875f,    0.5254f,    0.4980f);
    s.vertices[37] = float3( 0.1621f,    0.8496f,   -0.4980f);
    s.vertices[38] = float3( 0.5254f,    0.8496f,    0.0000f);
    s.vertices[39] = float3(-0.4238f,    0.8496f,   -0.3086f);
    s.vertices[40] = float3(-0.4238f,    0.8496f,    0.3086f);
    s.vertices[41] = float3( 0.1621f,    0.8496f,    0.4980f);
    for (u32 i = 0; i < 42; i++) { s.vertices[i] = math::scale(s.vertices[i], radius); }
    
    u16* i = s.indices;
    i[ 0] = 0;     i[1] = 13;   i[2] =  12;
    i[ 3] = 1;     i[4] = 13;   i[5] =  15;
    i[ 6] = 0;     i[7] = 12;   i[8] =  17;
    i[ 9] = 0;     i[10] = 17;  i[11] =  19;
    i[ 12] = 0;    i[13] = 19;  i[14] =  16;
    i[ 15] = 1;    i[16] = 15;  i[17] =  22;
    i[ 18] = 2;    i[19] = 14;  i[20] =  24;
    i[ 21] = 3;    i[22] = 18;  i[23] =  26;
    i[ 24] = 4;    i[25] = 20;  i[26] =  28;
    i[ 27] = 5;    i[28] = 21;  i[29] =  30;
    i[ 30] = 1;    i[31] = 22;  i[32] =  25;
    i[ 33] = 2;    i[34] = 24;  i[35] =  27;
    i[ 36] = 3;    i[37] = 26;  i[38] =  29;
    i[ 39] = 4;    i[40] = 28;  i[41] =  31;
    i[ 42] = 5;    i[43] = 30;  i[44] =  23;
    i[ 45] = 6;    i[46] = 32;  i[47] =  37;
    i[ 48] = 7;    i[49] = 33;  i[50] =  39;
    i[ 51] = 8;    i[52] = 34;  i[53] =  40;
    i[ 54] = 9;    i[55] = 35;  i[56] =  41;
    i[ 57] = 10;   i[58] = 36;  i[59] =  38;
    i[ 60] = 38;   i[61] = 41;  i[62] =  11;
    i[ 63] = 38;   i[64] = 36;  i[65] =  41;
    i[ 66] = 36;   i[67] = 9;   i[68] =  41;
    i[ 69] = 41;   i[70] = 40;  i[71] =  11;
    i[ 72] = 41;   i[73] = 35;  i[74] =  40;
    i[ 75] = 35;   i[76] = 8;   i[77] =  40;
    i[ 78] = 40;   i[79] = 39;  i[80] =  11;
    i[ 81] = 40;   i[82] = 34;  i[83] =  39;
    i[ 84] = 34;   i[85] = 7;   i[86] =  39;
    i[ 87] = 39;   i[88] = 37;  i[89] =  11;
    i[ 90] = 39;   i[91] = 33;  i[92] =  37;
    i[ 93] = 33;   i[94] = 6;   i[95] =  37;
    i[ 96] = 37;   i[97] = 38;  i[98] =  11;
    i[ 99] = 37;   i[100] = 32; i[101] =  38;
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
}

namespace renderer {

namespace driver {
    
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
    void clear_RT(const RscRenderTarget& rt, u32 flags);
    void clear_RT(const RscRenderTarget& rt, u32 flags, Color32 color);
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
        allocator::Arena& arena;
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

    void load_shader_cache(ShaderCache& shaderCache, const char* path, allocator::Arena* arena, const u32 maxShaders);
    void write_shader_cache(ShaderCache& shaderCache);
    struct ShaderResult {
        char error[128];
        bool compiled;
    };
    struct VertexShaderRuntimeCompileParams {
        ShaderCache* shader_cache;
        const char* shader_str;
        const VertexAttribDesc* attribs;
        u32 shader_length;
		u32 attrib_count;
    };
    ShaderResult create_shader_vs(RscVertexShader&, const VertexShaderRuntimeCompileParams&);
    struct PixelShaderRuntimeCompileParams {
        ShaderCache* shader_cache;
        const char* shader_str;
        u32 shader_length;
    };
    ShaderResult create_shader_ps(RscPixelShader&, const PixelShaderRuntimeCompileParams&);
    struct CBufferBindingDesc {
        const char* name;
        u32 stageMask;
    };
    struct TextureBindingDesc {
        //driver::RscTexture texture; TODO???
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
        RenderTargetWriteMask::Enum renderTargetWriteMask;
        bool blendEnable;
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
        CompFunc::Enum depth_func;
        DepthWriteMask::Enum depth_writemask;
        StencilOp::Enum stencil_failOp;
        StencilOp::Enum stencil_depthFailOp;
        StencilOp::Enum stencil_passOp;
        CompFunc::Enum stencil_func;
        bool depth_enable;
        bool stencil_enable;
        u8 stencil_readmask;
        u8 stencil_writemask;
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
    void bind_cbuffers(const RscShaderSet& ss, const RscCBuffer* cb, const u32 count);

    void set_marker_name(Marker_t& wide, const char* ansi);
    void set_marker(Marker_t);
    void start_event(Marker_t);
    void end_event();

} // Driver

void create_indexed_vertex_buffer_from_untextured_mesh(renderer::driver::RscIndexedVertexBuffer& buffer, float3* vertices, u32 vertices_count, u16* indices, u32 indices_count) {
    renderer::driver::IndexedVertexBufferDesc bufferParams;
    bufferParams.vertexData = vertices;
    bufferParams.indexData = indices;
    bufferParams.vertexSize = sizeof(f32) * 3 * vertices_count;
    bufferParams.vertexCount = vertices_count;
    bufferParams.indexSize = sizeof(u16) * indices_count;
    bufferParams.indexCount = indices_count;
    bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
    bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
    bufferParams.indexType = renderer::driver::BufferItemType::U16;
    bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
    driver::VertexAttribDesc attribs[] = {
        driver::make_vertexAttribDesc("POSITION", 0, sizeof(float3), driver::BufferAttributeFormat::R32G32B32_FLOAT)
    };
    renderer::driver::create_indexed_vertex_buffer(buffer, bufferParams, attribs, countof(attribs));
}
void create_indexed_vertex_buffer_from_colored_mesh(renderer::driver::RscIndexedVertexBuffer& buffer, ColoredCube::Vertex* vertices, u32 vertices_count, u16* indices, u32 indices_count) {
    renderer::driver::IndexedVertexBufferDesc bufferParams;
    bufferParams.vertexData = vertices;
    bufferParams.indexData = indices;
    bufferParams.vertexSize = sizeof(f32) * 3 * vertices_count;
    bufferParams.vertexCount = vertices_count;
    bufferParams.indexSize = sizeof(u16) * indices_count;
    bufferParams.indexCount = indices_count;
    bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
    bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
    bufferParams.indexType = renderer::driver::BufferItemType::U16;
    bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
    driver::VertexAttribDesc attribs[] = {
        driver::make_vertexAttribDesc("POSITION", offsetof(ColoredCube::Vertex, pos), sizeof(ColoredCube::Vertex), driver::BufferAttributeFormat::R32G32B32_FLOAT),
        driver::make_vertexAttribDesc("COLOR", offsetof(ColoredCube::Vertex, color), sizeof(ColoredCube::Vertex), driver::BufferAttributeFormat::R8G8B8A8_UNORM)
    };
    renderer::driver::create_indexed_vertex_buffer(buffer, bufferParams, attribs, countof(attribs));
}

struct ShaderDesc {
    driver::ShaderCache* shader_cache;
    const driver::VertexAttribDesc* vertexAttrs;
    const driver::CBufferBindingDesc* bufferBindings;
    const driver::TextureBindingDesc* textureBindings;
    const char* vs_name;
    const char* ps_name;
    const char* vs_src;
    const char* ps_src;
    u32 vertexAttr_count;
    u32 bufferBinding_count;
    u32 textureBinding_count;
};

void compile_shader(driver::RscShaderSet& shader, const ShaderDesc& desc) {
    renderer::driver::RscVertexShader vs;
    renderer::driver::RscPixelShader ps;
    renderer::driver::ShaderResult pixelResult;
    renderer::driver::ShaderResult vertexResult;
    vertexResult = renderer::driver::create_shader_vs(vs, { desc.shader_cache, desc.vs_src, desc.vertexAttrs, (u32)strlen(desc.vs_src), desc.vertexAttr_count });
    if (!vertexResult.compiled) {
        platform::debuglog("%s: %s\n", desc.vs_name, vertexResult.error);
        return;
    }
    pixelResult = renderer::driver::create_shader_ps(ps, { desc.shader_cache, desc.ps_src, (u32)strlen(desc.ps_src) });
    if (!pixelResult.compiled) {
        platform::debuglog("%s: %s\n", desc.ps_name, pixelResult.error);
        return;
    }
    renderer::driver::ShaderResult result = renderer::driver::create_shader_set(shader, { vs, ps, desc.bufferBindings, desc.textureBindings, desc.bufferBinding_count, desc.textureBinding_count });
    if (!result.compiled) {
        platform::debuglog("Linking %s & %s: %s\n", desc.vs_name, desc.ps_name, result.error);
    }
}

} // Renderer

#endif // __WASTELADNS_RENDERER_H__
