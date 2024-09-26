#include <proc.h>

void context_uload(PCB *pcb, const char *filename);

#define MAX_NR_PROC 3

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
    current = &pcb_boot;
    asm volatile("csrw mscratch, %0" : : "r" (current->stack + sizeof(current->stack)));
}

void init_proc() {
    switch_boot_pcb();

    Log("Initializing processes...");

    // load program here
    context_uload(&pcb[0], "/bin/file-test");
    context_uload(&pcb[1], "/bin/dummy");
    context_uload(&pcb[2], "/bin/timer-test");
}

Context *schedule(Context *prev) {
    if (current == &pcb_boot) {
        pcb_boot.cp = prev;
        current = &pcb[0];
        return current->cp;
    }

    for (unsigned i = 0; i < MAX_NR_PROC; i++) {
        if (current == &pcb[i]) {
            current->cp = prev;
            for (unsigned j = (i + 1) % MAX_NR_PROC; j != i; j = (j + 1) % MAX_NR_PROC) {
                if (!pcb[j].running) continue;
                current = &pcb[j];
                return current->cp;
            }
            if (!pcb[i].running) panic("No running process");
            return prev;
        }
    }

    return prev;
}
