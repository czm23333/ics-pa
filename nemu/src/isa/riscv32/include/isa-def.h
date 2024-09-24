/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

typedef struct {
    word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
    vaddr_t pc;
    word_t csr[4096];
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);

// decode
typedef struct {
    union {
        uint32_t val;
    } inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

// macuse
#define MCAUSE_ECALL 11
#define MCAUSE_LOAD_PAGE_FAULT 13
#define MCAUSE_STORE_PAGE_FAULT 15

// virtual memory
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

#define PAGESIZE 4096

#endif
