#include <proc.h>
#include <externc.h>
#include <mlist.h>

EXTERNC void context_uload(PCB *pcb, const char *filename, int argc, char* const argv[], int envc, char* const envp[]);

mlist<PCB> pcbs;
static PCB pcb_boot = {};
decltype(pcbs)::iterator currentIter = pcbs.end();
PCB *current = nullptr;

void switch_boot_pcb() {
    current = &pcb_boot;
    asm volatile("csrw mscratch, %0" : : "r" (current->stack + sizeof(current->stack)));
}

void load_program(const char *filename) {
    auto iter = pcbs.emplace_back();
    char* const argv[] = {nullptr};
    char* const envp[] = {nullptr};
    context_uload(&*iter, filename, 0, argv, 0, envp);
}

EXTERNC void destroy_pcb(PCB *pcb) {
    unprotect(&pcb->as);
    memset(pcb, 0, sizeof(PCB));
}

EXTERNC void init_proc() {
    switch_boot_pcb();

    Log("Initializing processes...");

    // load program here
    load_program("/bin/nterm");
}

EXTERNC Context *schedule(Context *prev) {
    if (current == &pcb_boot) {
        pcb_boot.cp = prev;
        if (pcbs.size() == 0) panic("No running process");
        currentIter = pcbs.begin();
        current = &*currentIter;
        return current->cp;
    }

    current->cp = prev;
    auto nxtIter = currentIter;
    ++nxtIter;
    if (!current->running) {
        disable_virtual();
        destroy_pcb(current);
        pcbs.erase(currentIter);
    }
    if (nxtIter != pcbs.end()) {
        currentIter = nxtIter;
        current = &*currentIter;
        return current->cp;
    }

    if (pcbs.size() == 0) panic("No running process");

    currentIter = pcbs.begin();
    current = &*currentIter;
    return current->cp;
}
