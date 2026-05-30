// 3DS entry point for Descent 3
// Replaces sdlmain.cpp on the 3DS target.

#include <3ds.h>
#include <stdio.h>

#include "ctr_platform.h"
#include "args.h"
#include "init.h"

int main(int argc, char *argv[])
{
    ctr_platform_init();

    printf("Descent3 3DS\n");
    printf("Build: " __DATE__ " " __TIME__ "\n");

    // Fake argv so GatherArgs has something to work with
    char arg0[] = "Descent3";
    char *fakeArgs[] = { arg0 };
    GatherArgs(fakeArgs);

    printf("Calling PreInitD3Systems()...\n");
    PreInitD3Systems();
    printf("PreInitD3Systems() returned!\n");

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
