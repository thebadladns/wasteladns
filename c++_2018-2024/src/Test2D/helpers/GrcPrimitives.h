#ifndef __DEBUGDRAW_H__
#define __DEBUGDRAW_H__

#include "Color.h"
#include "Vector.h"
#include "Mat44.h"

class Mat44;

namespace GrcPrimitives
{
    void segment(const Vec3& vStart, const Vec3& vEnd, const Col color);
    void ray(const Vec3& vStart, const Vec3& vDir, Col color);
    void openSegment(const Vec3& vStart, const Vec3& vDir, const Col color);
    void line(const Vec3& vPos, const Vec3& vDir, const Col color);
    
    void poly(const Vec3* aPos, const u8 count, Col color);
    void circle(const Vec3& vPos, const Vec3 vAxis, const f32 radius, const Col color);
    void sphere(const Vec3& vPos, const f32 radius, const Col color);
    
    void matrix(const Mat44& matrix);
    
#ifdef GRC_PRIMITIVES_ENABLE_TEXT
    struct TextParams {
        TextParams()
        : scale(1.f)
        , color(1.f, 1.f, 1.f, 1.f)
        {}
        
        Vec3 pos;
        const char* text;
        f32 scale;
        Col color;
    };
    void text(const Vec3& pos, const char* data);
    void text(const TextParams& params);
#endif
    
    struct GraphParams {
        GraphParams()
        : w(300.f)
        , h(60.f)
        , count(0)
        , color(1.f, 1.f, 1.f, 1.f)
        {}
        
        Vec3 topLeft;
        f32* values;
        f32 min, max;
        f32 w, h;
        u32 count;
        Col color;
    };
    void graph(const GraphParams& params);
};

/*******************
 * Implementation
 ******************/

#include "GrcPrimitives.inl"

#endif
