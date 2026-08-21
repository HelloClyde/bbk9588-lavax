#ifndef J2ME9588_STRING_H
#define J2ME9588_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* memcpy(void* destination, const void* source, size_t size);
void* memmove(void* destination, const void* source, size_t size);
void* memset(void* destination, int value, size_t size);
int memcmp(const void* a, const void* b, size_t size);
void* memchr(const void* data, int value, size_t size);

size_t strlen(const char* text);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t size);
int strcasecmp(const char* a, const char* b);
int strncasecmp(const char* a, const char* b, size_t size);
char* strcpy(char* destination, const char* source);
char* strncpy(char* destination, const char* source, size_t size);
char* strcat(char* destination, const char* source);
char* strncat(char* destination, const char* source, size_t size);
char* strchr(const char* text, int value);
char* strrchr(const char* text, int value);
char* strstr(const char* text, const char* needle);
char* strdup(const char* text);
char* strerror(int error);
size_t strlcpy(char* destination, const char* source, size_t size);
size_t strlcat(char* destination, const char* source, size_t size);

#ifdef __cplusplus
}
#endif

#endif
