#include <fs.h>

struct FDInfo;

typedef size_t (*ReadFn)(void* arg, struct FDInfo* fd, void *buf, size_t len);
typedef size_t (*WriteFn)(void* arg, struct FDInfo* fd, const void *buf, size_t len);
typedef size_t (*SeekFn)(void* arg, struct FDInfo* fd, size_t offset);

typedef struct {
    char *name;
    size_t size;
    size_t disk_offset;
} Finfo;

typedef struct FDInfo {
    int fd;
    unsigned refcount;
    void* arg;
    size_t offset;
    ReadFn read;
    WriteFn write;
    SeekFn seek;
} FDInfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

size_t invalid_read(void *buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
#include "files.h"
};

void init_fs() {
    // TODO: initialize the size of /dev/fb
}
