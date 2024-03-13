#ifndef __WASTELADNS_ALLOCATOR_H__
#define __WASTELADNS_ALLOCATOR_H__

namespace Allocator {
    struct Arena {
        u8* curr;
        u8* end;
        __DEBUGDEF(u8* highmark = 0;)
    };

    void init_arena(Arena& arena, size_t capacity) {
        arena.curr = (u8*)malloc(capacity);
        arena.end = arena.curr + capacity;
    }

    void* alloc_arena(Arena& arena, ptrdiff_t size, ptrdiff_t align) {
        assert((align & (align - 1)) == 0); // Alignment needs to be a power of two
        uintptr_t curr_aligned = ((uintptr_t)arena.curr + (align - 1)) & -align;
        if (curr_aligned + size < (uintptr_t)arena.end) {
            arena.curr = (u8*)(curr_aligned + size);
            __DEBUGEXP(arena.highmark = Math::max(arena.highmark, arena.curr));
            return (void*)curr_aligned;
        }
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

    Allocator::Arena frameArena;

    template <class T>
    struct ArenaSTL {
        typedef T value_type;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        template <class U>
        struct rebind { typedef ArenaSTL<U> other; };

        pointer address(reference value) const { return &value; }
        const_pointer address(const_reference value) const { return &value; }

        ArenaSTL() {}
        ArenaSTL(const ArenaSTL&) {}
        template <class U>
        ArenaSTL(const ArenaSTL<U>&) {}
        ~ArenaSTL() {}

        size_type max_size() const {
            return ((uintptr_t)Allocator::frameArena.end - (uintptr_t)Allocator::frameArena.curr) / sizeof(T);
        }
        pointer allocate(size_type num, const void* = 0) {
            return (pointer)alloc_arena(Allocator::frameArena, num * sizeof(T), alignof(T));
        }
        void construct(pointer p, const T& value) {
            new((void*)p)T(value);
        }
        void destroy(pointer p) {
            p->~T();
        }
        void deallocate(pointer p, size_type num) {
            return free_arena(Allocator::frameArena, p, num * sizeof(T));
        }
        
        static void* static_allocate(size_t bytes) {
            return alloc_arena(Allocator::frameArena, bytes, alignof(T));
        }
        static void static_deallocate(void* ptr, size_t bytes) {
            return free_arena(Allocator::frameArena, ptr, bytes);
        }
    };

    template <class T1, class T2>
    bool operator== (const ArenaSTL<T1>&, const ArenaSTL<T2>&) throw() {
        return true;
    }
    template <class T1, class T2>
    bool operator!= (const ArenaSTL<T1>&, const ArenaSTL<T2>&) throw() {
        return false;
    }
}

#endif // __WASTELADNS_ALLOCATOR_H__
