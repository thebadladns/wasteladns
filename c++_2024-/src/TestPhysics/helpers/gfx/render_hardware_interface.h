#ifndef __WASTELADNS_RHI_H__
#define __WASTELADNS_RHI_H__


namespace gfx {
namespace rhi { // render hardware interface template for all platforms

struct RscMainRenderTarget;
enum { RenderTarget_MaxCount = 4 };
struct RscRenderTarget;
struct RscTexture;
struct RscBlendState;
struct RscRasterizerState;
struct RscDepthStencilState;
struct ShaderCache;
struct RscVertexShader;
struct RscPixelShader;
struct RscShaderSet;
//typedef (something) VertexAttribDesc;
//VertexAttribDesc make_vertexAttribDesc(
// const char* name, size_t offset, size_t stride, BufferAttributeFromat::Enum format);
struct RscInputLayout;
struct RscVertexBuffer;
struct RscIndexedVertexBuffer;
struct CBufferStageMask { enum Enum { VS = 1, PS = 2 }; };
struct RscCBuffer;
//typedef (something) Marker_t;

} // rhi
} // gfx

#if __DX11
#include "render_hardware_interface_definitions_dx11.h"
#elif __GL33
#include "loader_gl33.h"
#include "render_hardware_interface_definitions_gl33.h"
#endif

namespace gfx {
namespace rhi { // render hardware interface
    
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
force_inline void bind_RT(const RscRenderTarget& rt);
force_inline void clear_RT(const RscRenderTarget& rt, u32 flags);
force_inline void clear_RT(const RscRenderTarget& rt, u32 flags, Color32 color);
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
force_inline void set_VP(const ViewportParams&);

struct TextureFromFileParams {
    allocator::PagedArena& arena;
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
force_inline void bind_textures(const RscTexture* textures, const u32 count);

void load_shader_cache(ShaderCache& shaderCache, const char* path, allocator::PagedArena* arena, const u32 maxShaders);
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
    //rhi::RscTexture texture; TODO???
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
force_inline void bind_shader(const RscShaderSet& ss);

struct BlendStateParams {
    RenderTargetWriteMask::Enum renderTargetWriteMask;
    bool blendEnable;
};
void create_blend_state(RscBlendState&, const BlendStateParams&);
force_inline void bind_blend_state(const RscBlendState& bs);

struct RasterizerStateParams {
    RasterizerFillMode::Enum fill;
    RasterizerCullMode::Enum cull;
    bool scissor;
};
void create_RS(RscRasterizerState&, const RasterizerStateParams&);
force_inline void bind_RS(const RscRasterizerState& rs);
force_inline void set_scissor(const u32, const u32, const u32, const u32);

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
force_inline void bind_DS(const RscDepthStencilState&, const u32);

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
force_inline void bind_vertex_buffer(const RscVertexBuffer&);
force_inline void draw_vertex_buffer(const RscVertexBuffer&);
    
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
force_inline void bind_indexed_vertex_buffer(const RscIndexedVertexBuffer&);
force_inline void draw_indexed_vertex_buffer(const RscIndexedVertexBuffer&);
force_inline void draw_instances_indexed_vertex_buffer(const RscIndexedVertexBuffer&, const u32);

force_inline void draw_fullscreen();

struct CBufferCreateParams {
    u32 byteWidth;
};
void create_cbuffer(RscCBuffer& cb, const CBufferCreateParams& params);
force_inline void update_cbuffer(RscCBuffer& cb, const void* data);
force_inline void bind_cbuffers(const RscShaderSet& ss, const RscCBuffer* cb, const u32 count);

#if __PROFILE
force_inline void start_event(const char*);
force_inline void end_event();
#else
// dummy impl
force_inline void start_event(const char*) {}
force_inline void end_event() {}
#endif

} // rhi
} // gfx

#if __DX11
#include "render_hardware_interface_dx11.h"
#elif __GL33
#include "render_hardware_interface_gl33.h"
#endif

#endif // __WASTELADNS_RHI_H__
