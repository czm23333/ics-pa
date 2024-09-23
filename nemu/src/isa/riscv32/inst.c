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

#include <limits.h>

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <cpu/stacktrace.h>

#include "local-include/csr.h"

#define R(i) gpr(i)
#define C(i) csr(i)
#define Mr vaddr_read
#define Mw vaddr_write

#define DECODE_I() { rd = inst_dec.in_i.rd; rs1 = inst_dec.in_i.rs1; src1 = R(rs1); imm = (uint32_t) inst_dec.in_i.imm; }
#define DECODE_I2() { rd = inst_dec.in_i.rd; rs1 = inst_dec.in_i.rs1; src1 = rs1; imm = (uint32_t) inst_dec.in_i.imm; }
#define DECODE_U() { rd = inst_dec.in_u.rd; imm = inst_dec.in_u.imm << 12; }
#define DECODE_S() { rs1 = inst_dec.in_s.rs1; rs2 = inst_dec.in_s.rs2; src1 = R(rs1); src2 = R(rs2); imm = (((uint32_t) inst_dec.in_s.imm1) << 5) | inst_dec.in_s.imm0; }
#define DECODE_R() { rd = inst_dec.in_r.rd; rs1 = inst_dec.in_r.rs1; rs2 = inst_dec.in_r.rs2; src1 = R(rs1); src2 = R(rs2); }
#define DECODE_J() { rd = inst_dec.in_j.rd; imm = (((uint32_t) inst_dec.in_j.imm3) << 20) | (inst_dec.in_j.imm2 << 12) | (inst_dec.in_j.imm1 << 11) | (inst_dec.in_j.imm0 << 1); }
#define DECODE_B() { rs1 = inst_dec.in_b.rs1; rs2 = inst_dec.in_b.rs2; src1 = R(rs1); src2 = R(rs2); imm = (((uint32_t) inst_dec.in_b.imm3) << 12) | (inst_dec.in_b.imm2 << 11) | (inst_dec.in_b.imm1 << 5) | (inst_dec.in_b.imm0 << 1); }

