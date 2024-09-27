#include <NDL.h>
#include <SDL.h>
#include <string.h>

static bool audio_playing = false;

static void (*audio_callback)(void *userdata, uint8_t *stream, int len);

static void *audio_callback_userdata;

static uint8_t audio_buffer[4096];

void SDL_try_callback() {
    if (!audio_playing) return;
    static uint32_t last = 0;
    uint32_t now = NDL_GetTicks();
    if (now - last >= 1000 / 60) {
        printf("Audio call\n");
        last = now;
        audio_callback(audio_callback_userdata, audio_buffer, sizeof(audio_buffer));
        NDL_PlayAudio(audio_buffer, sizeof(audio_buffer));
    }
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
    if (obtained != NULL) memcpy(obtained, desired, sizeof(SDL_AudioSpec));
    NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
    audio_callback = desired->callback;
    audio_callback_userdata = desired->userdata;
    return 0;
}

void SDL_CloseAudio() {
    audio_playing = false;
}

void SDL_PauseAudio(int pause_on) {
    audio_playing = !pause_on;
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
    return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
}

void SDL_LockAudio() {
}

void SDL_UnlockAudio() {
}
