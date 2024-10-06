#ifndef __WASTELADNS_RENDERER_GL33_H__
#define __WASTELADNS_RENDERER_GL33_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {
namespace Driver {

    void create_main_RT(RscMainRenderTarget& rt, const MainRenderTargetParams& params) {
        rt.mask = GL_COLOR_BUFFER_BIT;
        if (params.depth) { rt.mask = rt.mask | GL_DEPTH_BUFFER_BIT; }
    }
    void clear_main_RT(RscMainRenderTarget& rt, Color32 color) {
        glClearColor( RGBA_PARAMS(color) );
        glClear(rt.mask | GL_STENCIL_BUFFER_BIT);
    }
    void bind_main_RT(RscMainRenderTarget& rt) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void create_RT(RscRenderTarget& rt, const RenderTargetParams& params) {
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
            GLuint attachments[COUNT_OF(rt.textures)];
            for (u32 i = 0; i < params.count; i++) {
                Renderer::Driver::TextureRenderTargetCreateParams texParams;
                texParams.width = params.width;
                texParams.height = params.height;
                texParams.format = params.textureFormat;
                texParams.internalFormat = params.textureInternalFormat;
                texParams.type = params.textureFormatType;

                create_texture_empty(rt.textures[i], texParams);
                u32 colorAttachment = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachment, GL_TEXTURE_2D, rt.textures[i].id, 0);
                attachments[i] = colorAttachment;
            }
            rt.count = params.count;
            glDrawBuffers(params.count, attachments);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    void bind_RT(const RscRenderTarget& rt) {
        glBindFramebuffer(GL_FRAMEBUFFER, rt.buffer);
    }
    void clear_RT(const RscRenderTarget& rt) {
        glClear(rt.mask | GL_STENCIL_BUFFER_BIT);
    }
    void clear_RT(const RscRenderTarget& rt, Color32 color) {
        glClearColor(RGBA_PARAMS(color));
        glClear(rt.mask | GL_STENCIL_BUFFER_BIT);
    }
    void copy_RT_to_main_RT(RscMainRenderTarget& dst, const RscRenderTarget& src, const RenderTargetCopyParams& params) {
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

            t.id = texId;
            
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

        t.id = texId;
    }
    void bind_textures(const RscTexture* textures, const u32 count) {
        for (u32 i = 0; i < count; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
    }
    void bind_shader_samplers(RscShaderSet& ss, const char** params, const u32 count) {

    }
    
    ShaderResult create_shader_vs(RscVertexShader& vs, const VertexShaderRuntimeCompileParams& params) {
        GLuint vertexShader;
        
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &params.shader_str, nullptr);
        glCompileShader(vertexShader);
        
        vs.id = vertexShader;
        
        GLint compiled;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);

        ShaderResult result;
        result.compiled = compiled != 0;
        if (result.compiled) {} else {
            GLint infoLogLength;
            glGetShaderiv(vs.id, GL_INFO_LOG_LENGTH, &infoLogLength);
            if (infoLogLength > 0) {
                glGetShaderInfoLog(vs.id, Math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
            }
        }

        return result;
    }
    ShaderResult create_shader_ps(RscPixelShader& ps, const PixelShaderRuntimeCompileParams& params) {
        GLuint pixelShader;
        
        pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(pixelShader, 1, &params.shader_str, nullptr);
        glCompileShader(pixelShader);
        
        ps.id = pixelShader;
        
        GLint compiled;
        glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &compiled);

