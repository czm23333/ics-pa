#pragma once
#include <fs.h>
size_t serial_write(void *arg, FDInfo *fd, const void *buf, size_t len);
size_t events_read(void *arg, FDInfo *fd, void *buf, size_t len);
size_t dispinfo_read(void *arg, FDInfo *fd, void *buf, size_t len);
size_t sbctl_read(void *arg, FDInfo *fd, void *buf, size_t len);
size_t sbctl_write(void *arg, FDInfo *fd, const void *buf, size_t len);
size_t sb_write(void *arg, FDInfo *fd, const void *buf, size_t len);
