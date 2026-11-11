#include <nds.h>
#include <filesystem.h>
#include <fat.h>
#include <stdio.h>

#define W 256
#define H 192
#define NGAMES 3
#define CHUNK 16384

static u16 buf[W * H];
static int game = 0;
static int track = 0;
static u16 *topGfx, *botGfx;

static FILE *music = NULL;
static u8 chunk[CHUNK];
static int chanBusy = -1;

static const char *trackPath[NGAMES] = {
    "/smds3das/track0.raw",
    "/smds3das/track1.raw",
    "/smds3das/track2.raw"
};

static bool chanPlaying(int n) {
    return (*(vu32 *)(0x04000400 + 16 * n + 8)) & (1u << 31);
}

static void drawFile(const char *path, u16 *dst, u16 fallback) {
    FILE *f = fopen(path, "rb");
    if (f) {
        size_t n = fread(buf, 2, W * H, f);
        fclose(f);
        if (n == W * H) {
            dmaCopy(buf, dst, W * H * 2);
            return;
        }
    }
    for (int i = 0; i < W * H; i++) dst[i] = fallback;
}

static void showGame(void) {
    char p[48];
    snprintf(p, sizeof p, "nitro:/top%d.bin", game);
    drawFile(p, topGfx, RGB15(31, 0, 0) | BIT(15));
    snprintf(p, sizeof p, "nitro:/title%d.bin", game);
    drawFile(p, botGfx, RGB15(0, 31, 0) | BIT(15));
}

static void stopMusic(void) {
    if (chanBusy >= 0) soundKill(chanBusy);
    chanBusy = -1;
    if (music) { fclose(music); music = NULL; }
}

static void openTrack(int t) {
    stopMusic();
    music = fopen(trackPath[t], "rb");
}

static void feedMusic(void) {
    if (!music) return;
    if (chanBusy >= 0 && chanPlaying(chanBusy)) return;

    size_t n = fread(chunk, 1, CHUNK, music);
    if (n == 0) {
        rewind(music);
        return;
    }
    for (size_t i = 0; i
