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

EXTERNC void load_program(const char *filename);
EXTERNC void exit_last_program();

size_t events_read(void *arg, FDInfo *fd, void *buf, size_t len) {
    AM_INPUT_KEYBRD_T key;
    ioe_read(AM_INPUT_KEYBRD, &key);
    if (key.keycode == AM_KEY_NONE) return 0;
    const char *kn = keyname[key.keycode];
    size_t elen = 4 + strlen(kn);
    if (len < elen) return 0;

    char *bufc = static_cast<char *>(buf);
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

    if (!key.keydown) {
        if (key.keycode == AM_KEY_F1) load_program("/bin/nterm");
        else if (key.keycode == AM_KEY_F2) exit_last_program();
    }

    return elen;
}

size_t dispinfo_read(void *arg, FDInfo *fd, void *buf, size_t len) {
    AM_GPU_CONFIG_T gpuConf;
    ioe_read(AM_GPU_CONFIG, &gpuConf);
    return sprintf(static_cast<char *>(buf), "WIDTH:%d\nHEIGHT:%d", gpuConf.width, gpuConf.height);
}

size_t sbctl_read(void *arg, FDInfo *fd, void *buf, size_t len) {
    if (len < sizeof(AM_AUDIO_STATUS_T)) return 0;
    ioe_read(AM_AUDIO_STATUS, buf);
    return sizeof(AM_AUDIO_STATUS_T);
}

size_t sbctl_write(void *arg, FDInfo *fd, const void *buf, size_t len) {
    if (len != sizeof(AM_AUDIO_CTRL_T)) return 0;
    ioe_write(AM_AUDIO_CTRL, const_cast<void *>(buf));
    return sizeof(AM_AUDIO_CTRL_T);
}

size_t sb_write(void *arg, FDInfo *fd, const void *buf, size_t len) {
    AM_AUDIO_PLAY_T play;
    play.buf = Area{const_cast<void *>(buf), static_cast<char *>(const_cast<void *>(buf)) + len};
    ioe_write(AM_AUDIO_PLAY, &play);
    return len;
}

EXTERNC void init_device() {
    Log("Initializing devices...");
    ioe_init();
}
