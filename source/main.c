#include <nds.h>
#include <stdio.h>

int main(void) {
    consoleDemoInit();
    iprintf("\x1b[2J");
    iprintf("MENU TEST OK\n");
    iprintf("Press START\n");

    while (1) {
        swiWaitForVBlank();
        scanKeys();
        if (keysDown() & KEY_START) {
            iprintf("START pressed\n");
        }
    }
    return 0;
}
