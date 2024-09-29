#include <common.h>
#include "syscall.h"
#include <fs.h>
#include "proc.h"

Context *schedule(Context *prev);

int mm_brk(uintptr_t brk);

void destroy_pcb(PCB *pcb);

void context_uload(PCB *pcb, const char *filename, int argc, char *const argv[], int envc, char *const envp[]);

Context *do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;

    switch (a[0]) {
        case SYS_exit:
            current->running = false;
            Log("Exit with %d", a[1]);
            return schedule(c);
        case SYS_yield:
            return schedule(c);
        case SYS_open:
            c->GPRx = fs_open((const char *) a[1], a[2], a[3]);
            break;
        case SYS_read:
            c->GPRx = fs_read(a[1], (void *) a[2], a[3]);
            break;
        case SYS_write:
            c->GPRx = fs_write(a[1], (const void *) a[2], a[3]);
            break;
        case SYS_close:
            c->GPRx = fs_close(a[1]);
            break;
        case SYS_lseek:
            c->GPRx = fs_lseek(a[1], a[2], a[3]);
            break;
        case SYS_brk:
            c->GPRx = mm_brk(a[1]);
            break;
        case SYS_gettimeofday:
            uint64_t time;
            ioe_read(AM_TIMER_UPTIME, &time);
            struct timeval *tv = (struct timeval *) a[1];
            tv->tv_sec = time / 1000000;
            tv->tv_usec = time % 1000000;
            c->GPRx = 0;
            break;
        case SYS_fbdraw:
            ioe_write(AM_GPU_FBDRAW, (void *) a[1]);
            break;
        case SYS_execve:
            const char *fnP = (const char *) a[1];
            char *const *argvP = (char * const*) a[2];
            char *const *envpP = (char * const*) a[3];

            char *filename = malloc(strlen(fnP) + 1);
            strcpy(filename, fnP);
            int argc = 0;
            while (argvP[argc]) ++argc;
            int envc = 0;
            while (envpP[envc]) ++envc;
            char **argv = malloc(sizeof(char *) * (argc + 1));
            for (int i = 0; i < argc; ++i) {
                char *arg = argvP[i];
                argv[i] = malloc(strlen(arg) + 1);
                strcpy(argv[i], arg);
            }
            argv[argc] = NULL;
            char **envp = malloc(sizeof(char *) * (envc + 1));
            for (int i = 0; i < envc; ++i) {
                char *env = envpP[i];
                envp[i] = malloc(strlen(env) + 1);
                strcpy(envp[i], env);
            }
            envp[envc] = NULL;

            destroy_pcb(current);
            disable_virtual();

            context_uload(current, filename, argc, argv, envc, envp);

            free(filename);
            for (int i = 0; i < argc; ++i) free(argv[i]);
            free(argv);
            for (int i = 0; i < envc; ++i) free(envp[i]);
            free(envp);

            return current->cp;
        default: panic("Unhandled syscall ID = %d", a[0]);
    }

    return c;
}
