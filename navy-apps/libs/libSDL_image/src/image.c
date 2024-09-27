#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface *IMG_Load_RW(SDL_RWops *src, int freesrc) {
    assert(src->type == RW_TYPE_MEM);
    assert(freesrc == 0);
    return NULL;
}

SDL_Surface *IMG_Load(const char *filename) {
    FILE* file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t* buf = malloc(size);
    fread(buf, size, 1, file);
    fclose(file);
    SDL_Surface *surface = STBIMG_LoadFromMemory(buf, size);
    free(buf);
    return surface;
}

int IMG_isPNG(SDL_RWops *src) {
    return 0;
}

SDL_Surface *IMG_LoadJPG_RW(SDL_RWops *src) {
    return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
    return "Navy does not support IMG_GetError()";
}
