// 3DS stubs for subsystems not yet ported.
// Each stub group is labelled with the originating subsystem.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <3ds.h>

// -----------------------------------------------------------------------
// sndlib — sound arrays and InitSounds (from sndlib/soundload.cpp and
//           sndlib/ddsoundload.cpp, which are not compiled on 3DS)
// -----------------------------------------------------------------------
#include "ssl_lib.h"   // sound_info, sound_file_info, MAX_SOUNDS, MAX_SOUND_FILES

sound_info      Sounds[MAX_SOUNDS];
sound_file_info SoundFiles[MAX_SOUND_FILES];
int Num_sounds      = 0;
int Num_sound_files = 0;

void InitSounds() {
  for (int i = 0; i < MAX_SOUNDS; i++) {
    Sounds[i].used    = 0;
    Sounds[i].name[0] = '\0';
    Sounds[i].flags   = 0;
  }
  Num_sounds = 0;

  for (int i = 0; i < MAX_SOUND_FILES; i++) {
    SoundFiles[i].name[0] = '\0';
    SoundFiles[i].used    = 0;
  }
  Num_sound_files = 0;
}

// rend_FreePreUploadedTexture is defined in ctr_renderer.cpp

// -----------------------------------------------------------------------
// manage stubs — mission_download / Mission.cpp
// -----------------------------------------------------------------------
// Avoid pulling in mission_download.h (it needs network_address).
// Reproduce only what we need.
#define MAX_MISSION_URL_LEN 300
#define MAX_MISSION_URL_COUNT 5
struct msn_urls {
  char msnname[260];  // _MAX_PATH
  char URL[MAX_MISSION_URL_COUNT][MAX_MISSION_URL_LEN];
};
msn_urls Net_msn_URLs;
bool mng_SetAddonTable(const char *) { return false; }

// -----------------------------------------------------------------------
// gamesave globals
// -----------------------------------------------------------------------
int Times_game_restored = 0;

// -----------------------------------------------------------------------
// player stubs (Mission.cpp calls these when loading)
// -----------------------------------------------------------------------
bool PlayerResetShipPermissions(int, bool) { return true; }
bool PlayerSetShipPermission(int, char *, bool) { return true; }

// DoMessageBox / DoMessageBoxAdvanced are now provided by newui.cpp

// -----------------------------------------------------------------------
// dedicated server stubs
// -----------------------------------------------------------------------
#include <cstdarg>
void PrintDedicatedMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

// -----------------------------------------------------------------------
// Osiris stubs (script engine — not on 3DS)
// -----------------------------------------------------------------------
void Osiris_CloseOMMS() {}
void Osiris_ClearExtractedScripts(bool) {}

// -----------------------------------------------------------------------
// gamesequence globals (referenced by bitmap/tga.cpp via gamesequence.h)
// -----------------------------------------------------------------------
int paged_in_count = 0;
int paged_in_num   = 0;

// -----------------------------------------------------------------------
// game.cpp stubs — StartFrame / EndFrame / grtext_SetParameters
// game.cpp is not compiled on 3DS; these thin wrappers call the renderer
// directly, which is all InitMessage needs.
// -----------------------------------------------------------------------
#include "renderer.h"

extern int Game_window_x, Game_window_y, Game_window_w, Game_window_h;

void StartFrame(bool clear) {
  rend_StartFrame(Game_window_x, Game_window_y,
                  Game_window_x + Game_window_w,
                  Game_window_y + Game_window_h,
                  clear ? RF_CLEAR_ZBUFFER : 0);
}
void StartFrame(int x, int y, int x2, int y2, bool /*clear*/, bool /*push*/) {
  rend_StartFrame(x, y, x2, y2);
}
void EndFrame() {
  rend_EndFrame();
}

// grtext_SetParameters is now provided by grtext.cpp

// -----------------------------------------------------------------------
// game.cpp screen mode — on 3DS we're always in menu mode
// -----------------------------------------------------------------------
#include "game.h"
static int s_screen_mode = SM_MENU;
int  GetScreenMode() { return s_screen_mode; }
void SetScreenMode(int sm, bool /*force*/) {
    printf("[3DS] SetScreenMode(%d)\n", sm);
    s_screen_mode = sm;
}

// MainLoop, SetFunctionMode, GetFunctionMode are provided by descent.cpp

// -----------------------------------------------------------------------
// Multiplayer UI bail flag
// -----------------------------------------------------------------------
bool Multi_bail_ui_menu = false;

// -----------------------------------------------------------------------
// Screenshot — not supported on 3DS
// -----------------------------------------------------------------------
void DoScreenshot() {}

// -----------------------------------------------------------------------
// d3music stubs
// -----------------------------------------------------------------------
#include "d3music.h"
tMusicSeqInfo Game_music_info{};
void D3MusicDoFrame(tMusicSeqInfo *) {}
void D3MusicStart(const char *) {}
void D3MusicStop() {}
void D3MusicSetRegion(int16_t, bool) {}
void D3MusicStartCinematic() {}

// -----------------------------------------------------------------------
// Sound name lookup stub
// -----------------------------------------------------------------------
#include "soundload.h"
int FindSoundName(const char *) { return -1; }

// -----------------------------------------------------------------------
// hlsSystem Sound_system — stub instance (no audio hardware on this path)
// -----------------------------------------------------------------------
#include "hlsoundlib.h"
hlsSystem Sound_system;

