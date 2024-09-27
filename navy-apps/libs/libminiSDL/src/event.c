#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
    "NONE",
    _KEYS(keyname)
};

void SDL_try_callback();

int SDL_PushEvent(SDL_Event *ev) {
    return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
    SDL_try_callback();
    char buf[128];
    if (!NDL_PollEvent(buf, sizeof(buf))) return 0;
    if (strncmp("kd", buf, 2) == 0) ev->key.type = SDL_KEYDOWN;
    else ev->key.type = SDL_KEYUP;

    const char *tmp = buf + 3;
    for (uint8_t i = 0; i < sizeof(keyname) / sizeof(keyname[0]); ++i) {
        if (strcmp(keyname[i], tmp) == 0) {
            ev->key.keysym.sym = i;
            break;
        }
    }
    return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
    while (!SDL_PollEvent(event)) {}
    return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
    return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) {
    return NULL;
}
