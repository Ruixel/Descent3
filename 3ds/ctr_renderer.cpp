// 3DS renderer stub — returns safe no-ops for all renderer calls.
// Real citro3d rendering to be wired in once the platform layer is solid.

#include <stdint.h>
#include <stddef.h>

// Renderer init / shutdown
extern "C" int  rend_Init(int, void *, int *) { return 1; }
extern "C" void rend_Close() {}

// Frame control
extern "C" void rend_StartFrame(int, int, int, int) {}
extern "C" void rend_EndFrame() {}
extern "C" void rend_Flip() {}

// State
extern "C" void rend_SetZBufferState(int) {}
extern "C" void rend_SetAlphaType(int) {}
extern "C" void rend_SetAlphaValue(uint8_t) {}
extern "C" void rend_SetFlatColor(uint32_t) {}
extern "C" void rend_SetTextureType(int) {}
extern "C" void rend_SetColorModel(int) {}
extern "C" void rend_SetLighting(int) {}
extern "C" void rend_SetOverlayType(int) {}

// Drawing — all no-ops for now
extern "C" void rend_DrawPolygon3D(int, void *, int, int) {}
extern "C" void rend_DrawFlatPolygon3D(void *, int) {}
extern "C" void rend_DrawScaledBitmap(int, int, int, int, int, float) {}
