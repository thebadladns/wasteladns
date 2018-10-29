#ifndef UNITYBUILD
#include "../platform.h"
#include "input.h"
#endif

namespace Platform {
    
namespace GLFW {

    template <typename _GameData>
    int main(int argc, char** argv) {
        
        if (!glfwInit()) {
            return 0;
        }
        
        ::Platform::Screen screen;
        GLFWwindow* windowHandle;
        
        Platform::State platform = {};
        
        // Launch window
        {
            Platform::WindowConfig config;
            Platform::loadLaunchConfig(config);

            glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
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
        glfwSwapInterval(1);
        
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
                
                // Needed since GLFW_DOUBLEBUFFER is GL_TRUE
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
