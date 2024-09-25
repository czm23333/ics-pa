#include <am.h>
#include <klib.h>

static Context * (*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c) {
    __am_get_cur_as(c);
    if (user_handler) {
        Event ev = {0};
        switch (c->mcause) {
            case 11: {
                c->mepc += 4;
                if (c->GPR1 == -1) ev.event = EVENT_YIELD;
                else ev.event = EVENT_SYSCALL;
                break;
            }
            default: ev.event = EVENT_ERROR;
                break;
        }

        c = user_handler(ev, c);
        assert(c != NULL);
    }

    __am_switch(c);
    return c;
}

extern void __am_asm_trap(void);

static char default_intr_stack[4096];

bool cte_init(Context *(*handler)(Event, Context *)) {
    // initialize exception entry
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
    asm volatile("csrw mscratch, %0" : : "r"(default_intr_stack + sizeof(default_intr_stack)));

    // register event handler
    user_handler = handler;

    return true;
}

Context *kcontext(Area kstack, Area kRunningStack, void (*entry)(void *), void *arg) {
    Context* ptr = kstack.end - sizeof(Context);
    memset(ptr, 0, sizeof(Context));
    ptr->mepc = (uintptr_t) entry;
    ptr->gpr[2] = (uintptr_t) kRunningStack.end; // sp
    ptr->gpr[10] = (uintptr_t) arg; // a0
    return ptr;
}

void yield() {
#ifdef __riscv_e
    asm volatile("li a5, -1; ecall");
#else
    asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
    return false;
}

void iset(bool enable) {
}
