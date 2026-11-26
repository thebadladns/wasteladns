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
    assert(size > 0);
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
#define ALLOC_BYTES(arena, type, size, align) (type*)allocator::alloc_arena(arena, size, align)
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

// Pool allocator
// Stores an array of elements, with additional metadata for each item as a footer, so that each
// can be reinterpret-casted to the original type easily. Non-allocated items use the element's
// memory to store a pointer of the next free available item in the pool. The pool then stores the
// first item in this free list.
// Pool generates handles that are expected to have an index and a generation, as well as a "data"
// field that will be 0 when uninitialized. For this reason, indexes are internally stored with an
// offset of 1, so the idx=0,gen=0 can be used as an uninitialized sentinel value.
template<u32 index_bits, u32 generation_bits>
struct PoolHandle_Base {
    static constexpr u32 IndexBits = index_bits;
    static constexpr u32 GenerationBits = generation_bits;
    static constexpr u32 TotalBits = index_bits + generation_bits;
    union {
        struct {
            u32 gen : GenerationBits;
            u32 idx : IndexBits;
        };
        u32 data;
    };
    static_assert(TotalBits <= sizeof(u32) * 8);
};
// To be defined per type using the pool
template<typename T>
struct PoolHandle;
template<typename T>
struct Pool {
    // important: _T must be the first member of the union, so we can assume _T* = Slot<_T>*
    static_assert(sizeof(T) > 2);
    struct Slot {
        union {
            T live;
            Slot* next;
        } state;
        u32 alive : 1;
        u32 gen : PoolHandle<T>::GenerationBits;
    };
    __DEBUGDEF(const char* name;)
    __DEBUGDEF(PagedArena* regrow_arena;)
    Slot* data;
    Slot* firstAvailable;
    ptrdiff_t cap;
    ptrdiff_t count;
};
template<typename T>
T* resolve_pool_handle(Pool<T>& pool, PoolHandle<T> handle) {
    if (handle.idx <= 0 || handle.idx >= pool.cap)  { return nullptr; }
    typedef typename Pool<T>::Slot Slot;
    Slot& slot = pool.data[handle.idx - 1];
    if (handle.gen != slot.gen) { return nullptr; }
    return &slot.state.live;
}
template<typename T>
void init_pool(Pool<T>& pool, ptrdiff_t cap, PagedArena& arena __DEBUGDEF(, const char* name) ) {
    typedef typename Pool<T>::Slot Slot;
    pool.cap = cap;
    pool.data = (Slot*)allocator::alloc_arena(arena, sizeof(Slot) * cap, alignof(Slot));
    pool.firstAvailable = pool.data;
    for (u32 i = 0; i < cap - 1; i++)
    { pool.data[i].state.next = &(pool.data[i+1]); pool.data[i].alive = pool.data[i].gen = 0; }
    pool.data[cap - 1].state.next = nullptr; pool.data[cap - 1].alive = pool.data[cap - 1].gen = 0;
    pool.count = 0;
    __DEBUGDEF(pool.name = name; pool.regrow_arena = &arena;)
}
template<typename T>
struct AllocPoolResult {
    PoolHandle<T> handle;
    T* elem;
};
template<typename T>
AllocPoolResult<T> alloc_pool(Pool<T>& pool) {
    typedef typename Pool<T>::Slot Slot;
#if __DEBUG
    if (!pool.firstAvailable) {
        Buffer_t regrow_buffer = { (u8*)pool.data, pool.cap, pool.cap };
        grow(regrow_buffer, sizeof(Slot), alignof(Slot), *pool.regrow_arena);
        pool.data = (Slot*)regrow_buffer.data;
        for (u32 i = (u32)pool.cap; i < regrow_buffer.cap; i++) {
            pool.data[i].alive = pool.data[i].gen = 0; // init newly allocated members
        }
        pool.cap = regrow_buffer.cap;
        u32 last_free = (u32)pool.cap - 1; // rebuild free list so the last free slot is at the end
        for (u32 i = (u32)pool.cap - 1; i-- > 0;) {
            if (!pool.data[i].alive) {
                pool.data[i].state.next = &pool.data[last_free];
                last_free = i;
            }
        }
        pool.firstAvailable = &pool.data[last_free];
    }
#else
    assert(pool.firstAvailable);
#endif
    Slot& slot = *pool.firstAvailable;
    slot.alive = 1;
    pool.firstAvailable = pool.firstAvailable->state.next;
    pool.count++;
    AllocPoolResult<T> out;
    out.elem = &slot.state.live;
    out.handle.idx = (u32)(&slot - pool.data) + 1;
    out.handle.gen = slot.gen;
    return out;
}
template<typename T>
void free_pool(Pool<T>& pool, PoolHandle<T> handle) {
    typedef typename Pool<T>::Slot Slot;
    T* elem = resolve_pool_handle(pool, handle);
    if (!elem) { return; }
    Slot& slot = *(Slot*)elem;
    slot.state.next = pool.firstAvailable;
    slot.alive = 0;
    slot.gen++;
    pool.firstAvailable = &slot;
    pool.count--;
}

}


#endif // __WASTELADNS_ALLOCATOR_H__
