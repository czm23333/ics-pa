#include <fs.h>
#include <mlist.h>
#include <algorithm>
#include <cstdio>
#include <ramdisk.h>
#include <klib.h>
#include <device.h>

#include <externc.h>

struct SPFInfo {
    const char *name;
    ReadFn read;
    WriteFn write;
    SeekFn seek;

    bool operator==(const char *s) const {
        return strcmp(name, s) == 0;
    }
};

struct Finfo {
    const char *name;
    size_t size;
    size_t disk_offset;

    bool operator==(const char *s) const {
        return strcmp(name, s) == 0;
    }
};

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

int fdId = 256;
mlist<FDInfo> fdList;

size_t invalid_read(void *arg, FDInfo *fd, void *buf, size_t len) {
    return 0;
}

size_t invalid_write(void *arg, FDInfo *fd, const void *buf, size_t len) {
    return 0;
}

off_t invalid_seek(void *arg, FDInfo *fd, off_t offset, int whence) {
    return -1;
}

size_t file_read(void *arg, FDInfo *fd, void *buf, size_t len) {
    auto finfo = static_cast<Finfo *>(arg);
    size_t off = fd->offset;
    size_t left = finfo->size - off;
    len = std::min(left, len);
    off += finfo->disk_offset;
    ramdisk_read(buf, off, len);
    fd->offset += len;
    return len;
}

size_t file_write(void *arg, FDInfo *fd, const void *buf, size_t len) {
    auto finfo = static_cast<Finfo *>(arg);
    size_t off = fd->offset;
    size_t left = finfo->size - off;
    len = std::min(left, len);
    off += finfo->disk_offset;
    ramdisk_write(buf, off, len);
    fd->offset += len;
    return len;
}

off_t file_seek(void *arg, FDInfo *fd, off_t offset, int whence) {
    auto finfo = static_cast<Finfo *>(arg);
    switch (whence) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            offset = fd->offset + offset;
            break;
        case SEEK_END:
            offset = finfo->size + offset;
            break;
        default:
            return -1;
    }
    if (offset < 0 || static_cast<size_t>(offset) > finfo->size) return -1;
    return fd->offset = offset;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
#include "files.h"
};

static SPFInfo special_file_table[] = {
    {"/dev/events", events_read, invalid_write, invalid_seek}
};

FDInfo *findFD(int fd) {
    auto it = std::find(fdList.begin(), fdList.end(), fd);
    if (it == fdList.end()) return nullptr;
    return &*it;
}

int fs_open(const char *pathname, int flags, int mode) {
    for (auto& spf : special_file_table) {
        if (spf == pathname) {
            fdList.emplace_back(fdId, 1, nullptr, 0, spf.read, spf.write, spf.seek);
            return fdId++;
        }
    }

    for (auto &file: file_table) {
        if (file == pathname) {
            fdList.emplace_back(fdId, 1, &file, 0, file_read, file_write, file_seek);
            return fdId++;
        }
    }
    return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
    FDInfo *fdi = findFD(fd);
    if (fdi == nullptr) return 0;
    return fdi->read(fdi->arg, fdi, buf, len);
}

size_t fs_write(int fd, const void *buf, size_t len) {
    FDInfo *fdi = findFD(fd);
    if (fdi == nullptr) return 0;
    return fdi->write(fdi->arg, fdi, buf, len);
}

off_t fs_lseek(int fd, off_t offset, int whence) {
    FDInfo *fdi = findFD(fd);
    if (fdi == nullptr) return -1;
    return fdi->seek(fdi->arg, fdi, offset, whence);
}

off_t fs_tell(int fd) {
    FDInfo *fdi = findFD(fd);
    if (fdi == nullptr) return -1;
    return fdi->offset;
}

int fs_close(int fd) {
    auto it = std::find(fdList.begin(), fdList.end(), fd);
    if (it == fdList.end()) return -1;
    --it->refcount;
    if (it->refcount == 0) fdList.erase(it);
    return 0;
}

EXTERNC void init_fs() {
    fdList.emplace_back(FD_STDIN, 0xFFFFFFu, nullptr, 0, invalid_read, invalid_write, invalid_seek);
    fdList.emplace_back(FD_STDOUT, 0xFFFFFFu, nullptr, 0, invalid_read, serial_write, invalid_seek);
    fdList.emplace_back(FD_STDERR, 0xFFFFFFu, nullptr, 0, invalid_read, serial_write, invalid_seek);
    fdList.emplace_back(FD_FB, 0xFFFFFFu, nullptr, 0, invalid_read, invalid_write, invalid_seek);
}
