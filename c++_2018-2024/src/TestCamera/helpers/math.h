#ifndef __WASTELADNS_MATH_H__
#define __WASTELADNS_MATH_H__

#ifndef __WASTELADNS_C_MATH_H__
#include <math.h>
#define __WASTELADNS_C_MATH_H__
#endif

#ifndef __WASTELADNS_TYPES_H__
#include "types.h"
#endif

#ifndef __WASTELADNS_TEMPLATE_DEFINES_H__
#include "template_defines.h"
#endif

// g=generic, s=specialized
#define MATH_TEMPLATES(g,s,...) \
    g(RT_PT(abs, __VA_ARGS__)) \
    g(RT_PTT(min, __VA_ARGS__)) \
    g(RT_PTT(max, __VA_ARGS__)) \
    g(RT_PTTT(clamp, __VA_ARGS__))
#define MATH_TEMPLATES_R(g,s,...) \
    s(RT_PT(sqrt, __VA_ARGS__)) \
    g(RT_PT(square, __VA_ARGS__)) \
    g(RT_PT(exp_taylor, __VA_ARGS__)) \
    g(RT_PT(bias, __VA_ARGS__)) \
    g(RT_PT(unbias, __VA_ARGS__)) \
    g(RT_PT(round, __VA_ARGS__)) \
    g(RT_PTTT(lerp, __VA_ARGS__))

namespace Math {
    
DEFINE_TEMPLATES(MATH_TEMPLATES)
DEFINE_TEMPLATES(MATH_TEMPLATES_R)
    
template<typename _T> const _T eps;
template<typename _T> const _T e;
template <> const f32 eps<f32> = 1.19e-07f;
template <> const f64 eps<f64> = 2.22e-16;
template <> const f32 e<f32> = 2.7182818284590452353602874713527f;
template <> const f64 e<f64> = 2.7182818284590452353602874713527;
    
}

#endif // __WASTELADNS_MATH_H__

#ifdef __WASTELADNS_MATH_IMPL__
#undef __WASTELADNS_MATH_IMPL__

namespace Math {
    
    template<typename _T>
    _T abs(_T a) {
        return a > 0.f ? a : -a;
    }
    
    template <typename _T>
    _T min(_T a, _T b) {
        return a < b ? a : b;
    }
    
    template <typename _T>
    _T max(_T a, _T b) {
        return a > b ? a : b;
    }
    
    template <typename _T>
    _T clamp(_T x, _T a, _T b) {
        return min(max(x, a), b);
    }
    
    template <>
    f32 sqrt(f32 a) {
        return ::sqrtf(a);
    }
    template <>
    f64 sqrt(f64 a) {
        return ::sqrt(a);
    }
    
    template <typename _T>
    _T square(_T a) {
        return a * a;
    }
    
    template <typename _T>
    _T exp_taylor(_T x) {
        const int order = 3;
        switch (order) {
            case 2:
                return (_T) (1.0 + x + x * x * 0.5);
            case 3:
                return (_T) (1.0 + x + x * x * 0.5 + x * x * x / 6.0);
            case 4:
                return (_T) (1.0 + x + x * x * 0.5 + x * x * x / 6.0 + x * x * x * x / 24.0);
            default:
                return (_T) pow(e<_T>, x);
        }
    }
    
    template <typename _T>
    _T bias(_T a) {
        return (a + 1.f) * 0.5f;
    }
    
    template <typename _T>
    _T unbias(_T a) {
        return a * 2.f - 1.f;
    }
    
    template <typename _T>
    _T round(_T a) {
        return ::round(a);
    }
    
    template <typename _T>
    _T lerp(_T t, _T a, _T b) {
        return a + (b - a) * t;
    }
    
INSTANTIATE_TEMPLATES(MATH_TEMPLATES, u8)
INSTANTIATE_TEMPLATES(MATH_TEMPLATES, f32)
INSTANTIATE_TEMPLATES(MATH_TEMPLATES, f64)

INSTANTIATE_TEMPLATES(MATH_TEMPLATES_R, f32)
INSTANTIATE_TEMPLATES(MATH_TEMPLATES_R, f64)

}
    
#endif // __WASTELADNS_MATH_IMPL__
