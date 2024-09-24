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

#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>
#include "../local-include/csr.h"

int isa_mmu_check(vaddr_t vaddr, int len, int type) {
    SATPParts satp;
    satp.val = cpu.csr[SATP_INDEX];
    if (satp.MODE == 0) return MMU_DIRECT;
    return MMU_TRANSLATE;
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
    if (isa_mmu_check(vaddr, len, type) == MMU_DIRECT) return vaddr;

    SATPParts satp;
    satp.val = cpu.csr[SATP_INDEX];
    Sv32VAParts vaParts;
    vaParts.val = vaddr;
    Sv32PTE *pe1 = (Sv32PTE *) (satp.PPN * PAGE_SIZE + vaParts.pageNumber1 * sizeof(Sv32PTE));
    if (!pe1->priv.V) panic("page fault at " FMT_PADDR, vaddr);
    Sv32PTE *pe2 = (Sv32PTE *) (pe1->PPN * PAGE_SIZE + vaParts.pageNumber2 * sizeof(Sv32PTE));
    if (!pe2->priv.V) panic("page fault at " FMT_PADDR, vaddr);
    return pe2->PPN * PAGE_SIZE + vaParts.pageOffset;
}
