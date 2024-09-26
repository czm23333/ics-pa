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

void map_range_aligned(AddrSpace* space, uintptr_t begin, uintptr_t end, uint8_t priv) {
    while (begin < end) {
        map(space, (void *) begin, new_page(1), priv);
        begin += PGSIZE;
    }
}

void map_range(AddrSpace* space, uintptr_t begin, uintptr_t end, uint8_t priv) {
    map_range_aligned(space, ROUNDDOWN(begin, PGSIZE), ROUNDUP(end, PGSIZE), priv);
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
    uintptr_t newbrk = ROUNDUP(brk, PGSIZE);
    uintptr_t prev = current->brk;
    Log("mmbrk %d %d", prev, newbrk);
    if (prev == newbrk) return 0;
    if (prev < newbrk) {
        Sv32Priv priv;
        priv.val = 0;
        priv.V = 1;
        map_range_aligned(&current->as, prev, newbrk, priv.val);
    } else {
        Sv32Priv priv;
        priv.val = 0;
        map_range_aligned(&current->as, newbrk, prev, priv.val);
    }
    current->brk = newbrk;
    return 0;
}

void init_mm() {
#ifdef HAS_VME
    vme_init(pg_alloc, free_page);
#endif
}
