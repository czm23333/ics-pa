#include <NDL.h>
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

static bool audio_playing = false;

static void (*audio_callback)(void *userdata, uint8_t *stream, int len);

static void *audio_callback_userdata;

static uint8_t *audio_buffer;
static uint32_t audio_buffer_size;

void SDL_try_callback() {
    static bool in_callback = false;
    if (in_callback) return;
    in_callback = true;
    if (!audio_playing) return;
    static uint32_t last = 0;
    uint32_t now = NDL_GetTicks();
    if (now - last >= 1000 / 60) {
        last = now;
        if (audio_callback != NULL) audio_callback(audio_callback_userdata, audio_buffer, audio_buffer_size);
        NDL_PlayAudio(audio_buffer, audio_buffer_size);
    }
    in_callback = false;
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
    audio_buffer_size = desired->samples * desired->channels * desired->format / 8;
    audio_buffer = malloc(audio_buffer_size);
    if (obtained != NULL) {
        memcpy(obtained, desired, sizeof(SDL_AudioSpec));
        obtained->size = audio_buffer_size;
    }
    NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
    audio_callback = desired->callback;
    audio_callback_userdata = desired->userdata;
    audio_playing = true;
    return 0;
}

void SDL_CloseAudio() {
    audio_playing = false;
    free(audio_buffer);
    audio_buffer = NULL;
}

void SDL_PauseAudio(int pause_on) {
    audio_playing = !pause_on;
}

uint32_t saturate_add32(uint32_t a, uint32_t b) {
    uint32_t sum = a + b;
    if (sum < a || sum < b)
        return ~(uint32_t) 0;
    return sum;
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
    int16_t *d = (int16_t *) dst;
    const int16_t *s = (int16_t *) src;
    len /= 2;
    while (len--) {
        int16_t tmp;
        bool overflow = __builtin_add_overflow(*d, *s / SDL_MIX_MAXVOLUME * volume, &tmp);
        if (overflow) *d = *d < 0 ? INT16_MIN : INT16_MAX;
        else *d = tmp;
        ++d;
        ++s;
    }
}

typedef struct WAV_chunk_desc {
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint32_t format;
} WAV_chunk_desc;

typedef struct WAV_fmt_chunk {
    uint32_t fmt_id;
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} WAV_fmt_chunk;

typedef struct WAV_data_chunk {
    uint32_t chunk_id;
    uint32_t chunk_size;
} WAV_data_chunk;

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
    FILE *f = fopen(file, "rb");
    if (f == NULL) return NULL;
    WAV_chunk_desc desc;
    fread(&desc, sizeof(WAV_chunk_desc), 1, f);
    WAV_fmt_chunk fmt_chunk;
    fread(&fmt_chunk, sizeof(WAV_fmt_chunk), 1, f);
    fseek(f, fmt_chunk.fmt_size - 16, SEEK_CUR);
    WAV_data_chunk data_chunk;
    fread(&data_chunk, sizeof(WAV_data_chunk), 1, f);
    uint8_t *buf = malloc(data_chunk.chunk_size);
    fread(buf, 1, data_chunk.chunk_size, f);
    fclose(f);
    *audio_buf = buf;
    *audio_len = data_chunk.chunk_size;
    spec->freq = fmt_chunk.sample_rate;
    spec->channels = fmt_chunk.num_channels;
    spec->samples = 1024;
    spec->format = fmt_chunk.bits_per_sample;
    return spec;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
    free(audio_buf);
}

void SDL_LockAudio() {
}

void SDL_UnlockAudio() {
}
