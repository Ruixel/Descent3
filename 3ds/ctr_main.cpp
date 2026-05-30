// 3DS entry point for Descent 3
// Replaces sdlmain.cpp on the 3DS target.

#include <3ds.h>
#include <stdio.h>

#include "ctr_platform.h"
#include "args.h"
#include "init.h"
#include "cfile.h"

int main(int argc, char *argv[])
{
    ctr_platform_init();

    printf("Descent3 3DS\n");
    printf("Build: " __DATE__ " " __TIME__ "\n");

    // Fake argv so GatherArgs has something to work with
    char arg0[] = "Descent3";
    char *fakeArgs[] = { arg0, nullptr };  // nullptr terminates the argv array
    GatherArgs(fakeArgs);

    printf("Calling PreInitD3Systems()...\n");
    PreInitD3Systems();
    printf("PreInitD3Systems() returned!\n");

    // Mount the SD card filesystem
    printf("Mounting sdmc...\n");
    fsInit();
    archiveMountSdmc();

    // Init cfile base directories (sdmc:/descent3 set via CMake DEFAULT_ADDITIONAL_DIRS)
    printf("Init cfile base dirs...\n");
    cf_AddDefaultBaseDirectories();
    cf_AddBaseDirectory("sdmc:/descent3");

    // Try opening the main HOG file
    printf("Opening d3.hog...\n");
    int hog = cf_OpenLibrary("d3.hog");
    if (hog) {
        printf("d3.hog opened! handle=%d\n", hog);
    } else {
        printf("Failed to open d3.hog - check sdmc:/descent3/\n");
    }

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
