#ifndef __WASTELADNS_ALLOCATOR_H__
#define __WASTELADNS_ALLOCATOR_H__

namespace allocator {

ptrdiff_t pagesize = 4 * 1024; // defaults to 4KB

// Simple arena buffer header: only the current offset is stored, as well as the capacity
// It can be used as a stack allocator by simply copying this header into a scoped variable
// For more details, see: https://nullprogram.com/blog/2023/09/27/
struct Arena { u8* curr; u8* end; };
void init_arena(Arena& arena, u8* mem, size_t capacity) {
    arena.curr = mem;
    arena.end = arena.curr + capacity;
}
void* alloc_arena(Arena& arena, ptrdiff_t size, ptrdiff_t align) {
    assert((align & (align - 1)) == 0); // Alignment needs to be a power of two
    uintptr_t curr_aligned = ((uintptr_t)arena.curr + (align - 1)) & -align;
    if (curr_aligned + size <= (uintptr_t)arena.end) {
        arena.curr = (u8*)(curr_aligned + size);
        return (void*)curr_aligned;
    }
    assert(0); // out of memory: consider using VirtualArena
    return 0;
}

// Same as Arena, except we reserve 4GB of virtual memory first, and commit 4KB pages as needed
// For arenas that are passed by copy, we store a separate pointer to their highmark value.
// This value is necessary to know when to commit more pages for scoped copies.
// It also lets us keep track of the highest allocation at any time.
struct PagedArena {
    u8* curr;
    u8* end;
    uintptr_t* highmark; // pointer to track overall allocations of scoped copies (see above)
};
u8* reserve_pages(const uintptr_t end, const uintptr_t start) {
    const size_t commitsize = end - start; assert(end > start);
    size_t commitsize_aligned = (commitsize + (pagesize - 1)) & -pagesize;
    platform::mem_commit((void*)start, commitsize_aligned);
    return (u8*)(start + commitsize_aligned);
};
void init_arena(PagedArena& arena, size_t capacity) {
    // reserve 4GB of virtual memory (we'll crash if we touch anything past that)
    const size_t capacity_aligned = 4ULL * 1024ULL * 1024ULL * 1024ULL;
    arena.curr = (u8*)platform::mem_reserve(capacity_aligned);
    arena.end = reserve_pages(uintptr_t(arena.curr + capacity), (uintptr_t)arena.curr);
    arena.highmark = nullptr;
}
void* alloc_arena(PagedArena& arena, ptrdiff_t size, ptrdiff_t align) {
    assert((align & (align - 1)) == 0); // Alignment needs to be a power of two
    uintptr_t curr_aligned = ((uintptr_t)arena.curr + (align - 1)) & -align;
    uintptr_t end_aligned = curr_aligned + size;
    uintptr_t highmark = (uintptr_t)arena.end;
    if (arena.highmark) { highmark = math::max(*arena.highmark, highmark); };
    if (end_aligned <= highmark) {
        // allocation successful, update highmark if needed
        if (arena.highmark) { *arena.highmark = math::max(*arena.highmark, end_aligned); };
    } else {
        // allocation goes past our committed memory space
        // commit however many pages we need
        arena.end = reserve_pages(end_aligned, highmark);
        if (arena.highmark) { *arena.highmark = (uintptr_t)arena.end; }
    }
    arena.curr = (u8*)end_aligned;
    return (void*)curr_aligned;
}
void* realloc_arena(PagedArena& arena, void* oldptr, ptrdiff_t oldsize, ptrdiff_t newsize, ptrdiff_t align) {
    void* oldbuff = (void*)((uintptr_t)arena.curr - oldsize);
    if (oldbuff == oldptr) {
        return alloc_arena(arena, newsize - oldsize, align);
    } else {
        void* data = alloc_arena(arena, newsize, align);
        if (oldsize) memcpy(data, oldptr, oldsize);
        return data;
    }
}
void free_arena(PagedArena& arena, void* ptr, ptrdiff_t size) {
    // because of alignment differences between allocations, we don't have enough information
    // to guarantee freeing all allocations. However, we can still try and see if the requested
    // pointer coincides with the last allocation
    void* oldbuff = (void*)((uintptr_t)arena.curr - size);
    if (oldbuff == ptr) {
        arena.curr = (u8*)oldbuff;
    }
}
#define ALLOC_BYTES(arena, type, size) (type*)allocator::alloc_arena(arena, size, alignof(type))
#define ALLOC_ARRAY(arena, type, count) (type*)allocator::alloc_arena(arena, (count) * sizeof(type), alignof(type))

// Dynamic array. When used with the same arena without any external allocations in between calls
// to allocator::push, the array will continue to grow in place. Note that, in any other case,
// calls to allocator::grow will not free the previous array (this is useful if, for example, the
// array is initialized with some stack-stored data, but is intended to grow past it)
// For more details, see https://nullprogram.com/blog/2023/10/05/
struct Buffer_t {
    u8* data;
    ptrdiff_t len;
    ptrdiff_t cap;
};
void grow(Buffer_t& b, ptrdiff_t size, ptrdiff_t align, PagedArena& arena) {
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
u8& push(Buffer_t& b, ptrdiff_t size, ptrdiff_t align, PagedArena& arena) {
    if (b.len >= b.cap) { grow(b, size, align, arena); }
    return *(b.data + size * b.len++);
}
void reserve(Buffer_t& b, ptrdiff_t cap, ptrdiff_t size, ptrdiff_t align, PagedArena& arena) {
    assert(b.cap == 0); // buffer is not empty
    b.data = (u8*)alloc_arena(arena, cap * size, align);
    b.len = 0;
    b.cap = cap;
}
template<typename T>
struct Buffer {
    T* data;
    ptrdiff_t len;
    ptrdiff_t cap;
};
template<typename T>
T& push(Buffer<T>& b, PagedArena& arena) {
    if (b.len >= b.cap) { grow(*(Buffer_t*)&b, sizeof(T), alignof(T), arena); }
    return *(b.data + b.len++);
}
template<typename T>
void reserve(Buffer<T>& b, ptrdiff_t cap, PagedArena& arena) {
    assert(b.cap == 0); // buffer is not empty
    b.data = (T*)alloc_arena(arena, sizeof(T) * cap, alignof(T));
    b.len = 0;
    b.cap = cap;
}

// tmp pool: mostly untested
template<typename T>
struct Pool {
    // important: _T must be the first member of the union, so we can assume _T* = Slot<_T>*
    struct Slot {
        union {
            T live;
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
template<typename T>
void init_pool(Pool<T>& pool, ptrdiff_t cap, PagedArena& arena) {
	typedef typename Pool<T>::Slot Slot;
	pool.cap = cap;
    pool.data = (Slot*)allocator::alloc_arena(arena, sizeof(Slot) * cap, alignof(Slot));
    pool.firstAvailable = pool.data;
	for (u32 i = 0; i < cap - 1; i++)
    { pool.data[i].state.next = &(pool.data[i+1]); pool.data[i].alive = 0; }
	pool.data[cap - 1].state.next = nullptr; pool.data[cap - 1].alive = 0;
	pool.count = 0;
}
template<typename T>
T&  alloc_pool(Pool<T>& pool) {
    assert(pool.firstAvailable); // can't regrow without messing up existing pointers to the pool
    T& out = pool.firstAvailable->state.live;
    pool.firstAvailable->alive = 1;
    pool.firstAvailable = pool.firstAvailable->state.next;
	pool.count++;
    return out;
}
template<typename T>
void free_pool(Pool<T>& pool, T& slot) {
    typedef typename Pool<T>::Slot Slot;
    assert((Slot*)slot >= &pool.data && (Slot*)slot < &pool.data + pool.cap); // object didn't come from this pool
    ((Slot*)&slot)->state.next = pool.firstAvailable;
    ((Slot*)&slot)->alive = 0;
    pool.firstAvailable = (Slot*)slot;
	pool.count--;
}
template<typename T>
u32 get_pool_index(Pool<T>& pool, T& slot)
{ return (u32)(((typename Pool<T>::Slot*)&slot) - pool.data); }
template<typename T>
T& get_pool_slot(Pool<T>& pool, const u32 index) { return pool.data[index].state.live; }

}


#endif // __WASTELADNS_ALLOCATOR_H__
