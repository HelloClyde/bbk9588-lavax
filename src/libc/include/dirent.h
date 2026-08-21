#ifndef LAVAX9588_DIRENT_H
#define LAVAX9588_DIRENT_H

typedef struct lavax9588_dir DIR;

struct dirent {
    char d_name[260];
};

DIR *opendir(const char *path);
struct dirent *readdir(DIR *directory);
int closedir(DIR *directory);

#endif
