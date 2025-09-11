#ifndef __WASTELADNS_RHI_GL33_H__
#define __WASTELADNS_RHI_GL33_H__

namespace gfx {
namespace rhi { // render hardware interface

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
    rt.width = params.width;
    rt.height = params.height;
    if (params.flags & RenderTargetParams::Flags::EnableDepth) {
        if (params.flags & RenderTargetParams::Flags::ReadDepth) {
            // create a texture so we can sample depth as a shader input
            glGenTextures(1, &rt.depthStencil.id);
            glBindTexture(GL_TEXTURE_2D, rt.depthStencil.id);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
                params.width, params.height, 0,
                GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            // ensure the parameters are unpacked correctly
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(
                GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, rt.depthStencil.id, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } else {
            GLuint depthBuffer;
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, params.width, params.height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            rt.depthBuffer = depthBuffer;
            rt.depthStencil = {}; // todo: assert if use as shader input
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    {
        glBindFramebuffer(GL_FRAMEBUFFER, rt.buffer);
        GLuint attachments[countof(rt.textures)];
        for (u32 i = 0; i < params.count; i++) {
            gfx::rhi::TextureRenderTargetCreateParams texParams;
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
void clear_RT(const RscRenderTarget& rt, u32 flags) {
    if (flags & RenderTargetClearFlags::Depth) { glEnable(GL_DEPTH_TEST), glDepthMask(DepthWriteMask::All); } // todo: restore??
    glClear(flags);
}
void clear_RT(const RscRenderTarget& rt, u32 flags, Color32 color) {
    if (flags & RenderTargetClearFlags::Depth) { glEnable(GL_DEPTH_TEST), glDepthMask(DepthWriteMask::All); } // todo: restore??
    glClearColor(RGBA_PARAMS(color));
    glClear(flags | GL_COLOR_BUFFER_BIT);
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
    u8* data = stbi_load_arena(params.path, &w, &h, &channels, 4, params.arena);
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
void bind_shader_samplers(RscShaderSet& ss, const char** params, const u32 count) {}

ShaderResult create_shader_vs(RscVertexShader& vs, const VertexShaderRuntimeCompileParams& params) {
    GLuint vertexShader;
        
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char**)&params.shader_src, nullptr);
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
            glGetShaderInfoLog(vs.id, math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
        }
    }

    return result;
}
ShaderResult create_shader_ps(RscPixelShader& ps, const PixelShaderRuntimeCompileParams& params) {
    GLuint pixelShader;
        
    pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pixelShader, 1, (const char**)&params.shader_src, nullptr);
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
            glGetShaderInfoLog(ps.id, math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
        }
    }

    return result;
}
#if __DEBUG
ShaderResult recompile_shaderfile(RscShaderSet& shader, const char* path, GLenum type) {
    // read contents of file, relying on malloc, since this is intended for debug only
    // consider using a scratch allocator if this is ever used on release
    char* buff;
    {
        FILE* f;
        io::fopen(&f, path, "rb");
        fseek(f, 0, SEEK_END);
        s32 filesize = io::ftell(f);
        fseek(f, 0, SEEK_SET); // or rewind(f)
        buff = (char*)malloc(filesize + 1);
        fread(buff, filesize, 1, f);
        io::fclose(f);
        buff[filesize] = 0; // c-style buffer
    }

    GLuint shaderProgram = glCreateShader(type);
    glShaderSource(shaderProgram, 1, (const char**)&buff, nullptr);
    glCompileShader(shaderProgram);

    GLint compiled;
    glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &compiled);

    ShaderResult result;
    result.compiled = compiled != 0;
    if (result.compiled) {

        GLuint shaders[8];
        GLsizei count;
        glGetAttachedShaders(shader.id, countof(shaders), &count, shaders);

        for (GLsizei i = 0; i < count; i++) {
            GLint shaderType;
            glGetShaderiv(shaders[i], GL_SHADER_TYPE, &shaderType);
            if (shaderType == type) {
                glDetachShader(shader.id, shaders[i]);
                glAttachShader(shader.id, shaderProgram);
                glLinkProgram(shader.id);
                break;
            }
        }
    }
    else {
        GLint infoLogLength;
        glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0) {
            glGetShaderInfoLog(
                shaderProgram,
                math::min(infoLogLength, 
                    (GLint)(sizeof(result.error) / sizeof(result.error[0]))), 
                nullptr, &result.error[0]);
        }
    }

    glDeleteShader(shaderProgram);
    free(buff);

    return result;
}
ShaderResult recompile_shaderfile_vs(RscShaderSet& shader, const char* path) {
    return recompile_shaderfile(shader, path, GL_VERTEX_SHADER);
}
ShaderResult recompile_shaderfile_ps(RscShaderSet& shader, const char* path) {
    return recompile_shaderfile(shader, path, GL_FRAGMENT_SHADER);
}
#endif // __DEBUG
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
                glUniformBlockBinding(ss.id, index, i);
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
            glGetProgramInfoLog(ss.id, math::min(infoLogLength, (GLint)(sizeof(result.error)/sizeof(result.error[0]))), nullptr, &result.error[0]);
        }
    }

