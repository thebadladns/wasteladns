#define TEST_MIRRORS 0
#define TEST_PHYSICS 1

#define COMPILE_TARGET TEST_PHYSICS

#if COMPILE_TARGET == TEST_MIRRORS
    #include "TestMirrors/main.cpp"
#elif COMPILE_TARGET == TEST_PHYSICS
    #include "TestPhysics/main.cpp"
#endif
