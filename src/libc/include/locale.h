#ifndef J2ME9588_FREESTANDING_LOCALE_H
#define J2ME9588_FREESTANDING_LOCALE_H

#include <stddef.h>

#define LC_ALL 0
#define LC_CTYPE 1

#ifdef __cplusplus
extern "C" {
#endif

char *setlocale(int category, const char *locale);

#ifdef __cplusplus
}
#endif

#endif
