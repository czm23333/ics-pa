#include <common.h>
#include "syscall.h"

#include "proc.h"

Context *schedule(Context *prev);

Context *do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;

    switch (a[0]) {
        case SYS_exit:
            current->running = false;
            return schedule(c);
        case SYS_yield:
            return schedule(c);
        default: panic("Unhandled syscall ID = %d", a[0]);
    }

    return c;
}
