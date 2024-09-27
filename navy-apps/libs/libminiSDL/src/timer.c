#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>

extern uint32_t sdlInitTime;

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  return 1;
}

void SDL_try_callback();

uint32_t SDL_GetTicks() {
  SDL_try_callback();
  return NDL_GetTicks() - sdlInitTime;
}

void SDL_Delay(uint32_t ms) {
}
