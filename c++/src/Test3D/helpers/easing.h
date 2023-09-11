#ifndef __WASTELADNS_EASING_H__
#define __WASTELADNS_EASING_H__

#ifndef UNITYBUILD
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
    
    curr = target + (curr - target) / Math::exp_taylor( timeDelta * Math::e<_T> / timeHorizon );
    
    return curr;
};

template <typename _T>
_T subsampled_eappr(_T curr, _T target, _T timeHorizon, _T timeDelta, _T stepTime) {
    
    s32 steps = (s32)(0.5f + timeDelta / stepTime);
    
    _T t = 0.f;
    for (s32 i = 0; i < steps; i++) {
        curr = target + (curr - target) / Math::exp_taylor(stepTime * Math::e<_T> / (timeHorizon - t));
        t += stepTime;
    }
    curr = target + (curr - target) / Math::exp_taylor((timeDelta - t) * Math::e<_T> / (timeHorizon - timeDelta));
    
    return curr;
};

}

#endif // __WASTELADNS_EASING_H__
