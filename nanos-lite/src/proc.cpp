#include <proc.h>
#include <externc.h>
#include <mlist.h>

EXTERNC void context_uload(PCB *pcb, const char *filename);

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
    context_uload(&*iter, filename);
}

EXTERNC void init_proc() {
    switch_boot_pcb();

    Log("Initializing processes...");

    // load program here
    load_program("/bin/file-test");
    load_program("/bin/dummy");
    load_program("bin/nslider");
    load_program("/bin/timer-test");
    load_program("/bin/event-test");
    load_program("/bin/cpp-test");
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
    if (!current->running) pcbs.erase(currentIter);
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
