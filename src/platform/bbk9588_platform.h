#ifndef LAVAX9588_PLATFORM_H
#define LAVAX9588_PLATFORM_H

int bbk9588_platform_open(void);
void bbk9588_platform_close(void);
void bbk9588_platform_tick(void);
void bbk9588_platform_idle(void);
int bbk9588_platform_should_exit(void);
int bbk9588_platform_key_state(unsigned code);
void bbk9588_platform_touch(int *pushed, int *x, int *y);
void bbk9588_platform_present(
    const unsigned char *indices, int width, int height,
    const unsigned char *palette_bgr
);
void bbk9588_platform_status(const char *line1, const char *line2);
const char *lavax9588_runtime_root(void);
int lavax9588_runtime_available(void);
int lavax9588_normalize_path(const char *source, char *target, unsigned size);

#endif
