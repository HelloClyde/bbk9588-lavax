#ifndef J2ME9588_TIME_H
#define J2ME9588_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

typedef long time_t;
typedef long clock_t;

#define CLOCKS_PER_SEC 1000L

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

time_t time(time_t *result);
clock_t clock(void);

#ifdef __cplusplus
}
#endif

#endif