static int decode_exec(Decode *s) {
    int rd = 0;
    int rs1, rs2 = 0;
    word_t src1 = 0, src2 = 0, imm = 0;
    s->dnpc = s->snpc;

    union {
        uint32_t val;

        struct {
            uint32_t d1: 7 /*0:6*/, d2: 5 /*7:11*/, d3: 3 /*12:14*/, d4: 5 /*15:19*/, d5: 5 /*20:24*/, d6: 7 /*25:31*/;
        };

        struct {
            uint32_t unused1: 7, rd: 5, unused2: 3, rs1: 5;
            int32_t imm: 12;
        } in_i;

        struct {
            uint32_t unused: 7, rd: 5, imm: 20;
        } in_u;

        struct {
            uint32_t unused1: 7, imm0: 5, unused2: 3, rs1: 5, rs2: 5;
            int32_t imm1: 7;
        } in_s;

        struct {
            uint32_t unused1: 7, rd: 5, unused2: 3, rs1: 5, rs2: 5, unused3: 7;
        } in_r;

        struct {
            uint32_t unused: 7, rd: 5, imm2: 8, imm1: 1, imm0: 10;
            int32_t imm3: 1;
        } in_j;

        struct {
            uint32_t unused1: 7, imm2: 1, imm0: 4, unused2: 3, rs1: 5, rs2: 5, imm1: 6;
            int32_t imm3: 1;
        } in_b;
    } inst_dec;
    inst_dec.val = s->isa.inst.val;

#define INST_INV() { INV(s->pc); break; }
#define INST_CHECK(index, v) if (inst_dec.d##index != v) { INV(s->pc); break; }
#define INST_RESOLVE(pattern, name, type, ... /* execute body */ ) { \
__VA_ARGS__ ; \
break; \
}

    switch (inst_dec.d1) {
        case 0b0010011:
            DECODE_I()
            switch (inst_dec.d3) {
                case 0b000: INST_RESOLVE("??????? ????? ????? 000 ????? 00100 11", addi, I, R(rd) = src1 + imm)
                case 0b010: INST_RESOLVE("??????? ????? ????? 010 ????? 00100 11", slti, I,
                                         R(rd) = (int32_t)src1 < (int32_t)imm)
                case 0b011: INST_RESOLVE("??????? ????? ????? 011 ????? 00100 11", sltiu, I, R(rd) = src1 < imm)
                case 0b100: INST_RESOLVE("??????? ????? ????? 100 ????? 00100 11", xori, I, R(rd) = src1 ^ imm)
                case 0b110: INST_RESOLVE("??????? ????? ????? 110 ????? 00100 11", ori, I, R(rd) = src1 | imm)
                case 0b111: INST_RESOLVE("??????? ????? ????? 111 ????? 00100 11", andi, I, R(rd) = src1 & imm)
                case 0b001: INST_CHECK(6, 0b0000000)
                    INST_RESOLVE("0000000 ????? ????? 001 ????? 00100 11", slli, I,
                                 R(rd) = src1 << (imm & 0x1Fu))
                case 0b101:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 101 ????? 00100 11", srli, I,
                                                     R(rd) = src1 >> (imm & 0x1Fu))
                        case 0b0100000: INST_RESOLVE("0100000 ????? ????? 101 ????? 00100 11", srai, I,
                                                     R(rd) = (uint32_t)((int32_t)src1 >> (imm & 0x1Fu)))
                        default: INST_INV()
                    }
                    break;
                default: INST_INV()
            }
            break;
        case 0b0010111:
            DECODE_U()
            INST_RESOLVE("??????? ????? ????? ??? ????? 00101 11", auipc, U, R(rd) = s->pc + imm)
        case 0b0110111:
            DECODE_U()
            INST_RESOLVE("??????? ????? ????? ??? ????? 01101 11", lui, U, R(rd) = imm);
        case 0b0110011:
            DECODE_R()
            switch (inst_dec.d3) {
                case 0b000:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 000 ????? 01100 11", add, R,
                                                     R(rd) = src1 + src2)
                        case 0b0100000: INST_RESOLVE("0100000 ????? ????? 000 ????? 01100 11", sub, R,
                                                     R(rd) = src1 - src2)
                        case 0b0000001: INST_RESOLVE("0000001 ????? ????? 000 ????? 01100 11", mul, R,
                                                     R(rd) = src1 * src2)
                        default: INST_INV()
                    }
                    break;
                case 0b001:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 001 ????? 01100 11", sll, R,
                                                     R(rd) = src1 << (src2 & 0x1Fu))
                        case 0b0000001: INST_RESOLVE("0000001 ????? ????? 001 ????? 01100 11", mulh, R,
                                                     R(rd) = (uint32_t)((uint64_t)((int64_t)(int32_t)src1 * (
                                                         int32_t)src2) >> 32))
                        default: INST_INV()
                    }
                    break;
                case 0b010:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 010 ????? 01100 11", slt, R,
                                                     R(rd) = (int32_t)src1 < (int32_t)src2)
                        case 0b0000001: INST_RESOLVE("0000001 ????? ????? 010 ????? 01100 11", mulhsu, R,
                                                     R(rd) = (uint32_t)((uint64_t)((int64_t)(int32_t)src1 * (
                                                         int64_t)src2) >> 32))
                        default: INST_INV()
                    }
                    break;
                case 0b011:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 011 ????? 01100 11", sltu, R,
                                                     R(rd) = src1 < src2)
                        case 0b0000001: INST_RESOLVE("0000001 ????? ????? 011 ????? 01100 11", mulhu, R,
                                                     R(rd) = (uint32_t)((uint64_t)src1 * src2 >> 32))
                        default: INST_INV()
                    }
                    break;
                case 0b100:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 100 ????? 01100 11", xor, R,
                                                     R(rd) = src1 ^ src2)
                        case 0b0000001: INST_RESOLVE("0000001 ????? ????? 100 ????? 01100 11", div, R,
                                                     if (src2 == 0) R(rd) = -1;
                                                     else if ((int32_t)src1 == INT32_MIN && (int32_t)src2 == -1)
                                                     R(rd) = INT32_MIN;
                                                     else R(rd) = (int32_t)src1 / (int32_t)src2)
                        default: INST_INV()
                    }
                    break;
                case 0b101:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 101 ????? 01100 11", srl, R,
                                                     R(rd) = src1 >> (src2 & 0x1Fu))
                        case 0b0100000: INST_RESOLVE("0100000 ????? ????? 101 ????? 01100 11", sra, R,
                                                     R(rd) = (uint32_t)((int32_t)src1 >> (src2 & 0x1Fu)))
                        case 0b0000001: INST_RESOLVE("0000001 ????? ????? 101 ????? 01100 11", divu, R,
                                                     if (src2 == 0) R(rd) = UINT32_MAX;
                                                     else R(rd) = src1 / src2);
                        default: INST_INV()
                    }
                    break;
                case 0b110:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 110 ????? 01100 11", or, R,
                                                     R(rd) = src1 | src2)
                        case 0b0000001: INST_RESOLVE("0000001 ????? ????? 110 ????? 01100 11", rem, R,
                                                     if (src2 == 0) R(rd) = src1;
                                                     else if ((int32_t)src1 == INT32_MIN && (int32_t)src2 == -1)
                                                     R(rd) = 0;
                                                     else R(rd) = (int32_t)src1 % (int32_t)src2);
                        default: INST_INV()
                    }
                    break;
                case 0b111:
                    switch (inst_dec.d6) {
                        case 0b0000000: INST_RESOLVE("0000000 ????? ????? 111 ????? 01100 11", and, R,
                                                     R(rd) = src1 & src2)
                        case 0b0000001: INST_RESOLVE("0000001 ????? ????? 111 ????? 01100 11", remu, R,
                                                     if (src2 == 0) R(rd) = src1;
                                                     else R(rd) = src1 % src2)
                        default: INST_INV()
                    }
                    break;
                default: INST_INV()
            }
            break;
        case 0b1101111:
            DECODE_J()
            INST_RESOLVE("??????? ????? ????? ??? ????? 11011 11", jal, J, R(rd) = s->snpc,
                         s->dnpc = s->pc + imm;
                         IFDEF(CONFIG_STACK_TRACE, if (rd == RA_INDEX) onCall(s->pc, s->dnpc)))
        case 0b1100111:
            INST_CHECK(3, 0b000)
            DECODE_I()
            INST_RESOLVE("??????? ????? ????? 000 ????? 11001 11", jalr, I, R(rd) = s->snpc,
                         s->dnpc = (src1 + imm) >> 1u << 1u;
                         IFDEF(CONFIG_STACK_TRACE, if (rs1 == RA_INDEX && imm == 0) onRet();
                             else if (rd == RA_INDEX) onCall(s->pc, s->dnpc)))
        case 0b1100011:
            DECODE_B()
            switch (inst_dec.d3) {
                case 0b000: INST_RESOLVE("??????? ????? ????? 000 ????? 11000 11", beq, B,
                                         if (src1 == src2) s->dnpc = s->pc + imm)
                case 0b001: INST_RESOLVE("??????? ????? ????? 001 ????? 11000 11", bne, B,
                                         if (src1 != src2) s->dnpc = s->pc + imm)
                case 0b100: INST_RESOLVE("??????? ????? ????? 100 ????? 11000 11", blt, B,
                                         if ((int32_t)src1 < (int32_t)src2) s->dnpc = s->pc + imm)
                case 0b101: INST_RESOLVE("??????? ????? ????? 101 ????? 11000 11", bge, B,
                                         if ((int32_t)src1 >= (int32_t)src2) s->dnpc = s->pc + imm)
                case 0b110: INST_RESOLVE("??????? ????? ????? 110 ????? 11000 11", bltu, B,
                                         if (src1 < src2) s->dnpc = s->pc + imm)
                case 0b111: INST_RESOLVE("??????? ????? ????? 111 ????? 11000 11", bgeu, B,
                                         if (src1 >= src2) s->dnpc = s->pc + imm)
                default: INST_INV()
            }
            break;
        case 0b0000011:
            DECODE_I()
            switch (inst_dec.d3) {
                case 0b000: INST_RESOLVE("??????? ????? ????? 000 ????? 00000 11", lb, I,
                                         R(rd) = SEXT(Mr(src1 + imm, 1), 8))
                case 0b100: INST_RESOLVE("??????? ????? ????? 100 ????? 00000 11", lbu, I,
                                         R(rd) = Mr(src1 + imm, 1))
                case 0b001: INST_RESOLVE("??????? ????? ????? 001 ????? 00000 11", lh, I,
                                         R(rd) = SEXT(Mr(src1 + imm, 2), 16))
                case 0b101: INST_RESOLVE("??????? ????? ????? 101 ????? 00000 11", lhu, I,
                                         R(rd) = Mr(src1 + imm, 2))
                case 0b010: INST_RESOLVE("??????? ????? ????? 010 ????? 00000 11", lw, I,
                                         R(rd) = Mr(src1 + imm, 4))
                default: INST_INV()
            }
            break;
        case 0b0100011:
            DECODE_S()
            switch (inst_dec.d3) {
                case 0b000: INST_RESOLVE("??????? ????? ????? 000 ????? 01000 11", sb, S,
                                         Mw(src1 + imm, 1, src2))
                case 0b001: INST_RESOLVE("??????? ????? ????? 001 ????? 01000 11", sh, S,
                                         Mw(src1 + imm, 2, src2))
                case 0b010: INST_RESOLVE("??????? ????? ????? 010 ????? 01000 11", sw, S,
                                         Mw(src1 + imm, 4, src2))
                default: INST_INV()
            }
            break;
        case 0b0001111: INST_RESOLVE("??????? ????? ????? ??? ????? 00011 11", fence, N,)
        case 0b1110011:
            switch (inst_dec.d3) {
                case 0b000:
                    INST_CHECK(2, 0b00000)
                    INST_CHECK(4, 0b00000)
                    switch (inst_dec.d5) {
                        case 0b00000:
                            INST_CHECK(6, 0b0000000)
                            INST_RESOLVE("0000000 00000 00000 000 00000 11100 11", ecall, N,
                                         s->dnpc = isa_raise_intr(R(A7_INDEX), s->pc))
                        case 0b00001:
                            INST_CHECK(6, 0b0000000)
                            INST_RESOLVE("0000000 00001 00000 000 00000 11100 11", ebreak, N,
                                         NEMUTRAP(s->pc, R(A0_INDEX)))
                        case 0b00010:
                            INST_CHECK(6, 0b0011000)
                            INST_RESOLVE("0011000 00010 00000 000 00000 11100 11", mret, N,
                                         s->dnpc = C(MEPC_INDEX))
                        default: INST_INV()
                    }
                    break;
                case 0b001:
                    DECODE_I()
                    INST_RESOLVE("??????? ????? ????? 001 ????? 11100 11", csrrw, I,
                                 if (rd != X0_INDEX) R(rd) = C(imm); C(imm) = src1)
                case 0b010:
                    DECODE_I()
                    INST_RESOLVE("??????? ????? ????? 010 ????? 11100 11", csrrs, I,
                                 if (rd != X0_INDEX) R(rd) = C(imm); if (rs1 != X0_INDEX) C(imm) |=
                                 src1)
                case 0b011:
                    DECODE_I()
                    INST_RESOLVE("??????? ????? ????? 011 ????? 11100 11", csrrc, I,
                                 if (rd != X0_INDEX) R(rd) = C(imm); if (rs1 != X0_INDEX) C(imm) &= ~
                                 src1)
                case 0b101:
                    DECODE_I2()
                    INST_RESOLVE("??????? ????? ????? 101 ????? 11100 11", csrrwi, I2,
                                 if (rd != X0_INDEX) R(rd) = C(imm); C(imm) = src1)
                case 0b110:
                    DECODE_I2()
                    INST_RESOLVE("??????? ????? ????? 110 ????? 11100 11", csrrsi, I2,
                                 if (rd != X0_INDEX) R(rd) = C(imm); C(imm) |= src1)
                case 0b111:
                    DECODE_I2()
                    INST_RESOLVE("??????? ????? ????? 111 ????? 11100 11", csrrci, I2,
                                 if (rd != X0_INDEX) R(rd) = C(imm); C(imm) &= ~src1)
                default: INST_INV()
            }
            break;
        default: INST_INV()
    }
    R(0) = 0; // reset $zero to 0

    return 0;
}

int isa_exec_once(Decode *s) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s);
}
