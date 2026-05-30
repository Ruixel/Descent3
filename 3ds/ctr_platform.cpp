#include "ctr_platform.h"

#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>

#include "chrono_timer.h"

// Console handle — using the bottom screen so the top screen is free for 3D.
static PrintConsole g_console;

void ctr_platform_init(void)
{
    gfxInitDefault();

    // Initialise the libctru console on the bottom screen.
    consoleInit(GFX_BOTTOM, &g_console);

    // Initialize the portable timer (used by timer_GetTime macro)
    D3::ChronoTimer::Initialize();

    printf("Descent3 3DS stub started\n");

    // Initialise citro3d — deferred until rendering is actually needed
    // C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
}

void ctr_platform_fini(void)
{
    // C3D_Fini();
    gfxExit();
}
