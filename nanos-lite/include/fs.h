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
EXTERNC int fs_close(int fd);

#endif
