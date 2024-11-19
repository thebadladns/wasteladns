#ifndef __WASTELADNS_RENDERER_H__
#define __WASTELADNS_RENDERER_H__

namespace renderer {

void generate_matrix_ortho_zneg1to1(float4x4& matrix, const WindowProjection::Config& config) {
    f32* matrixCM = matrix.m;
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
    f32* matrixCM = matrix.m;
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
    const f32 h = 1.f / math::tan(config.fov * 0.5f * math::d2r_f);
    const f32 w = h / config.aspect;
    
    // maps each xyz axis to [-1,1], not reversed-z (left handed, y points up z moves away from the viewer)
    f32 (&matrixCM)[16] = matrixRHwithYup.m;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = w;
    matrixCM[5] = h;
    matrixCM[10] = -(config.far + config.near) / (config.far - config.near);
    matrixCM[11] = -1.f;
    matrixCM[14] = -(2.f * config.far * config.near) / (config.far - config.near);
    // inverse is
    //matrixCM[0] = 1.f / w;
    //matrixCM[5] = 1.f / h;
    //matrixCM[10] = 0.f;
    //matrixCM[11] = -(config.far - config.near) / (2.f * config.far * config.near);
    //matrixCM[14] = -1.f;
    //matrixCM[15] = (config.far + config.near) / (2.f * config.far * config.near);
}
void add_oblique_plane_to_persp_zneg1to1(float4x4& projectionMatrix, const float4& planeCameraSpace) {
    // from https://terathon.com/lengyel/Lengyel-Oblique.pdf
    // The near plane is (0, 0, 1, 1), so it can be expressed in the basis of the projection matrix as near = M3 (third row) + M4 (fourth row).
    // We want the near plane to be equal to a clip plane C, new_M3 = aC - M4. The original far plane is (0,0,-1,1), or with the basis above, F = M4-M3
    // new_F = M4 - new_M3 = 2*M4 - a*C. since M4 is aways (0,0,-1,0), a point P=(x,y,0,w) on our clip plane (dot(C,P)=0) will be such that
    // dot(new_F,P) = 2*dot(M4,P) - a*dot(C,P) = 0, which means the new far plane intersects with the C plane on P, which is on the xy plane (no z coord). We find a point Q_proj on the edge
    // of the original far plane, Q_proj=(sign(C_proj.x),sign(C_proj.y),1,1), and claim that the sign of C_proj will be the same that of C.
    // We want our new far plane to go through Q_proj=(sign(C.x),sign(C.y),1,1), so in camera space, dot(new_F,Q)=0, then dot(new_F,Q) = 2*dot(M4,Q) - a*dot(C,Q) --> a = 2*dot(M4,Q) / dot(C,Q)
    // Again, M4 is always (0,0,-1,0), so a = -2*Qz / dot(C,Q). With the projection P and inverse P_inv matrices in renderer::generate_matrix_persp_zneg1to1,
    // Q = P_inv * Q_proj = (sign(C.x)/w,sign(C.y)/h,-1,1/f), which is Q = (sign(C.x)/P[0], sign(C.y)/P[5],-1.f,(1 + P[10]) / P[14])
    // This makes a = 2 / dot(C,Q), which combined with M3 = a*C - M4 = a*C - (0,0,-1,0), gives us this
    float4 q;
    q.x = math::sign(planeCameraSpace.x) / projectionMatrix.m[0];
    q.y = math::sign(planeCameraSpace.y) / projectionMatrix.m[5];
    q.z = -1.f;
    q.w = (1.f + projectionMatrix.m[10]) / projectionMatrix.m[14];
    float4 c = math::scale(planeCameraSpace, (2.f / math::dot(planeCameraSpace, q)));
    projectionMatrix.m[2] = c.x;
    projectionMatrix.m[6] = c.y;
    projectionMatrix.m[10] = c.z + 1.f;
    projectionMatrix.m[14] = c.w;
}
void generate_matrix_persp_z0to1(float4x4& matrixRHwithYup, const PerspProjection::Config& config) {
    const f32 h = 1.f / math::tan(config.fov * 0.5f * math::d2r_f);
    const f32 w = h / config.aspect;
    // maps each xyz axis to [0,1], not reversed-z (left handed, y points up z moves away from the viewer)
    f32(&matrixCM)[16] = matrixRHwithYup.m;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = w;
    matrixCM[5] = h;
    matrixCM[10] = config.far / (config.near - config.far);
    matrixCM[11] = -1.f;
    matrixCM[14] = config.far * config.near / (config.near - config.far);
    // inverse is
    //matrixCM[0] = 1.f / w;
    //matrixCM[5] = 1.f / h;
    //matrixCM[10] = 0.f;
    //matrixCM[11] = (config.near - config.far) / (config.far * config.near);
    //matrixCM[14] = -1.f;
    //matrixCM[15] = 1.f / config.near;
}
void add_oblique_plane_to_persp_z0to1(float4x4& projectionMatrix, const float4& planeCameraSpace) { // todo: investigate issues with this
    // adapted to work with z [0,1] from https://terathon.com/lengyel/Lengyel-Oblique.pdf
    // The near plane is (0, 0, 1, 0), so it can be expressed in the basis of the projection matrix as near = M3 (third row).
    // We want the near plane to be equal to a clip plane C, new_M3 = a*C. The original far plane is (0,0,-1,1), or with the basis above, F = M4 - M3
    // new_F = M4 - new_M3 = M4 - a*C. since M4 is aways (0,0,-1,0), a point P=(x,y,0,w) on our clip plane (dot(C,P) = 0) will be such that
    // dot(new_F,P) = dot(M4,P) - a*dot(C,P) = 0, which means the new far plane intersects with the C plane on P, which is on the xy plane (no z coord). We find a point Q_proj on the edge
    // of the original far plane, Q_proj=(sign(C_proj.x),sign(C_proj.y),1,1), and claim that the sign of C_proj will be the same that of C.
    // We want our new far plane to go through Q_proj=(sign(C.x),sign(C.y),1,1), so in camera space, dot(new_F,Q) = 0, then dot(new_F,Q) = dot(M4,Q) - a*dot(C,Q) -> a = dot(M4,Q) / dot(C,Q)
    // Again, M4 is always (0,0,-1,0), so a = -Qz / dot(C,Q). With the projection P and inverse P_inv matrices in renderer::generate_matrix_persp_z0to1,
    // Q = P_inv * Q_proj = (sign(C.x)/w,sign(C.y)/h,-1,1/f), which is Q = (sign(C.x)/P[0],sign(C.y)/P[5],-1.f,(1 + P[10]) / P[14])
    // This makes a = 1 / dot(C,Q), which combined with M3 = a*C, gives us this

    float4 q;
    q.x = math::sign(planeCameraSpace.x) / projectionMatrix.m[0];
    q.y = math::sign(planeCameraSpace.y) / projectionMatrix.m[5];
    q.z = -1.f;
    q.w = (1.f + projectionMatrix.m[10]) / projectionMatrix.m[14];
    float4 c = math::scale(planeCameraSpace, (1.f / math::dot(planeCameraSpace, q)));
    projectionMatrix.m[2] = c.x;
    projectionMatrix.m[6] = c.y;
    projectionMatrix.m[10] = c.z;
    projectionMatrix.m[14] = c.w;
}

void generate_MV_matrix(float4x4& modelview, const Transform& t) {
    
    const float4x4 mRHwithYUp = math::toEyeSpace(t);

    // Simplified inverse
    // https://www.gamedev.net/forums/topic/647149-d3dxmatrixlookatlh-internally/?tab=comments#comment-5089654

    const f32 tx = -math::dot(mRHwithYUp.col3, mRHwithYUp.col0);
    const f32 ty = -math::dot(mRHwithYUp.col3, mRHwithYUp.col1);
    const f32 tz = -math::dot(mRHwithYUp.col3, mRHwithYUp.col2);

    modelview.m[0] = mRHwithYUp.col0.x;
    modelview.m[4] = mRHwithYUp.col0.y;
    modelview.m[8] = mRHwithYUp.col0.z;

    modelview.m[1] = mRHwithYUp.col1.x;
    modelview.m[5] = mRHwithYUp.col1.y;
    modelview.m[9] = mRHwithYUp.col1.z;

    modelview.m[2] = mRHwithYUp.col2.x;
    modelview.m[6] = mRHwithYUp.col2.y;
    modelview.m[10] = mRHwithYUp.col2.z;

    modelview.m[12] = tx;
    modelview.m[13] = ty;
    modelview.m[14] = tz;

    modelview.m[3] = mRHwithYUp.col0.w;
    modelview.m[7] = mRHwithYUp.col1.w;;
    modelview.m[11] = mRHwithYUp.col2.w;
    modelview.m[15] = mRHwithYUp.col3.w;
}

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
struct CubeCreateParams {
    f32 width;
    f32 height;
    f32 depth;
    float3 offset;
};
void create_colored_cube_coords(ColoredCube& cube, const CubeCreateParams& params) {

    f32 pos_w = params.width + params.offset.x;
    f32 pos_h = params.height + params.offset.y;
    f32 pos_d = params.depth + params.offset.z;
    f32 neg_w = -params.width + params.offset.x;
    f32 neg_h = -params.height + params.offset.y;
    f32 neg_d = -params.depth + params.offset.z;
    u32 c = Color32(1.f, 1.f, 1.f, 1.f).ABGR();
    ColoredCube::Vertex* v = cube.vertices;

    v[0] = { { pos_w, pos_d, neg_h }, c }; v[1] = { { neg_w, pos_d, neg_h }, c }; v[2] = { { neg_w, pos_d, pos_h }, c }; v[3] = { { pos_w, pos_d, pos_h }, c };     // +y quad
    v[4] = { { neg_w, neg_d, neg_h }, c }; v[5] = { { pos_w, neg_d, neg_h }, c }; v[6] = { { pos_w, neg_d, pos_h }, c }; v[7] = { { neg_w, neg_d, pos_h }, c };     // -y quad

    v[8] = { { pos_w, neg_d, neg_h}, c };   v[9] = { { pos_w, pos_d, neg_h }, c };  v[10] = { { pos_w, pos_d, pos_h }, c }; v[11] = { { pos_w, neg_d, pos_h }, c }; // +x quad
    v[12] = { { neg_w, pos_d, neg_h }, c }; v[13] = { { neg_w, neg_d, neg_h }, c }; v[14] = { { neg_w, neg_d, pos_h }, c }; v[15] = { { neg_w, pos_d, pos_h }, c }; // -x quad

    v[16] = { { neg_w, neg_d, pos_h }, c }; v[17] = { { pos_w, neg_d, pos_h }, c }; v[18] = { { pos_w, pos_d, pos_h }, c }; v[19] = { { neg_w, pos_d, pos_h }, c }; // +z quad
    v[20] = { { pos_w, pos_d, neg_h }, c }; v[21] = { { pos_w, neg_d, neg_h }, c }; v[22] = { { neg_w, neg_d, neg_h }, c }; v[23] = { { neg_w, pos_d, neg_h }, c }; // -z quad

    u16* i = cube.indices;

    i[0] = 2; i[1] = 1; i[2] = 0; i[3] = 2; i[4] = 0; i[5] = 3;             // +y tris
    i[6] = 6; i[7] = 5; i[8] = 4; i[9] = 6; i[10] = 4; i[11] = 7;           // -y tris

    i[12] = 10; i[13] = 9; i[14] = 8; i[15] = 10; i[16] = 8; i[17] = 11;    // +x tris
    i[18] = 14; i[19] = 13; i[20] = 12; i[21] = 14; i[22] = 12; i[23] = 15; // -x tris

    i[24] = 18; i[25] = 17; i[26] = 16; i[27] = 18; i[28] = 16; i[29] = 19; // +z tris
    i[30] = 22; i[31] = 21; i[32] = 20; i[33] = 22; i[34] = 20; i[35] = 23; // -z tris
}
void create_untextured_cube_coords(UntexturedCube& c, const CubeCreateParams& params) {

    f32 pos_w = params.width + params.offset.x;
    f32 pos_h = params.height + params.offset.y;
    f32 pos_d = params.depth + params.offset.z;
    f32 neg_w = -params.width + params.offset.x;
    f32 neg_h = -params.height + params.offset.y;
    f32 neg_d = -params.depth + params.offset.z;
    float3* v = c.vertices;

    v[0] = { pos_w, pos_d, neg_h }; v[1] = { neg_w, pos_d, neg_h }; v[2] = { neg_w, pos_d, pos_h }; v[3] = { pos_w, pos_d, pos_h };     // +y quad
    v[4] = { neg_w, neg_d, neg_h }; v[5] = { pos_w, neg_d, neg_h }; v[6] = { pos_w, neg_d, pos_h }; v[7] = { neg_w, neg_d, pos_h };     // -y quad

    v[8] = { pos_w, neg_d, neg_h}; v[9] = { pos_w, pos_d, neg_h }; v[10] = { pos_w, pos_d, pos_h }; v[11] = { pos_w, neg_d, pos_h };    // +x quad
    v[12] = { neg_w, pos_d, neg_h }; v[13] = { neg_w, neg_d, neg_h }; v[14] = { neg_w, neg_d, pos_h }; v[15] = { neg_w, pos_d, pos_h }; // -x quad

    v[16] = { neg_w, neg_d, pos_h }; v[17] = { pos_w, neg_d, pos_h }; v[18] = { pos_w, pos_d, pos_h }; v[19] = { neg_w, pos_d, pos_h }; // +z quad
    v[20] = { pos_w, pos_d, neg_h }; v[21] = { pos_w, neg_d, neg_h }; v[22] = { neg_w, neg_d, neg_h }; v[23] = { neg_w, pos_d, neg_h }; // -z quad

    u16* i = c.indices;

    i[0] = 2; i[1] = 1; i[2] = 0; i[3] = 2; i[4] = 0; i[5] = 3;             // +y tris
    i[6] = 6; i[7] = 5; i[8] = 4; i[9] = 6; i[10] = 4; i[11] = 7;           // -y tris

    i[12] = 10; i[13] = 9; i[14] = 8; i[15] = 10; i[16] = 8; i[17] = 11;    // +x tris
    i[18] = 14; i[19] = 13; i[20] = 12; i[21] = 14; i[22] = 12; i[23] = 15; // -x tris

    i[24] = 18; i[25] = 17; i[26] = 16; i[27] = 18; i[28] = 16; i[29] = 19; // +z tris
    i[30] = 22; i[31] = 21; i[32] = 20; i[33] = 22; i[34] = 20; i[35] = 23; // -z tris
}
};

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

