#include "libc/stdio.h"
#include "libc/stdlib.h"
#include "libc/string.h"
#include "libc/ctype.h"
#include "libc/math.h"
#include "mem.h"
#include "vga.h"

FILE* stdout = (FILE*)1;
FILE* stderr = (FILE*)2;

extern uint32_t doom_wad_start;
extern uint32_t doom_wad_size;

// Memory
void* malloc(size_t size) {
    return kmalloc(size);
}
void free(void* ptr) {
    kfree(ptr); // Does nothing in bump allocator, but Doom only frees rarely or on exit
}
void* calloc(size_t nmemb, size_t size) {
    void* ptr = kmalloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}
void* realloc(void* ptr, size_t size) {
    // Naive realloc since we don't know the old size and have a bump allocator
    // Doom generic doesn't actually use realloc much if at all.
    void* new_ptr = kmalloc(size);
    if (ptr && new_ptr) {
        // We can't safely copy without size, but let's hope it's small or we just don't support it
        // Z_Malloc in Doom does not use realloc
    }
    return new_ptr;
}

// String
size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) dest[i] = src[i];
    for ( ; i < n; i++) dest[i] = '\0';
    return dest;
}
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}
int strncmp(const char* s1, const char* s2, size_t n) {
    if (n == 0) return 0;
    while (n-- > 0 && *s1 && *s2) {
        if (*s1 != *s2) return *(unsigned char*)s1 - *(unsigned char*)s2;
        s1++; s2++;
    }
    if (n == (size_t)-1) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}
char* strcat(char* dest, const char* src) {
    strcpy(dest + strlen(dest), src);
    return dest;
}
char* strncat(char* dest, const char* src, size_t n) {
    char* d = dest + strlen(dest);
    while (n-- > 0 && *src) *d++ = *src++;
    *d = '\0';
    return dest;
}
int strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        int diff = tolower(*s1) - tolower(*s2);
        if (diff != 0) return diff;
        s1++; s2++;
    }
    return tolower(*s1) - tolower(*s2);
}
int strncasecmp(const char* s1, const char* s2, size_t n) {
    if (n == 0) return 0;
    while (n-- > 0 && *s1 && *s2) {
        int diff = tolower(*s1) - tolower(*s2);
        if (diff != 0) return diff;
        s1++; s2++;
    }
    if (n == (size_t)-1) return 0;
    return tolower(*s1) - tolower(*s2);
}
void* memcpy(void* dest, const void* src, size_t n) {
    char* d = dest; const char* s = src;
    while (n--) *d++ = *s++;
    return dest;
}
void* memmove(void* dest, const void* src, size_t n) {
    char* d = dest; const char* s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}
void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}
char* strdup(const char* s) {
    size_t len = strlen(s);
    char* d = malloc(len + 1);
    if (d) strcpy(d, s);
    return d;
}
char* strrchr(const char* s, int c) {
    char* ret = NULL;
    do {
        if (*s == (char)c) ret = (char*)s;
    } while (*s++);
    return ret;
}
char* strchr(const char* s, int c) {
    while (*s != (char)c) {
        if (!*s++) return NULL;
    }
    return (char*)s;
}
char* strstr(const char* haystack, const char* needle) {
    size_t n = strlen(needle);
    while (*haystack) {
        if (!memcmp(haystack, needle, n)) return (char*)haystack;
        haystack++;
    }
    return NULL;
}

// Ctype
int toupper(int c) { return (c >= 'a' && c <= 'z') ? c - 32 : c; }
int tolower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }
int isspace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; }
int isdigit(int c) { return c >= '0' && c <= '9'; }
int isalpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
int isalnum(int c) { return isalpha(c) || isdigit(c); }
int isprint(int c) { return c >= 0x20 && c <= 0x7E; }

// Stdlib
int atoi(const char* str) {
    int res = 0, sign = 1;
    while (isspace(*str)) str++;
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;
    while (isdigit(*str)) res = res * 10 + (*str++ - '0');
    return res * sign;
}
void exit(int status) {
    vga_puts("DOOM EXITED!\n");
    while(1) { __asm__ __volatile__("hlt"); }
}
int abs(int j) { return j < 0 ? -j : j; }
int system(const char* command) { return -1; }
double atof(const char* str) { return 0.0; } // Stub

// Math stub
double sin(double x) { return 0; }
double cos(double x) { return 0; }
double sqrt(double x) { return 0; }
double fabs(double x) { return x < 0 ? -x : x; }
double atan2(double y, double x) { return 0; }

