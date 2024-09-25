#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

struct Context {
    union {
        struct {
            void* pdir;
            uintptr_t _registers[NR_REGS - 1];
        };
        uintptr_t gpr[NR_REGS];
    };
    uintptr_t mcause, mstatus, mepc;
};

typedef union MSTATUSParts {
    uint32_t val;

    struct {
        uint32_t : 3, MIE: 1, : 3, MPIE: 1,: 24;
    };
} MSTATUSParts;

typedef union SATPParts {
    uint32_t val;
    struct {
        uint32_t PPN : 22, ASID : 9, MODE : 1;
    };
} SATPParts;

typedef union Sv32Priv {
    struct {
        bool V: 1, R: 1, W: 1, X: 1, U: 1, G: 1, A: 1, D: 1;
    };

    uint8_t val;
} Sv32Priv;

typedef union Sv32VAParts {
    struct {
        uint32_t pageOffset: 12, pageNumber2: 10, pageNumber1: 10;
    };

    uint32_t val;
} Sv32VAParts;

typedef union Sv32PTE {
    uint32_t val;

    struct {
        uint32_t priv_v: 8;
        uint32_t RSW: 2;
        uint32_t PPN: 22;
    };

    Sv32Priv priv;
} Sv32PTE;

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7
#endif

#define GPR2 gpr[10] // a0
#define GPR3 gpr[11] // a1
#define GPR4 gpr[12] // a2
#define GPRx gpr[10] // a0

#endif
