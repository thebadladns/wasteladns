#ifndef __WASTELADNS_RENDERER_GLFW_H__
#define __WASTELADNS_RENDERER_GLFW_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {

    void create_textured_quad(RenderTargetTexturedQuad& q) {
        // Generally we use uv=(0,0) as top left, but only
        // for texture data which is interpreted by opengl
        // as bottom-to-top (they are stored top-to-bottom)
        // Render targets are drawn bottom-to-top, hence (0,0)
        // being the bottom left here
        Renderer::Layout_TexturedVec2* v = q.vertices;
        u16* i = q.indices;
        v[0] = { { -1.f, 1.f }, { 0.f, 1.f } };
        v[1] = { { -1.f, -1.f }, { 0.f, 0.f } };
        v[2] = { { 1.f, 1.f }, { 1.f, 1.f } };
        v[3] = { { 1.f, -1.f }, { 1.f, 0.f } };
        i[0] = 0; i[1] = 1; i[2] = 2;
        i[3] = 2; i[4] = 1; i[5] = 3;
    }

namespace Driver {

    void create_main_RT(RscMainRenderTarget& rt, const MainRenderTargetParams& params) {
        rt.mask = GL_COLOR_BUFFER_BIT;
        if (params.depth) {
            rt.mask = rt.mask | GL_DEPTH_BUFFER_BIT;
        }
    }
    void clear_main_RT(RscMainRenderTarget& rt, Col color) {
        glClearColor( RGBA_PARAMS(color) );
        glClear(rt.mask | GL_STENCIL_BUFFER_BIT);
    }
    void bind_main_RT(RscMainRenderTarget& rt) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }
    template<u32 _attachments>
    void create_RT(RscRenderTarget<_attachments>& rt, const RenderTargetParams& params) {
        GLuint buffer;
        glGenFramebuffers(1, &buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        rt.buffer = buffer;
        rt.mask = GL_COLOR_BUFFER_BIT;
        rt.width = params.width;
        rt.height = params.height;
        if (params.depth) {
            GLuint depthBuffer;
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, params.width, params.height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            rt.depthBuffer = depthBuffer;
            rt.mask = rt.mask | GL_DEPTH_BUFFER_BIT;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        {
            glBindFramebuffer(GL_FRAMEBUFFER, rt.buffer);
            GLuint attachments[_attachments];
            for (u32 i = 0; i < _attachments; i++) {
                Renderer::Driver::TextureRenderTargetCreateParams texParams;
                texParams.width = params.width;
                texParams.height = params.height;
                texParams.format = params.textureFormat;
                texParams.internalFormat = params.textureInternalFormat;
                texParams.type = params.textureFormatType;

                create_texture_empty(rt.textures[i], texParams);
                u32 colorAttachment = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachment, GL_TEXTURE_2D, rt.textures[i].texId, 0);
                attachments[i] = colorAttachment;
            }
            glDrawBuffers(_attachments, attachments);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    template<u32 _attachments>
    void bind_RT(const RscRenderTarget<_attachments>& rt) {
        glBindFramebuffer(GL_FRAMEBUFFER, rt.buffer);
    }
    template<u32 _attachments>
    void clear_RT(const RscRenderTarget<_attachments>& rt) {
        glClear(rt.mask | GL_STENCIL_BUFFER_BIT);
    }
    template<u32 _attachments>
    void clear_RT(const RscRenderTarget<_attachments>& rt, Col color) {
        glClearColor(RGBA_PARAMS(color));
        glClear(rt.mask | GL_STENCIL_BUFFER_BIT);
    }
    template<u32 _attachments>
    void copy_RT_to_main_RT(RscMainRenderTarget& dst, const RscRenderTarget<_attachments>& src, const RenderTargetCopyParams& params) {
        // TODO: UNTESTED
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src.buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // backbuffer only has one attachment
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        // Todo: ensure matching formats?
        GLuint flags = 0;
        if (params.depth) {
            flags = flags | GL_DEPTH_BUFFER_BIT;
        }
        glBlitFramebuffer(0, 0, src.width, src.height, 0, 0, src.width, src.height, flags, GL_NEAREST);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void set_VP(const ViewportParams& params) {
        glViewport((GLint)params.topLeftX, (GLint)params.topLeftY, (GLsizei)params.width, (GLsizei)params.height);
    }
    
    void create_texture_from_file(RscTexture& t, const TextureFromFileParams& params) {
        s32 w, h, channels;
        u8* data = stbi_load(params.path, &w, &h, &channels, 4);
        if (data) {
            GLenum format = GL_RGBA;
            GLenum type = GL_UNSIGNED_BYTE;
            switch (channels) {
            case 1: format = GL_RED; type = GL_FLOAT; break;
            case 4: format = GL_RGBA; type = GL_UNSIGNED_BYTE;  break;
            default: assert("unhandled texture format");
            }
            GLuint texId;
            
            // image data comes from top to bottom, but opengl will store it the other way
            // (first element is assumed to be the bottom left, uv=(0,0))
            // this reversal is fine: the rest of the code will assume uv=(0,0) is top right
            glGenTextures(1, &texId);
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, type, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            
            // TODO: parameters!!
            if (w <= 64) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glBindTexture(GL_TEXTURE_2D, 0);

            t.texId = texId;
            
            stbi_image_free(data);
        }
    }
    void create_texture_empty(RscTexture& t, const TextureRenderTargetCreateParams& params) {
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexImage2D(GL_TEXTURE_2D, 0, params.internalFormat, params.width, params.height, 0, params.format, params.type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        t.texId = texId;
    }
    void bind_textures(const RscTexture* textures, const u32 count) {
        for (u32 i = 0; i < count; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i].texId);
        }
    }
    template <typename _vertexLayout, typename _cbufferLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
    void bind_shader_samplers(RscShaderSet<_vertexLayout, _cbufferLayout, _cbufferUsage, _drawType>& ss, const char** params, const u32 count) {
        glUseProgram(ss.shaderObject);
        for (u32 i = 0; i < count; i++) {
            const s32 samplerLoc = glGetUniformLocation(ss.shaderObject, params[i]);
            glUniform1i(samplerLoc, i);
        }
    }
    
    template <typename _vertexLayout, typename _cbufferLayout, Shaders::VSDrawType::Enum _drawType>
    ShaderResult create_shader_vs(RscVertexShader<_vertexLayout, _cbufferLayout, _drawType>& vs, const VertexShaderRuntimeCompileParams& params) {
        GLuint vertexShader;
        
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &params.shaderStr, nullptr);
        glCompileShader(vertexShader);
        
        vs.shaderObject = vertexShader;
        
        GLint compiled;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);

        ShaderResult result;
        result.compiled = compiled != 0;
        if (result.compiled) {} else {
            GLint infoLogLength;
            glGetShaderiv(vs.shaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if (infoLogLength > 0) {
                glGetShaderInfoLog(vs.shaderObject, Math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
            }
        }

        return result;
    }
    template <Shaders::PSCBufferUsage::Enum _cbufferUsage>
    ShaderResult create_shader_ps(RscPixelShader<_cbufferUsage>& ps, const PixelShaderRuntimeCompileParams& params) {
        GLuint pixelShader;
        
        pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(pixelShader, 1, &params.shaderStr, nullptr);
        glCompileShader(pixelShader);
        
        ps.shaderObject = pixelShader;
        
        GLint compiled;
        glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &compiled);

        ShaderResult result;
        result.compiled = compiled != 0;
        if (result.compiled) {} else {
            GLint infoLogLength;
            glGetShaderiv(ps.shaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if (infoLogLength > 0) {
                glGetShaderInfoLog(ps.shaderObject, Math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
            }
        }

        return result;
    }
    template <typename _vertexLayout, typename _cbufferLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
    ShaderResult create_shader_set(RscShaderSet<_vertexLayout, _cbufferLayout, _cbufferUsage, _drawType>& ss, const ShaderSetRuntimeCompileParams<_vertexLayout, _cbufferLayout, _cbufferUsage, _drawType>& params) {
        GLuint shader;
        GLuint vs = params.rscVS.shaderObject;
        GLuint ps = params.rscPS.shaderObject;
        
        shader = glCreateProgram();
        glAttachShader(shader, vs);
        glAttachShader(shader, ps);
        glLinkProgram(shader);
        
        ss.shaderObject = shader;
        
        GLint compiled;
        glGetProgramiv(shader, GL_LINK_STATUS, &compiled);

        ShaderResult result;
        result.compiled = compiled != 0;
        if (result.compiled) {
            if (params.cbuffers) {
                bind_cbuffers_to_shader(ss, params.cbuffers);
            }
        } else {
            GLint infoLogLength;
            glGetProgramiv(ss.shaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
            if (infoLogLength > 0) {
                glGetProgramInfoLog(ss.shaderObject, Math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
            }
        }
        
        glDetachShader(shader, vs);
        glDetachShader(shader, ps);
        glDeleteShader(vs);
        glDeleteShader(ps);
        
        return result;
    }
    template <typename _vertexLayout, typename _cbufferLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
    void bind_shader(const RscShaderSet<_vertexLayout, _cbufferLayout, _cbufferUsage, _drawType>& ss) {
        glUseProgram(ss.shaderObject);
    }
    
    void create_blend_state(RscBlendState& bs, const BlendStateParams& params) {
        bs.enable = params.enable;
    }
    void bind_blend_state(const RscBlendState& bs) {
        if (bs.enable) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }
    
    void create_RS(RscRasterizerState& rs, const RasterizerStateParams& params) {
        rs.fillMode = (GLenum) params.fill;
        rs.cullFace = (GLenum) params.cull;
    }
    void bind_RS(const RscRasterizerState& rs) {
        glPolygonMode(GL_FRONT_AND_BACK, rs.fillMode);
        if (rs.cullFace != 0) {
            glEnable(GL_CULL_FACE);
            glCullFace(rs.cullFace);
        } else {
            glDisable(GL_CULL_FACE);
        }
        glFrontFace(GL_CW); // match dx
    }

    void create_DS(RscDepthStencilState& ds, const DepthStencilStateParams& params) {
        ds.enable = params.enable;
        ds.func = (GLenum) params.func;
        ds.writemask = (GLenum) params.writemask;
    }
    void bind_DS(const RscDepthStencilState& ds) {
        if (ds.enable) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(ds.func);
            glDepthMask(ds.writemask);
        }
        else {
            glDisable(GL_DEPTH_TEST);
        }
    }
    
    template <typename _layout>
    void create_vertex_buffer(RscBuffer<_layout>& t, const BufferParams& params) {
        GLuint vertexBuffer, arrayObject;
        
        // Vertex buffer binding is not part of the VAO state
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, params.vertexSize, params.vertexData, params.memoryUsage);
        glGenVertexArrays(1, &arrayObject);
        glBindVertexArray(arrayObject);
        {
            bind_vertex_layout<_layout>();
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        t.vertexBuffer = vertexBuffer;
        t.arrayObject = arrayObject;
        t.type = (GLenum) params.type;
        t.vertexCount = params.vertexCount;
    }
    template <typename _layout>
    void update_vertex_buffer(RscBuffer<_layout>& b, const BufferUpdateParams& params) {
        glBindBuffer(GL_ARRAY_BUFFER, b.vertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, params.vertexSize, params.vertexData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        b.vertexCount = params.vertexCount;
    }
    template <typename _layout>
    void bind_vertex_buffer(const RscBuffer<_layout>& b) {
        glBindVertexArray(b.arrayObject);
    }
    template <typename _layout>
    void draw_vertex_buffer(const RscBuffer<_layout>& b) {
        glDrawArrays(b.type, 0, b.vertexCount);
    }
    
    template <typename _layout>
    void create_indexed_vertex_buffer(RscIndexedBuffer<_layout>& t, const IndexedBufferParams& params) {
        GLuint vertexBuffer, indexBuffer, arrayObject;
        
        // Vertex buffer binding is not part of the VAO state
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, params.vertexSize, params.vertexData, params.memoryUsage);
        glGenVertexArrays(1, &arrayObject);
        glBindVertexArray(arrayObject);
        {
            bind_vertex_layout<_layout>();
            
            glGenBuffers(1, &indexBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, params.indexSize, params.indexData, params.memoryUsage);
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        t.vertexBuffer = vertexBuffer;
        t.indexBuffer = indexBuffer;
        t.arrayObject = arrayObject;
        t.indexType = (GLenum) params.indexType;
        t.type = (GLenum) params.type;
        t.indexCount = params.indexCount;
    }
    template <typename _layout>
    void update_indexed_vertex_buffer(RscIndexedBuffer<_layout>& b, const IndexedBufferUpdateParams& params) {
        glBindBuffer(GL_ARRAY_BUFFER, b.vertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, params.vertexSize, params.vertexData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.indexBuffer);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, params.indexSize, params.indexData);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        b.indexCount = params.indexCount;
    }
    template <typename _layout>
    void bind_indexed_vertex_buffer(const RscIndexedBuffer<_layout>& b) {
        glBindVertexArray(b.arrayObject);
    }
    template <typename _layout>
    void draw_indexed_vertex_buffer(const RscIndexedBuffer<_layout>& b) {
        glDrawElements(b.type, b.indexCount, b.indexType, nullptr);
    }
    template <typename _layout>
    void draw_instances_indexed_vertex_buffer(const RscIndexedBuffer<_layout>& b, const u32 instanceCount) {
        glDrawElementsInstanced(b.type, b.indexCount, b.indexType, nullptr, instanceCount);
    }

    void draw_fullscreen() {
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    template <typename _layout>
    void create_cbuffer(RscCBuffer& cb, const CBufferCreateParams& params) {
        GLuint buffer, index;
        
        glGenBuffers(1, &buffer);
        index = buffer; // todo, index should be controlled by renderer?
        
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(_layout), nullptr, GL_STATIC_DRAW);
        
        glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer);
        
        cb.index = index;
        cb.bufferObject = buffer;
    }
    template <typename _layout>
    void update_cbuffer(RscCBuffer& cb, const _layout& data) {
        glBindBuffer(GL_UNIFORM_BUFFER, cb.bufferObject);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(_layout), &data);
    }
    void bind_cbuffers(const RscCBuffer* cb, const u32 count, const CBufferBindParams& params) {}

    void set_marker(Marker_t) {
    }
    void start_event(Marker_t data) {
        if (glPushDebugGroup) {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, data);
        }
    }
    void end_event() {
        if (glPopDebugGroup) {
            glPopDebugGroup();
        }
    }
}
}

