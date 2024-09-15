#ifndef __WASTELADNS_TEMPLATE_DEFINES_H__
#define __WASTELADNS_TEMPLATE_DEFINES_H__

// Helpers
#define RT_PT(name, type, DECO, DECOEND) \
    DECO type name DECOEND(type v);
#define RT_PTT(name, type, DECO, DECOEND) \
    DECO type name DECOEND(type a, type b);
#define RT_PTTT(name, type, DECO, DECOEND) \
    DECO type name DECOEND(type a, type b, type c);
#define RT_PTTTT(name, type, DECO, DECOEND) \
    DECO type name DECOEND(type a, type b, type c, type d);
#define RT_PTTTTT(name, type, DECO, DECOEND) \
    DECO type name DECOEND(type a, type b, type c, type d, type e);
#define RT2_PT(name, type, DECO, DECOEND) \
    DECO Vector2<type> name DECOEND(type v);
#define RT_PT2(name, type, DECO, DECOEND) \
    DECO type name DECOEND(const Vector2<type>& v);
#define RT_PT3(name, type, DECO, DECOEND) \
    DECO type name DECOEND(const Vector3<type>& v);
#define RT_PT4(name, type, DECO, DECOEND) \
    DECO type name DECOEND(const Vector4<type>& v);
#define RT2_PT2(name, type, DECO, DECOEND) \
    DECO Vector2<type> name DECOEND(const Vector2<type>& v);
#define RT3_PT3(name, type, DECO, DECOEND) \
    DECO Vector3<type> name DECOEND(const Vector3<type>& v);
#define RT4_PT4(name, type, DECO, DECOEND) \
    DECO Vector4<type> name DECOEND(const Vector4<type>& v);
#define RB_PT2(name, type, DECO, DECOEND) \
    DECO bool name DECOEND(Vector2<type>& v);
#define RB_PT3(name, type, DECO, DECOEND) \
    DECO bool name DECOEND(Vector3<type>& v);
#define RB_PT4(name, type, DECO, DECOEND) \
    DECO bool name DECOEND(Vector4<type>& v);
#define RT2_PT2T(name, type, DECO, DECOEND) \
    DECO Vector2<type> name DECOEND(const Vector2<type>& v, type a);
#define RT3_PT3T(name, type, DECO, DECOEND) \
    DECO Vector3<type> name DECOEND(const Vector3<type>& v, type a);
#define RT4_PT4T(name, type, DECO, DECOEND) \
    DECO Vector4<type> name DECOEND(const Vector4<type>& v, type a);
#define RT_PT2T2(name, type, DECO, DECOEND) \
    DECO type name DECOEND(const Vector2<type>& a, const Vector2<type>& b);
#define RT_PT3T3(name, type, DECO, DECOEND) \
    DECO type name DECOEND(const Vector3<type>& a, const Vector3<type>& b);
#define RT_PT4T4(name, type, DECO, DECOEND) \
    DECO type name DECOEND(const Vector4<type>& a, const Vector4<type>& b);
#define RT2_PT2T2(name, type, DECO, DECOEND) \
    DECO Vector2<type> name DECOEND(const Vector2<type>& a, const Vector2<type>& b);
#define RT3_PT3T3(name, type, DECO, DECOEND) \
    DECO Vector3<type> name DECOEND(const Vector3<type>& a, const Vector3<type>& b);
#define RT4_PT4T4(name, type, DECO, DECOEND) \
    DECO Vector4<type> name DECOEND(const Vector4<type>& a, const Vector4<type>& b);

#define TEMPLATE_FILTER_ON(f) f
#define TEMPLATE_FILTER_OFF(f)

#define DEFINE_TEMPLATES(group) group(TEMPLATE_FILTER_ON,TEMPLATE_FILTER_ON,_T,template<typename _T>,)
#define INSTANTIATE_TEMPLATES(group, type) group(TEMPLATE_FILTER_ON,TEMPLATE_FILTER_OFF,type,template,<type>)

#endif // __WASTELADNS_TEMPLATE_DEFINES_H__
