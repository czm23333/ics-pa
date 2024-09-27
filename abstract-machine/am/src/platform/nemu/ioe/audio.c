#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define AUDIO_PAD_ADDR     (AUDIO_ADDR + 0x18)
#define AUDIO_LOCK_ADDR     (AUDIO_ADDR + 0x1c)

static int sbuf_size;

void __am_audio_init() {
    sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->bufsize = sbuf_size;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
    outl(AUDIO_FREQ_ADDR, ctrl->freq);
    outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
    outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
    outl(AUDIO_INIT_ADDR, 1);
}

static uint32_t get_count() {
    return inl(AUDIO_COUNT_ADDR);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
    stat->count = get_count();
}

void __am_audio_lock() {
    outl(AUDIO_LOCK_ADDR, 1);
}

void __am_audio_unlock() {
    outl(AUDIO_LOCK_ADDR, 0);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
    uint8_t* ptr = ctl->buf.start;
    beg:__am_audio_lock();
    uint32_t count = get_count();
    uint32_t pad = inl(AUDIO_PAD_ADDR);
    uint32_t index = (pad + count) % sbuf_size;
    while (ptr != ctl->buf.end) {
        if (count >= sbuf_size) {
            outl(AUDIO_COUNT_ADDR, count);
            __am_audio_unlock();
            while (get_count() >= sbuf_size) {}
            goto beg;
        }
        outb(AUDIO_SBUF_ADDR + index * sizeof(uint8_t), *ptr++);
        index = (index + 1) % sbuf_size;
        ++count;
    }
    outl(AUDIO_COUNT_ADDR, count);
    __am_audio_unlock();
}
