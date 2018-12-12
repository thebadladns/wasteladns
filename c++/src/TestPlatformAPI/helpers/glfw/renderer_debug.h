#ifndef __WASTELADNS_RENDERER_DEBUG_GLFW_H__
#define __WASTELADNS_RENDERER_DEBUG_GLFW_H__

#ifndef UNITYBUILD
#include "../color.h"
#include "../angle.h"
#include "../camera.h"
#include "../renderer_debug.h"
#include "core.h"
#endif

namespace Renderer
{
    namespace Immediate
    {
        void load(Buffer& buffer) {

            // 3d
            {
                u32 vertexBuffer, layoutBuffer;

                // Vertex buffer binding is not part of the VAO state
                glGenBuffers(1, &vertexBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Buffer::vertexMemory), nullptr, GL_DYNAMIC_DRAW);
                glGenVertexArrays(1, &layoutBuffer);
                glBindVertexArray(layoutBuffer);
                {
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &(((Vertex*)0)->pos));
                    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), &(((Vertex*)0)->color));
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                }
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                buffer.vertexBufferIndex = vertexBuffer;
                buffer.layoutBufferIndex = layoutBuffer;
            }
            // 2d
            {
                u32 vertexBuffer, indexBuffer, layoutBuffer;

                // Vertex buffer binding is not part of the VAO state
                glGenBuffers(1, &vertexBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Buffer::charVertexMemory), nullptr, GL_DYNAMIC_DRAW);
                glGenVertexArrays(1, &layoutBuffer);
                glBindVertexArray(layoutBuffer);
                {
                    // Shader expects vec3, we'll pass in a vec2 and expect z to default to 0
                    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 12, 0);
                    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 12, (void*)8);
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);

                    glGenBuffers(1, &indexBuffer);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Buffer::charIndexVertexMemory), nullptr, GL_DYNAMIC_DRAW);
                }
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                buffer.charVertexBufferIndex = vertexBuffer;
                buffer.charIndexBufferIndex = indexBuffer;
                buffer.charLayoutBufferIndex = layoutBuffer;
            }

            u32 coloredVertexProgram = {};
            {
                u32 vertexShader;
                u32 pixelShader;

                s32 compiled, infoLogLength;

                vertexShader = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vertexShader, 1, &coloredVertexShaderStr, nullptr);
                glCompileShader(vertexShader);
                glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
                if (compiled) {
                    pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
                    glShaderSource(pixelShader, 1, &pixelShaderStr, nullptr);
                    glCompileShader(pixelShader);
                    glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &compiled);
                    if (compiled) {
                        coloredVertexProgram = glCreateProgram();
                        glAttachShader(coloredVertexProgram, vertexShader);
                        glAttachShader(coloredVertexProgram, pixelShader);
                        glLinkProgram(coloredVertexProgram);

                        glGetProgramiv(coloredVertexProgram, GL_LINK_STATUS, &compiled);
                        if (compiled) {
                            u32 renderGroupBlock = glGetUniformBlockIndex(coloredVertexProgram, "PerRenderGroup");
                            glUniformBlockBinding(coloredVertexProgram, renderGroupBlock, 2);
                        } else {
                            glGetProgramiv(coloredVertexProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
                            if (infoLogLength) {
                                char error[128];
                                glGetProgramInfoLog(coloredVertexProgram, Math::min(infoLogLength, 128), nullptr, &error[0]);
                                Platform::printf("link: %s", error);
                            }
                        }
                        glDetachShader(coloredVertexProgram, vertexShader);
                        glDetachShader(coloredVertexProgram, pixelShader);
                    } else {
                        glGetShaderiv(pixelShader, GL_INFO_LOG_LENGTH, &infoLogLength);
                        if (infoLogLength) {
                            char error[128];
                            glGetShaderInfoLog(pixelShader, Math::min(infoLogLength, 128), nullptr, &error[0]);
                            Platform::printf("PS: %s", error);
                        }
                    }
                    glDeleteShader(pixelShader);
                } else {
                    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
                    if (infoLogLength) {
                        char error[128];
                        glGetShaderInfoLog(vertexShader, Math::min(infoLogLength, 128), nullptr, &error[0]);
                        Platform::printf("VS: %s", error);
                    }
                }
                glDeleteShader(vertexShader);

                buffer.shader = coloredVertexProgram;
            }

        }
        
        void present3d(Buffer& buffer, const Mat4& projMatrix, const Mat4& viewMatrix, u32* cbuffers) {

            glUseProgram(buffer.shader);

            Mat4 mvp = Math::mult(projMatrix, viewMatrix);
            glBindBuffer(GL_UNIFORM_BUFFER, cbuffers[2]);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Mat4), mvp.dataCM);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glBindBuffer(GL_ARRAY_BUFFER, buffer.vertexBufferIndex);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * buffer.vertexIndex, &buffer.vertexMemory);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(buffer.layoutBufferIndex);
            glDrawArrays(GL_LINES, 0, buffer.vertexIndex);
            glBindVertexArray(0);

            glUseProgram(0);
        }
        
        void present2d(Buffer& buffer, const Mat4& projMatrix, u32* cbuffers) {

            glUseProgram(buffer.shader);

            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);

            glBindBuffer(GL_UNIFORM_BUFFER, cbuffers[2]);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Mat4), projMatrix.dataCM);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            u32 indexCount = (6 * buffer.charVertexIndex / 4) / sizeof(CharVertex);
            glBindBuffer(GL_ARRAY_BUFFER, buffer.charVertexBufferIndex);
            glBufferSubData(GL_ARRAY_BUFFER, 0, buffer.charVertexIndex, &buffer.charVertexMemory);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.charIndexBufferIndex);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexCount * sizeof(u32), &buffer.charIndexVertexMemory);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glBindVertexArray(buffer.charLayoutBufferIndex);
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);

            glUseProgram(0);
        }
    }
}
#endif // __WASTELADNS_RENDERER_DEBUG_GLFW_H__
