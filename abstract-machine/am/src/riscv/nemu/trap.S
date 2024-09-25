#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS_LO16(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15)
#ifndef __riscv_e
#define REGS_HI16(f) \
                                    f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)
#define NR_REGS 32
#else
#define REGS_HI16(f)
#define NR_REGS 16
#endif

#define REGS(f) REGS_LO16(f) REGS_HI16(f)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((NR_REGS + 3) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  ((NR_REGS + 0) * XLEN)
#define OFFSET_STATUS ((NR_REGS + 1) * XLEN)
#define OFFSET_EPC    ((NR_REGS + 2) * XLEN)

#define EN_INTR csrsi mstatus, 8
#define DIS_INTR csrci mstatus, 8

#define NESTED_INTR_LIMIT 4
#define INTR_RUNNING_STACK_SIZE_LOG2 12
#define INTR_RUNNING_STACK_SIZE 4096
nest_cnt: .word 0
intr_running_stacks: .zero INTR_RUNNING_STACK_SIZE * NESTED_INTR_LIMIT
intr_running_stacks_end:

.align 3
.globl __am_asm_trap
__am_asm_trap:
  csrrw sp, mscratch, sp
  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)
  # save previous sp
  csrr a0, mscratch
  STORE a0, (2 * XLEN)(sp)
  # set new intr sp
  csrw mscratch, sp

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)

  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp

  # increase nest level, set intr running stack
  la t2, nest_cnt
  LOAD t0, 0(t2)
  slli t1, t0, INTR_RUNNING_STACK_SIZE_LOG2
  addi t0, t0, 1
  STORE t0, 0(t2)
  la t2, intr_running_stacks_end
  sub sp, t2, t1

  li a1, 0
  # set nest_enabled if we should
  # check MPIE
  csrr t2, mstatus
  andi t2, t2, 0x80
  beqz t2, no_intr
  # check nest limit
  li t2, NESTED_INTR_LIMIT
  bge t0, t2, no_intr
  li a1, 1

no_intr:
  call __am_irq_handle
  DIS_INTR
  mv sp, a0

  # decrease nest level
  la t1, nest_cnt
  LOAD t0, 0(t1)
  addi t0, t0, -1
  STORE t0, 0(t1)

  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw mstatus, t1
  csrw mepc, t2

  # load new sp
  LOAD a0, (2 * XLEN)(sp)
  csrw mscratch, a0

  MAP(REGS, POP)

  addi sp, sp, CONTEXT_SIZE

  # set new sp, save intr sp
  csrrw sp, mscratch, sp
  mret
