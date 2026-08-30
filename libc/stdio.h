#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

#define EOF (-1)
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct {
    unsigned char* data;
    size_t size;
    size_t pos;
} FILE;

extern FILE* stderr;
extern FILE* stdout;

FILE* fopen(const char* filename, const char* mode);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int fseek(FILE* stream, long int offset, int whence);
long int ftell(FILE* stream);
int fclose(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);
int fflush(FILE* stream);

int printf(const char* format, ...);
int fprintf(FILE* stream, const char* format, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t size, const char* format, ...);
int vprintf(const char* format, va_list ap);
int vfprintf(FILE* stream, const char* format, va_list ap);
int vsnprintf(char* str, size_t size, const char* format, va_list ap);
int vsprintf(char* str, const char* format, va_list ap);

int puts(const char* str);
int putchar(int c);

int remove(const char* filename);
int rename(const char* old_filename, const char* new_filename);

int sscanf(const char* str, const char* format, ...);

#endif
