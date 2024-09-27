#include <common.h>
#include <externc.h>
#include <fs.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [AM_KEY_NONE] = "NONE",
    AM_KEYS(NAME)
};

size_t serial_write(void *arg, FDInfo *fd, const void *buf, size_t len) {
    auto bufc = static_cast<const char *>(buf);
    for (size_t i = 0; i < len; i++) putch(*bufc++);
    return len;
}

size_t events_read(void *arg, FDInfo *fd, void *buf, size_t len) {
    AM_INPUT_KEYBRD_T key;
    ioe_read(AM_INPUT_KEYBRD, &key);
    if (key.keycode == AM_KEY_NONE) return 0;
    const char* kn = keyname[key.keycode];
    size_t elen = 4 + strlen(kn);
    if (len < elen) return 0;

    char* bufc = static_cast<char *>(buf);
    if (key.keydown) {
        bufc[0] = 'k';
        bufc[1] = 'd';
    } else {
        bufc[0] = 'k';
        bufc[1] = 'u';
    }
    bufc[2] = ' ';
    bufc += 3;
    strcpy(bufc, kn);

    return elen;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    return 0;
}

EXTERNC void init_device() {
    Log("Initializing devices...");
    ioe_init();
}
