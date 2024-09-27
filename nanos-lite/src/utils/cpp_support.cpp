#include <stddef.h>
#include <common.h>
#include <memory>

#include "debug.h"

void *operator new(size_t size) {
    auto res = aligned_alloc(__STDCPP_DEFAULT_NEW_ALIGNMENT__, size);
    if (res == nullptr) panic("bad alloc");
    return res;
}

void *operator new(size_t size, std::align_val_t alignment) {
    auto res = aligned_alloc(static_cast<size_t>(alignment), size);
    if (res == nullptr) panic("bad alloc");
    return res;
}

void *operator new[](size_t size) {
    auto res = aligned_alloc(__STDCPP_DEFAULT_NEW_ALIGNMENT__, size);
    if (res == nullptr) panic("bad alloc");
    return res;
}

void *operator new[](size_t size, std::align_val_t alignment) {
    auto res = aligned_alloc(static_cast<size_t>(alignment), size);
    if (res == nullptr) panic("bad alloc");
    return res;
}

void operator delete(void *p) {
    free(p);
}

void operator delete(void *p, std::size_t, std::align_val_t) {
    free(p);
}

void operator delete[](void *p) {
    free(p);
}

void operator delete[](void *p, std::size_t, std::align_val_t) {
    free(p);
}

void operator delete(void *p, size_t) {
    free(p);
}

void operator delete[](void *p, size_t) {
    free(p);
}
