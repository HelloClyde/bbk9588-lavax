#ifndef J2ME9588_STDLIB_H
#define J2ME9588_STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define alloca(size) __builtin_alloca(size)

#ifdef __cplusplus
extern "C" {
#endif

void abort(void) __attribute__((noreturn));
void exit(int status) __attribute__((noreturn));
void* malloc(size_t size);
void* calloc(size_t count, size_t size);
void* realloc(void* pointer, size_t size);
void free(void* pointer);
int rand(void);
void srand(unsigned int seed);
int atoi(const char* text);
int abs(int value);
long strtol(const char* text, char** end, int base);
unsigned long strtoul(const char* text, char** end, int base);

#ifdef __cplusplus
}
#endif

#endif
