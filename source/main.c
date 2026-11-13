#include <nds.h>
#include <fat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    videoSetModeSub(MODE_0_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    if (!fatInitDefault()) {
        iprintf("NO SD CARD\n");
    } else {
        DIR *d = opendir("/roms/nds");
        if (!d) {
            iprintf("no /roms/nds\n");
        } else {
            struct dirent *e;
            int n = 0;
            iprintf("NDS FILES:\n");
            while ((e = readdir(d)) && n < 10) {
                if (strstr(e->d_name, ".nds")) {
                    iprintf("%s\n", e->d_name);
                    n++;
                }
            }
            closedir(d);
            if (n == 0) iprintf("(empty)\n");
        }
    }

    while (1) {
        swiWaitForVBlank();
    }
    return 0;
}
