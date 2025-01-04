#ifndef __WASTELADNS_MATH_H__
#define __WASTELADNS_MATH_H__

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F // max value
#endif
#ifndef FLT_MIN
#define FLT_MIN 1.175494351e-38F // min positive value
#endif

namespace math {
const f32 eps32 = 1.19e-07f;
const f64 eps64 = 2.22e-16;
const f32 e32 = 2.7182818284590452353602874713527f;
const f64 e64 = 2.7182818284590452353602874713527;
}

namespace math {

    f32 rand() { return ::rand() / (f32) RAND_MAX; }
    u8 min(u8 a, u8 b) { return (b < a) ? b : a; }
    s8 min(s8 a, s8 b) { return (b < a) ? b : a; }
    u16 min(u16 a, u16 b) { return (b < a) ? b : a; }
    s16 min(s16 a, s16 b) { return (b < a) ? b : a; }
    f32 min(f32 a, f32 b) { return (b < a) ? b : a; }
    f64 min(f64 a, f64 b) { return (b < a) ? b : a; }
    u32 min(u32 a, u32 b) { return (b < a) ? b : a; }
    s32 min(s32 a, s32 b) { return (b < a) ? b : a; }
    u64 min(u64 a, u64 b) { return (b < a) ? b : a; }
    s64 min(s64 a, s64 b) { return (b < a) ? b : a; }
    u8 max(u8 a, u8 b) { return (a < b) ? b : a; }
    s8 max(s8 a, s8 b) { return (a < b) ? b : a; }
    u16 max(u16 a, u16 b) { return (a < b) ? b : a; }
    s16 max(s16 a, s16 b) { return (a < b) ? b : a; }
    f32 max(f32 a, f32 b) { return (a < b) ? b : a; }
    f64 max(f64 a, f64 b) { return (a < b) ? b : a; }
    u32 max(u32 a, u32 b) { return (a < b) ? b : a; }
    s32 max(s32 a, s32 b) { return (a < b) ? b : a; }
    u64 max(u64 a, u64 b) { return (a < b) ? b : a; }
    s64 max(s64 a, s64 b) { return (a < b) ? b : a; }
    u8 clamp(u8 x, u8 a, u8 b) { return min(max(x, a), b); }
    s8 clamp(s8 x, s8 a, s8 b) { return min(max(x, a), b); }
    u16 clamp(u16 x, u16 a, u16 b) { return min(max(x, a), b); }
    s16 clamp(s16 x, s16 a, s16 b) { return min(max(x, a), b); }
    f32 clamp(f32 x, f32 a, f32 b) { return min(max(x, a), b); }
    f64 clamp(f64 x, f64 a, f64 b) { return min(max(x, a), b); }
    u32 clamp(u32 x, u32 a, u32 b) { return min(max(x, a), b); }
    s32 clamp(s32 x, s32 a, s32 b) { return min(max(x, a), b); }
    u64 clamp(u64 x, u64 a, u64 b) { return min(max(x, a), b); }
    s64 clamp(s64 x, s64 a, s64 b) { return min(max(x, a), b); }
#ifndef _MSC_VER // outside of msvc, uintptr_t / ptrdiff_t types are not implicitly convertible to u64 / s64
    uintptr_t min(uintptr_t a, uintptr_t b) { return (b < a) ? b : a; }
    uintptr_t max(uintptr_t a, uintptr_t b) { return (a < b) ? b : a; }
    uintptr_t clamp(uintptr_t x, uintptr_t a, uintptr_t b) { return min(max(x, a), b); }
    ptrdiff_t min(ptrdiff_t a, ptrdiff_t b) { return (b < a) ? b : a; }
    ptrdiff_t max(ptrdiff_t a, ptrdiff_t b) { return (a < b) ? b : a; }
    ptrdiff_t clamp(ptrdiff_t x, ptrdiff_t a, ptrdiff_t b) { return min(max(x, a), b); }
#endif

    f32 ceil(f32 a) { return ::ceilf(a); }
    f64 ceil(f64 a) { return ::ceil(a); }
    f32 sqrt(f32 a) { return ::sqrtf(a); }
    f64 sqrt(f64 a) { return ::sqrt(a); }
    f32 rsqrt(f32 a) { return 1.f / ::sqrtf(a); }   // todo: ensure this
    f64 rsqrt(f64 a) { return 1.0 / ::sqrt(a); }    // calls _mm_rsqrt_ss
    f32 square(f32 a) { return a * a; }
    f64 square(f64 a) { return a * a; }
    f32 abs(f32 a) { return ::fabsf(a); }
    f64 abs(f64 a) { return ::fabs(a); }
    s32 abs(s32 a) { return ::abs(a); }
    s64 abs(s64 a) { return ::llabs(a); }
    f32 exp_taylor(f32 x) {
        const int order = 3;
        switch (order) {
            case 2: return 1.f + x + x * x * 0.f;
            case 3: return 1.f + x + x * x * 0.5f + x * x * x / 6.f;
            case 4: return 1.f + x + x * x * 0.5f + x * x * x / 6.f + x * x * x * x / 24.f;
            default: return powf(e32, x);
        }
    }
    f32 bias(f32 a) { return (a + 1.f) * 0.5f; }
    f64 bias(f64 a) { return (a + 1.0) * 0.5; }
    f32 unbias(f32 a) { return a * 2.f - 1.f; }
    f64 unbias(f64 a) { return a * 2.0 - 1.0; }
    f32 round(f32 a) { return ::roundf(a); }
    f64 round(f64 a) { return ::round(a); }
    f32 sign(f32 a) { if (a > 0.f) return 1.f; else if (a < 0.f) return -1.f; else return 0.f; }
    f64 sign(f64 a) { if (a > 0) return 1; else if (a < 0) return -1; else return 0; }
    f32 lerp(f32 t, f32 a, f32 b) { return a + (b - a) * t; }
    f64 lerp(f64 t, f64 a, f64 b) { return a + (b - a) * t; }

}

// tmp
typedef s64 (*CompFunc)(const void*, const void*);
void swap(void* __restrict a, void* __restrict b, size_t size) {
	const ptrdiff_t inc = 4096;
	for (ptrdiff_t p = 0; p < (ptrdiff_t)size; p += inc) {
		char tmp[inc];
		ptrdiff_t s = math::min((ptrdiff_t)size - p, inc);
		memcpy(tmp, (u8*)a + p, s);
		memcpy((u8*)a + p, (u8*)b + p, s);
		memcpy((u8*)b + p, tmp, s);
    }
}
void qsort(void* __restrict elems, s32 low, s32 high, size_t stride, CompFunc cmp) {
    if (low < high) {
		void* p = (void*)((u8*)elems + stride * low);
		s32 i = low;
		s32 j = high;
		while (i < j) {
			while (cmp((void*)((u8*)elems + stride * i), p) <= 0 && i <= high - 1) { i++; }
			while (cmp((void*)((u8*)elems + stride * j), p) > 0 && j >= low + 1) { j--; }
			if (i < j) {
				swap((void*)((u8*)elems + stride * i), (void*)((u8*)elems + stride * j), stride);
			}
		}
		swap((void*)((u8*)elems + stride * low), (void*)((u8*)elems + stride * j), stride);
		s32 pi = j;

        qsort(elems, low, pi - 1, stride, cmp);
        qsort(elems, pi + 1, high, stride, cmp);
    };
};
    
#endif // __WASTELADNS_MATH_H__
