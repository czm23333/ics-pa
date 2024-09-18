/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>
#include <threads.h>

enum {
    reg_freq,
    reg_channels,
    reg_samples,
    reg_sbuf_size,
    reg_init,
    reg_count,
    reg_pad,
    reg_lock_flag,
    nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;
static bool audio_initialized = false;

static mtx_t audio_mutex;

static void SDLCALL audio_callback(void *userdata, Uint8 *stream, int len) {
    mtx_lock(&audio_mutex);
    memset(stream, 0, len);
    uint8_t *p = sbuf + audio_base[reg_pad], *end = sbuf + CONFIG_SB_SIZE;
    uint32_t output = audio_base[reg_count];
    if (len < output) output = len;
    audio_base[reg_count] -= output;
    audio_base[reg_pad] += output, audio_base[reg_pad] %= CONFIG_SB_SIZE;
    while (output--) {
        *stream++ = *p++;
        if (p == end) p = sbuf;
    }
    mtx_unlock(&audio_mutex);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
    if (!is_write) return;
    if (len != 4) return;
    if (offset == reg_init * sizeof(uint32_t) && audio_base[reg_init] && !audio_initialized) {
        SDL_InitSubSystem(SDL_INIT_AUDIO);
        SDL_AudioSpec spec;
        memset(&spec, 0, sizeof(spec));
        spec.format = AUDIO_S16SYS;
        spec.freq = audio_base[reg_freq];
        spec.channels = audio_base[reg_channels];
        spec.samples = audio_base[reg_samples];
        spec.callback = audio_callback;
        SDL_OpenAudio(&spec, NULL);
        SDL_PauseAudio(0);
        audio_initialized = true;
    }
    if (offset == reg_lock_flag * sizeof(uint32_t) && audio_initialized) {
        if (audio_base[reg_lock_flag])
            mtx_lock(&audio_mutex);
        else
            mtx_unlock(&audio_mutex);
    }
}

void init_audio() {
    mtx_init(&audio_mutex, mtx_plain);

    uint32_t space_size = sizeof(uint32_t) * nr_reg;
    audio_base = (uint32_t *) new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
    add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

    sbuf = (uint8_t *) new_space(CONFIG_SB_SIZE);
    audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
    add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
