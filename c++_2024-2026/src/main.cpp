#define TEST_MIRRORS 0
#define TEST_SDF 1

#define COMPILE_TARGET TEST_SDF

#if COMPILE_TARGET == TEST_MIRRORS
    #include "TestMirrors/main.cpp"
#elif COMPILE_TARGET == TEST_SDF
    #include "TestSDF/main.cpp"
#endif
