// 3DS sound stubs — silent no-ops until ndsp/DSP layer is wired in.
#include "hlsoundlib.h"

// hlsSystem method stubs
hlsSystem::hlsSystem()  { m_f_hls_system_init = 0; m_sounds_played = 0; m_master_volume = 1.0f; m_pause_new = false; }
void hlsSystem::KillSoundLib(bool)      {}
void hlsSystem::BeginSoundFrame(bool)   {}
void hlsSystem::EndSoundFrame()         {}
void hlsSystem::StopAllSounds()         {}
bool hlsSystem::IsSoundPlaying(int)     { return false; }
int  hlsSystem::Play2dSound(int, float, float, uint16_t) { return -1; }
int  hlsSystem::Play2dSound(int, int, float, float, uint16_t) { return -1; }

extern "C" int  Sound_Init(int, int, int) { return 1; }
extern "C" void Sound_Close() {}
extern "C" int  Sound_Play(int, void *, int) { return -1; }
extern "C" void Sound_Stop(int) {}
extern "C" void Sound_SetVolume(int, float) {}
