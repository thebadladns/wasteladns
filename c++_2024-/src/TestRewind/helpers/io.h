#ifndef __WASTELADNS_IO_GLFW_H__
#define __WASTELADNS_IO_GLFW_H__

namespace platform {
    const auto format = snprintf;
    const auto format_va = vsnprintf;
#if __DEBUG
    const void debuglog(const char* format, ...) {
        char text[1024];
        va_list va;
        va_start(va, format);
        platform::format_va(text, sizeof(text), format, va);
        va_end(va);

        consoleLog(text);
    }
#else
    const void debuglog(const char* format, ...) {}
#endif
#if _MSC_VER
    int fopen(FILE** f, const char* name, const char* mode) { return ::fopen_s(f, name, mode); }
    int fscanf(FILE* f, const char* name, const char* mode) { return ::fscanf_s(f, name, mode); }
    int strncpy(char* dst, const char* src, size_t num) {return ::strncpy_s(dst, num, src, num); }
#else
    int fopen(FILE** f, const char* name, const char* mode) {
        *f = ::fopen(name, mode);
        return *f == 0;
    }
    const auto fscanf = ::fscanf;
    int strncpy(char* dst, const char* src, size_t num) { return ::strncpy(dst, src, num) != 0; }
#endif
    const auto fclose = ::fclose;
    const auto fgetc = ::fgetc;

    void append(char*& curr, const char* last, const char* format, ...) {
        va_list va;
        va_start(va, format);
        curr += platform::format_va(curr, (int)(last - curr), format, va);
        va_end(va);
    }
    struct StrBuilder {
        char* str;
        u32 bytes;
        char* curr = str;
        const char* last = str + bytes;
        bool full() { return curr >= last; }
    };
}

#endif // __WASTELADNS_IO_GLFW_H__
