#ifndef __WASTELADNS_TYPES_H__
#define __WASTELADNS_TYPES_H__

#ifndef UNITYBUILD
#include "stdint.h"
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

#define COUNT_OF(x) (sizeof(x)/sizeof(0[x]))

template <bool>
struct ConditionalImpl { template<typename _true, typename _false> using type = _true; };
template<>
struct ConditionalImpl<false> { template<typename _true, typename _false> using type = _false; };
template<bool _b, class _true, class _false>
using Conditional_t = typename ConditionalImpl<_b>::template type<_true, _false>;

#endif // __WASTELADNS_TYPES_H__
