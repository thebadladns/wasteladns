#ifndef __WASTELADNS_MATH_H__
#define __WASTELADNS_MATH_H__

#ifndef UNITYBUILD
#include <math.h>
#include "types.h"
#endif

namespace Math {
    
template<typename _T> const _T eps;
template<typename _T> const _T e;
template <> const f32 eps<f32> = 1.19e-07f;
template <> const f64 eps<f64> = 2.22e-16;
template <> const f32 e<f32> = 2.7182818284590452353602874713527f;
template <> const f64 e<f64> = 2.7182818284590452353602874713527;
    
}

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
    
    template <typename _T> _T sqrt(_T a);
    
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

}
    
#endif // __WASTELADNS_MATH_H__