#if !__DEBUG
    // in debug mode, we'll remember which shader was attached,
    // so that we can easily hot-swap them
    glDetachShader(shader, vs);
    glDetachShader(shader, ps);
    glDeleteShader(vs);
    glDeleteShader(ps);
#endif
        
    return result;
}
void bind_shader(const RscShaderSet& ss) {
    glUseProgram(ss.id);
}
    
void create_blend_state(RscBlendState& bs, const BlendStateParams& params) {
    bs.blendEnable = params.blendEnable;
    bs.writeColor = params.renderTargetWriteMask;
}
void bind_blend_state(const RscBlendState& bs) {
    if (bs.blendEnable) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
    if (bs.writeColor) {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    } else {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
}
    
void create_RS(RscRasterizerState& rs, const RasterizerStateParams& params) {
    rs.fillMode = (GLenum) params.fill;
    rs.cullFace = (GLenum) params.cull;
    rs.scissor = params.scissor;
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
    if (rs.scissor) { glEnable(GL_SCISSOR_TEST); }
    else { glDisable(GL_SCISSOR_TEST); }
}
void set_scissor(const u32 left, const u32 top, const u32 right, const u32 bottom) {
    // todo: test y
    glScissor(left, top, right - left, bottom - top);
}
void create_DS(RscDepthStencilState& ds, const DepthStencilStateParams& params) {
    ds.depth_enable = params.depth_enable;
    ds.depth_func = (GLenum) params.depth_func;
    ds.depth_writemask = (GLenum) params.depth_writemask;
    ds.stencil_enable = params.stencil_enable;

    ds.stencil_enable = params.stencil_enable;
    ds.stencil_readmask = params.stencil_readmask;
    ds.stencil_writemask = params.stencil_writemask;
    ds.stencil_failOp = (GLenum)params.stencil_failOp;
    ds.stencil_depthFailOp = (GLenum)params.stencil_depthFailOp;
    ds.stencil_passOp = (GLenum)params.stencil_passOp;
    ds.stencil_func = (GLenum)params.stencil_func;
}
void bind_DS(const RscDepthStencilState& ds, const u32 stencilRef = 0) {
    if (ds.depth_enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(ds.depth_func);
        glDepthMask(ds.depth_writemask);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    if (ds.stencil_enable) {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(ds.stencil_func, stencilRef, ds.stencil_readmask);
        glStencilOp(ds.stencil_failOp, ds.stencil_depthFailOp, ds.stencil_passOp);
        glStencilMask(ds.stencil_writemask);
    } else {
        glDisable(GL_STENCIL_TEST);
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
        glVertexAttribPointer(i, attr.size, attr.type, attr.normalized, (GLsizei)attr.stride, (const void*)attr.offset);
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
        glVertexAttribPointer(i, attr.size, attr.type, attr.normalized, (GLsizei)attr.stride, (const void*)attr.offset);
        glEnableVertexAttribArray(i);
    }
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, params.indexSize, params.indexData, params.memoryUsage);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
    t.vertexBuffer = vertexBuffer;
    t.indexBuffer = indexBuffer;
    t.arrayObject = arrayObject;
    t.indexType = (GLenum) params.indexType;
    t.type = (GLenum) params.type;
    t.indexCount = params.indexCount;
    t.indexOffset = 0;
}
void update_indexed_vertex_buffer(RscIndexedVertexBuffer& b, const IndexedBufferUpdateParams& params) {
    glBindVertexArray(0); // make sure we don't accidentally unbind any buffers here
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
    const size_t index_size = (b.indexType == BufferItemType::Enum::U16) ? sizeof(u16) : sizeof(u32);
    glDrawElements(b.type, b.indexCount, b.indexType, (void*)(b.indexOffset * index_size));
}
void draw_instances_indexed_vertex_buffer(const RscIndexedVertexBuffer& b, const u32 instanceCount) {
    glDrawElementsInstanced(b.type, b.indexCount, b.indexType, nullptr, instanceCount);
}

void draw_fullscreen() {
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
    
void create_cbuffer(RscCBuffer& cb, const CBufferCreateParams& params) {
    GLuint buffer;
        
    glGenBuffers(1, &buffer);
        
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, params.byteWidth, nullptr, GL_STATIC_DRAW);
        
    cb.id = buffer;
	cb.byteWidth = params.byteWidth;
}
void update_cbuffer(RscCBuffer& cb, const void* data) {
    glBindBuffer(GL_UNIFORM_BUFFER, cb.id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, cb.byteWidth, data);
}
void bind_cbuffers(const RscShaderSet&, const RscCBuffer* cb, const u32 count) {
    for (u32 i = 0; i < count; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, i, cb[i].id);
    }
}

#if __PROFILE
void start_event(const char* name) { if (glPushDebugGroup) { glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name); } }
void end_event() { if (glPopDebugGroup) { glPopDebugGroup(); } }
#endif
}
}
#endif // __WASTELADNS_RHI_GL33_H__
