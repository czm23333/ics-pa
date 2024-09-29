#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
    static char buf[256] = {};
    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(buf, 256, format, ap);
    va_end(ap);
    term->write(buf, len);
}

static void sh_banner() {
    sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
    sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
    auto cmd_buf = new char[strlen(cmd) + 1];
    strcpy(cmd_buf, cmd);
    char* fileName = strtok(cmd_buf, " ");
    if (fileName == nullptr) {
        delete[] cmd_buf;
        return;
    }
    auto args = new char*[1024];
    int argc = 0;
    while ((args[argc] = strtok(nullptr, " ")) != nullptr)
        ++argc;
    char* envp[] = {nullptr};
    execve(fileName, args, envp);

    delete[] cmd_buf;
    delete[] args;
}

void builtin_sh_run() {
    sh_banner();
    sh_prompt();

    while (1) {
        SDL_Event ev;
        if (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
                const char *res = term->keypress(handle_key(&ev));
                if (res) {
                    sh_handle_cmd(res);
                    sh_prompt();
                }
            }
        }
        refresh_terminal();
    }
}
