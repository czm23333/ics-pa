#include <stddef.h>
#include <common.h>

#include "debug.h"

void *operator new(size_t size) {
    auto res = aligned_alloc(__STDCPP_DEFAULT_NEW_ALIGNMENT__, size);
    if (res == nullptr) panic("bad alloc");
    return res;
}

void *operator new[](size_t size) {
    auto res = aligned_alloc(__STDCPP_DEFAULT_NEW_ALIGNMENT__, size);
    if (res == nullptr) panic("bad alloc");
    return res;
}

void operator delete(void *p) {
    free(p);
}

void operator delete[](void *p) {
    free(p);
}

void operator delete(void *p, size_t) {
    free(p);
}

void operator delete[](void *p, size_t) {
    free(p);
}

void *operator new(size_t, void *p) {
    return p;
}

void *operator new[](size_t, void *p) {
    return p;
}

void operator delete(void *, void *) {
}

void operator delete[](void *, void *){
}
