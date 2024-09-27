#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;

uint32_t NDL_GetTicks() {
    struct timeval cur;
    gettimeofday(&cur, NULL);
    return cur.tv_sec * 1000 + cur.tv_usec / 1000;
}

static int eventFile;

int NDL_PollEvent(char *buf, int len) {
    size_t res = read(eventFile, buf, len);
    return res != 0;
}

void NDL_OpenCanvas(int *w, int *h) {
    if (*w == 0 && *h == 0) {
        *w = screen_w;
        *h = screen_h;
    }
    canvas_w = *w;
    canvas_h = *h;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
    gpu_fbdraw_op drawOp;
    drawOp.sync = true;
    drawOp.pixels = pixels;
    x += (screen_w - canvas_w) / 2;
    y += (screen_h - canvas_h) / 2;
    drawOp.x = x;
    drawOp.y = y;
    drawOp.w = w;
    drawOp.h = h;
    _fbdraw(&drawOp);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
    return 0;
}

int NDL_QueryAudio() {
    return 0;
}

int NDL_Init(uint32_t flags) {
    eventFile = open("/dev/events", 0);

    int dispInfo = open("/proc/dispinfo", 0);
    char buf[128];
    read(dispInfo, buf, 128);
    sscanf(buf, "WIDTH:%d\nHEIGHT:%d", &screen_w, &screen_h);
    close(dispInfo);
    return 0;
}

void NDL_Quit() {
    close(eventFile);
}
