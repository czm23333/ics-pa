#ifndef __FS_H__
#define __FS_H__

#include <common.h>
#include <sys/types.h>
#include <externc.h>

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

EXTERNC int fs_open(const char *pathname, int flags, int mode);
EXTERNC size_t fs_read(int fd, void *buf, size_t len);
EXTERNC size_t fs_write(int fd, const void *buf, size_t len);
EXTERNC off_t fs_lseek(int fd, off_t offset, int whence);
EXTERNC off_t fs_tell(int fd);
EXTERNC int fs_close(int fd);

#ifdef __cplusplus
struct FDInfo;

typedef size_t (*ReadFn)(void *arg, FDInfo *fd, void *buf, size_t len);

typedef size_t (*WriteFn)(void *arg, FDInfo *fd, const void *buf, size_t len);

typedef off_t (*SeekFn)(void *arg, FDInfo *fd, off_t offset, int whence);

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

    bool operator==(int fd) const {
        return this->fd == fd;
    }
};
#endif

#endif
