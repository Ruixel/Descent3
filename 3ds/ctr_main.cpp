// 3DS entry point for Descent 3
// Replaces sdlmain.cpp on the 3DS target.
// Start minimal: boot the platform, show the console, then exit cleanly.
// Wire in PreInitD3Systems() / Descent3() progressively as stubs are filled.

#include <3ds.h>
#include <stdio.h>

#include "ctr_platform.h"

int main(int argc, char *argv[])
{
    ctr_platform_init();

    printf("Descent3 3DS\n");
    printf("Build: " __DATE__ " " __TIME__ "\n");
    printf("\nPress START to exit.\n");

    // Main loop — keep the applet alive until the user quits.
    while (aptMainLoop())
    {
        hidScanInput();
        u32 keys = hidKeysDown();

        if (keys & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    ctr_platform_fini();
    return 0;
}
