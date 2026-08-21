#include <math.h>

#define LAVAX_PI 3.14159265358979323846
#define LAVAX_HALF_PI 1.57079632679489661923

static double absolute(double value) { return value < 0.0 ? -value : value; }

double sqrt(double value)
{
    double estimate;
    int iteration;
    if (value <= 0.0) return 0.0;
    estimate = value > 1.0 ? value : 1.0;
    for (iteration = 0; iteration < 14; ++iteration) {
        estimate = (estimate + value / estimate) * 0.5;
    }
    return estimate;
}

static double wrap_angle(double value)
{
    while (value > LAVAX_PI) value -= 2.0 * LAVAX_PI;
    while (value < -LAVAX_PI) value += 2.0 * LAVAX_PI;
    return value;
}

double sin(double value)
{
    double squared;
    value = wrap_angle(value);
    if (value > LAVAX_HALF_PI) value = LAVAX_PI - value;
    if (value < -LAVAX_HALF_PI) value = -LAVAX_PI - value;
    squared = value * value;
    return value * (1.0 - squared / 6.0 + squared * squared / 120.0 -
        squared * squared * squared / 5040.0);
}

double cos(double value) { return sin(value + LAVAX_HALF_PI); }
double tan(double value) { return sin(value) / cos(value); }

double atan(double value)
{
    int negative = value < 0.0;
    double result;
    if (negative) value = -value;
    if (value > 1.0) {
        result = LAVAX_HALF_PI - atan(1.0 / value);
    } else {
        result = value / (1.0 + 0.280872 * value * value);
    }
    return negative ? -result : result;
}

double asin(double value)
{
    if (value >= 1.0) return LAVAX_HALF_PI;
    if (value <= -1.0) return -LAVAX_HALF_PI;
    return atan(value / sqrt(1.0 - value * value));
}

double acos(double value) { return LAVAX_HALF_PI - asin(value); }

double exp(double value)
{
    int exponent = 0;
    double term = 1.0;
    double result = 1.0;
    int index;
    while (value > 0.6931471805599453) { value -= 0.6931471805599453; ++exponent; }
    while (value < -0.6931471805599453) { value += 0.6931471805599453; --exponent; }
    for (index = 1; index < 18; ++index) {
        term *= value / (double)index;
        result += term;
    }
    while (exponent > 0) { result *= 2.0; --exponent; }
    while (exponent < 0) { result *= 0.5; ++exponent; }
    return result;
}

double log(double value)
{
    int exponent = 0;
    int divisor;
    double y, squared, term, sum = 0.0;
    if (value <= 0.0) return -1.0e30;
    while (value >= 1.4142135623730951) { value *= 0.5; ++exponent; }
    while (value < 0.7071067811865476) { value *= 2.0; --exponent; }
    y = (value - 1.0) / (value + 1.0);
    squared = y * y;
    term = y;
    for (divisor = 1; divisor <= 21; divisor += 2) {
        sum += term / (double)divisor;
        term *= squared;
    }
    return 2.0 * sum + (double)exponent * 0.6931471805599453;
}
