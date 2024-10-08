#ifndef UNITYBUILD
#include "../platform.h"
#include "input.h"
#include "io.h"
#include "game.h"
#endif

struct LogLevel { enum Enum { None = -1, Debug, Low, Med, High }; };
struct LogData {
    u32 indent;
    LogLevel::Enum minLevel;
};
void GLAPIENTRY debugMessageCallback(
        GLenum source
    , GLenum type
    , GLuint id
    , GLenum severity
    , GLsizei length
    , const GLchar* message
    , const void* userParam 
) {
    LogLevel::Enum logLevel;
    const char* severityStr ="";
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: severityStr = "HIGH"; logLevel = LogLevel::High; break;
    case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "MEDIUM"; logLevel = LogLevel::Med; break;
    case GL_DEBUG_SEVERITY_LOW: severityStr = "LOW"; logLevel = LogLevel::Low; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "NOTIFICATION"; logLevel = LogLevel::Debug; break;
    default: severityStr = "N/A"; logLevel = LogLevel::None; break;
    }
    LogData& logData = *(LogData*)userParam;
    if (logLevel < logData.minLevel) {
        return;
    }

    u32 indent = logData.indent;
    const char* sourceStr;
    switch (source) {
    case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "WINDOW SYSTEM"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "SHADER COMPILER"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "THIRD PARTY"; break;
    case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "APPLICATION"; break;
    case GL_DEBUG_SOURCE_OTHER: sourceStr = "OTHER"; break;
    default: sourceStr = "N/A"; break;
    }
    const char* typeStr ="";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR: typeStr = "ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "DEPRECATED BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "UNDEFINED BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY: typeStr = "PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER: typeStr = "OTHER"; break;
    case GL_DEBUG_TYPE_MARKER: typeStr = "MARKER"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "PUSH"; logData.indent++; break;
    case GL_DEBUG_TYPE_POP_GROUP: typeStr = "POP"; logData.indent--, indent--; break;
    }

    Platform::debuglog( "%*s=>GL[%s,src:%s,severity:%s,id:%d]: %s\n",
        indent*2, "", typeStr, sourceStr, severityStr, id, message );
}

// globals so callbacks can access it
Platform::State platform = {};

int main(int argc, char** argv) {
        
    if (!glfwInit()) {
        return 0;
    }
        
    GLFWwindow* windowHandle;
        
    // Launch window
    {
        Platform::WindowConfig config;
        Game::loadLaunchConfig(config);

        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
            
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            
#ifdef __MACOS
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        windowHandle = glfwCreateWindow(
              config.window_width
            , config.window_height
            , config.title
            , nullptr /*monitor*/
            , nullptr /*share*/
        );
        if (!windowHandle) {
            return 0;
        }
        s32 effectiveWidth, effectiveHeight;
        glfwGetWindowSize(windowHandle, &effectiveWidth, &effectiveHeight);
        
        int w, h;
        glfwGetFramebufferSize(windowHandle, &w, &h);
        
        platform.screen.window_width = w;
        platform.screen.window_height = h;
        platform.screen.width = config.game_width;
        platform.screen.height = config.game_height;
        platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
        platform.screen.fullscreen = config.fullscreen;
        __DEBUGEXP(platform.screen.text_scale = w / effectiveWidth);
    }

    glfwMakeContextCurrent(windowHandle);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

#if __GPU_DEBUG
    if (GLAD_GL_KHR_debug) {
        LogData logLevel = {};
        logLevel.minLevel = LogLevel::Debug;
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debugMessageCallback, &logLevel);
    } else {
        assert(0);
    }
#endif

    // Input setup
	glfwSetInputMode(windowHandle, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(windowHandle, GLFW_STICKY_MOUSE_BUTTONS, 1);
        
    memset(platform.input.pads, 0, sizeof(platform.input.pads));
        
    platform.time.running = 0.0;
    platform.time.now = platform.time.start = glfwGetTime();
        
    Game::Instance game;
    Platform::GameConfig config;
    Game::start(game, config, platform);

	struct MouseCallback {
		static void fn(GLFWwindow *, double xoffset, double yoffset) {
            platform.input.mouse.scrolldx = (f32)xoffset;
            platform.input.mouse.scrolldy = (f32)yoffset;
		}
	};
    glfwSetScrollCallback(windowHandle, &MouseCallback::fn);
        
    do {
        if (platform.time.now >= config.nextFrame) {
                
            // Input
            {
                ::Input::Mouse::resetState(platform.input.mouse);
                glfwPollEvents();
                ::Input::Keyboard::pollState(platform.input.keyboard, windowHandle);
                ::Input::Mouse::pollState(platform.input.mouse, windowHandle);
                ::Input::Gamepad::pollState(platform.input.pads, platform.input.padCount, COUNT_OF(platform.input.pads));
            }

            Game::update(game, config, platform);
              
            glfwSwapBuffers(windowHandle);
                
            if (config.quit) {
                glfwSetWindowShouldClose(windowHandle, 1);
            }
        }
        platform.time.now = glfwGetTime();
        platform.time.running = platform.time.now - platform.time.start;
    } while (!glfwWindowShouldClose(windowHandle));
        
    // Internally handles all glfwDestroyWindow
    glfwTerminate();
        
    return 0;
}
