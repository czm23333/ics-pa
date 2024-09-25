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

#include "isa.h"
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

void dev_raise_intr();

#ifndef CONFIG_TARGET_AM
static atomic_bool update_timer_trigger = false;
static atomic_bool intr_timer_trigger = false;
#endif

void device_update() {
#ifdef CONFIG_TARGET_AM
    static uint64_t last = 0;
    uint64_t now = get_time();
    if (now - last < 1000000 / TIMER_HZ) {
        return;
    }
    last = now;
#else
    if (!update_timer_trigger) return;
    update_timer_trigger = false;
#endif

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

void check_timer_intr() {
#ifdef CONFIG_TARGET_AM
    static uint64_t last = 0;
    uint64_t now = get_time();
    if (now - last < 10000) {
        return;
    }
    last = now;
#else
    if (!intr_timer_trigger) return;
    intr_timer_trigger = false;
#endif
    
    dev_raise_intr();
}

void sdl_clear_event_queue() {
#ifndef CONFIG_TARGET_AM
    SDL_Event event;
    while (SDL_PollEvent(&event));
#endif
}

#ifndef CONFIG_TARGET_AM
static timer_t update_timer_id;
void update_timer_callback(__sigval_t) {
    update_timer_trigger = true;
}

static timer_t intr_timer_id;
void intr_timer_callback(__sigval_t) {
    intr_timer_trigger = true;
}

void register_timer(timer_t* timer_id, void(*callback)(__sigval_t), __syscall_slong_t interval) {
    struct sigevent event = {0};
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = callback;
    timer_create(CLOCK_MONOTONIC, &event, timer_id);

    struct itimerspec timer_spec = {0};
    timer_spec.it_interval.tv_nsec = interval;
    timer_spec.it_value.tv_nsec = interval;
    timer_settime(timer_id, 0, &timer_spec, NULL);
}

void register_update_timer() {
    register_timer(&update_timer_id, update_timer_callback, 1000000000l / TIMER_HZ);
}

void register_intr_timer() {
    register_timer(&intr_timer_id, intr_timer_callback, 10000000l);
}
#endif

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

    IFNDEF(CONFIG_TARGET_AM, register_update_timer());
    IFNDEF(CONFIG_TARGET_AM, register_intr_timer());
}
