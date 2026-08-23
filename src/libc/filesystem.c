#include "bda_filesystem.h"
#include "bbk9588_platform.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define LAVAX_PATH_MAX 260u
#define LAVAX_FIND_FILE_ATTRIBUTES 0x27u
/* Include the FAT directory bit (0x10) when discovering Shell entries. */
#define LAVAX_FIND_ENTRY_ATTRIBUTES 0x37u

struct j2me9588_file {
    int descriptor;
    int error;
    int eof;
    int standard;
};

struct lavax9588_dir {
    bda_fs_find_data_t find;
    struct dirent entry;
    int first_pending;
    int valid;
};

static char g_runtime_root[16];

static int path_exists_raw(const char *path)
{
    bda_fs_find_data_t find;
    int result;
    bda_fs_find_data_init(&find);
    result = bda_fs_findfirst(path, LAVAX_FIND_FILE_ATTRIBUTES, &find);
    if (result == -1) return 0;
    (void)bda_fs_findclose(&find);
    return 1;
}

const char *lavax9588_runtime_root(void)
{
    if (!g_runtime_root[0]) {
        const char *root = path_exists_raw("B:\\LavaXOS\\System\\Shell.sys")
            ? "B:\\LavaXOS" : "A:\\LavaXOS";
        (void)strlcpy(g_runtime_root, root, sizeof(g_runtime_root));
    }
    return g_runtime_root;
}

int lavax9588_runtime_available(void)
{
    char path[LAVAX_PATH_MAX];
    (void)snprintf(
        path, sizeof(path), "%s\\System\\Shell.sys",
        lavax9588_runtime_root()
    );
    return path_exists_raw(path);
}

int lavax9588_normalize_path(
    const char *source, char *target, unsigned size
)
{
    static const char prefix[] = "fat:/LavaXOS";
    const char *tail = source;
    unsigned used = 0u;
    if (!source || !target || size == 0u) return 0;
    if (strncmp(source, prefix, sizeof(prefix) - 1u) == 0) {
        const char *root = lavax9588_runtime_root();
        while (*root && used + 1u < size) target[used++] = *root++;
        tail = source + sizeof(prefix) - 1u;
    }
    while (*tail && used + 1u < size) {
        char character = *tail++;
        target[used++] = character == '/' ? '\\' : character;
    }
    target[used] = 0;
    return *tail == 0;
}

FILE *fopen(const char *path, const char *mode)
{
    char normalized[LAVAX_PATH_MAX];
    FILE *stream;
    int descriptor;
    if (!lavax9588_normalize_path(path, normalized, sizeof(normalized))) {
        return 0;
    }
    descriptor = bda_fs_fopen_raw(normalized, mode);
    if (!bda_fs_file_is_valid(descriptor)) return 0;
    stream = (FILE *)malloc(sizeof(*stream));
    if (!stream) {
        (void)bda_fs_close_raw(descriptor);
        return 0;
    }
    stream->descriptor = descriptor;
    stream->error = 0;
    stream->eof = 0;
    stream->standard = 0;
    return stream;
}

int fclose(FILE *stream)
{
    int result;
    if (!stream || stream->standard) return EOF;
    result = bda_fs_close_raw(stream->descriptor);
    free(stream);
    return result;
}

size_t fread(void *buffer, size_t size, size_t count, FILE *stream)
{
    int result;
    if (!stream || stream->standard || size == 0u || count == 0u) return 0u;
    result = bda_fs_fread_raw(buffer, size, count, stream->descriptor);
    if (result < 0) {
        stream->error = 1;
        return 0u;
    }
    if ((size_t)result < count) stream->eof = 1;
    return (size_t)result;
}

size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream)
{
    int result;
    if (!stream || stream->standard || size == 0u || count == 0u) return 0u;
    result = bda_fs_fwrite_raw(buffer, size, count, stream->descriptor);
    if (result < 0) {
        stream->error = 1;
        return 0u;
    }
    if ((size_t)result < count) stream->error = 1;
    return (size_t)result;
}

int fseek(FILE *stream, long offset, int origin)
{
    int result;
    if (!stream || stream->standard) return -1;
    result = bda_fs_seek_raw(stream->descriptor, (s32)offset, origin);
    if (result < 0) {
        stream->error = 1;
        return -1;
    }
    stream->eof = 0;
    return 0;
}

