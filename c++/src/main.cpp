#define TEST_2D 0
#define TEST_TEXT 1
#define TEST_SMOOTHMOTION 2
#define TEST_TRAJECTORY 3

#define COMPILE_TARGET TEST_SMOOTHMOTION

#if COMPILE_TARGET == TEST_2D
#include "Test2D/main.cpp"
#elif COMPILE_TARGET == TEST_TEXT
#include "TestText/main.cpp"
#elif COMPILE_TARGET == TEST_SMOOTHMOTION
#include "TestSmoothMotion/main.cpp"
#elif COMPILE_TARGET == TEST_TRAJECTORY
#include "TestTrajectory/main.cpp"
#endif
