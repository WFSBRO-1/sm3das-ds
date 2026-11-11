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
    for (int i = 0; i < 16; i++) SCHANNEL_CR(i) = 0;
    chanBusy = -1;
    if (music) { fclose(music); music = NULL; }
}

static void openTrack(int t) {
    stopMusic();
    music = fopen(trackPath[t], "rb");
}

static void feedMusic(void) {
    if (!music) return;
    if (chanBusy >= 0 && (SCHANNEL_CR(chanBusy) & SOUND_ENABLE)) return;

    size_t n = fread(chunk, 1, CHUNK, music);
    if (n == 0) {
        rewind(music);
        return;
    }
    for (size_t i = 0; i < n; i++) chunk[i] ^= 0x80;
    chanBusy = soundPlaySample(chunk, SoundFormat_8Bit, n, 22050, 100, 64, false, 0);
}

int main(void) {
    vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
                        VRAM_C_SUB_BG, VRAM_D_LCD);
    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_5_2D);

    int bt = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    int bb = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    topGfx = bgGetGfxPtr(bt);
    botGfx = bgGetGfxPtr(bb);

    nitroFSInit(NULL);
    fatInitDefault();
    soundEnable();

    showGame();
    openTrack(track);

    while (1) {
        swiWaitForVBlank();
        scanKeys();
        int down = keysDown();

        if ((down & KEY_RIGHT) && game < NGAMES - 1) { game++; showGame(); }
        if ((down & KEY_LEFT) && game > 0) { game--; showGame(); }
        if (down & KEY_Y) {
            track = (track + 1) % NGAMES;
            openTrack(track);
        }
        if (down & (KEY_START | KEY_A)) {
        }
        if (down & KEY_TOUCH) {
            touchPosition t;
            touchRead(&t);
            if (t.py > H) {
            }
        }

        feedMusic();
    }
    return 0;
}
