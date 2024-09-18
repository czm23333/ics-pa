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

#define MTVEC_INDEX 0x305
#define MEPC_INDEX 0x341
#define MCAUSE_INDEX 0x342

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
    /* Trigger an interrupt/exception with ``NO''.
     * Then return the address of the interrupt/exception vector.
     */
    cpu.csr[MCAUSE_INDEX] = NO;
    cpu.csr[MEPC_INDEX] = epc;

    return cpu.csr[MTVEC_INDEX];
}

word_t isa_query_intr() {
    return INTR_EMPTY;
}
