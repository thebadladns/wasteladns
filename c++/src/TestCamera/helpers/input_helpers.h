#ifndef __WASTELADNS_INPUT_HELPERS_H__
#define __WASTELADNS_INPUT_HELPERS_H__

namespace Input {
    
    template <s32 _InputButtonCount>
    struct DigitalInputToAxisParams {
        const DigitalState<_InputButtonCount>* input;
        f32* axis;
        s32 plus_key;
        s32 minus_key;
    };
    template <s32 _InputButtonCount>
    void digitalInputToAxis(DigitalInputToAxisParams<_InputButtonCount> params) {
        const auto& input = *params.input;
        
        f32& axis = *params.axis;
        bool downMinusKey = input.down(params.minus_key);
        bool downPlusKey = input.down(params.plus_key);
        bool pressedMinusKey = input.pressed(params.minus_key);
        bool pressedPlusKey = input.pressed(params.plus_key);
        bool releasedMinusKey = axis < 0.f && !downMinusKey;
        bool releasedPlusKey = axis > 0.f && !downPlusKey;
        
        axis -= 3 * pressedMinusKey + downMinusKey - releasedMinusKey;
        axis += 3 * pressedPlusKey + downPlusKey - releasedPlusKey;
        axis = Math::clamp(*params.axis, -1.f, 1.f);
    }
};

#endif // __WASTELADNS_INPUT_HELPERS_H__
