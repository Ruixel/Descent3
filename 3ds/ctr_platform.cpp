#include "ctr_platform.h"

#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>

// Console handle — using the bottom screen so the top screen is free for 3D.
static PrintConsole g_console;

void ctr_platform_init(void)
{
    gfxInitDefault();

    // Initialise the libctru console on the bottom screen.
    consoleInit(GFX_BOTTOM, &g_console);

    // Initialise citro3d (needed even before real rendering; harmless stub-only).
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    printf("Descent3 3DS stub started\n");
}

void ctr_platform_fini(void)
{
    C3D_Fini();
    gfxExit();
}