        ShaderResult result;
        result.compiled = compiled != 0;
        if (result.compiled) {} else {
            GLint infoLogLength;
            glGetShaderiv(ps.id, GL_INFO_LOG_LENGTH, &infoLogLength);
            if (infoLogLength > 0) {
                glGetShaderInfoLog(ps.id, Math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
            }
        }

        return result;
    }
    ShaderResult create_shader_set(RscShaderSet& ss, const ShaderSetRuntimeCompileParams& params) {
        GLuint shader;
        GLuint vs = params.vs.id;
        GLuint ps = params.ps.id;
        
        shader = glCreateProgram();
        glAttachShader(shader, vs);
        glAttachShader(shader, ps);
        glLinkProgram(shader);
        
        ss.id = shader;
        
        GLint compiled;
        glGetProgramiv(shader, GL_LINK_STATUS, &compiled);

        ShaderResult result;
        result.compiled = compiled != 0;
        if (result.compiled) {
			for (u32 i = 0; i < params.cbuffer_count; i++) {
				const CBufferBindingDesc& binding = params.cbufferBindings[i];
                GLuint index = glGetUniformBlockIndex(ss.id, binding.name);
                if (index != GL_INVALID_INDEX) {
                    glUniformBlockBinding(ss.id, index, binding.cbuffer.binding_index);
                }
			}
            glUseProgram(ss.id);
            for (u32 i = 0; i < params.texture_count; i++) {
                const TextureBindingDesc& binding = params.textureBindings[i];
                const s32 index = glGetUniformLocation(ss.id, binding.name);
                glUniform1i(index, i);
            }
        } else {
            GLint infoLogLength;
            glGetProgramiv(ss.id, GL_INFO_LOG_LENGTH, &infoLogLength);
            if (infoLogLength > 0) {
                glGetProgramInfoLog(ss.id, Math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
            }
        }
        
        glDetachShader(shader, vs);
        glDetachShader(shader, ps);
        glDeleteShader(vs);
        glDeleteShader(ps);
        
        return result;
    }
    void bind_shader(const RscShaderSet& ss) {
        glUseProgram(ss.id);
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

    void create_vertex_buffer(RscVertexBuffer& t, const VertexBufferDesc& params, const VertexAttribDesc* attrs, const u32 attr_count) {
        GLuint vertexBuffer, arrayObject;
        
        // Vertex buffer binding is not part of the VAO state
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, params.vertexSize, params.vertexData, params.memoryUsage);
        glGenVertexArrays(1, &arrayObject);
        glBindVertexArray(arrayObject);
        for (u32 i = 0; i < attr_count; i++) { // bind vertex layout
            const VertexAttribDesc& attr = attrs[i];
            glVertexAttribPointer(i, attr.size, attr.type, attr.normalized, attr.stride, (const void*)attr.offset);
            glEnableVertexAttribArray(i);
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        t.vertexBuffer = vertexBuffer;
        t.arrayObject = arrayObject;
        t.type = (GLenum) params.type;
        t.vertexCount = params.vertexCount;
    }
    void update_vertex_buffer(RscVertexBuffer& b, const BufferUpdateParams& params) {
        glBindBuffer(GL_ARRAY_BUFFER, b.vertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, params.vertexSize, params.vertexData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        b.vertexCount = params.vertexCount;
    }
    void bind_vertex_buffer(const RscVertexBuffer& b) {
        glBindVertexArray(b.arrayObject);
    }
    void draw_vertex_buffer(const RscVertexBuffer& b) {
        glDrawArrays(b.type, 0, b.vertexCount);
    }
    
    void create_indexed_vertex_buffer(RscIndexedVertexBuffer& t, const IndexedVertexBufferDesc& params, const VertexAttribDesc* attrs, const u32 attr_count) {
        GLuint vertexBuffer, indexBuffer, arrayObject;
        
        // Vertex buffer binding is not part of the VAO state
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, params.vertexSize, params.vertexData, params.memoryUsage);
        glGenVertexArrays(1, &arrayObject);
        glBindVertexArray(arrayObject);
        for (u32 i = 0; i < attr_count; i++) { // bind vertex layout
            const VertexAttribDesc& attr = attrs[i];
            glVertexAttribPointer(i, attr.size, attr.type, attr.normalized, attr.stride, (const void*)attr.offset);
            glEnableVertexAttribArray(i);
        }
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, params.indexSize, params.indexData, params.memoryUsage);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        t.vertexBuffer = vertexBuffer;
        t.indexBuffer = indexBuffer;
        t.arrayObject = arrayObject;
        t.indexType = (GLenum) params.indexType;
        t.type = (GLenum) params.type;
        t.indexCount = params.indexCount;
    }
    void update_indexed_vertex_buffer(RscIndexedVertexBuffer& b, const IndexedBufferUpdateParams& params) {
        glBindBuffer(GL_ARRAY_BUFFER, b.vertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, params.vertexSize, params.vertexData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.indexBuffer);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, params.indexSize, params.indexData);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        b.indexCount = params.indexCount;
    }
    void bind_indexed_vertex_buffer(const RscIndexedVertexBuffer& b) {
        glBindVertexArray(b.arrayObject);
    }
    void draw_indexed_vertex_buffer(const RscIndexedVertexBuffer& b) {
        glDrawElements(b.type, b.indexCount, b.indexType, nullptr);
    }
    void draw_instances_indexed_vertex_buffer(const RscIndexedVertexBuffer& b, const u32 instanceCount) {
        glDrawElementsInstanced(b.type, b.indexCount, b.indexType, nullptr, instanceCount);
    }

    void draw_fullscreen() {
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    void create_cbuffer(RscCBuffer& cb, const CBufferCreateParams& params) {
        GLuint buffer, index;
        
        glGenBuffers(1, &buffer);
        index = params.bindingIndex;
        
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferData(GL_UNIFORM_BUFFER, params.byteWidth, nullptr, GL_STATIC_DRAW);
        
        // TODO: can these be common? if not, why use a separate index??
        glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer);
        
        cb.id = buffer;
        cb.binding_index = index;
		cb.byteWidth = params.byteWidth;
    }
    void update_cbuffer(RscCBuffer& cb, const void* data) {
        glBindBuffer(GL_UNIFORM_BUFFER, cb.id);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, cb.byteWidth, data);
    }
    void bind_cbuffers(const RscCBuffer* cb, const u32 count, const CBufferBindParams& params) {}

    void set_marker_name(Marker_t& wide, const char* ansi) { wide = ansi; }
    void start_event(Marker_t data) { if (glPushDebugGroup) { glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, data); } }
    void end_event() { if (glPopDebugGroup) { glPopDebugGroup(); } }
}
}
#endif // __WASTELADNS_RENDERER_GL33_H__
