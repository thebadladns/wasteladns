#ifndef __WASTELADNS_LOADER_GL33_H__
#define __WASTELADNS_LOADER_GL33_H__

#if __WIN64
#define APIENTRYP APIENTRY *
namespace gfx {
namespace rhi { // render hardware interface
static HMODULE glModule;
void loadGLFramework() {
    glModule = LoadLibraryA("opengl32.dll");
}
void* getGLProcAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name); // load newer functions via wglGetProcAddress
    // if the function is not found, try directly from the GL lib
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        p = (void*)GetProcAddress(glModule, name); // could be an OpenGL 1.1 function, try in the OpenGL lib
    }
    return p;
}
}}  // gfx::rhi
#elif __MACOS
#define APIENTRY
#define APIENTRYP APIENTRY *
namespace gfx {
namespace rhi { // render hardware interface
CFBundleRef glFramework;
void loadGLFramework() {
    glFramework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
}
void* getGLProcAddress(const char* procname) {
    CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault, procname, kCFStringEncodingASCII);
    void* symbol = CFBundleGetFunctionPointerForName(glFramework, symbolName);
    CFRelease(symbolName);
    return symbol;
}
}}  // gfx::rhi
#endif

// Define necessary OpenGL types
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef int8_t GLbyte;
typedef uint8_t GLubyte;
typedef int16_t GLshort;
typedef uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef int32_t GLclampx;
typedef int GLsizei;
typedef float_t GLfloat;
typedef float_t GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void* GLeglClientBufferEXT;
typedef void* GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
typedef ptrdiff_t GLsizeiptr;
typedef unsigned short GLhalfARB;
typedef unsigned short GLhalf;
typedef GLint GLfixed;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
typedef int64_t GLint64;
typedef uint64_t GLuint64;
typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;
typedef int64_t GLint64EXT;
typedef uint64_t GLuint64EXT;
typedef struct __GLsync* GLsync;

#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#define GL_DEBUG_SOURCE_API 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_SOURCE_OTHER 0x824B
#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_OTHER 0x8251
#define GL_DEBUG_TYPE_MARKER 0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#define GL_DEBUG_TYPE_POP_GROUP 0x826A

#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_FRAMEBUFFER 0x8D40
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_DEPTH_STENCIL 0x84F9
#define GL_INVALID_INDEX 0xFFFFFFFF

#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_FALSE 0
#define GL_TRUE 1
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_RGB8 0x8051
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_FRONT_AND_BACK 0x0408
#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207
#define GL_ZERO 0
#define GL_ONE 1
#define GL_NONE 0
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_DEPTH_COMPONENT 0x1902
#define GL_UNSIGNED_INT_24_8 0x84FA
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_REPEAT 0x2901
#define GL_BLEND_DST 0x0BE0
#define GL_BLEND_SRC 0x0BE1
#define GL_BLEND 0x0BE2
#define GL_CULL_FACE 0x0B44
#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_DEPTH_TEST 0x0B71
#define GL_STENCIL_TEST 0x0B90
#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01
#define GL_INCR 0x1E02
#define GL_DECR 0x1E03
#define GL_INVERT 0x150A
#define GL_SCISSOR_TEST 0x0C11

