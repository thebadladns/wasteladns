#ifndef __WASTELADNS_ALLOCATOR_H__
#define __WASTELADNS_ALLOCATOR_H__

namespace Allocator {
struct Arena {
    u8* curr;
    u8* end;
    __DEBUGDEF(uintptr_t* highmark;) // pointer to track overall allocations of scoped copies
};

__DEBUGDEF(Arena emergencyArena;) // to prevent crashes

void init_arena(Arena& arena, size_t capacity) {
    arena.curr = (u8*)malloc(capacity);
    arena.end = arena.curr + capacity;
    __DEBUGEXP(arena.highmark = nullptr);
}

void* alloc_arena(Arena& arena, ptrdiff_t size, ptrdiff_t align) {
    assert((align & (align - 1)) == 0); // Alignment needs to be a power of two
    uintptr_t curr_aligned = ((uintptr_t)arena.curr + (align - 1)) & -align;
    if (curr_aligned + size <= (uintptr_t)arena.end) {
        arena.curr = (u8*)(curr_aligned + size);
        __DEBUGEXP(if (arena.highmark) { *arena.highmark = Math::max(*arena.highmark, (uintptr_t)arena.curr); });
        return (void*)curr_aligned;
    }
    assert(0);
    return 0;
}

void free_arena(Arena& arena, void* ptr, ptrdiff_t size) {
    // because of alignment differences between allocations, we don't have enough information
    // to guarantee freeing all allocations. However, we can still try and see if the requested
    // pointer coincides with the last allocation
    void* oldbuff = (void*)((uintptr_t)arena.curr - size);
    if (oldbuff == ptr) {
        arena.curr = (u8*)oldbuff;
    }
}

struct Buffer_t {
    u8* data;
    ptrdiff_t len;
    ptrdiff_t cap;
};

void grow(Buffer_t& b, ptrdiff_t size, ptrdiff_t align, Arena& arena) {
    ptrdiff_t doublecap = b.cap ? 2 * b.cap : 2;
    if (b.data + size * b.cap == arena.curr) {
        alloc_arena(arena, size * b.cap, align);
    } else {
        u8* data = (u8*)alloc_arena(arena, doublecap * size, align);
        if (b.len) { memcpy(data, b.data, size * b.len); }
        b.data = data;
    }
    b.cap = doublecap;
}

u8& push(Buffer_t& b, ptrdiff_t size, ptrdiff_t align, Arena& arena) {
    if (b.len >= b.cap) { grow(b, size, align, arena); }
    return *(b.data + size * b.len++);
}

void reserve(Buffer_t& b, ptrdiff_t cap, ptrdiff_t size, ptrdiff_t align, Arena& arena) {
    assert(b.cap == 0); // buffer is not empty
    b.data = (u8*)alloc_arena(arena, cap * size, align);
    b.len = 0;
    b.cap = cap;
}

template<typename _T>
struct Buffer {
    _T* data;
    ptrdiff_t len;
    ptrdiff_t cap;
};

template<typename _T>
_T& push(Buffer<_T>& b, Arena& arena) {
    if (b.len >= b.cap) { grow(*(Buffer_t*)&b, sizeof(_T), alignof(_T), arena); }
    return *(b.data + b.len++);
}

template<typename _T>
void reserve(Buffer<_T>& b, ptrdiff_t cap, Arena& arena) {
    assert(b.cap == 0); // buffer is not empty
    b.data = (_T*)alloc_arena(arena, sizeof(_T) * cap, alignof(_T));
    b.len = 0;
    b.cap = cap;
}

template<typename _T>
struct Pool {
    // important: _T must be the first member of the union, so we can assume _T* = Slot<_T>*
    struct Slot {
        union {
			_T live;
		    Slot* next;
        } state;
        u32 alive;
    };
    __DEBUGDEF(const char* name;)
    Slot* data;
	Slot* firstAvailable;
	ptrdiff_t cap;
    ptrdiff_t count;
};

template<typename _T>
void init_pool(Pool<_T>& pool, ptrdiff_t cap, Arena& arena) {
	typedef typename Pool<_T>::Slot _Slot;
	pool.cap = cap;
    pool.data = (_Slot*)Allocator::alloc_arena(arena, sizeof(_Slot) * cap, alignof(_Slot));
    pool.firstAvailable = pool.data;
	for (u32 i = 0; i < cap - 1; i++) { pool.data[i].state.next = &(pool.data[i+1]); pool.data[i].alive = 0; }
	pool.data[cap - 1].state.next = nullptr; pool.data[cap - 1].alive = 0;
	pool.count = 0;
}

template<typename _T>
_T&  alloc_pool(Pool<_T>& pool) {
#if __DEBUG
    if (!pool.firstAvailable) { // can't regrow without messing up the pointers that have already been handed out
        // todo: remove, this breaks everything basically (counts, address to indexes, etc)
		Platform::debuglog("Pool %s ran out of slots, defaulting to emergency allocator\n", pool.name);
        return *(_T*)Allocator::alloc_arena(Allocator::emergencyArena, sizeof(_T), alignof(_T));
    }
#endif
	_T& out = pool.firstAvailable->state.live;
    pool.firstAvailable->alive = 1;
    pool.firstAvailable = pool.firstAvailable->state.next;
	pool.count++;
    return out;
}
template<typename _T>
void free_pool(Pool<_T>& pool, _T& slot) {
    typedef typename Pool<_T>::Slot _Slot;
    assert((_Slot*)slot >= &pool.data && (_Slot*)slot < &pool.data + pool.cap); // object didn't come from this pool
    ((_Slot*)&slot)->state.next = pool.firstAvailable;
    ((_Slot*)&slot)->alive = 0;
    pool.firstAvailable = (_Slot*)slot;
	pool.count--;
}
template<typename _T>
u32 get_pool_index(Pool<_T>& pool, _T& slot) { return (u32)(((typename Pool<_T>::Slot*)&slot) - pool.data); }
template<typename _T>
_T& get_pool_slot(Pool<_T>& pool, const u32 index) { return pool.data[index].state.live; }

}


#endif // __WASTELADNS_ALLOCATOR_H__
