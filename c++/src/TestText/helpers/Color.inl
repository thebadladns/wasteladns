#include <math.h>
#include <type_traits>

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::Color(const float fR, const float fG, const float fB, const float fA) {
    set(fR, fG, fB, fA);
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::Color(const ValueType uR, const ValueType uG, const ValueType uB, const ValueType uA) {
    set(uR, uG, uB, uA);
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::~Color() {
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
float Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getRf() const {
    const ValueType uR = getRu();
    return (float) uR/ ((1 << redBitCount) - 1);
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
float Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getGf() const {
    const ValueType uG = getGu();
    return (float) uG / ((1 << greenBitCount) - 1);
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
float Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getBf() const {
    const ValueType uB = getBu();
    return (float) uB / ((1 << blueBitCount) - 1);
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
float Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getAf() const {
    const ValueType uA = getAu();
    return (float) uA / ((1 << alphaBitCount) - 1);
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
ValueType Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getRu() const {
    const ValueType uRedMask = (1 << redBitCount) - 1;
    return (color >> (greenBitCount + blueBitCount + alphaBitCount)) & uRedMask;
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
ValueType Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getGu() const {
    const ValueType uGreenMask = (1 << greenBitCount) - 1;
    return (color >> (blueBitCount + alphaBitCount)) & uGreenMask;
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
ValueType Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getBu() const {
    const ValueType uBlueMask = (1 << blueBitCount) - 1;
    return (color >> (alphaBitCount)) & uBlueMask;
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
ValueType Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::getAu() const {
    const ValueType uAlphaMask = (1 << alphaBitCount) - 1;
    return color & uAlphaMask;
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
void Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::set(const float fR, const float fG, const float fB, const float fA) {
    
    static_assert((8 * sizeof(ValueType)) == (redBitCount + greenBitCount + blueBitCount + alphaBitCount),
                  "The type specified to store the color value cannot contain the given masks.");

    const ValueType uR = (ValueType)roundf(fR * ((1 << redBitCount) - 1));
    const ValueType uG = (ValueType)roundf(fG * ((1 << greenBitCount) - 1));
    const ValueType uB = (ValueType)roundf(fB * ((1 << blueBitCount) - 1));
    const ValueType uA = (ValueType)roundf(fA * ((1 << alphaBitCount) - 1));
    set(uR, uG, uB, uA);
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
void Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount>::set(const ValueType uR, const ValueType uG, const ValueType uB, const ValueType uA) {
    color = (uR << (greenBitCount + blueBitCount + alphaBitCount)) + (uG << (blueBitCount + alphaBitCount)) + (uB << (alphaBitCount)) + uA;
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> operator*(const float a,
                                                                                    const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &color)
{
    const float aR = color.getRf();
    const float aG = color.getGf();
    const float aB = color.getBf();
    const float aA = color.getAf();
    
    const float oR = fmin(fmax(aR * a, 0.f), 1.f);
    const float oG = fmin(fmax(aG * a, 0.f), 1.f);
    const float oB = fmin(fmax(aB * a, 0.f), 1.f);
    const float oA = fmin(fmax(aA * a, 0.f), 1.f);
    
    Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> output(oR, oG, oB, oA);
    
    return output;
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> operator*(const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &color
                                                                                    , const float a)
{
    return a * color;
}

template<class ValueType, u8 redBitCount, u8 greenBitCount, u8 blueBitCount, u8 alphaBitCount>
Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> operator+(const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &a,
                                                                                    const Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> &b)
{
    const float aR = a.getRf();
    const float aG = a.getGf();
    const float aB = a.getBf();
    const float aA = a.getAf();

    const float bR = b.getRf();
    const float bG = b.getGf();
    const float bB = b.getBf();
    const float bA = b.getAf();

    const float oR = fmin(fmax(aR + bR, 0.f), 1.f);
    const float oG = fmin(fmax(aG + bG, 0.f), 1.f);
    const float oB = fmin(fmax(aB + bB, 0.f), 1.f);
    const float oA = fmin(fmax(aA + bA, 0.f), 1.f);
    
    Color<ValueType, redBitCount, greenBitCount, blueBitCount, alphaBitCount> output(oR, oG, oB, oA);
    
    return output;
}