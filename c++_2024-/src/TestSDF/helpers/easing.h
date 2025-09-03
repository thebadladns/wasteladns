#ifndef __WASTELADNS_EASING_H__
#define __WASTELADNS_EASING_H__

namespace math {
    
f32 lappr(f32 current, f32 target, f32 rate, f32 timeDelta) { // linear approach
    if (current < target) { return math::min(current + rate * timeDelta, target); }
    else { return math::max(current - rate * timeDelta, target); }
}
f32 eappr(f32 curr, f32 target, f32 timeHorizon, f32 timeDelta) { // exponential approach
    return target + (curr - target) / math::exp_taylor( timeDelta * math::e32 / timeHorizon );
};
f32 subsampled_eappr(f32 curr, f32 target, f32 timeHorizon, f32 timeDelta, f32 stepTime) {
    s32 steps = (s32)(0.5f + timeDelta / stepTime);
    f32 t = 0.f;
    for (s32 i = 0; i < steps; i++) {
        curr = target + (curr - target) / math::exp_taylor(stepTime * math::e32 / (timeHorizon - t));
        t += stepTime;
    }
    return target + (curr - target) / math::exp_taylor((timeDelta - t) * math::e32 / (timeHorizon - timeDelta));
};
}

#endif // __WASTELADNS_EASING_H__
