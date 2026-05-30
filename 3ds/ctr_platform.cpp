#include "ctr_platform.h"

#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>

#include "chrono_timer.h"

// Console handle — using the bottom screen so the top screen is free for 3D.
static PrintConsole g_console;

// Exposed to ctr_renderer.cpp
bool g_c3d_ready = false;

void ctr_platform_init(void)
{
    // Initialise graphics — top screen owned by citro3d, bottom by libctru console.
    // gfxInitDefault sets both screens to linear RGB8; we override top for citro3d.
    gfxInitDefault();

    // Initialise citro3d now so the top screen is ready for rendering.
    // This must happen before consoleInit on the bottom screen.
    g_c3d_ready = C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialise the libctru console on the bottom screen.
    consoleInit(GFX_BOTTOM, &g_console);

    // Initialize the portable timer (used by timer_GetTime macro)
    D3::ChronoTimer::Initialize();

    printf("Descent3 3DS stub started\n");
    printf("C3D_Init: %s\n", g_c3d_ready ? "OK" : "FAILED");
}

void ctr_platform_fini(void)
{
    C3D_Fini();
    gfxExit();
}
