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
        axis -= 3 * input.pressed(params.minus_key) + input.down(params.minus_key) - input.released(params.minus_key);
        axis += 3 * input.pressed(params.plus_key) + input.down(params.plus_key) - input.released(params.plus_key);
        axis = Math::clamp(*params.axis, -1.f, 1.f);
    }
};

#endif // __WASTELADNS_INPUT_HELPERS_H__
