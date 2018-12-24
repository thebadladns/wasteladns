#ifndef __WASTELADNS_RENDERER_GLFW_H__
#define __WASTELADNS_RENDERER_GLFW_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {
namespace Driver {
    GLenum toGLActiveTexture[] = { GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE3, GL_TEXTURE4 };
    GLenum toGLRasterizerFillMode[] = { GL_FILL, GL_LINE };
    GLenum toGLBufferMemoryMode[] = { GL_STATIC_DRAW, GL_DYNAMIC_DRAW };
    GLenum toGLBufferItemType[] = { GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };
    GLenum toGLBufferTopologyType[] = { GL_TRIANGLES, GL_LINES };
    
    void create(RscMainRenderTarget& rt, const RenderTargetParams& params) {
        rt.mask = GL_COLOR_BUFFER_BIT;
        if (params.depth) {
            rt.mask = rt.mask | GL_DEPTH_BUFFER_BIT;
            glEnable(GL_DEPTH_TEST);
        }
    }
    void clear(const RscMainRenderTarget& rt, Col color) {
        glClearColor( RGBA_PARAMS(color) );
        glClear(rt.mask);
    }
    
    struct TextureFromFileParams {
        const char* path;
    };
    void create(RscTexture& t, const TextureFromFileParams& params) {
        s32 w, h, channels;
        u8* data = stbi_load(params.path, &w, &h, &channels, 0);
        if (data) {
            GLenum format;
            switch (channels) {
                case 1: format = GL_RED; break;
                case 3: format = GL_RGB; break;
                case 4: format = GL_RGBA; break;
            }
            GLuint texId;
            GLenum error = glGetError();
            
            glGenTextures(1, &texId);
            error = glGetError();
            glBindTexture(GL_TEXTURE_2D, texId);
            error = glGetError();
            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
            error = glGetError();
            glGenerateMipmap(GL_TEXTURE_2D);
            error = glGetError();
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            error = glGetError();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            error = glGetError();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            error = glGetError();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            error = glGetError();
            
            t.texId = texId;
            
            stbi_image_free(data);
        }
    }
    void bind(const RscTexture* textures, const u32 count) {
        for (u32 i = 0; i < count; i++) {
            glActiveTexture(GL_TEXTURE0);//toGLActiveTexture[i]);
            glBindTexture(GL_TEXTURE_2D, textures[i].texId);
        }
    }
    
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create(RscVertexShader<_vertexLayout, _cbufferLayout>& vs, const VertexShaderRuntimeCompileParams& params) {
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
    ShaderResult create(RscPixelShader& ps, const PixelShaderRuntimeCompileParams& params) {
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
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create(RscShaderSet<_vertexLayout, _cbufferLayout>& ss, const ShaderSetRuntimeCompileParams<_vertexLayout, _cbufferLayout>& params) {
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
            bindCBuffers(ss, params.cbuffers);
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
    template <typename _vertexLayout, typename _cbufferLayout>
    void bind(const RscShaderSet<_vertexLayout, _cbufferLayout>& ss) {
        glUseProgram(ss.shaderObject);
    }
    
    void create(RscBlendState& bs, const BlendStateParams& params) {
        bs.enable = params.enable;
    }
    void bind(const RscBlendState bs) {
        if (bs.enable) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
    }
    
    void create(RscRasterizerState& rs, const RasterizerStateParams& params) {
        rs.fillMode = toGLRasterizerFillMode[params.fill];
        rs.cullFace = params.cullFace;
    }
    void bind(const RscRasterizerState rs) {
        glPolygonMode(GL_FRONT_AND_BACK, rs.fillMode);
        if (rs.cullFace) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }
    
    template <typename _layout>
    void create(RscBuffer<_layout>& t, const BufferParams& params) {
        GLuint vertexBuffer, arrayObject;
        GLenum glMemoryMode = toGLBufferMemoryMode[params.memoryMode];
        
        // Vertex buffer binding is not part of the VAO state
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, params.vertexSize, params.vertexData, glMemoryMode);
        glGenVertexArrays(1, &arrayObject);
        glBindVertexArray(arrayObject);
        {
            bindLayout<_layout>();
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        t.vertexBuffer = vertexBuffer;
        t.arrayObject = arrayObject;
        t.type = toGLBufferTopologyType[params.type];
        t.vertexCount = params.vertexCount;
    }
    template <typename _layout>
    void update(RscBuffer<_layout>& b, const BufferUpdateParams& params) {
        glBindBuffer(GL_ARRAY_BUFFER, b.vertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, params.vertexSize, params.vertexData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        b.vertexCount = params.vertexCount;
    }
    template <typename _layout>
    void bind(const RscBuffer<_layout>& b) {
        glBindVertexArray(b.arrayObject);
    }
    template <typename _layout>
    void draw(const RscBuffer<_layout>& b) {
        glDrawArrays(b.type, 0, b.vertexCount);
    }
    
    template <typename _layout>
    void create(RscIndexedBuffer<_layout>& t, const IndexedBufferParams& params) {
        GLuint vertexBuffer, indexBuffer, arrayObject;
        GLenum glMemoryMode = toGLBufferMemoryMode[params.memoryMode];
        
        // Vertex buffer binding is not part of the VAO state
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, params.vertexSize, params.vertexData, glMemoryMode);
        glGenVertexArrays(1, &arrayObject);
        glBindVertexArray(arrayObject);
        {
            bindLayout<_layout>();
            
            glGenBuffers(1, &indexBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, params.indexSize, params.indexData, glMemoryMode);
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        t.vertexBuffer = vertexBuffer;
        t.indexBuffer = indexBuffer;
        t.arrayObject = arrayObject;
        t.indexType = toGLBufferItemType[params.indexType];
        t.type = toGLBufferTopologyType[params.type];
        t.indexCount = params.indexCount;
    }
    template <typename _layout>
    void update(RscIndexedBuffer<_layout>& b, const IndexedBufferUpdateParams& params) {
        glBindBuffer(GL_ARRAY_BUFFER, b.vertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, params.vertexSize, params.vertexData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.indexBuffer);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, params.indexSize, params.indexData);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        b.indexCount = params.indexCount;
    }
    template <typename _layout>
    void bind(const RscIndexedBuffer<_layout>& b) {
        glBindVertexArray(b.arrayObject);
    }
    template <typename _layout>
    void draw(const RscIndexedBuffer<_layout>& b) {
        glDrawElements(b.type, b.indexCount, b.indexType, nullptr);
    }
    template <typename _layout>
    void drawInstances(const RscIndexedBuffer<_layout>& b, const u32 instanceCount) {
        glDrawElementsInstanced(b.type, b.indexCount, b.indexType, nullptr, instanceCount);
    }
    
    template <typename _layout>
    void create(RscCBuffer& cb, const CBufferCreateParams& params) {
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
    void update(RscCBuffer& cb, const _layout& data) {
        glBindBuffer(GL_UNIFORM_BUFFER, cb.bufferObject);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(_layout), &data);
    }
    void bind(const RscCBuffer* cb, u32 count) {}

}
}

// supported vertex layouts
namespace Renderer {    
namespace Driver {
    
    template <>
    void bindLayout<Layout_Vec3>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3), (void*)0);
        glEnableVertexAttribArray(0);
    }
    template <>
    void bindLayout<Layout_TexturedVec3>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedVec3), &(((Layout_TexturedVec3*)0)->pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedVec3), &(((Layout_TexturedVec3*)0)->uv));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_TexturedVec3), &(((Layout_TexturedVec3*)0)->normal));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
    }
    template <>
    void bindLayout<Layout_Vec2Color4B>() {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec2Color4B), &(((Layout_Vec2Color4B*)0)->pos));
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Layout_Vec2Color4B), &(((Layout_Vec2Color4B*)0)->color));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    template <>
    void bindLayout<Layout_Vec3Color4B>() {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Layout_Vec3Color4B), &(((Layout_Vec3Color4B*)0)->pos));
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Layout_Vec3Color4B), &(((Layout_Vec3Color4B*)0)->color));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    template <typename _vertexLayout>
    void bindCBuffers(RscShaderSet<_vertexLayout, Layout_CBuffer_3DScene::Buffers>& ss, const RscCBuffer* cbuffers) {
        GLuint perScene = glGetUniformBlockIndex(ss.shaderObject, "PerScene");
        GLuint perRenderGroup = glGetUniformBlockIndex(ss.shaderObject, "PerGroup");
        
        glUniformBlockBinding(ss.shaderObject, perScene, cbuffers[Layout_CBuffer_3DScene::Buffers::SceneData].index);
        glUniformBlockBinding(ss.shaderObject, perRenderGroup, cbuffers[Layout_CBuffer_3DScene::Buffers::GroupData].index);
    }
    template <typename _vertexLayout>
    void bindCBuffers(RscShaderSet<_vertexLayout, Layout_CBuffer_DebugScene::Buffers>& ss, const RscCBuffer* cbuffers) {
        u32 renderGroupBlock = glGetUniformBlockIndex(ss.shaderObject, "PerGroup");
        glUniformBlockBinding(ss.shaderObject, renderGroupBlock, cbuffers[Layout_CBuffer_DebugScene::Buffers::GroupData].index);
    }
}
}
#endif // __WASTELADNS_RENDERER_GLFW_H__
