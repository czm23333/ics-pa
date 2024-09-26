#pragma once
#include <externc.h>

EXTERNC size_t ramdisk_read(void *buf, size_t offset, size_t len);

EXTERNC size_t ramdisk_write(const void *buf, size_t offset, size_t len);