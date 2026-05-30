// 3DS renderer stub — returns safe no-ops for all renderer calls.
// Real citro3d rendering to be wired in once the platform layer is solid.

#include <stdint.h>
#include <stddef.h>
#include <filesystem>
#include "d3movie.h"    // mve API
#include "renderer.h"   // renderer_type etc.

// Renderer init / shutdown
int  rend_Init(int, void *, int *) { return 1; }
void rend_Close() {}

// Frame control
void rend_StartFrame(int, int, int, int) {}
void rend_EndFrame() {}
void rend_Flip() {}

// State
void rend_SetZBufferState(int) {}
void rend_SetAlphaType(int) {}
void rend_SetAlphaValue(uint8_t) {}
void rend_SetFlatColor(uint32_t) {}
void rend_SetTextureType(int) {}
void rend_SetColorModel(int) {}
void rend_SetLighting(int) {}
void rend_SetOverlayType(int) {}

// Drawing — all no-ops for now
void rend_DrawPolygon3D(int, void *, int, int) {}
void rend_DrawFlatPolygon3D(void *, int) {}
void rend_DrawScaledBitmap(int, int, int, int, int, float) {}

// ---------------------------------------------------------------------------
// MVE movie library stubs (libmve aliased to platform3ds)
// ---------------------------------------------------------------------------
int  mve_Init()  { return MVELIB_NOERROR; }
int  mve_PlayMovie(const std::filesystem::path &, oeApplication *) { return 0; }
intptr_t mve_SequenceStart(const char *, void *, oeApplication *, bool) { return 0; }
intptr_t mve_SequenceFrame(intptr_t, void *, bool, int *bm_handle) {
    if (bm_handle) *bm_handle = -1;
    return 0;
}
bool mve_SequenceClose(intptr_t, void *) { return true; }
void mve_SetRenderProperties(int16_t, int16_t, int16_t, int16_t, renderer_type, bool) {}
void mve_SetCallback(MovieFrameCallback_fp) {}
void mve_Puts(int16_t, int16_t, ddgr_color, const char *) {}
void mve_ClearRect(int16_t, int16_t, int16_t, int16_t) {}
