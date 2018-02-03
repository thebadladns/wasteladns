#define TEST_2D 0
#define TEST_TEXT 1
#define TEST_SMOOTHMOTION 2

#define COMPILE_TARGET TEST_TEXT

#if COMPILE_TARGET == TEST_2D
#include "Test2D/main.cpp"
#elif COMPILE_TARGET == TEST_TEXT
#include "TestText/main.cpp"
#elif COMPILE_TARGET == TEST_SMOOTHMOTION
#include "TestSmoothMotion/main.cpp"
#endif
