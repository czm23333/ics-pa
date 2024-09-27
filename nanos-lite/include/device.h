#pragma once
#include <fs.h>
size_t serial_write(void *arg, FDInfo *fd, const void *buf, size_t len);
size_t events_read(void *arg, FDInfo *fd, void *buf, size_t len);