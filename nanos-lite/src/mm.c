#include <memory.h>

#include "proc.h"

void *new_page(size_t nr_page) {
    return aligned_alloc(PGSIZE, PGSIZE * nr_page);
}

#ifdef HAS_VME
static void *pg_alloc(int n) {
    void *res = aligned_alloc(PGSIZE, n);
    memset(res, 0, n);
    return res;
}
#endif

void free_page(void *p) {
    free(p);
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
    uintptr_t newbrk = ROUNDUP(brk, PGSIZE);
    uintptr_t prev = current->brk;
    if (prev == newbrk) return 0;
    if (prev < newbrk) {
        Sv32Priv priv;
        priv.val = 0;
        priv.V = 1;
        while (prev < newbrk) {
            map(&current->as, (void *) prev, new_page(1), priv.val);
            prev += PGSIZE;
        }
    } else {
        Sv32Priv priv;
        priv.val = 0;
        prev -= PGSIZE;
        while (prev >= newbrk) {
            map(&current->as, (void *) prev, 0, priv.val);
            if (prev == 0) break;
            prev -= PGSIZE;
        }
    }
    current->brk = newbrk;
    return 0;
}

void init_mm() {
#ifdef HAS_VME
    vme_init(pg_alloc, free_page);
#endif
}
