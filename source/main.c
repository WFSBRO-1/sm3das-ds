#include <nds.h>
#include <fat.h>
#include <stdio.h>

#define W 256
#define H 192
#define NGAMES 3
#define KEY_Y_BIT (1 << 11)

static u16 buf[W * H];
static int game = 0;
static int track = 0;
static int bgTop, bgBot;

static bool loadBin(const char *path, u16 *dst) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    size_t n = fread(dst, 2, W * H, f);
    fclose(f);
    return n == W * H;
}

static void showGame(void) {
    char p[64];
    snprintf(p, sizeof p, "/ds_menu/top%d.bin", game);
    if (loadBin(p, buf)) dmaCopy(buf, bgGetGfxPtr(bgTop), W * H * 2);
    snprintf(p, sizeof p, "/ds_menu/title%d.bin", game);
    if (loadBin(p, buf)) dmaCopy(buf, bgGetGfxPtr(bgBot), W * H * 2);
}

static void startGame(void) {
}

int main(void) {
    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_5_2D);
    vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_MAIN_BG, VRAM_C_SUB_BG, VRAM_D_LCD);

    bgTop = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bgBot = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

    fatInitDefault();
    showGame();

    while (1) {
        while (REG_VCOUNT >= 192);
        while (REG_VCOUNT < 192);

        scanKeys();
        int down = keysDown();

        if (down & KEY_RIGHT) { game = (game + 1) % NGAMES; showGame(); }
        if (down & KEY_LEFT)  { game = (game + NGAMES - 1) % NGAMES; showGame(); }
        if (down & KEY_Y_BIT) { track = (track + 1) % NGAMES; }
        if (down & (KEY_START | KEY_A)) startGame();
        if (down & KEY_TOUCH) {
            touchPosition t;
            touchRead(&t);
            if (t.py > H) startGame();
        }
    }
    return 0;
}
