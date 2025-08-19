#ifndef __WASTELADNS_LIBS_H__
#define __WASTELADNS_LIBS_H__

// STB

allocator::PagedArena* Allocator_stb_arena = nullptr;
struct Allocator_stb {
	static void* malloc(size_t size) {
		return allocator::alloc_arena(*Allocator_stb_arena, size, 16);
	}
	static void* realloc(void* oldptr, size_t oldsize, size_t newsize) {
		return allocator::realloc_arena(*Allocator_stb_arena, oldptr, oldsize, newsize, 16);
	}
	static void free(void* ptr) {}
};
#define STBI_MALLOC(sz)						Allocator_stb::malloc(sz)
#define STBI_REALLOC(p,newsz)				Allocator_stb::realloc(p,0,newsz)
#define STBI_FREE(p)						Allocator_stb::free(p)
#define STBI_REALLOC_SIZED(p,oldsz,newsz)	Allocator_stb::realloc(p,oldsz,newsz)

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"
#include "lib/stb/stb_easy_font.h"

force_inline stbi_uc* stbi_load_arena(
char const* filename, int* x, int* y, int* comp, int req_comp, allocator::PagedArena& arena) {
	Allocator_stb_arena = &arena;
	unsigned char* result = stbi_load(filename, x, y, comp, req_comp);
	Allocator_stb_arena = nullptr;
	return result;
}

// UFBX

struct Allocator_ufbx {
    static void* alloc_fn(void* user, size_t size) {
        allocator::PagedArena* arena = (allocator::PagedArena*)user;
        return allocator::alloc_arena(*arena, size, 16);
    }
    static void* realloc_fn(void* user, void* old_ptr, size_t old_size, size_t new_size) {
        allocator::PagedArena* arena = (allocator::PagedArena*)user;
        return allocator::realloc_arena(*arena, old_ptr, old_size, new_size, 16);
    }
    static void free_fn(void* user, void* ptr, size_t size) {
        allocator::PagedArena* arena = (allocator::PagedArena*)user;
        allocator::free_arena(*arena, ptr, size);
    }
};
#define UFBX_REAL_IS_FLOAT
#define UFBX_MINIMAL
#define UFBXI_FEATURE_TRIANGULATION 1
#include "lib/fbx/ufbx.c"

force_inline void setup_fbx_arena(ufbx_allocator& allocator, allocator::PagedArena& arena) {
    allocator.alloc_fn = &Allocator_ufbx::alloc_fn;
    allocator.realloc_fn = &Allocator_ufbx::realloc_fn;
    allocator.free_fn = &Allocator_ufbx::free_fn;
    allocator.user = &arena;
}

#endif // __WASTELADNS_LIBS_H__
