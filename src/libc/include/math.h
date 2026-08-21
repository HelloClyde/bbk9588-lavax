#ifndef J2ME9588_MATH_H
#define J2ME9588_MATH_H

#define HUGE_VAL (__builtin_huge_val())
#define HUGE_VALF (__builtin_huge_valf())
#define INFINITY (__builtin_inff())
#define NAN (__builtin_nanf(""))
#define isnan(value) __builtin_isnan(value)
#define finite(value) __builtin_isfinite(value)

#ifdef __cplusplus
extern "C" {
#endif

double floor(double value);
double ceil(double value);
double fmod(double value, double divisor);
double sqrt(double value);
double sin(double value);
double cos(double value);
double tan(double value);
double asin(double value);
double acos(double value);
double atan(double value);
double exp(double value);
double log(double value);
float floorf(float value);
float ceilf(float value);
float fmodf(float value, float divisor);
float sqrtf(float value);

#ifdef __cplusplus
}
#endif

#endif
