#include <nds.h>
#include "nds_loader_arm9.h"
#include <filesystem.h>
#include <fat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#define W 256
#define H 192
#define NGAMES 3
#define GAME_DIR "/roms/nds/sm3dsas"

static u16 buf[W * H];
static int game = 0;
static int nFound = 0;
static char names[NGAMES][64];
static u16 *topGfx, *botGfx;

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

static void scanGames(void) {
    DIR *d = opendir(GAME_DIR);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) && nFound < NGAMES) {
        if (strstr(e->d_name, ".nds")) {
            strncpy(names[nFound], e->d_name, 63);
            names[nFound][63] = 0;
            nFound++;
        }
    }
    closedir(d);
}

static void launchGame(void) {
    if (game >= nFound) return;
    char p[160];
    snprintf(p, sizeof p, "fat:%s/%s", GAME_DIR, names[game]);
    runNdsFile(p, 0, NULL);
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
    scanGames();
    showGame();

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
        if (down & (KEY_A | KEY_START)) {
            launchGame();
        }
        if (down & KEY_TOUCH) {
            touchPosition t;
            touchRead(&t);
            if (t.py > H) launchGame();
        }
    }
    return 0;
}
