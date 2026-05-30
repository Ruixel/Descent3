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
#include "ctr_app.h"
#include "ctr_database.h"

// ---------------------------------------------------------------------------
// Globals that normally live in descent.cpp
// ---------------------------------------------------------------------------
static oeLnxApplication g_ctr_app;
oeApplication *Descent = &g_ctr_app;

static oeCtrAppDatabase g_ctr_database;
oeAppDatabase *Database = &g_ctr_database;

grScreen *Game_screen = nullptr;

bool Descent_overrided_intro = false;
std::filesystem::path orig_pwd = "sdmc:/descent3";
std::filesystem::path Descent3_temp_directory = "sdmc:/descent3/tmp";
bool Katmai = false;

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

// ---------------------------------------------------------------------------
// Stubs for subsystems not yet ported
// ---------------------------------------------------------------------------

// grtext (renderer-dependent text system)
void grtext_Init() {
  printf("[3DS] grtext_Init stub\n");
}

// InitGraphics — renderer not yet ported
static void InitGraphics_stub() {
  printf("[3DS] InitGraphics stub — calling bm_InitBitmaps\n");
  bm_InitBitmaps();   // initialises lightmaps/bumpmaps too; needed before lighting init
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
// InitD3Systems2 — stub until table files / objects / menu are ported
// ---------------------------------------------------------------------------
void InitD3Systems2(bool editor) {
  printf("[3DS] InitD3Systems2() stub\n");
}
