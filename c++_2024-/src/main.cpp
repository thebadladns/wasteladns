#define TEST_MIRRORS 0

#define COMPILE_TARGET TEST_MIRRORS

#if COMPILE_TARGET == TEST_REWIND
    #include "TestMirrors/main.cpp"
#endif
