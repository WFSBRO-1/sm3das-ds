#include <nds.h>

int main(void) {
    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_5_2D);
    vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_MAIN_BG, VRAM_C_SUB_BG, VRAM_D_LCD);

    int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    int bs = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

    u16 *top = bgGetGfxPtr(bg);
    u16 *bot = bgGetGfxPtr(bs);
    for (int i = 0; i < 256 * 192; i++) {
        top[i] = RGB15(31, 0, 0);
        bot[i] = RGB15(0, 31, 0);
    }

    while (1) {
        while (REG_VCOUNT >= 192);
        while (REG_VCOUNT < 192);
    }
    return 0;
}