void create_indexed_vertex_buffer_from_untextured_cube(renderer::driver::RscIndexedVertexBuffer& buffer, const CubeCreateParams& params) {
    renderer::UntexturedCube cube;
    renderer::create_untextured_cube_coords(cube, params);
    renderer::driver::IndexedVertexBufferDesc bufferParams;
    bufferParams.vertexData = cube.vertices;
    bufferParams.indexData = cube.indices;
    bufferParams.vertexSize = sizeof(cube.vertices);
    bufferParams.vertexCount = COUNT_OF(cube.vertices);
    bufferParams.indexSize = sizeof(cube.indices);
    bufferParams.indexCount = COUNT_OF(cube.indices);
    bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
    bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
    bufferParams.indexType = renderer::driver::BufferItemType::U16;
    bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
    driver::VertexAttribDesc attribs[] = {
        driver::make_vertexAttribDesc("POSITION", 0, sizeof(float3), driver::BufferAttributeFormat::R32G32B32_FLOAT)
    };
    renderer::driver::create_indexed_vertex_buffer(buffer, bufferParams, attribs, COUNT_OF(attribs));
}
void create_indexed_vertex_buffer_from_colored_cube(renderer::driver::RscIndexedVertexBuffer& buffer, const CubeCreateParams& params) {
    renderer::ColoredCube cube;
    renderer::create_colored_cube_coords(cube, params);
    renderer::driver::IndexedVertexBufferDesc bufferParams;
    bufferParams.vertexData = cube.vertices;
    bufferParams.indexData = cube.indices;
    bufferParams.vertexSize = sizeof(cube.vertices);
    bufferParams.vertexCount = COUNT_OF(cube.vertices);
    bufferParams.indexSize = sizeof(cube.indices);
    bufferParams.indexCount = COUNT_OF(cube.indices);
    bufferParams.memoryUsage = renderer::driver::BufferMemoryUsage::GPU;
    bufferParams.accessType = renderer::driver::BufferAccessType::GPU;
    bufferParams.indexType = renderer::driver::BufferItemType::U16;
    bufferParams.type = renderer::driver::BufferTopologyType::Triangles;
    driver::VertexAttribDesc attribs[] = {
        driver::make_vertexAttribDesc("POSITION", OFFSET_OF(ColoredCube::Vertex, pos), sizeof(ColoredCube::Vertex), driver::BufferAttributeFormat::R32G32B32_FLOAT),
        driver::make_vertexAttribDesc("COLOR", OFFSET_OF(ColoredCube::Vertex, color), sizeof(ColoredCube::Vertex), driver::BufferAttributeFormat::R8G8B8A8_UNORM)
    };
    renderer::driver::create_indexed_vertex_buffer(buffer, bufferParams, attribs, COUNT_OF(attribs));
}
struct ShaderDesc {
    const driver::VertexAttribDesc* vertexAttrs;
    const renderer::driver::CBufferBindingDesc* bufferBindings;
    const renderer::driver::TextureBindingDesc* textureBindings;
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
    vertexResult = renderer::driver::create_shader_vs(vs, { desc.vs_src, desc.vertexAttrs, (u32)strlen(desc.vs_src), desc.vertexAttr_count });
    if (!vertexResult.compiled) {
        platform::debuglog("%s: %s\n", desc.vs_name, vertexResult.error);
        return;
    }
    pixelResult = renderer::driver::create_shader_ps(ps, { desc.ps_src, (u32)strlen(desc.ps_src) });
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
