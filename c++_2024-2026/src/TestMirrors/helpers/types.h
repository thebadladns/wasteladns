#ifndef __WASTELADNS_TYPES_H__
#define __WASTELADNS_TYPES_H__

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

#define countof(x) (sizeof(x)/sizeof(0[x]))

#if _MSC_VER
#define force_inline __forceinline
#elif __clang__
# define force_inline __attribute__((always_inline))
#endif

#if !DISABLE_INTRINSICS

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

#endif // DISABLE_INTRINSICS

#endif // __WASTELADNS_TYPES_H__
