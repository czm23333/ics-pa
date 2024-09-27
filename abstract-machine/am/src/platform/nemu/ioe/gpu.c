#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static uint32_t width;
static uint32_t height;

void __am_gpu_init() {
    uint32_t ctl = inl(VGACTL_ADDR);
    width = ctl >> 16;
    height = ctl & 0xFFFFu;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    *cfg = (AM_GPU_CONFIG_T){
        .present = true, .has_accel = false,
        .width = width, .height = height,
        .vmemsz = width * height * sizeof(uint32_t)
    };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    uintptr_t ptr = FB_ADDR + (ctl->x + ctl->y * width) * sizeof(uint32_t);
    uint32_t* pixels = ctl->pixels;
    for (uint32_t i = 0; i < ctl->h; ++i) {
        for (uint32_t j = 0; j < ctl->w; ++j)
            outl(ptr + j * sizeof(uint32_t), pixels[j]);
        ptr += width * sizeof(uint32_t);
        pixels += ctl->w;
    }

    if (ctl->sync)
        outl(SYNC_ADDR, 1);
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}
