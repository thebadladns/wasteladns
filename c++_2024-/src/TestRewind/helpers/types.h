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
//
//#if !defined(UFBX_STANDARD_C) && defined(_MSC_VER)
//    #define ufbxi_noinline __declspec(noinline)
//    #define ufbxi_forceinline __forceinline
//    #define ufbxi_restrict __restrict
//    #if defined(__cplusplus) && _MSC_VER >= 1900 && defined(_MSVC_LANG) && _MSVC_LANG >= 201703L
//        #define ufbxi_nodiscard [[nodiscard]]
//    #elif defined(_Check_return_)
//        #define ufbxi_nodiscard _Check_return_
//    #else
//        #define ufbxi_nodiscard
//    #endif
//    #define ufbxi_unused
//    #define ufbxi_unlikely(cond) (cond)
//#elif !defined(UFBX_STANDARD_C) && (defined(__GNUC__) || defined(__clang__))
//    #define ufbxi_noinline __attribute__((noinline))
//    #define ufbxi_forceinline inline __attribute__((always_inline))
//    #define ufbxi_restrict __restrict
//    #define ufbxi_nodiscard __attribute__((warn_unused_result))
//    #define ufbxi_unused __attribute__((unused))
//    #define ufbxi_unlikely(cond) __builtin_expect((cond), 0)
//#else
//    #define ufbxi_noinline
//    #define ufbxi_forceinline
//    #define ufbxi_nodiscard
//    #define ufbxi_restrict
//    #define ufbxi_unused
//    #define ufbxi_unlikely(cond) (cond)
//#endif



template <bool>
struct ConditionalImpl { template<typename _true, typename _false> using type = _true; };
template<>
struct ConditionalImpl<false> { template<typename _true, typename _false> using type = _false; };
template<bool _b, class _true, class _false>
using Conditional_t = typename ConditionalImpl<_b>::template type<_true, _false>;

#endif // __WASTELADNS_TYPES_H__
