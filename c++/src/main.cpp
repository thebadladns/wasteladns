#define TEST_WORKSPACE -1
#define TEST_2D 0
#define TEST_TEXT 1
#define TEST_SMOOTHMOTION 2
#define TEST_TRAJECTORY 3
#define TEST_GAMEPAD 4
#define TEST_CAMERA 5
#define TEST_PLATFORMAPI 6
#define TEST_ILLUMINATION 7
#define TEST_SPATIAL 8
#define TEST_3D 9

#define COMPILE_TARGET TEST_3D

#if COMPILE_TARGET == TEST_WORKSPACE
    #include "workspace_main.cpp"
#elif COMPILE_TARGET == TEST_2D
    #if __DX11
    #error "app-dx11 target not supported, use glfw"
    #elif __MACOS
    #error "app-macos target not supported, use glfw"
    #else
    #include "Test2D/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_TEXT
    #if __DX11
    #error "app-dx11 target not supported, use glfw"
    #elif __MACOS
    #error "app-macos target not supported, use glfw"
    #else
    #include "TestText/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_SMOOTHMOTION
    #if __DX11
    #error "app-dx11 target not supported, use glfw"
    #elif __MACOS
    #error "app-macos target not supported, use glfw"
    #else
    #include "TestSmoothMotion/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_TRAJECTORY
    #if __DX11
    #error "app-dx11 target not supported, use glfw"
    #elif __MACOS
    #error "app-macos target not supported, use glfw"
    #else
    #include "TestTrajectory/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_GAMEPAD
    #if __DX11
    #error "app-dx11 target not supported, use glfw"
    #elif __MACOS
    #error "app-macos target not supported, use glfw"
    #else
    #include "TestGamepad/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_CAMERA
    #if __DX11
    #error "app-dx11 target not supported, use glfw"
    #elif __MACOS
    #error "app-macos target not supported, use glfw"
    #else
    #include "TestCamera/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_PLATFORMAPI
    #if __MACOS
    #error "app-macos target not supported, use glfw or dx11"
    #else
    #include "TestPlatformAPI/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_ILLUMINATION
    #if __MACOS
    #error "app-macos target not supported, use glfw or dx11"
    #else
    #include "TestIllumination/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_SPATIAL
    #if __MACOS
    #error "app-macos target not supported, use glfw or dx11"
    #else
    #include "TestSpatial/main.cpp"
    #endif
#elif COMPILE_TARGET == TEST_3D
    #if __GLFW
    #error "app-glfw target not supported, use dx11 or macos"
    #else
    #include "Test3D/main.cpp"
    #endif
#endif