// Printf implementation
static void itoa_buf(int value, char* str, int base) {
    char* rc;
    char* ptr;
    char* low;
    if (base < 2 || base > 36) { *str = '\0'; return; }
    rc = ptr = str;
    if (value < 0 && base == 10) { *ptr++ = '-'; }
    low = ptr;
    do {
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);
    *ptr-- = '\0';
    while (low < ptr) {
        char tmp = *low; *low++ = *ptr; *ptr-- = tmp;
    }
}

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t count = 0;
    while (*format && count < size - 1) {
        if (*format == '%') {
            format++;
            int pad_zero = 0;
            int width = 0;
            if (*format == '0') { pad_zero = 1; format++; }
            while (*format >= '0' && *format <= '9') { width = width * 10 + (*format - '0'); format++; }
            if (*format == '.') {
                format++;
                pad_zero = 1;
                width = 0;
                while (*format >= '0' && *format <= '9') { width = width * 10 + (*format - '0'); format++; }
            }
            if (*format == 'd' || *format == 'i') {
                int val = va_arg(ap, int);
                char buf[32];
                itoa_buf(val, buf, 10);
                size_t l = strlen(buf);
                int pad_len = width - l;
                if (pad_len < 0) pad_len = 0;
                while (pad_len > 0 && count < size - 1) { str[count++] = pad_zero ? '0' : ' '; pad_len--; }
                for (size_t i = 0; i < l && count < size - 1; i++) { str[count++] = buf[i]; }
            } else if (*format == 'x' || *format == 'X') {
                unsigned int val = va_arg(ap, unsigned int);
                char buf[32];
                itoa_buf(val, buf, 16);
                size_t l = strlen(buf);
                int pad_len = width - l;
                if (pad_len < 0) pad_len = 0;
                while (pad_len > 0 && count < size - 1) { str[count++] = pad_zero ? '0' : ' '; pad_len--; }
                for (size_t i = 0; i < l && count < size - 1; i++) { str[count++] = buf[i]; }
            } else if (*format == 's') {
                char* s = va_arg(ap, char*);
                if (!s) s = "(null)";
                size_t l = strlen(s);
                for (size_t i = 0; i < l && count < size - 1; i++) { str[count++] = s[i]; }
            } else if (*format == 'c') {
                char c = (char)va_arg(ap, int);
                if (count < size - 1) { str[count++] = c; }
            } else {
                if (count < size - 1) { str[count++] = *format; }
            }
        } else {
            str[count++] = *format;
        }
        format++;
    }
    str[count] = '\0';
    return count;
}
int vsprintf(char* str, const char* format, va_list ap) {
    return vsnprintf(str, 99999, format, ap);
}
int sprintf(char* str, const char* format, ...) {
    va_list ap; va_start(ap, format); int ret = vsprintf(str, format, ap); va_end(ap); return ret;
}
int snprintf(char* str, size_t size, const char* format, ...) {
    va_list ap; va_start(ap, format); int ret = vsnprintf(str, size, format, ap); va_end(ap); return ret;
}
int vprintf(const char* format, va_list ap) {
    char buf[512]; vsnprintf(buf, sizeof(buf), format, ap); vga_puts(buf); return strlen(buf);
}
int printf(const char* format, ...) {
    va_list ap; va_start(ap, format); int ret = vprintf(format, ap); va_end(ap); return ret;
}
int vfprintf(FILE* stream, const char* format, va_list ap) {
    return vprintf(format, ap);
}
int fprintf(FILE* stream, const char* format, ...) {
    va_list ap; va_start(ap, format); int ret = vfprintf(stream, format, ap); va_end(ap); return ret;
}
int puts(const char* str) { vga_puts(str); vga_puts("\n"); return 1; }
int putchar(int c) { char buf[2] = {c, 0}; vga_puts(buf); return c; }
int sscanf(const char* str, const char* format, ...) { return 0; } // Stub

// File I/O
FILE wad_file_handle;

FILE* fopen(const char* filename, const char* mode) {
    // Only support DOOM1.WAD from memory
    if (strcasecmp(filename, "doom1.wad") == 0) {
        if (doom_wad_start == 0) return NULL; // Not loaded via grub!
        wad_file_handle.data = (unsigned char*)doom_wad_start;
        wad_file_handle.size = doom_wad_size;
        wad_file_handle.pos = 0;
        return &wad_file_handle;
    }
    return NULL;
}
size_t fread(void* ptr, size_t size, size_t count, FILE* stream) {
    if (!stream || !stream->data) return 0;
    size_t total = size * count;
    if (stream->pos + total > stream->size) {
        total = stream->size - stream->pos;
        count = total / size;
    }
    memcpy(ptr, stream->data + stream->pos, total);
    stream->pos += total;
    return count;
}
int fseek(FILE* stream, long int offset, int origin) {
    if (!stream || !stream->data) return -1;
    if (origin == SEEK_SET) stream->pos = offset;
    else if (origin == SEEK_CUR) stream->pos += offset;
    else if (origin == SEEK_END) stream->pos = stream->size + offset;
    if (stream->pos > stream->size) stream->pos = stream->size;
    return 0;
}
long int ftell(FILE* stream) {
    if (!stream || !stream->data) return -1;
    return stream->pos;
}
int fclose(FILE* stream) {
    return 0; // Nothing to do
}
int feof(FILE* stream) {
    if (!stream || !stream->data) return 1;
    return stream->pos >= stream->size;
}
int errno = 0;
int mkdir(const char *pathname, int mode) { return -1; }

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return 0; // Stub
}
int ferror(FILE* stream) { return 0; }
int fflush(FILE* stream) { return 0; }
int remove(const char* filename) { return -1; }
int rename(const char* old_filename, const char* new_filename) { return -1; }
