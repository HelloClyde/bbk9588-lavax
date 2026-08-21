#ifndef LAVAX9588_SYS_STAT_H
#define LAVAX9588_SYS_STAT_H

#define S_IFDIR 0x4000
#define S_IFREG 0x8000

struct stat {
    unsigned long st_mode;
    unsigned long st_size;
};

int stat(const char *path, struct stat *status);
int mkdir(const char *path, int mode);

#endif
