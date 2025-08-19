#ifndef __WASTELADNS_COLOR_H__
#define __WASTELADNS_COLOR_H__

#define RGBA_PARAMS(color) color.getRf(), color.getGf(), color.getBf(), color.getAf()

struct Color32 {
    Color32() = default;
    Color32(const f32 fR, const f32 fG, const f32 fB, const f32 fA = 1.f) { set(fR, fG, fB, fA); }
    Color32(const u8 uR, const u8 uG, const u8 uB, const u8 uA = 255) { set(uR, uG, uB, uA); }

    f32 getRf() const { return getRu() / 255.f; }
    f32 getGf() const { return getGu() / 255.f; }
    f32 getBf() const { return getBu() / 255.f; }
    f32 getAf() const { return getAu() / 255.f; }

    u8 getRu() const { return (c >> 24) & 255; }
    u8 getGu() const { return (c >> 16) & 255; }
    u8 getBu() const { return (c >> 8) & 255; }
    u8 getAu() const { return c & 255; }

    u32 ABGR() const { return Color32((u8)(c & 255), (u8)((c >> 8) & 255), (u8)((c >> 16) & 255), (u8)((c >> 24) & 255)).c; }
    u32 RGBA() const { return c; }
    float4 RGBAv4() const { return float4(getRf(), getGf(), getBf(), getAf()); }
    
    void set(const f32 fR, const f32 fG, const f32 fB, const f32 fA = 1.f) {
        c = ((u32)(roundf(fR * 255))<<24)
          | ((u32)(roundf(fG * 255)) << 16)
          | ((u32)(roundf(fB * 255)) << 8)
          | (u32)roundf(fA * 255);
    }
	void set(const u8 uR, const u8 uG, const u8 uB, const u8 uA = 255) {
        c = (uR << 24) | (uG << 16) | (uB << 8) | uA;
    }
    
    static Color32 add(const Color32& a, const Color32& b) {
        return Color32(
            (u8)math::max(a.getRu() + b.getRu(), 255),
            (u8)math::max(a.getGu() + b.getGu(), 255),
            (u8)math::max(a.getBu() + b.getBu(), 255),
            (u8)math::max(a.getAu() + b.getAu(), 255));
    }
    static Color32 scale(const Color32& a, f32 s) {
        return Color32(
            math::max(u8(a.getRu() * s), (u8)255),
            math::max(u8(a.getGu() * s), (u8)255),
            math::max(u8(a.getBu() * s), (u8)255),
            math::max(u8(a.getAu() * s), (u8)255));
    }
    
	u32 c; // we could store a union with u8[4], but it'd be confusing since a u32 rgba would be rgba = {a, b, g, r} (index 3 would be r);
};

#endif // __WASTELADNS_COLOR_H__
