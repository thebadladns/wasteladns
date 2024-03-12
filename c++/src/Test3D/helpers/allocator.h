#ifndef __WASTELADNS_ALLOCATOR_H__
#define __WASTELADNS_ALLOCATOR_H__

namespace Allocator {
    struct Arena {
        u8* curr;
        u8* end;
    };

    void init_arena(Arena& arena, size_t capacity) {
        arena.curr = (u8*)malloc(capacity);
        arena.end = arena.curr + capacity;
    }

    void* alloc_arena(Arena& arena, ptrdiff_t size, ptrdiff_t align) {
        assert((align & (align - 1)) == 0); // Alignment needs to be a power of two")
        uintptr_t next = ((uintptr_t)arena.curr + size + (align - 1)) & -align;
        if (next < (uintptr_t)arena.end) {
            void* ptr = (void*)arena.curr;
            arena.curr = (u8*)next;
            return ptr;
        }
        return 0;
    }

    void free_arena(Arena&) {}

    Allocator::Arena frameArena;
    struct FrameArena {
        static void* static_allocate(size_t bytes) {
            return alloc_arena(Allocator::frameArena, bytes, 16);
        }
        static void static_deallocate(void* /*ptr*/, size_t /*bytes*/) {}
    };
}

#endif // __WASTELADNS_ALLOCATOR_H__
