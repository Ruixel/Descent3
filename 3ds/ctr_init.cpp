// 3DS shim for Descent 3 init systems.
// Provides PreInitD3Systems(), InitD3Systems1(), and InitD3Systems2(),
// plus all global variables those functions normally define elsewhere.

#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <ctime>

#include "args.h"
#include "init.h"
#include "mem.h"
#include "pserror.h"
#include "log.h"
#include "descent.h"
#include "localization.h"
#include "manage.h"    // LocalD3Dir, NetD3Dir

// Game subsystem headers (InitD3Systems1 callees)
#include "objinfo.h"
#include "vclip.h"
#include "room.h"
#include "lightmap_info.h"
#include "special_face.h"
#include "lighting.h"
#include "Mission.h"
#include "ship.h"
#include "matcen.h"
#include "door.h"
#include "gamefile.h"
#include "terrain.h"
#include "soundload.h"
#include "findintersection.h"
#include "polymodel.h"
#include "bitmap.h"
#include "psrand.h"
// 3DS-specific includes
#include "grtext.h"
#include "gamefont.h"
#include "ctr_app.h"
#include "ctr_database.h"
#include "renderer.h"
#include "newui.h"
#include "ui.h"
// Forward declarations from game.cpp (not compiled on 3DS)
extern void StartFrame(bool clear = true);
extern void EndFrame();
// Graphics ready flag — checked by our InitMessage replacement
bool CTR_Graphics_init = false;

// ---------------------------------------------------------------------------
// App/database instances — pointers live in descent.cpp, we fill them here.
// ---------------------------------------------------------------------------
static oeLnxApplication g_ctr_app;
static oeCtrAppDatabase g_ctr_database;

std::filesystem::path orig_pwd = "sdmc:/descent3";

// ---------------------------------------------------------------------------
// Globals from manage.cpp (LocalD3Dir, NetD3Dir)
// ---------------------------------------------------------------------------
char LocalD3Dir[256] = "sdmc:/descent3";
char NetD3Dir[256]   = "";

// ---------------------------------------------------------------------------
// Globals from various other game .cpp files
// ---------------------------------------------------------------------------
bool Use_file_xfer = true;
float Min_allowed_frametime = 0.0f;
float Mouselook_sensitivity = 1.0f;
float Mouse_sensitivity = 1.0f;

// From descent.cpp / init.cpp
bool Init_in_editor = false;
int  Gameport = 0;
int  PXOPort  = 0;
bool Dedicated_server = false;
int  ServerTimeout = 0;
float LastPacketReceived = 0.0f;

// From game.cpp — screen/window dimensions
// D3 UI and game logic runs in 640x480 logical space.
// The 3DS top screen (400x240) is a scaled-down render target; the renderer
// applies sx=400/640, sy=240/480 to all drawing calls.
int Game_window_x = 0;
int Game_window_y = 0;
int Game_window_w = 640;
int Game_window_h = 480;
int Max_window_w  = 640;
int Max_window_h  = 480;

// ---------------------------------------------------------------------------
// Stubs for subsystems not yet ported
// ---------------------------------------------------------------------------

// InitGraphics — initialise bitmaps and the citro3d renderer
static void InitGraphics_stub() {
  printf("[3DS] InitGraphics: bm_InitBitmaps\n");
  bm_InitBitmaps();   // must come before lighting init

  printf("[3DS] InitGraphics: rend_Init\n");
  renderer_preferred_state pref{};
  pref.width     = Game_window_w;
  pref.height    = Game_window_h;
  pref.bit_depth = 32;
  int ok = rend_Init(RENDERER_OPENGL, Descent, &pref);
  if (ok) {
    CTR_Graphics_init = true;
    printf("[3DS] InitGraphics: renderer ready\n");
  } else {
    printf("[3DS] InitGraphics: rend_Init FAILED\n");
  }
}

// Sound system globals (declared extern in hlsoundlib.h, but we don't compile hlsoundlib.cpp)
// These are referenced by files like soundload.cpp indirectly.
// The actual hlsSystem Sound_system lives in ctr_sound.cpp.
char Sound_mixer   = 0;
char Sound_quality = 0;

// Gamespy — not on 3DS
void gspy_Init() {}

// Networking — skipped via -nonetwork arg in fakeArgs

// ---------------------------------------------------------------------------
// PreInitD3Systems
// ---------------------------------------------------------------------------
void PreInitD3Systems() {
  printf("[3DS] PreInitD3Systems() start\n");

  // Wire up the app/database pointers that descent.cpp owns (initialized NULL)
  Descent = &g_ctr_app;
  Database = &g_ctr_database;

  bool debugging = false;
#ifndef RELEASE
  debugging = (FindArg("-debug") != 0);
#endif

  error_Init(debugging, "Descent3");

  if (FindArg("-lowmem"))
    Mem_low_memory_mode = true;

  mem_Init();

  Min_allowed_frametime = (1.0f / 30.0f) * 1000.0f;

  grtext_Init();

  printf("[3DS] PreInitD3Systems() done!\n");
}