long ftell(FILE *stream)
{
    int result;
    if (!stream || stream->standard) return -1;
    result = bda_fs_tell_raw(stream->descriptor);
    if (result < 0) stream->error = 1;
    return (long)result;
}

int feof(FILE *stream) { return stream ? stream->eof : 0; }
int ferror(FILE *stream) { return stream ? stream->error : 0; }

int fgetc(FILE *stream)
{
    unsigned char value;
    return fread(&value, 1u, 1u, stream) == 1u ? value : EOF;
}

int fputc(int character, FILE *stream)
{
    unsigned char value = (unsigned char)character;
    return fwrite(&value, 1u, 1u, stream) == 1u ? value : EOF;
}

void rewind(FILE *stream) { (void)fseek(stream, 0, SEEK_SET); }

int remove(const char *path)
{
    (void)path;
    /* The verified public 9588 SDK does not expose file deletion. */
    return -1;
}

int mkdir(const char *path, int mode)
{
    char normalized[LAVAX_PATH_MAX];
    (void)mode;
    if (!lavax9588_normalize_path(path, normalized, sizeof(normalized))) {
        return -1;
    }
    return bda_fs_mkdir(normalized);
}

int stat(const char *path, struct stat *status)
{
    char normalized[LAVAX_PATH_MAX];
    bda_fs_find_data_t find;
    int result;
    if (!status ||
        !lavax9588_normalize_path(path, normalized, sizeof(normalized))) {
        return -1;
    }
    bda_fs_find_data_init(&find);
    result = bda_fs_findfirst(
        normalized, LAVAX_FIND_FILE_ATTRIBUTES, &find
    );
    if (result != -1) {
        status->st_mode = S_IFREG;
        status->st_size = find.size_or_aux;
        (void)bda_fs_findclose(&find);
        return 0;
    }
    bda_fs_find_data_init(&find);
    result = bda_fs_findfirst(
        normalized, LAVAX_FIND_ENTRY_ATTRIBUTES, &find
    );
    if (result == -1) return -1;
    status->st_mode = S_IFDIR;
    status->st_size = 0u;
    (void)bda_fs_findclose(&find);
    return 0;
}

static const char *path_leaf(const char *path)
{
    const char *leaf = path;
    while (*path) {
        if (*path == '\\' || *path == '/') leaf = path + 1;
        ++path;
    }
    return leaf;
}

DIR *opendir(const char *path)
{
    char normalized[LAVAX_PATH_MAX];
    char pattern[LAVAX_PATH_MAX];
    DIR *directory;
    unsigned length;
    if (!lavax9588_normalize_path(path, normalized, sizeof(normalized))) {
        return 0;
    }
    length = (unsigned)strlen(normalized);
    if (length + 3u > sizeof(pattern)) return 0;
    (void)strlcpy(pattern, normalized, sizeof(pattern));
    if (length && pattern[length - 1u] != '\\') pattern[length++] = '\\';
    pattern[length++] = '*';
    pattern[length] = 0;
    directory = (DIR *)malloc(sizeof(*directory));
    if (!directory) return 0;
    memset(directory, 0, sizeof(*directory));
    bda_fs_find_data_init(&directory->find);
    if (bda_fs_findfirst(
            pattern, LAVAX_FIND_ENTRY_ATTRIBUTES, &directory->find
        ) == -1) {
        free(directory);
        return 0;
    }
    directory->first_pending = 1;
    directory->valid = 1;
    return directory;
}

struct dirent *readdir(DIR *directory)
{
    const char *leaf;
    if (!directory || !directory->valid) return 0;
    if (directory->first_pending) {
        directory->first_pending = 0;
    } else if (bda_fs_findnext(&directory->find) == -1) {
        return 0;
    }
    leaf = path_leaf(directory->find.name_or_path);
    (void)strlcpy(directory->entry.d_name, leaf, sizeof(directory->entry.d_name));
    return &directory->entry;
}

int closedir(DIR *directory)
{
    int result;
    if (!directory) return -1;
    result = directory->valid ? bda_fs_findclose(&directory->find) : 0;
    free(directory);
    return result;
}
