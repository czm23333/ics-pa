#include <proc.h>
#include <elf.h>
#include <mm.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

struct as_guard {
    as_guard(AddrSpace* as) {
        as_enter(as);
    }

    ~as_guard() {
        disable_virtual();
    }
};

void context_uload(PCB *pcb, const char *filename) {
    protect(&pcb->as);
    as_guard guard(&pcb->as);

    fs_open(filename, 0, 0);

}
