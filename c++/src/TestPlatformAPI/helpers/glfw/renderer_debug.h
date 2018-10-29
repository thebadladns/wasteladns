#ifndef __WASTELADNS_RENDERER_DEBUG_GLFW_H__
#define __WASTELADNS_RENDERER_DEBUG_GLFW_H__

#ifndef UNITYBUILD
#include "../color.h"
#include "../angle.h"
#include "core.h"
#endif

namespace Renderer
{
    namespace Immediate
    {
        void present3d(Buffer& buffer) {
            // Vertex
            glInterleavedArrays(GL_C4UB_V3F, 0, buffer.vertexMemory);
            glDrawArrays(GL_LINES, 0, buffer.vertexIndex);
            // Disable client state implicitly set by glInterleavedArrays
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
        }
        
        void present2d(Buffer& buffer) {
            // Reverse stb_easy_font y axis
            glScalef(1.f, -1.f, 1.f);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 16, buffer.charVertexMemory);
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4, GL_UNSIGNED_BYTE, 16, buffer.charVertexMemory + 12);
            glDrawArrays(GL_QUADS, 0, buffer.charVertexIndex / kTextVertexSize);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
        }
    }
}
#endif // __WASTELADNS_RENDERER_DEBUG_GLFW_H__
