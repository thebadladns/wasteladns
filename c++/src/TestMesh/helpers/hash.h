#ifndef __WASTELADNS_HASH_H__
#define __WASTELADNS_HASH_H__

#ifndef UNITYBUILD
#include "types.h"
#endif

// TODO: understand this
namespace Hash {
    u32 fnv(const char* name) {
        const u8* data = (const u8*)name;
        u32 val = 3759247821;
        while(*data){
            val ^= *data++;
            val *= 0x01000193;
        }
        val &= 0x7fffffff;
        val |= val==0;
        return val;
    }
}

struct str32 {
    str32()
    : hash(0)
#ifdef __WASTELADNS_HASH_DEBUG__
    , debug_name(0)
#endif
    {}
    
    str32(const char* name)
    : hash(Hash::fnv(name))
#ifdef __WASTELADNS_HASH_DEBUG__
    , debug_name(name)
#endif
    {}
    
    u32 hash;
#ifdef __WASTELADNS_HASH_DEBUG__
    const char* debug_name;
#endif
};

bool operator==(const str32& a, const str32& b){ return a.hash == b.hash; }

#endif // __WASTELADNS_HASH_H__
