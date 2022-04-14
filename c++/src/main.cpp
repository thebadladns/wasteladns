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

#define COMPILE_TARGET TEST_SPATIAL

#if COMPILE_TARGET == TEST_WORKSPACE
#include "workspace_main.cpp"
#elif COMPILE_TARGET == TEST_2D
#include "Test2D/main.cpp"
#elif COMPILE_TARGET == TEST_TEXT
#include "TestText/main.cpp"
#elif COMPILE_TARGET == TEST_SMOOTHMOTION
#include "TestSmoothMotion/main.cpp"
#elif COMPILE_TARGET == TEST_TRAJECTORY
#include "TestTrajectory/main.cpp"
#elif COMPILE_TARGET == TEST_GAMEPAD
#include "TestGamepad/main.cpp"
#elif COMPILE_TARGET == TEST_CAMERA
#include "TestCamera/main.cpp"
#elif COMPILE_TARGET == TEST_PLATFORMAPI
#include "TestPlatformAPI/main.cpp"
#elif COMPILE_TARGET == TEST_ILLUMINATION
#include "TestIllumination/main.cpp"
#elif COMPILE_TARGET == TEST_MESH
#include "TestMesh/main.cpp"
#endif