// ---------------------------------------------------------------------------
// InitD3Systems1 — mirrors init.cpp's InitD3Systems1 for 3DS
// ---------------------------------------------------------------------------
void InitD3Systems1(bool editor) {
  printf("[3DS] InitD3Systems1() start\n");

  Init_in_editor = editor;

  // I/O system — paths already set up in main (cfile base dirs + HOG opened)
  // We skip ddio_Init / mouse mode / pref path discovery; cfile is ready.
  printf("[3DS] I/O system ready (sdmc:/descent3)\n");

  // String table
  printf("[3DS] InitStringTable...\n");
  Localization_SetLanguage(LANGUAGE_ENGLISH);
  int string_count = LoadStringTables();
  if (string_count == 0)
    printf("[3DS] WARNING: string table not loaded\n");
  else
    printf("[3DS] %d strings loaded\n", string_count);

  // Graphics — stub (no renderer yet), but init bitmaps/lightmaps now
  InitGraphics_stub();

  // Data structures
  printf("[3DS] InitObjectInfo...\n");  InitObjectInfo();
  printf("[3DS] InitVClips...\n");      InitVClips();
  printf("[3DS] InitRooms...\n");       InitRooms();

  // Lighting
  printf("[3DS] InitLightmapInfo...\n"); InitLightmapInfo();
  printf("[3DS] InitSpecialFaces...\n"); InitSpecialFaces();
  printf("[3DS] InitDynamicLighting...\n"); InitDynamicLighting();

  // Mission
  printf("[3DS] InitMission...\n");     InitMission();
  InitDefaultMissionFromCLI();

  // Ships
  printf("[3DS] InitShips...\n");       InitShips();

  // FVI (fast vertex intersection — used by physics/AI)
  printf("[3DS] InitFVI...\n");         InitFVI();

  // Matcens
  printf("[3DS] InitMatcens...\n");     InitMatcens();

  // Math tables (trig lookup tables in fix lib)
  printf("[3DS] InitMathTables...\n");  InitMathTables();

  // Random seed
  ps_srand((unsigned int)time(nullptr));

  // Sounds data structures (just zeroing arrays, no audio hardware init yet)
  printf("[3DS] InitSounds...\n");      InitSounds();

  // Terrain
  printf("[3DS] InitTerrain...\n");     InitTerrain();

  // Models
  printf("[3DS] InitModels...\n");      InitModels();

  // Doors
  printf("[3DS] InitDoors...\n");       InitDoors();

  // Gamefiles
  printf("[3DS] InitGamefiles...\n");   InitGamefiles();

  // Networking — skip (pass -nonetwork)
  // gspy — stub
  gspy_Init();

  // Sound library hardware init — stub (hlsSystem not yet compiled)
  printf("[3DS] Sound_system.InitSoundLib stub\n");

  // Cinematics — stub (mve/hlsSystem not yet ported)
  printf("[3DS] InitCinematics stub\n");

  printf("[3DS] InitD3Systems1() complete!\n");
}

// ---------------------------------------------------------------------------
// IntroScreen / InitMessage — implemented here for 3DS.
// init.cpp is not compiled on 3DS, so we provide these ourselves.
// ---------------------------------------------------------------------------

// Chunked bitmap for the title/loading screen (oemmenu.ogf)
static chunked_bitmap CTR_Title_bitmap;
static bool           CTR_Title_bitmap_init = false;

void InitMessage(const char *c, float /*progress*/) {
  if (!CTR_Graphics_init) {
    if (c) printf("[3DS] InitMessage: %s\n", c);
    return;
  }

  StartFrame(true);
  if (CTR_Title_bitmap_init) {
    rend_ClearScreen(GR_BLACK);
    int x = 0; //Game_window_w / 2 - CTR_Title_bitmap.pw / 2;
    int y = 0; //Game_window_h / 2 - CTR_Title_bitmap.ph / 2;
    rend_DrawChunkedBitmap(&CTR_Title_bitmap, x, y, 255);
  }
  if (c) printf("[3DS] InitMessage: %s\n", c);
  EndFrame();
  rend_Flip();
}

void IntroScreen() {
  printf("[3DS] IntroScreen: calling bm_AllocLoadFileBitmap...\n");
  int bm_handle = bm_AllocLoadFileBitmap("oemmenu.ogf", 0);
  printf("[3DS] IntroScreen: bm_handle=%d\n", bm_handle);
  if (bm_handle > -1) {
    printf("[3DS] IntroScreen: creating chunked bitmap...\n");
    if (!bm_CreateChunkedBitmap(bm_handle, &CTR_Title_bitmap))
      printf("[3DS] IntroScreen: failed to create chunked bitmap\n");
    else {
      CTR_Title_bitmap_init = true;
      printf("[3DS] IntroScreen: bitmap ready (%dx%d, %dx%d tiles)\n",
             CTR_Title_bitmap.pw, CTR_Title_bitmap.ph,
             CTR_Title_bitmap.w,  CTR_Title_bitmap.h);
    }
    bm_FreeBitmap(bm_handle);
    InitMessage(nullptr);
  } else {
    printf("[3DS] IntroScreen: oemmenu.ogf not found in HOG\n");
  }
}

// ---------------------------------------------------------------------------
// InitD3Systems2 — show intro screen then load table files / objects / etc.
// ---------------------------------------------------------------------------
void InitD3Systems2(bool /*editor*/) {
  printf("[3DS] InitD3Systems2() start\n");

  printf("[3DS] IntroScreen...\n");
  IntroScreen();

  printf("[3DS] LoadAllFonts...\n");
  LoadAllFonts();
  printf("[3DS] Fonts loaded\n");

  // Initialise the UI system (sets UI_screen_width/height, loads cursor, etc.)
  // Must happen after fonts are loaded (SMALL_FONT must be valid).
  printf("[3DS] ui_Init...\n");
  {
    tUIInitInfo uiinit;
    uiinit.window_font = SMALL_FONT;
    uiinit.w = 640;
    uiinit.h = 480;
    ui_Init(Descent, &uiinit);
  }
  ui_UseCursor("StdCursor.ogf");
  ui_Flush();

  printf("[3DS] NewUIInit...\n");
  NewUIInit();

  printf("[3DS] InitD3Systems2() complete\n");
}
