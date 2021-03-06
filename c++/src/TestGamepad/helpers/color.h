#ifndef __WASTELADNS_COLOR_H__
#define __WASTELADNS_COLOR_H__

#ifndef __WASTELADNS_C_MATH_H__
#include <math.h>
#define __WASTELADNS_C_MATH_H__
#endif

#ifndef __WASTELADNS_TYPES_H__
#include "types.h"
#endif

#ifndef __WASTELADNS_MATH_H__
#include "math.h"
#endif

#define RGBA_PARAMS(color) color.getRf(), color.getGf(), color.getBf(), color.getAf()

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
struct Color {
    Color(const f32 fR, const f32 fG, const f32 fB, const f32 fA = 1.f);
    Color(const ValueType uR, const ValueType uG, const ValueType uB, const ValueType a = ((1 << alphaBitCount) - 1));
    ~Color();
    
    f32 getRf() const;
    f32 getGf() const;
    f32 getBf() const;
    f32 getAf() const;
    
    ValueType getRu() const;
    ValueType getGu() const;
    ValueType getBu() const;
    ValueType getAu() const;
    
    u32 getABGR() const;
    u32 getRGBA() const;
    
    void set(const f32 fR, const f32 fG, const f32 fB, const f32 fA = 1.f);
    void set(const ValueType uR, const ValueType uG, const ValueType uB, const ValueType uA = ((1 << alphaBitCount) - 1));
    
    static Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> add(const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &a, const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &b);
    static Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> scale(const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &a, f32 b);
    
    ValueType color;
};

extern template struct Color<u32, 8, 8, 8, 8>;
typedef Color<u32, 8, 8, 8, 8> Col32;
typedef Col32 Col;

#endif // __WASTELADNS_COLOR_H__

#ifdef __WASTELADNS_COLOR_IMPL__
#undef __WASTELADNS_COLOR_IMPL__

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::Color(const f32 fR, const f32 fG, const f32 fB, const f32 fA) {
    set(fR, fG, fB, fA);
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::Color(const ValueType uR, const ValueType uG, const ValueType uB, const ValueType uA) {
    set(uR, uG, uB, uA);
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::~Color() {
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
f32 Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getRf() const {
    const ValueType uR = getRu();
    return (f32) uR/ ((1 << redBitCount) - 1);
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
f32 Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getGf() const {
    const ValueType uG = getGu();
    return (f32) uG / ((1 << greenBitCount) - 1);
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
f32 Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getBf() const {
    const ValueType uB = getBu();
    return (f32) uB / ((1 << blueBitCount) - 1);
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
f32 Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getAf() const {
    const ValueType uA = getAu();
    return (f32) uA / ((1 << alphaBitCount) - 1);
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
ValueType Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getRu() const {
    const ValueType uRedMask = (1 << redBitCount) - 1;
    return (color >> (greenBitCount + blueBitCount + alphaBitCount)) & uRedMask;
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
ValueType Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getGu() const {
    const ValueType uGreenMask = (1 << greenBitCount) - 1;
    return (color >> (blueBitCount + alphaBitCount)) & uGreenMask;
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
ValueType Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getBu() const {
    const ValueType uBlueMask = (1 << blueBitCount) - 1;
    return (color >> (alphaBitCount)) & uBlueMask;
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
ValueType Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getAu() const {
    const ValueType uAlphaMask = (1 << alphaBitCount) - 1;
    return color & uAlphaMask;
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
u32 Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getABGR() const {
    const ValueType uRedMask = (1 << redBitCount) - 1;
    const ValueType uGreenMask = (1 << greenBitCount) - 1;
    const ValueType uBlueMask = (1 << blueBitCount) - 1;
    const ValueType uAlphaMask = (1 << alphaBitCount) - 1;
    
    u32 abgr;
    ((u8*)&abgr)[0] = (color >> (greenBitCount + blueBitCount + alphaBitCount)) & uRedMask;
    ((u8*)&abgr)[1] = (color >> (blueBitCount + alphaBitCount)) & uGreenMask;
    ((u8*)&abgr)[2] = (color >> (alphaBitCount)) & uBlueMask;
    ((u8*)&abgr)[3] = (color & uAlphaMask);
    return abgr;
}
                       
template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
u32 Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getRGBA() const {
   return color;
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
void Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::set(const f32 fR, const f32 fG, const f32 fB, const f32 fA) {
    
    static_assert((8 * sizeof(ValueType)) == (redBitCount + greenBitCount + blueBitCount + alphaBitCount),
                  "The type specified to store the color value cannot contain the given masks.");
    
    const ValueType uR = (ValueType)roundf(fR * ((1 << redBitCount) - 1));
    const ValueType uG = (ValueType)roundf(fG * ((1 << greenBitCount) - 1));
    const ValueType uB = (ValueType)roundf(fB * ((1 << blueBitCount) - 1));
    const ValueType uA = (ValueType)roundf(fA * ((1 << alphaBitCount) - 1));
    set(uR, uG, uB, uA);
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
void Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::set(const ValueType uR, const ValueType uG, const ValueType uB, const ValueType uA) {
    color = (uR << (greenBitCount + blueBitCount + alphaBitCount)) + (uG << (blueBitCount + alphaBitCount)) + (uB << (alphaBitCount)) + uA;
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::add(const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &a, const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &b) {
    
    const float aR = a.getRf();
    const float aG = a.getGf();
    const float aB = a.getBf();
    const float aA = a.getAf();
    
    const float bR = b.getRf();
    const float bG = b.getGf();
    const float bB = b.getBf();
    const float bA = b.getAf();
    
    const float oR = Math::clamp(aR + bR, 0.f, 1.f);
    const float oG = Math::clamp(aG + bG, 0.f, 1.f);
    const float oB = Math::clamp(aB + bB, 0.f, 1.f);
    const float oA = Math::clamp(aA + bA, 0.f, 1.f);
    
    Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> output(oR, oG, oB, oA);
    
    return output;
}

template<typename ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::scale(const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &a, f32 b) {
    const float aR = a.getRf();
    const float aG = a.getGf();
    const float aB = a.getBf();
    const float aA = a.getAf();
    
    const float oR = Math::clamp(aR * b, 0.f, 1.f);
    const float oG = Math::clamp(aG * b, 0.f, 1.f);
    const float oB = Math::clamp(aB * b, 0.f, 1.f);
    const float oA = Math::clamp(aA * b, 0.f, 1.f);
    
    Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> output(oR, oG, oB, oA);
    
    return output;
}

template struct Color<u32, 8, 8, 8, 8>;

#endif // __WASTELADNS_COLOR_IMPL__
