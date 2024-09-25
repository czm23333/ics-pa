#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};

static void * (*pgalloc_usr)(int) = NULL;

static void (*pgfree_usr)(void *) = NULL;

static int vme_enable = 0;

static Area segments[] = {
    // Kernel memory mappings
    NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static void set_satp(void *pdir) {
    uintptr_t mode = 1ul << (__riscv_xlen - 1);
    asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t) pdir >> 12)));
}

static uintptr_t get_satp() {
    uintptr_t satp;
    asm volatile("csrr %0, satp" : "=r"(satp));
    return satp << 12;
}

void enable_virtual() {
    uintptr_t mode = 1ul << (__riscv_xlen - 1);
    asm volatile("csrs satp, %0" : : "r"(mode));
}

void disable_virtual() {
    uintptr_t mode = 1ul << (__riscv_xlen - 1);
    asm volatile("csrc satp, %0" : : "r"(mode));
}

bool vme_init(void * (*pgalloc_f)(int), void (*pgfree_f)(void *)) {
    pgalloc_usr = pgalloc_f;
    pgfree_usr = pgfree_f;

    kas.ptr = pgalloc_f(PGSIZE);

    Sv32Priv sysPriv;
    sysPriv.val = 0;
    sysPriv.V = true, sysPriv.G = true;

    int i;
    for (i = 0; i < LENGTH(segments); i++) {
        void *va = segments[i].start;
        for (; va < segments[i].end; va += PGSIZE) {
            map(&kas, va, va, sysPriv.val);
        }
    }

    set_satp(kas.ptr);
    vme_enable = 1;

    return true;
}

static void deep_copy(Sv32PTE *dst, Sv32PTE *src) {
    for (size_t i = 0; i < PGSIZE / sizeof(Sv32PTE); ++i) {
        Sv32Priv priv = src[i].priv;
        if (priv.V) {
            Sv32PTE *pageTable2 = (Sv32PTE *) (src[i].PPN * PGSIZE);
            Sv32PTE *newPageTable2 = pgalloc_usr(PGSIZE);
            memcpy(newPageTable2, pageTable2, PGSIZE);
            dst[i] = src[i];
            dst[i].PPN = (uintptr_t) newPageTable2 / PGSIZE;
        }
    }
}

void protect(AddrSpace *as) {
    Sv32PTE *updir = pgalloc_usr(PGSIZE);
    as->ptr = updir;
    as->area = USER_SPACE;
    as->pgsize = PGSIZE;
    deep_copy(updir, kas.ptr);
}

void unprotect(AddrSpace *as) {
    Sv32PTE *pageTable = as->ptr;
    for (size_t i = 0; i < PGSIZE / sizeof(Sv32PTE); ++i) {
        Sv32Priv priv = pageTable[i].priv;
        if (priv.V) {
            Sv32PTE *pageTable2 = (Sv32PTE *) (pageTable[i].PPN * PGSIZE);
            for (size_t j = 0; j < PGSIZE / sizeof(Sv32PTE); ++j) {
                Sv32PTE page = pageTable2[j];
                if (!page.priv.V) continue;
                if (page.priv.G) continue;
                pgfree_usr((void *) (page.PPN * PGSIZE));
            }
            pgfree_usr(pageTable2);
        }
    }
    pgfree_usr(pageTable);
}

void __am_get_cur_as(Context *c) {
    c->pdir = (vme_enable ? (void *) get_satp() : NULL);
}

void __am_switch(Context *c) {
    if (vme_enable && c->pdir != NULL) {
        set_satp(c->pdir);
    }
}

void map(AddrSpace *as, void *va, void *pa, uint32_t prot) {
    Sv32Priv protParts;
    protParts.val = prot;

    Sv32VAParts vaParts;
    vaParts.val = (uintptr_t) va;

    Sv32PTE *pTop = as->ptr + vaParts.pageNumber1 * sizeof(Sv32PTE);
    if (!pTop->priv.V) {
        if (!protParts.V) return;
        pTop->priv.V = true;
        pTop->PPN = (uintptr_t) pgalloc_usr(PGSIZE) / PGSIZE;
    }

    Sv32PTE *pSec = (Sv32PTE *) (pTop->PPN * PGSIZE + vaParts.pageNumber2 * sizeof(Sv32PTE));
    if (pSec->priv.V && !protParts.V) pgfree_usr((void *) (pSec->PPN * PGSIZE));
    pSec->priv.val = protParts.val;
    pSec->PPN = (uintptr_t) pa / PGSIZE;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
    Context* ptr = kstack.end - sizeof(Context);
    memset(ptr, 0, sizeof(Context));
    ptr->mepc = (uintptr_t) entry;
    ptr->gpr[2] = (uintptr_t) kstack.end; // sp
    ptr->pdir = as->ptr;

    // Enable intr
    MSTATUSParts mstatus;
    mstatus.val = ptr->mstatus;
    mstatus.MPIE = 1;
    ptr->mstatus = mstatus.val;
    return ptr;
}
