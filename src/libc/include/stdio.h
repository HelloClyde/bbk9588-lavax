#ifndef J2ME9588_STDIO_H
#define J2ME9588_STDIO_H

#include <stddef.h>
#include <stdarg.h>

typedef struct j2me9588_file FILE;

extern FILE* stdout;
extern FILE* stderr;

#define EOF (-1)
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifdef __cplusplus
extern "C" {
#endif

int snprintf(char* buffer, size_t size, const char* format, ...);
int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments);
int sprintf(char* buffer, const char* format, ...);
int vsprintf(char* buffer, const char* format, va_list arguments);
int sscanf(const char* text, const char* format, ...);
int printf(const char* format, ...);
int fprintf(FILE* stream, const char* format, ...);
int fputs(const char* text, FILE* stream);
int puts(const char* text);
int fflush(FILE* stream);
FILE* fopen(const char* path, const char* mode);
int fclose(FILE* stream);
size_t fread(void* buffer, size_t size, size_t count, FILE* stream);
size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
int fseek(FILE* stream, long offset, int origin);
long ftell(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);
int fgetc(FILE* stream);
int fputc(int character, FILE* stream);
void rewind(FILE* stream);
int remove(const char* path);

#define getc(stream) fgetc(stream)
#define putc(character, stream) fputc((character), (stream))

#ifdef __cplusplus
}
#endif

#endif
