#ifndef __WASTELADNS_MATH_H__
#define __WASTELADNS_MATH_H__

#ifndef __WASTELADNS_C_MATH_H__
#include <math.h>
#define __WASTELADNS_C_MATH_H__
#endif

#ifndef __WASTELADNS_TYPES_H__
#include "Types.h"
#endif

template<typename _T>
struct Math {
    static const _T eps;
    static const _T e;
    
    static _T abs(_T a);
    static _T min(_T a, _T b);
    static _T max(_T a, _T b);
    static _T clamp(_T x, _T a, _T b);
    static _T sqrt(_T a);
    
    template <u8 _order = 3>
    static _T exp_taylor(_T x);
};

#ifndef __WASTELADNS_MATH_IMPL__
extern template struct Math<f32>;
extern template struct Math<f64>;

extern template f32 Math<f32>::exp_taylor<3>(f32 x);
extern template f64 Math<f64>::exp_taylor<3>(f64 x);

#endif

#endif // __WASTELADNS_MATH_H__

#ifdef __WASTELADNS_MATH_IMPL__
#undef __WASTELADNS_MATH_IMPL__

template <> const f32 Math<f32>::eps = 0.0000001f;
template <> const f64 Math<f64>::eps = 0.0000001;
template <> const f32 Math<f32>::e = 2.7182818284590452353602874713527f;
template <> const f64 Math<f64>::e = 2.7182818284590452353602874713527;

template<typename _T>
_T Math<_T>::abs(_T a) {
    return a > 0.f ? a : -a;
}

template <typename _T>
_T Math<_T>::min(_T a, _T b) {
    return a < b ? a : b;
}

template <typename _T>
_T Math<_T>::max(_T a, _T b) {
    return a > b ? a : b;
}

template <typename _T>
_T Math<_T>::clamp(_T x, _T a, _T b) {
    return min(max(x, a), b);
}

template <typename _T>
_T Math<_T>::sqrt(_T a) {
    return ::sqrt(a);
}

template <typename _T>
template <u8 _order>
_T Math<_T>::exp_taylor(_T x) {
    switch (_order) {
        case 2:
            return (_T) (1.0 + x + x * x * 0.5);
        case 3:
            return (_T) (1.0 + x + x * x * 0.5 + x * x * x / 6.0);
        case 4:
            return (_T) (1.0 + x + x * x * 0.5 + x * x * x / 6.0 + x * x * x * x / 24.0);
        default:
            return (_T) pow(e, x);
    }
}

template f32 Math<f32>::exp_taylor<3>(f32 x);
template f64 Math<f64>::exp_taylor<3>(f64 x);

template struct Math<f32>;
template struct Math<f64>;

#endif // __WASTELADNS_MATH_IMPL__
