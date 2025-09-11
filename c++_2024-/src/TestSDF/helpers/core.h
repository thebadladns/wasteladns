#ifndef __WASTELADNS_CORE_H__
#define __WASTELADNS_CORE_H__

// C libs
#include <math.h>
#include <stdlib.h> // rand, mbstowcs_s
#include <stdint.h> // int8_t, int16_t, int32_t, etc
#include <assert.h>
#include <stdio.h> // printf

#if __WIN64
#include "core_win.h"
#elif __MACOS
#include "core_mac.h"
#endif

#if __DEBUG
 // for last time modified file queries
#include <sys/types.h>
#include <sys/stat.h>
#endif

typedef int64_t s64;
typedef uint64_t u64;
typedef int32_t s32;
typedef uint32_t u32;
typedef int16_t s16;
typedef uint16_t u16;
typedef int8_t s8;
typedef uint8_t u8;

typedef float f32;
typedef double f64;

#if __DEBUG
    #define SRC_PATH_FROM_BIN "../../src/TestSDF/"
#endif

#define countof(x) (sizeof(x)/sizeof(0[x]))

#if _MSC_VER
#define force_inline __forceinline
#elif __clang__
# define force_inline __attribute__((always_inline))
#endif

#if _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

struct m128_4 {
    __m128 xxxx;
    __m128 yyyy;
    __m128 zzzz;
    __m128 wwww;
};
struct m256_4 {
    __m256 vx;
    __m256 vy;
    __m256 vz;
    __m256 vw;
};

#endif // __WASTELADNS_CORE_H__
