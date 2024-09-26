#include <common.h>

Context *schedule(Context *prev);

Context *do_syscall(Context *c);

static Context *do_event(Event e, Context *c) {
    switch (e.event) {
        case EVENT_YIELD:
        case EVENT_IRQ_TIMER:
            return schedule(c);
        case EVENT_SYSCALL:
            return do_syscall(c);
        default: panic("Unhandled event ID = %d", e.event);
    }

    return c;
}

void init_irq(void) {
    Log("Initializing interrupt/exception handler...");
    cte_init(do_event);
}
