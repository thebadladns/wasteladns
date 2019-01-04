#ifndef UNITYBUILD
#include "../platform.h"
#include "input.h"
#include "io.h"
#endif

namespace Platform {

    const char* name = "GLFW";

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

        Platform::printf( "%*s=>GL[%s,src:%s,severity:%s,id:%d]: %s\n",
            indent*2, "", typeStr, sourceStr, severityStr, id, message );
    }

    
namespace GLFW {

    template <typename _GameData>
    int main(int argc, char** argv) {
        
        if (!glfwInit()) {
            return 0;
        }
        
        GLFWwindow* windowHandle;
        
        Platform::State platform = {};
        
        // Launch window
        {
            Platform::WindowConfig config;
            Platform::loadLaunchConfig(config);

            glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
            
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            
            // TODO: add define on cmakelist
#ifdef __APPLE__

            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

            platform.screen.width = config.window_width;
            platform.screen.height = config.window_height;
            platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
            platform.screen.fullscreen = config.fullscreen;
            windowHandle = glfwCreateWindow(
                  platform.screen.width
                , platform.screen.height
                , config.title
                , nullptr /*monitor*/
                , nullptr /*share*/
            );
            if (!windowHandle) {
                return 0;
            }
        }

        glfwMakeContextCurrent(windowHandle);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        glfwSwapInterval(1);

        // Todo: only on windows
//        LogData logLevel = {};
//        logLevel.minLevel = LogLevel::Low;
//        glEnable( GL_DEBUG_OUTPUT );
//        glDebugMessageCallback( debugMessageCallback, &logLevel );

        // Input setup
        glfwSetInputMode(windowHandle, GLFW_STICKY_KEYS, 1);
        
        ::Input::Gamepad::State pads[1];
        ::Input::Gamepad::KeyboardMapping keyboardPadMappings[1];
        platform.input.padCount = 1;
        platform.input.pads = pads;
        ::Input::Gamepad::load(keyboardPadMappings[0]);
        
        ::Input::Keyboard::Mapping keyboardMapping;
        ::Input::Keyboard::load(keyboardMapping);
        
        platform.time.running = 0.0;
        platform.time.now = platform.time.start = glfwGetTime();
        
        _GameData game;
        Platform::GameConfig config;
        Platform::start<_GameData>(game, config, platform);
        
        do {
            if (platform.time.now >= config.nextFrame) {
                
                // Input
                glfwPollEvents();
                if ((config.requestFlags & (Platform::RequestFlags::PollKeyboard)) != 0) {
                    ::Input::Keyboard::pollState(platform.input.keyboard, windowHandle, keyboardMapping);
                }
                for (u32 i = 0; i < platform.input.padCount; i++) {
                    :: Input::Gamepad::pollState(platform.input.pads[i], windowHandle, keyboardPadMappings[i]);
                }

                Platform::update<_GameData>(game, config, platform);
                
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
}
}
