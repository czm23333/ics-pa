#include <algorithm>
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
    as_guard(AddrSpace *as) {
        as_enter(as);
    }

    ~as_guard() {
        disable_virtual();
    }
};

struct fd_guard {
    int fd;

    fd_guard(int fd) : fd(fd) {
    }

    ~fd_guard() {
        fs_close(fd);
    }

    operator int() {
        return fd;
    }
};

EXTERNC void context_uload(PCB *pcb, const char *filename) {
    protect(&pcb->as);
    as_guard guard(&pcb->as);

    fd_guard fd = fs_open(filename, 0, 0);
    if (fd < 0)
        panic("file not found");

    Sv32Priv priv;
    priv.val = 0;
    priv.V = true;

    Elf_Ehdr ehdr;
    fs_read(fd, &ehdr, sizeof(ehdr));

    pcb->brk = 0;

    fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
    for (unsigned i = 0; i < ehdr.e_phnum; i++) {
        Elf_Phdr phdr;
        fs_read(fd, &phdr, sizeof(phdr));
        if (phdr.p_type == PT_LOAD) {
            auto off = fs_tell(fd);
            fs_lseek(fd, phdr.p_offset, SEEK_SET);

            pcb->brk = std::max(pcb->brk, phdr.p_vaddr + phdr.p_memsz);
            map_range(&pcb->as, phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz, priv.val);
            memset(reinterpret_cast<void *>(phdr.p_vaddr), 0, phdr.p_memsz);
            fs_read(fd, reinterpret_cast<void *>(phdr.p_vaddr), phdr.p_filesz);

            fs_lseek(fd, off, SEEK_SET);
        }
    }

    pcb->brk = ROUNDUP(pcb->brk, PGSIZE);

    constexpr uintptr_t stackTop = 0xE0000000;
    constexpr unsigned stackSize = 8 * 1024;
    map_range(&pcb->as, stackTop - stackSize, stackTop, priv.val);

    pcb->cp = ucontext(&pcb->as, Area{pcb->stack, pcb->stack + sizeof(pcb->stack)}, stackTop,
                       reinterpret_cast<void *>(ehdr.e_entry));

    pcb->running = true;
}
