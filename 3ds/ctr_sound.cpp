// 3DS sound stubs — silent no-ops until ndsp/DSP layer is wired in.

extern "C" int  Sound_Init(int, int, int) { return 1; }
extern "C" void Sound_Close() {}
extern "C" int  Sound_Play(int, void *, int) { return -1; }
extern "C" void Sound_Stop(int) {}
extern "C" void Sound_SetVolume(int, float) {}
