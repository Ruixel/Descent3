// 3DS entry point for Descent 3
// Replaces sdlmain.cpp on the 3DS target.

#include <3ds.h>
#include <stdio.h>

#include "ctr_platform.h"
#include "args.h"
#include "init.h"
#include "cfile.h"
#include "hogfile.h"

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

        // Peek inside — read the HOG directory directly
        FILE *fp = fopen("sdmc:/descent3/d3.hog", "rb");
        if (fp) {
            tHogHeader header{};
            ReadHogHeader(fp, &header);
            // Skip the 68-byte HDRINFO block between header and file table
            fseek(fp, HOG_HDR_SIZE, SEEK_SET);
            printf("HOG: %u files, data offset=%u\n", header.nfiles, header.file_data_offset);
            tHogFileEntry entry{};
            uint32_t show = header.nfiles < 20 ? header.nfiles : 20;
            for (uint32_t i = 0; i < show; i++) {
                ReadHogEntry(fp, &entry);
                printf("  %-24s %u bytes\n", entry.name, entry.len);
            }
            if (header.nfiles > 20)
                printf("  ... and %u more\n", header.nfiles - 20);
            fclose(fp);
        }
    } else {
        printf("Failed to open d3.hog - check sdmc:/descent3/\n");
    }

    // Now that HOG is mounted, run the first init wave
    printf("Calling InitD3Systems1()...\n");
    InitD3Systems1(false);
    printf("InitD3Systems1() done!\n");

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
