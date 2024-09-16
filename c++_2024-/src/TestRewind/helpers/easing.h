#ifndef __WASTELADNS_EASING_H__
#define __WASTELADNS_EASING_H__

namespace Math {
    
f32 lappr(f32 current, f32 target, f32 rate, f32 timeDelta) {
    if (current < target) { return Math::min(current + rate * timeDelta, target); }
    else { return Math::max(current - rate * timeDelta, target); }
}
f32 eappr(f32 curr, f32 target, f32 timeHorizon, f32 timeDelta) {
    return target + (curr - target) / Math::exp_taylor( timeDelta * Math::e_f / timeHorizon );
};
f32 subsampled_eappr(f32 curr, f32 target, f32 timeHorizon, f32 timeDelta, f32 stepTime) {
    s32 steps = (s32)(0.5f + timeDelta / stepTime);
    f32 t = 0.f;
    for (s32 i = 0; i < steps; i++) {
        curr = target + (curr - target) / Math::exp_taylor(stepTime * Math::e_f / (timeHorizon - t));
        t += stepTime;
    }
    return target + (curr - target) / Math::exp_taylor((timeDelta - t) * Math::e_f / (timeHorizon - timeDelta));
};
}

#endif // __WASTELADNS_EASING_H__
