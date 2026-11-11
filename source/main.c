#include <nds.h>
#include <stdio.h>

int main(void) {
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
    vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
    int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    u16 *top = bgGetGfxPtr(bg);
    for (int i = 0; i < 256 * 192; i++) top[i] = RGB15(31, 0, 0);

    videoSetModeSub(MODE_0_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
    iprintf("GRAPHICS TEST\n");
    iprintf("top = red?\n");

    while (1) {
        swiWaitForVBlank();
    }
    return 0;
}
