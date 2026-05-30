// 3DS shim for Descent 3 init systems.
// Provides PreInitD3Systems(), InitD3Systems1(), and InitD3Systems2(),
// plus all global variables those functions normally define elsewhere.

#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include "args.h"
#include "init.h"
#include "mem.h"
#include "pserror.h"
#include "log.h"
#include "descent.h"
#include "localization.h"

// 3DS-specific includes
#include "ctr_app.h"
#include "ctr_database.h"

// ---------------------------------------------------------------------------
// Globals that normally live in descent.cpp / other game files
// ---------------------------------------------------------------------------

// The application object — must be non-null before any subsystem that calls
// Descent->xxx().  ctr_app.h provides oeLnxApplication with no-op virtuals.
static oeLnxApplication g_ctr_app;
oeApplication *Descent = &g_ctr_app;

// The app database — stores user preferences.  Our stub returns defaults.
static oeCtrAppDatabase g_ctr_database;
oeAppDatabase *Database = &g_ctr_database;

// Screen object (not used until real rendering starts)
grScreen *Game_screen = nullptr;

// Other descent.h globals
bool Descent_overrided_intro = false;
std::filesystem::path orig_pwd = "sdmc:/descent3";
std::filesystem::path Descent3_temp_directory = "sdmc:/descent3/tmp";
bool Katmai = false;  // No SSE on ARM

// ---------------------------------------------------------------------------
// Globals that normally live in various game .cpp files
// ---------------------------------------------------------------------------
bool Use_file_xfer = true;
float Min_allowed_frametime = 0.0f;
float Mouselook_sensitivity = 1.0f;
float Mouse_sensitivity = 1.0f;

// grtext_Init stub — real version needs the renderer
void grtext_Init() {
  printf("[3DS] grtext_Init stub\n");
}

// ---------------------------------------------------------------------------
// PreInitD3Systems — called very early, before filesystem is even mounted
// ---------------------------------------------------------------------------
void PreInitD3Systems() {
  printf("[3DS] PreInitD3Systems() start\n");

  bool debugging = false;
#ifndef RELEASE
  debugging = (FindArg("-debug") != 0);
#endif

  printf("[3DS] error_Init...\n");
  error_Init(debugging, "Descent3");

  if (FindArg("-lowmem"))
    Mem_low_memory_mode = true;

  printf("[3DS] mem_Init...\n");
  mem_Init();

  Min_allowed_frametime = (1.0f / 30.0f) * 1000.0f;

  printf("[3DS] grtext_Init...\n");
  grtext_Init();

  printf("[3DS] PreInitD3Systems() done!\n");
}

// ---------------------------------------------------------------------------
// InitD3Systems1 — first wave: I/O, string table, basic graphics
// ---------------------------------------------------------------------------
void InitD3Systems1(bool editor) {
  printf("[3DS] InitD3Systems1() start\n");

  // String table — loads D3.STR from the HOG via cfile.
  // cf_OpenLibrary() must have been called before this.
  printf("[3DS] Loading string table...\n");
  Localization_SetLanguage(LANGUAGE_ENGLISH);
  int string_count = LoadStringTables();
  if (string_count == 0) {
    printf("[3DS] WARNING: string table not loaded (D3.STR missing?)\n");
  } else {
    printf("[3DS] String table loaded: %d strings\n", string_count);
  }

  printf("[3DS] InitD3Systems1() done!\n");
}

// ---------------------------------------------------------------------------
// InitD3Systems2 — second wave: table files, objects, etc.
// Stubbed until more subsystems are ported.
// ---------------------------------------------------------------------------
void InitD3Systems2(bool editor) {
  printf("[3DS] InitD3Systems2() stub\n");
}