// supported vertex layouts
namespace Renderer {
namespace Driver {
    
    template <>
    void bind_vertex_layout<Layout_Vec3>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3), (void*)0);
        glEnableVertexAttribArray(0);
    }
    template <>
    void bind_vertex_layout<Layout_TexturedVec2>() {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedVec2), &(((Layout_TexturedVec2*)0)->pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedVec2), &(((Layout_TexturedVec2*)0)->uv));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    template <>
    void bind_vertex_layout<Layout_TexturedVec3>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedVec3), &(((Layout_TexturedVec3*)0)->pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedVec3), &(((Layout_TexturedVec3*)0)->uv));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    template <>
    void bind_vertex_layout<Layout_TexturedSkinnedVec3>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedSkinnedVec3), &(((Layout_TexturedSkinnedVec3*)0)->pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedSkinnedVec3), &(((Layout_TexturedSkinnedVec3*)0)->uv));
        glVertexAttribPointer(2, 4, GL_BYTE, GL_FALSE, sizeof(Layout_TexturedSkinnedVec3), &(((Layout_TexturedSkinnedVec3*)0)->joint_indices));
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Layout_TexturedSkinnedVec3), &(((Layout_TexturedSkinnedVec3*)0)->joint_weights));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
    }
    template <>
    void bind_vertex_layout<Layout_Vec3TexturedMapped>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3TexturedMapped), &(((Layout_Vec3TexturedMapped*)0)->pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3TexturedMapped), &(((Layout_Vec3TexturedMapped*)0)->uv));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3TexturedMapped), &(((Layout_Vec3TexturedMapped*)0)->normal));
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3TexturedMapped), &(((Layout_Vec3TexturedMapped*)0)->tangent));
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3TexturedMapped), &(((Layout_Vec3TexturedMapped*)0)->bitangent));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
    }
    template <>
    void bind_vertex_layout<Layout_Vec2Color4B>() {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec2Color4B), &(((Layout_Vec2Color4B*)0)->pos));
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Layout_Vec2Color4B), &(((Layout_Vec2Color4B*)0)->color));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    template <>
    void bind_vertex_layout<Layout_Vec3Color4B>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3Color4B), &(((Layout_Vec3Color4B*)0)->pos));
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Layout_Vec3Color4B), &(((Layout_Vec3Color4B*)0)->color));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    template <>
    void bind_vertex_layout<Layout_Vec3Color4BSkinned>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3Color4BSkinned), &(((Layout_Vec3Color4BSkinned*)0)->pos));
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Layout_Vec3Color4BSkinned), &(((Layout_Vec3Color4BSkinned*)0)->color));
        glVertexAttribPointer(2, 4, GL_BYTE, GL_FALSE, sizeof(Layout_Vec3Color4BSkinned), &(((Layout_Vec3Color4BSkinned*)0)->joint_indices));
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Layout_Vec3Color4BSkinned), &(((Layout_Vec3Color4BSkinned*)0)->joint_weights));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
    }
    template <typename _vertexLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
    void bind_cbuffers_to_shader(RscShaderSet<_vertexLayout, Layout_CNone, _cbufferUsage, _drawType>& ss, const RscCBuffer* cbuffers) {}
    template <typename _vertexLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
    void bind_cbuffers_to_shader(RscShaderSet<_vertexLayout, Layout_CBuffer_3DScene, _cbufferUsage, _drawType>& ss, const RscCBuffer* cbuffers) {
        GLuint perScene = glGetUniformBlockIndex(ss.shaderObject, "type_PerScene");
        GLuint perRenderGroup = glGetUniformBlockIndex(ss.shaderObject, "type_PerGroup");
        GLuint perInstance = glGetUniformBlockIndex(ss.shaderObject, "type_PerInstance");
        if (perScene != GL_INVALID_INDEX)
            glUniformBlockBinding(ss.shaderObject, perScene, cbuffers[Layout_CBuffer_3DScene::Buffers::SceneData].index);
        if (perRenderGroup != GL_INVALID_INDEX)
            glUniformBlockBinding(ss.shaderObject, perRenderGroup, cbuffers[Layout_CBuffer_3DScene::Buffers::GroupData].index);
        if (perInstance != GL_INVALID_INDEX)
            glUniformBlockBinding(ss.shaderObject, perInstance, cbuffers[Layout_CBuffer_3DScene::Buffers::InstanceData].index);
    }
    template <typename _vertexLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
    void bind_cbuffers_to_shader(RscShaderSet<_vertexLayout, Layout_CBuffer_LightPass, _cbufferUsage, _drawType>& ss, const RscCBuffer* cbuffers) {
        GLuint perScene = glGetUniformBlockIndex(ss.shaderObject, "type_PerScene");
        glUniformBlockBinding(ss.shaderObject, perScene, cbuffers[Layout_CBuffer_LightPass::Buffers::SceneData].index);
    }
    template <typename _vertexLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
    void bind_cbuffers_to_shader(RscShaderSet<_vertexLayout, Layout_CBuffer_DebugScene, _cbufferUsage, _drawType>& ss, const RscCBuffer* cbuffers) {
        u32 renderGroupBlock = glGetUniformBlockIndex(ss.shaderObject, "type_PerGroup");
        glUniformBlockBinding(ss.shaderObject, renderGroupBlock, cbuffers[Layout_CBuffer_DebugScene::Buffers::GroupData].index);
    }
}
}
#endif // __WASTELADNS_RENDERER_GLFW_H__
