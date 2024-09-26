#include <fs.h>
#include <mlist.h>

#include <externc.h>

struct FDInfo;

typedef size_t (*ReadFn)(void *arg, FDInfo *fd, void *buf, size_t len);

typedef size_t (*WriteFn)(void *arg, FDInfo *fd, const void *buf, size_t len);

typedef void (*SeekFn)(void *arg, FDInfo *fd, size_t offset);

struct Finfo {
    const char *name;
    size_t size;
    size_t disk_offset;
};

struct FDInfo {
    int fd;
    unsigned refcount;
    void *arg;
    size_t offset;
    ReadFn read;
    WriteFn write;
    SeekFn seek;

    FDInfo(int fd, unsigned refcount, void *arg, size_t offset, ReadFn read, WriteFn write,
           SeekFn seek) : fd(fd), refcount(refcount),
                          arg(arg), offset(offset), read(read), write(write), seek(seek) {
    }
};

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

mlist<FDInfo> fdList;

size_t invalid_read(void *arg, FDInfo *fd, void *buf, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t invalid_write(void *arg, FDInfo *fd, const void *buf, size_t len) {
    panic("should not reach here");
    return 0;
}

void invalid_seek(void *arg, FDInfo *fd, size_t offset) {
    panic("should not reach here");
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
#include "files.h"
};

EXTERNC void init_fs() {
    fdList.emplace_back(FD_STDIN, 0xFFFFFFu, nullptr, 0, invalid_read, invalid_write, invalid_seek);
    fdList.emplace_back(FD_STDOUT, 0xFFFFFFu, nullptr, 0, invalid_read, invalid_write, invalid_seek);
    fdList.emplace_back(FD_STDERR, 0xFFFFFFu, nullptr, 0, invalid_read, invalid_write, invalid_seek);
    fdList.emplace_back(FD_FB, 0xFFFFFFu, nullptr, 0, invalid_read, invalid_write, invalid_seek);
}
