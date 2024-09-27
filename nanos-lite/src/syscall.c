#include <common.h>
#include "syscall.h"
#include <fs.h>
#include "proc.h"

Context *schedule(Context *prev);
int mm_brk(uintptr_t brk);

Context *do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;

    switch (a[0]) {
        case SYS_exit:
            current->running = false;
            Log("Exit with %d", a[1]);
            return schedule(c);
        case SYS_yield:
            return schedule(c);
        case SYS_open:
            c->GPRx = fs_open((const char*) a[1], a[2], a[3]);
            break;
        case SYS_read:
            c->GPRx = fs_read(a[1], (void*) a[2], a[3]);
            break;
        case SYS_write:
            c->GPRx = fs_write(a[1], (const void*) a[2], a[3]);
            break;
        case SYS_close:
            c->GPRx = fs_close(a[1]);
            break;
        case SYS_lseek:
            c->GPRx = fs_lseek(a[1], a[2], a[3]);
            break;
        case SYS_brk:
            c->GPRx = mm_brk(a[1]);
            break;
        case SYS_gettimeofday:
            uint64_t time;
            ioe_read(AM_TIMER_UPTIME, &time);
            struct timeval* tv = (struct timeval*)a[1];
            tv->tv_sec = time / 1000000;
            tv->tv_usec = time % 1000000;
            c->GPRx = 0;
            break;
        case SYS_fbdraw:
            ioe_write(AM_GPU_FBDRAW, (void*) a[1]);
            break;
        default: panic("Unhandled syscall ID = %d", a[0]);
    }

    return c;
}
