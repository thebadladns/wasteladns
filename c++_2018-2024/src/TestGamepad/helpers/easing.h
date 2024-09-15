#ifndef __WASTELADNS_EASING_H__
#define __WASTELADNS_EASING_H__

#ifndef __WASTELADNS_TYPES_H__
#include "types.h"
#endif

#ifndef __WASTELADNS_TEMPLATE_DEFINES_H__
#include "template_defines.h"
#endif

// g=generic, s=specialized
#define EASING_TEMPLATES(g,s,...) \
    g(RT_PTTTT(lappr, __VA_ARGS__)) \
    g(RT_PTTTT(eappr, __VA_ARGS__)) \
    g(RT_PTTTTT(subsampled_eappr, __VA_ARGS__))

namespace Math {
    
DEFINE_TEMPLATES(EASING_TEMPLATES)
    
}

#endif // __WASTELADNS_EASING_H__

#ifdef __WASTELADNS_EASING_IMPL__
#undef __WASTELADNS_EASING_IMPL__

#ifndef __WASTELADNS_MATH_H__
#include "math.h"
#endif

namespace Math {
    
    template <typename _T>
    _T lappr(_T current, _T target, _T rate, _T timeDelta) {
        if (current < target) {
            return Math::min(current + rate * timeDelta, target);
        } else {
            return Math::max(current - rate * timeDelta, target);
        }
    }
    
    template <typename _T>
    _T eappr(_T curr, _T target, _T timeHorizon, _T timeDelta) {
        
        curr = target + (curr - target) / Math::expTaylor( timeDelta * Math::e<_T> / timeHorizon );
        
        return curr;
    };
    
    template <typename _T>
    _T subsampled_eappr(_T curr, _T target, _T timeHorizon, _T timeDelta, _T stepTime) {
        
        s32 steps = (s32)(0.5f + timeDelta / stepTime);
        
        _T t = 0.f;
        for (s32 i = 0; i < steps; i++) {
            curr = target + (curr - target) / Math::expTaylor(stepTime * Math::e<_T> / (timeHorizon - t));
            t += stepTime;
        }
        curr = target + (curr - target) / Math::expTaylor((timeDelta - t) * Math::e<_T> / (timeHorizon - timeDelta));
        
        return curr;
    };
    
INSTANTIATE_TEMPLATES(EASING_TEMPLATES, f32)
INSTANTIATE_TEMPLATES(EASING_TEMPLATES, f64)

}

#endif // __WASTELADNS_EASING_IMPL__
