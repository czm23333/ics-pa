#include <common.h>
#include "syscall.h"
#include <fs.h>
#include "proc.h"

Context *schedule(Context *prev);

Context *do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;

    Log("Syscall %d", a[0]);

    switch (a[0]) {
        case SYS_exit:
            current->running = false;
            Log("Exit with %d", a[1]);
            return schedule(c);
        case SYS_yield:
            return schedule(c);
        case SYS_write:
            c->GPRx = fs_write(a[1], (const void*) a[2], a[3]);
            break;
        default: panic("Unhandled syscall ID = %d", a[0]);
    }

    return c;
}
