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
#include <utils.h>
#include <device/alarm.h>
#include <signal.h>
#include <time.h>
#include <stdatomic.h>
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>
#endif

void init_map();

void init_serial();

void init_timer();

void init_vga();

void init_i8042();

void init_audio();

void init_disk();

void init_sdcard();

void init_alarm();

void send_key(uint8_t, bool);

void vga_update_screen();

static atomic_bool timer_trigger = false;

void device_update() {
    if (!timer_trigger) return;
    timer_trigger = false;
    // bool expected = true;
    // if (!atomic_compare_exchange_strong_explicit(&timer_trigger, &expected, false, memory_order_relaxed, memory_order_relaxed)) return;

    IFDEF(CONFIG_HAS_VGA, vga_update_screen());

#ifndef CONFIG_TARGET_AM
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                nemu_state.state = NEMU_QUIT;
                break;
#ifdef CONFIG_HAS_KEYBOARD
            // If a key was pressed
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                uint8_t k = event.key.keysym.scancode;
                bool is_keydown = (event.key.type == SDL_KEYDOWN);
                send_key(k, is_keydown);
                break;
            }
#endif
            default: break;
        }
    }
#endif
}

void sdl_clear_event_queue() {
#ifndef CONFIG_TARGET_AM
    SDL_Event event;
    while (SDL_PollEvent(&event));
#endif
}

static timer_t timer_id;
void timer_callback(union sigval) {
    // atomic_store_explicit(&timer_trigger, true, memory_order_relaxed);
    timer_trigger = true;
}

void register_timer() {
    struct sigevent event = {0};
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = timer_callback;
    timer_create(CLOCK_MONOTONIC, &event, &timer_id);

    struct itimerspec timer_spec = {0};
    timer_spec.it_interval.tv_nsec = 1000000000l / TIMER_HZ;
    timer_spec.it_value.tv_nsec = 1000000000l / TIMER_HZ;
    timer_settime(timer_id, 0, &timer_spec, NULL);
}

void init_device() {
    IFDEF(CONFIG_TARGET_AM, ioe_init());
    init_map();

    IFDEF(CONFIG_HAS_SERIAL, init_serial());
    IFDEF(CONFIG_HAS_TIMER, init_timer());
    IFDEF(CONFIG_HAS_VGA, init_vga());
    IFDEF(CONFIG_HAS_KEYBOARD, init_i8042());
    IFDEF(CONFIG_HAS_AUDIO, init_audio());
    IFDEF(CONFIG_HAS_DISK, init_disk());
    IFDEF(CONFIG_HAS_SDCARD, init_sdcard());

    IFNDEF(CONFIG_TARGET_AM, init_alarm());

    register_timer();
}
