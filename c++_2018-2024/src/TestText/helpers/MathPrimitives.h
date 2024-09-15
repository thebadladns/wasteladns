#ifndef __MATHPRIMITIVES_H__
#define __MATHPRIMITIVES_H__

#define _USE_MATH_DEFINES
#include <math.h>
#include "TypeDefinitions.h"

const f64 kDegToRad = M_PI / 180.0f;
const f64 kRadToDeg = 1.0f / kDegToRad;
const f32 kVectorEqualityDelta = 0.0000001f;

#endif
