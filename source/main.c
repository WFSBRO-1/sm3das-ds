#include <nds.h>
#include <filesystem.h>
#include <fat.h>
#include <stdio.h>

#define W 256
#define H 192
#define NGAMES 3
#define CHUNK 8192
#define VOL 0
#define FADE 128

static u16 buf[W * H];
static int game = 0;
static u16 *topGfx, *botGfx;

static FILE *music = NULL;
static u8 chunkA[CHUNK], chunkB[CHUNK];
static u8 *cbuf[2] = { chunkA, chunkB };
static int ch[2] = { -1, -1 };
static int slot = 0;

static const char *trackPath[NGAMES] = {
    "/smds3das/track0.raw",
    "/smds3das/track1.raw",
    "/smds3das/track2.raw"
};

static bool chanPlaying(int n) {
    return ((*(vu32 *)(0x04000400 + 16 * n + 8)) & (1u << 31)) != 0;
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
    for (int i = 0; i < 2; i++) {
        if (ch[i] >= 0) soundKill(ch[i]);
        ch[i] = -1;
    }
    if (music) {
        fclose(music);
        music = NULL;
    }
}

static void openTrack(int t) {
    stopMusic();
    music = fopen(trackPath[t], "rb");
}

static void playSlot(int s) {
    if (!music) return;
    size_t n = fread(cbuf[s], 1, CHUNK, music);
    if (n == 0) {
        rewind(music);
        n = fread(cbuf[s], 1, CHUNK, music);
        if (n == 0) return;
    }
    for (size_t i = 0; i < n; i++) {
        int v = (int)cbuf[s][i] - 128;
        int g = 128;
        if (i < FADE) g = (int)i * 128 / FADE;
        else if (n - i < FADE) g = (int)(n - i) * 128 / FADE;
        cbuf[s][i] = (u8)(((v * g) >> 7) + 128);
    }
    ch[s] = soundPlaySample(cbuf[s], SoundFormat_8Bit, n, 22050, VOL, 64, false, 0);
}

static void feedMusic(void) {
    if (!music) return;
    if (ch[slot] < 0 || !chanPlaying(ch[slot])) {
        playSlot(slot);
        slot ^= 1;
    }
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
    openTrack(0);

    int track = 0;
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        int down = keysDown();

        if ((down & KEY_RIGHT) && game < NGAMES - 1) {
            game++;
            showGame();
        }
        if ((down & KEY_LEFT) && game > 0) {
            game--;
            showGame();
        }
        if (down & KEY_Y) {
            track = (track + 1) % NGAMES;
            openTrack(track);
        }

        feedMusic();
    }
    return 0;
}
