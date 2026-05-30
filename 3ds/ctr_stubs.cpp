// 3DS stubs for subsystems not yet ported.
// Each stub group is labelled with the originating subsystem.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>

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

// -----------------------------------------------------------------------
// UI stubs — DoMessageBox (newui.h) — use unsigned int to avoid pulling
//             in the full ddgr_color / grtext chain.
// -----------------------------------------------------------------------
typedef uint32_t ddgr_color;  // must match grdefs.h (uint32_t = unsigned long on ARM)
int DoMessageBox(const char *title, const char *msg, int /*type*/,
                 ddgr_color /*title_color*/, ddgr_color /*msg_color*/) {
  printf("[3DS] DoMessageBox: %s -- %s\n", title ? title : "", msg ? msg : "");
  return 0;
}
int DoMessageBoxAdvanced(const char *title, const char *msg, const char * /*btn0*/, int /*key0*/, ...) {
  printf("[3DS] DoMessageBoxAdvanced: %s -- %s\n", title ? title : "", msg ? msg : "");
  return 0;
}

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