int GL_KHR_debug = 0; //todo
typedef void (APIENTRY* GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

typedef void (APIENTRYP PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
PFNGLCLEARCOLORPROC glClearColor;
typedef void (APIENTRYP PFNGLCLEARPROC)(GLbitfield mask);
PFNGLCLEARPROC glClear;
typedef void (APIENTRYP PFNGLREADBUFFERPROC)(GLenum src);
PFNGLREADBUFFERPROC glReadBuffer;
typedef void (APIENTRYP PFNGLDRAWBUFFERPROC)(GLenum buf);
PFNGLDRAWBUFFERPROC glDrawBuffer;
typedef void (APIENTRYP PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
PFNGLVIEWPORTPROC glViewport;
typedef void (APIENTRYP PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
PFNGLGENTEXTURESPROC glGenTextures;
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
PFNGLBINDTEXTUREPROC glBindTexture;
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
PFNGLTEXIMAGE2DPROC glTexImage2D;
typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
PFNGLTEXPARAMETERIPROC glTexParameteri;
typedef void (APIENTRYP PFNGLCULLFACEPROC)(GLenum mode);
PFNGLCULLFACEPROC glCullFace;
typedef void (APIENTRYP PFNGLDEPTHFUNCPROC)(GLenum func);
PFNGLDEPTHFUNCPROC glDepthFunc;
typedef void (APIENTRYP PFNGLDEPTHMASKPROC)(GLboolean flag);
PFNGLDEPTHMASKPROC glDepthMask;
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
PFNGLDRAWARRAYSPROC glDrawArrays;
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);
PFNGLDRAWELEMENTSPROC glDrawElements;
typedef void (APIENTRYP PFNGLENABLEPROC)(GLenum cap);
PFNGLENABLEPROC glEnable;
typedef void (APIENTRYP PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
PFNGLBLENDFUNCPROC glBlendFunc;
typedef void (APIENTRYP PFNGLDISABLEPROC)(GLenum cap);
PFNGLDISABLEPROC glDisable;
typedef void (APIENTRYP PFNGLPOLYGONMODEPROC)(GLenum face, GLenum mode);
PFNGLPOLYGONMODEPROC glPolygonMode;
typedef void (APIENTRYP PFNGLFRONTFACEPROC)(GLenum mode);
PFNGLFRONTFACEPROC glFrontFace;
typedef void (APIENTRYP PFNGLSCISSORPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
PFNGLSCISSORPROC glScissor;

typedef GLuint(APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
typedef GLuint(APIENTRYP PFNGLCREATESHADERPROC)(GLenum type);
PFNGLCREATESHADERPROC glCreateShader = nullptr;
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint shader);
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
typedef void (APIENTRYP PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
typedef void (APIENTRYP PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint program);
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint program);
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
typedef void (APIENTRYP PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
typedef void (APIENTRYP PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
typedef void (APIENTRYP PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
PFNGLBUFFERDATAPROC glBufferData = nullptr;
typedef GLint(APIENTRYP PFNGLGETATTRIBLOCATIONPROC)(GLuint program, const GLchar* name);
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = nullptr;
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;

typedef void (APIENTRYP PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
typedef void (APIENTRYP PFNGLGENERATEMIPMAPPROC) (GLenum target);
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
typedef void (APIENTRYP PFNGLBLITFRAMEBUFFERPROC)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = nullptr;
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
typedef void (APIENTRYP PFNGLPUSHDEBUGGROUPPROC) (GLenum source, GLuint id, GLsizei length, const GLchar* message);
PFNGLPUSHDEBUGGROUPPROC glPushDebugGroup = nullptr;
typedef void (APIENTRYP PFNGLPOPDEBUGGROUPPROC) (void);
PFNGLPOPDEBUGGROUPPROC glPopDebugGroup = nullptr;
typedef void (APIENTRYP PFNGLBINDBUFFERBASEPROC)(GLenum target, GLuint index, GLuint buffer);
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC)(GLuint array);
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
PFNGLBUFFERSUBDATAPROC glBufferSubData;
typedef void (APIENTRYP PFNGLCOPYBUFFERSUBDATAPROC)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData;
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint* framebuffers);
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSPROC)(GLsizei n, GLuint* renderbuffers);
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
typedef void (APIENTRYP PFNGLBINDRENDERBUFFERPROC)(GLenum target, GLuint renderbuffer);
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
typedef void (APIENTRYP PFNGLDRAWBUFFERSPROC)(GLsizei n, const GLenum* bufs);
PFNGLDRAWBUFFERSPROC glDrawBuffers;
typedef void (APIENTRYP PFNGLDETACHSHADERPROC)(GLuint program, GLuint shader);
PFNGLDETACHSHADERPROC glDetachShader;
typedef void (APIENTRYP PFNGLDELETESHADERPROC)(GLuint shader);
PFNGLDELETESHADERPROC glDeleteShader;
typedef GLint(APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
typedef void (APIENTRYP PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
PFNGLUNIFORM1IPROC glUniform1i;
typedef void (APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
typedef GLuint(APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC)(GLuint program, const GLchar* uniformBlockName);
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
typedef void (APIENTRYP PFNGLDRAWELEMENTSINSTANCEDPROC)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
typedef void (APIENTRYP PFNGLCOLORMASKPROC)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
PFNGLCOLORMASKPROC glColorMask;
typedef void (APIENTRYP PFNGLSTENCILMASKPROC)(GLuint mask);
PFNGLSTENCILMASKPROC glStencilMask;
typedef void (APIENTRYP PFNGLSTENCILFUNCPROC)(GLenum func, GLint ref, GLuint mask);
PFNGLSTENCILFUNCPROC glStencilFunc;
typedef void (APIENTRYP PFNGLSTENCILOPPROC)(GLenum fail, GLenum zfail, GLenum zpass);
PFNGLSTENCILOPPROC glStencilOp;

typedef void (APIENTRYP PFNGLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC callback, const void* userParam);
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

namespace gfx {
namespace rhi { // render hardware interface
void loadGLExtensions() {
    
    loadGLFramework();

    //GL_KHR_debug
    
    glClearColor = (PFNGLCLEARCOLORPROC)getGLProcAddress("glClearColor");
    glClear = (PFNGLCLEARPROC)getGLProcAddress("glClear");
    glReadBuffer = (PFNGLREADBUFFERPROC)getGLProcAddress("glReadBuffer");
    glDrawBuffer = (PFNGLDRAWBUFFERPROC)getGLProcAddress("glDrawBuffer");
    glViewport = (PFNGLVIEWPORTPROC)getGLProcAddress("glViewport");
    glGenTextures = (PFNGLGENTEXTURESPROC)getGLProcAddress("glGenTextures");
    glBindTexture = (PFNGLBINDTEXTUREPROC)getGLProcAddress("glBindTexture");
    glTexImage2D = (PFNGLTEXIMAGE2DPROC)getGLProcAddress("glTexImage2D");
    glTexParameteri = (PFNGLTEXPARAMETERIPROC)getGLProcAddress("glTexParameteri");
    glCullFace = (PFNGLCULLFACEPROC)getGLProcAddress("glCullFace");
    glDepthFunc = (PFNGLDEPTHFUNCPROC)getGLProcAddress("glDepthFunc");
    glDepthMask = (PFNGLDEPTHMASKPROC)getGLProcAddress("glDepthMask");
    glDrawArrays = (PFNGLDRAWARRAYSPROC)getGLProcAddress("glDrawArrays");
    glDrawElements = (PFNGLDRAWELEMENTSPROC)getGLProcAddress("glDrawElements");
    glEnable = (PFNGLENABLEPROC)getGLProcAddress("glEnable");
    glBlendFunc = (PFNGLBLENDFUNCPROC)getGLProcAddress("glBlendFunc");
    glDisable = (PFNGLDISABLEPROC)getGLProcAddress("glDisable");
    glPolygonMode = (PFNGLPOLYGONMODEPROC)getGLProcAddress("glPolygonMode");
    glFrontFace = (PFNGLFRONTFACEPROC)getGLProcAddress("glFrontFace");
    glScissor = (PFNGLSCISSORPROC)getGLProcAddress("glScissor");
    
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)getGLProcAddress("glCreateProgram");
    glCreateShader = (PFNGLCREATESHADERPROC)getGLProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)getGLProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)getGLProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)getGLProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)getGLProcAddress("glGetShaderInfoLog");
    glAttachShader = (PFNGLATTACHSHADERPROC)getGLProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)getGLProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)getGLProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)getGLProcAddress("glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)getGLProcAddress("glUseProgram");
    glGenBuffers = (PFNGLGENBUFFERSPROC)getGLProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)getGLProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)getGLProcAddress("glBufferData");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)getGLProcAddress("glGetAttribLocation");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)getGLProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)getGLProcAddress("glVertexAttribPointer");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)getGLProcAddress("glBindFramebuffer");
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)getGLProcAddress("glGenerateMipmap");
    glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)getGLProcAddress("glBlitFramebuffer");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)getGLProcAddress("glActiveTexture");
    glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC)getGLProcAddress("glPushDebugGroup");
    glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC)getGLProcAddress("glPopDebugGroup");
    glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)getGLProcAddress("glBindBufferBase");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)getGLProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)getGLProcAddress("glBindVertexArray");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)getGLProcAddress("glBufferSubData");
    glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)getGLProcAddress("glCopyBufferSubData");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)getGLProcAddress("glGenFramebuffers");
    glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)getGLProcAddress("glGenRenderbuffers");
    glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)getGLProcAddress("glBindRenderbuffer");
    glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)getGLProcAddress("glRenderbufferStorage");
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)getGLProcAddress("glFramebufferRenderbuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)getGLProcAddress("glFramebufferTexture2D");
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC)getGLProcAddress("glDrawBuffers");
    glDetachShader = (PFNGLDETACHSHADERPROC)getGLProcAddress("glDetachShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)getGLProcAddress("glDeleteShader");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)getGLProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)getGLProcAddress("glUniform1i");
    glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)getGLProcAddress("glUniformBlockBinding");
    glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)getGLProcAddress("glGetUniformBlockIndex");
    glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)getGLProcAddress("glDrawElementsInstanced");
    glColorMask = (PFNGLCOLORMASKPROC)getGLProcAddress("glColorMask");
    glStencilMask = (PFNGLSTENCILMASKPROC)getGLProcAddress("glStencilMask");
    glStencilFunc = (PFNGLSTENCILFUNCPROC)getGLProcAddress("glStencilFunc");
    glStencilOp = (PFNGLSTENCILOPPROC)getGLProcAddress("glStencilOp");

    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)getGLProcAddress("glDebugMessageCallback");
}
}}  // gfx::rhi

#endif // __WASTELADNS_LOADER_GL33_H__