// -----------------------------------------------------------------------
// pilot_class stubs (constructor/destructor + filename methods)
// -----------------------------------------------------------------------
#include "pilot_class.h"
#include "Inventory.h"
pilot::pilot() {
  // Zero all POD/pointer members individually — can't use memset(this,...) because
  // std::string filename would have its SSO buffer corrupted.
  name          = nullptr;
  ship_logo     = nullptr;
  ship_model    = nullptr;
  audio1_file   = nullptr;
  audio2_file   = nullptr;
  audio3_file   = nullptr;
  audio4_file   = nullptr;
  guidebot_name = nullptr;
  picture_id    = 0;
  difficulty    = 0;
  hud_mode      = 0;
  profanity_filter_on = false;
  audiotaunts   = false;
  hud_stat      = 0;
  hud_graphical_stat = 0;
  game_window_w = 0;
  game_window_h = 0;
  num_missions_flown = 0;
  mission_data  = nullptr;
  memset(PrimarySelectList,   0, sizeof(PrimarySelectList));
  memset(SecondarySelectList, 0, sizeof(SecondarySelectList));
  memset(&gameplay_toggles,   0, sizeof(gameplay_toggles));
  // filename is default-constructed as empty std::string — leave it alone.
}
pilot::~pilot() {}
void pilot::set_filename(const std::string &f) { filename = f; }
std::string pilot::get_filename() { return filename; }

Inventory::Inventory()  {}
Inventory::~Inventory() {}
void Inventory::Reset(bool, int) {}
int pilot::find_mission_data(const char *) { return -1; }

// -----------------------------------------------------------------------
// Pilot system stubs
// -----------------------------------------------------------------------
#include "pilot.h"
pilot Current_pilot;
void  PltReadFile(pilot *, bool, bool) {}
int   PltWriteFile(pilot *, bool) { return 1; }
void  PilotSelect() {}
void  CurrentPilotUpdateMissionStatus(bool) {}
int   GetPilotShipPermissions(pilot *, const char *) { return 0; }

// -----------------------------------------------------------------------
// Multiplayer / networking stubs
// -----------------------------------------------------------------------
bool  MultiDLLGameStarting = false;
bool  Demo_looping         = false;
bool  Demo_restart         = false;
bool  TCP_active           = false;
int   Auto_login_port      = 0;
char  Auto_login_addr[256] = {};
bool  LoadMultiDLL(const char *) { return false; }
void  CallMultiDLL(int) {}
void  ReturnMultiplayerGameMenu() {}
void  MainMultiplayerMenu() {}
void  AutoConnectPXO() {}
void  AutoConnectLANIP() {}
void  AutoConnectHeat() {}

// -----------------------------------------------------------------------
// Game mode / state stubs
// -----------------------------------------------------------------------
void  SetGameMode(int) {}
bool  IsCheater = false;

// -----------------------------------------------------------------------
// Load/save game stubs
// -----------------------------------------------------------------------
void  LoadGameDialog() {}
bool  DoPathFileDialog(bool, std::filesystem::path &, const char *,
                       const std::vector<std::string> &, int) { return false; }
bool  SimpleStartLevel(const std::filesystem::path &) { return false; }

// -----------------------------------------------------------------------
// Options menu stub
// -----------------------------------------------------------------------
void  OptionsMenu() {}

// -----------------------------------------------------------------------
// Demo stubs
// -----------------------------------------------------------------------
void  LoadDemoDialog() {}

// -----------------------------------------------------------------------
// Cinematics stubs
// -----------------------------------------------------------------------
#include "cinematics.h"
tCinematic *StartMovie(const char *, bool) { return nullptr; }
bool        FrameMovie(tCinematic *, int, int, bool) { return false; }
void        EndMovie(tCinematic *) {}

// -----------------------------------------------------------------------
// Level / mission stubs
// -----------------------------------------------------------------------
#include "Mission.h"
bool  LoadLevelInfo(const std::filesystem::path &, level_info &) { return false; }
// DisplayLevelWarpDlg, MenuLoadLevel, MenuNewGame defined in menu.cpp

// -----------------------------------------------------------------------
// Game state (gamesequence.cpp not compiled on 3DS)
// -----------------------------------------------------------------------
#include "gamesequence.h"
tGameState Game_state = GAMESTATE_IDLE;

// -----------------------------------------------------------------------
// PlayGame / Credits_Display / FreeMultiDLL
// (game.cpp / credits.cpp / multidll.cpp — not compiled on 3DS)
// -----------------------------------------------------------------------
// PlayGame / QuickPlayGame — not yet implemented on 3DS.
// Must call SetFunctionMode(MENU_MODE) before returning, otherwise the
// MainLoop switch keeps re-entering GAME_MODE / LOADDEMO_MODE forever.
#include "descent.h"
void PlayGame()        { SetFunctionMode(MENU_MODE); }
void QuickPlayGame()   { SetFunctionMode(MENU_MODE); }
void Credits_Display() { /* MainLoop sets Function_mode = MENU_MODE after this */ }
void FreeMultiDLL()    {}

// -----------------------------------------------------------------------
// Players array (referenced by menu.cpp)
// -----------------------------------------------------------------------
#include "player.h"
player Players[MAX_PLAYERS]{};

// -----------------------------------------------------------------------
// ddio keyboard-to-ASCII (used by grfont key handling)
// -----------------------------------------------------------------------
int ddio_KeyToAscii(int key) {
    // Very minimal map — just printable ASCII range
    if (key >= 0x02 && key <= 0x0D) return '1' + (key - 0x02); // 1-0 row
    if (key >= 0x10 && key <= 0x19) return "qwertyuiop"[key - 0x10];
    if (key >= 0x1E && key <= 0x26) return "asdfghjkl"[key - 0x1E];
    if (key >= 0x2C && key <= 0x32) return "zxcvbnm"[key - 0x2C];
    if (key == 0x39) return ' ';
    return 0;
}
