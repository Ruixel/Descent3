// 3DS renderer — citro3d-backed 2D path.
// Handles rend_Init/Close, StartFrame/EndFrame, ClearScreen,
// and rend_DrawChunkedBitmap (the only call needed for the loading screen
// and main menu background). The 3D path remains stubbed for now.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <3ds.h>
#include <citro3d.h>

// Descent 3 headers
#include "renderer.h"
#include "bitmap.h"
#include "grdefs.h"
#include "d3movie.h"

// Generated PICA shader binary (built by picasso via CMake)
#include "vshader_2d_shbin.h"   // uint8_t vshader_2d_shbin[], int vshader_2d_shbin_size

// ---------------------------------------------------------------------------
// Screen constants
// The 3DS top screen is physically 400x240 (landscape).
// The PICA framebuffer is allocated rotated: width=240, height=400.
// D3's fixed menu resolution is 640x480 — we scale to fit 400x240.
// ---------------------------------------------------------------------------
#define CTR_TOP_W  400
#define CTR_TOP_H  240
#define CTR_FB_W   240   // citro3d render target dims (rotated)
#define CTR_FB_H   400

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------
namespace {

C3D_RenderTarget *s_top = nullptr;

// Shader
DVLB_s          *s_dvlb    = nullptr;
shaderProgram_s  s_prog;
int              s_proj_loc = -1;  // uniform location for projection matrix

// Vertex layout: pixel-space position (x,y,z,w) + texcoord (u,v,0,0) + colour (r,g,b,a)
struct Vert2D {
    float x, y, z, w;   // pixel coords, z=0 w=1
    float u, v, s, t;   // texcoord, s/t=0
    float r, g, b, a;   // colour tint
};

// Small immediate-mode VBO — 6 verts per quad (2 tris), max 64 quads per flush
static const int MAX_QUADS = 64;
static Vert2D s_vbo_data[MAX_QUADS * 6];
static int    s_vbo_count = 0;

// TEV set up once for "texture colour * vertex colour"
bool s_tev_ready = false;

// Cached clear colour (set by rend_ClearScreen, applied in StartFrame)
u32  s_clear_color = 0x000000FF;  // RGBA8

// Texture cache entry
struct TexEntry {
    int      bm_handle;  // D3 bitmap handle (-1 = free)
    C3D_Tex  tex;
};
static const int TEX_CACHE_SIZE = 256;
static TexEntry  s_tex_cache[TEX_CACHE_SIZE];
static bool      s_cache_init = false;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void init_tex_cache() {
    if (s_cache_init) return;
    for (int i = 0; i < TEX_CACHE_SIZE; i++)
        s_tex_cache[i].bm_handle = -1;
    s_cache_init = true;
}

// Next power of two >= n, minimum 8 (PICA texture minimum)
static uint32_t next_pot(uint32_t n) {
    if (n < 8) return 8;
    n--;
    n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8; n |= n >> 16;
    return n + 1;
}

// Swizzle a linear RGBA8 image to PICA Morton (Z-order) tile format.
// PICA textures must be tiled in 8x8 blocks with a specific Z-order curve.
// tile_order[i] gives the linear offset of the i-th texel in an 8x8 tile.
static const uint8_t kMorton[64] = {
     0,  1,  4,  5, 16, 17, 20, 21,
     2,  3,  6,  7, 18, 19, 22, 23,
     8,  9, 12, 13, 24, 25, 28, 29,
    10, 11, 14, 15, 26, 27, 30, 31,
    32, 33, 36, 37, 48, 49, 52, 53,
    34, 35, 38, 39, 50, 51, 54, 55,
    40, 41, 44, 45, 56, 57, 60, 61,
    42, 43, 46, 47, 58, 59, 62, 63,
};

// Write pixels to dst in tile-major Morton order.
static void swizzle_rgba8(const uint32_t *src, uint32_t *dst,
                           int w_src, int h_src, int tw, int th) {
    memset(dst, 0, tw * th * 4);
    for (int ty = 0; ty < th; ty += 8) {
        for (int tx = 0; tx < tw; tx += 8) {
            for (int k = 0; k < 64; k++) {
                int lx = tx + (kMorton[k] & 7);
                int ly = ty + (kMorton[k] >> 3);
                int src_y = (h_src - 1) - ly;  // flip Y
                uint32_t px = 0;
                if (lx < w_src && src_y >= 0 && src_y < h_src)
                    px = src[src_y * w_src + lx];
                int tile_idx = (ty / 8) * (tw / 8) + (tx / 8);
                dst[tile_idx * 64 + k] = px;
            }
        }
    }
}

// Convert D3's ARGB1555 pixel to RGBA8 (for GPU upload)
// D3 1555: bit15=alpha(1=opaque), bits14-10=R, 9-5=G, 4-0=B
static inline uint32_t argb1555_to_rgba8(uint16_t p) {
    uint8_t a = (p >> 15) ? 0xFF : 0x00;
    uint8_t r = ((p >> 10) & 0x1F) << 3;
    uint8_t g = ((p >>  5) & 0x1F) << 3;
    uint8_t b = ( p        & 0x1F) << 3;
    // RGBA8 as stored in memory (big-endian for PICA): R G B A
    return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | a;
}

// Upload a D3 bitmap handle to a C3D_Tex.
// Returns false on failure.
static bool upload_bitmap(int handle, C3D_Tex *tex) {
    if (handle < 0 || handle >= MAX_BITMAPS) return false;
    bms_bitmap *bm = &GameBitmaps[handle];
    if (!bm->used) { printf("[CTR] upload_bitmap %d: not used\n", handle); return false; }
    if (!bm->data16) { printf("[CTR] upload_bitmap %d: data16 is null\n", handle); return false; }

    int w = bm->width, h = bm->height;
    uint32_t tw = next_pot(w), th = next_pot(h);
    printf("[CTR] upload_bitmap %d: %dx%d -> POT %dx%d\n", handle, w, h, tw, th);

    // Convert ARGB1555 → RGBA8 linear
    uint32_t *linear = (uint32_t *)linearAlloc(tw * th * 4);
    if (!linear) { printf("[CTR] upload_bitmap: linearAlloc failed (%dx%d)\n", tw, th); return false; }
    memset(linear, 0, tw * th * 4);

    uint16_t *src = bm->data16;
    // Temporary staging buffer (not in linear mem)
    uint32_t *stage = (uint32_t *)malloc(w * h * 4);
    if (!stage) { linearFree(linear); return false; }
    for (int i = 0; i < w * h; i++)
        stage[i] = argb1555_to_rgba8(src[i]);

    swizzle_rgba8(stage, linear, w, h, tw, th);
    free(stage);

    if (!C3D_TexInit(tex, (u16)tw, (u16)th, GPU_RGBA8)) {
        linearFree(linear);
        return false;
    }
    C3D_TexSetFilter(tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
    C3D_TexUpload(tex, linear);
    C3D_TexFlush(tex);
    linearFree(linear);
    return true;
}

// Get or upload a texture for a D3 bitmap handle.
// Returns pointer to C3D_Tex, or nullptr on failure.
static C3D_Tex *get_tex(int handle) {
    if (handle < 0) return nullptr;
    // Search cache
    for (int i = 0; i < TEX_CACHE_SIZE; i++) {
        if (s_tex_cache[i].bm_handle == handle)
            return &s_tex_cache[i].tex;
    }
    // Find a free slot
    for (int i = 0; i < TEX_CACHE_SIZE; i++) {
        if (s_tex_cache[i].bm_handle == -1) {
            if (upload_bitmap(handle, &s_tex_cache[i].tex)) {
                s_tex_cache[i].bm_handle = handle;
                return &s_tex_cache[i].tex;
            }
            return nullptr;
        }
    }
    // Cache full — evict slot 0 (simplest strategy for now)
    C3D_TexDelete(&s_tex_cache[0].tex);
    s_tex_cache[0].bm_handle = -1;
    if (upload_bitmap(handle, &s_tex_cache[0].tex)) {
        s_tex_cache[0].bm_handle = handle;
        return &s_tex_cache[0].tex;
    }
    return nullptr;
}

// Flush queued quads for a given texture.
static void flush_quads(C3D_Tex *tex) {
    if (s_vbo_count == 0) return;

    C3D_TexBind(0, tex);

    // Upload vertex data via immediate mode
    C3D_ImmDrawBegin(GPU_TRIANGLES);
    for (int i = 0; i < s_vbo_count; i++) {
        Vert2D &v = s_vbo_data[i];
        C3D_ImmSendAttrib(v.x, v.y, 0.5f, 1.0f);  // position
        C3D_ImmSendAttrib(v.u, v.v, 0.0f, 0.0f);   // texcoord
        C3D_ImmSendAttrib(v.r, v.g, v.b, v.a);      // colour
    }
    C3D_ImmDrawEnd();

    s_vbo_count = 0;
}

// Push a textured quad (two triangles). Coords in screen pixels (0..CTR_TOP_W, 0..CTR_TOP_H).
// Flushes if the buffer is full.
static void push_quad(C3D_Tex *tex,
                      float sx, float sy, float sw, float sh,  // screen pixel rect
                      float u0, float v0, float u1, float v1,  // UV rect
                      float r, float g, float b, float a) {
    if (s_vbo_count + 6 > MAX_QUADS * 6)
        flush_quads(tex);

    // Feed pixel coordinates directly — the projection matrix handles the rest
    float x0 = sx,      y0 = sy;
    float x1 = sx + sw, y1 = sy + sh;

    Vert2D *v = &s_vbo_data[s_vbo_count];
    // Triangle 1
    v[0] = {x0, y0, 0, 1,  u0, v0, 0, 0,  r, g, b, a};
    v[1] = {x1, y0, 0, 1,  u1, v0, 0, 0,  r, g, b, a};
    v[2] = {x1, y1, 0, 1,  u1, v1, 0, 0,  r, g, b, a};
    // Triangle 2
    v[3] = {x0, y0, 0, 1,  u0, v0, 0, 0,  r, g, b, a};
    v[4] = {x1, y1, 0, 1,  u1, v1, 0, 0,  r, g, b, a};
    v[5] = {x0, y1, 0, 1,  u0, v1, 0, 0,  r, g, b, a};
    s_vbo_count += 6;
}

static void setup_tev() {
    if (s_tev_ready) return;
    // TEV stage 0: out = texture_colour * vertex_colour
    C3D_TexEnv *ev = C3D_GetTexEnv(0);
    C3D_TexEnvInit(ev);
    C3D_TexEnvSrc(ev, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(ev, C3D_Both, GPU_MODULATE);
    s_tev_ready = true;
}

} // namespace

// ---------------------------------------------------------------------------
// Public renderer API — called by Descent 3
// ---------------------------------------------------------------------------

renderer_type Renderer_type = RENDERER_OPENGL;  // closest match for D3's logic

// Set to true by ctr_platform_init after C3D_Init succeeds
extern bool g_c3d_ready;

int rend_Init(renderer_type /*type*/, oeApplication * /*app*/,
              renderer_preferred_state * /*pref*/) {
    printf("[CTR] rend_Init\n");

    if (!g_c3d_ready) {
        printf("[CTR] rend_Init: citro3d not initialised — aborting\n");
        return 0;
    }

    init_tex_cache();

    // C3D_Init is called once in ctr_platform_init() — do not call again here.

    // Create top-screen render target (rotated: fb is 240x400)
    s_top = C3D_RenderTargetCreate(CTR_FB_W, CTR_FB_H,
                                    GPU_RB_RGBA8, C3D_DEPTHTYPE(GPU_RB_DEPTH24));
    if (!s_top) {
        printf("[CTR] rend_Init: failed to create render target\n");
        return 0;
    }
    C3D_RenderTargetSetOutput(s_top, GFX_TOP, GFX_LEFT,
                               GX_TRANSFER_FLIP_VERT(0) |
                               GX_TRANSFER_OUT_TILED(0) |
                               GX_TRANSFER_RAW_COPY(0) |
                               GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
                               GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
                               GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

    // Load and compile the 2D vertex shader
    printf("[CTR] rend_Init: parsing shader binary (%u bytes)\n", (u32)vshader_2d_shbin_size);
    s_dvlb = DVLB_ParseFile((u32 *)vshader_2d_shbin, (u32)vshader_2d_shbin_size);
    if (!s_dvlb) { printf("[CTR] rend_Init: DVLB_ParseFile returned null!\n"); return 0; }
    printf("[CTR] rend_Init: DVLB ok, numDVLE=%lu\n", (unsigned long)s_dvlb->numDVLE);

    Result r;
    r = shaderProgramInit(&s_prog);
    printf("[CTR] rend_Init: shaderProgramInit: %ld\n", (long)r);
    r = shaderProgramSetVsh(&s_prog, &s_dvlb->DVLE[0]);
    printf("[CTR] rend_Init: shaderProgramSetVsh: %ld\n", (long)r);
    C3D_BindProgram(&s_prog);
    printf("[CTR] rend_Init: C3D_BindProgram\n");

    // Get projection uniform location
    s_proj_loc = shaderInstanceGetUniformLocation(s_prog.vertexShader, "projection");
    printf("[CTR] rend_Init: projection uniform loc=%d\n", s_proj_loc);

    // Upload orthographic projection: pixel coords (0..400, 0..240) -> clip space
    // The FB is rotated (240x400 internally), so we map:
    //   x: 0..400 -> -1..+1  (along FB height axis)
    //   y: 0..240 -> +1..-1  (along FB width axis, flipped)
    // Matrix is column-major, 4 rows of vec4.
    // ortho: scale x by 2/400, scale y by -2/240, translate -1,+1
    C3D_Mtx proj;
    Mtx_Identity(&proj);
    Mtx_OrthoTilt(&proj, 0.0f, CTR_TOP_W, CTR_TOP_H, 0.0f, -1.0f, 1.0f, true);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, s_proj_loc, &proj);
    printf("[CTR] rend_Init: projection matrix uploaded\n");

    // Attribute layout — all 4 floats per attrib for immediate mode compatibility
    C3D_AttrInfo *ai = C3D_GetAttrInfo();
    AttrInfo_Init(ai);
    AttrInfo_AddLoader(ai, 0, GPU_FLOAT, 4);  // v0: x, y, z, w
    AttrInfo_AddLoader(ai, 1, GPU_FLOAT, 4);  // v1: u, v, s, t
    AttrInfo_AddLoader(ai, 2, GPU_FLOAT, 4);  // v2: r, g, b, a
    printf("[CTR] rend_Init: AttrInfo set\n");
    setup_tev();
    printf("[CTR] rend_Init: TEV set\n");

    // Disable back-face culling for 2D quads
    C3D_CullFace(GPU_CULL_NONE);
    printf("[CTR] rend_Init: CullFace set\n");

    // Depth test off — 2D quads drawn in submission order
    C3D_DepthTest(false, GPU_ALWAYS, GPU_WRITE_COLOR);

    printf("[CTR] rend_Init: OK\n");
    return 1;
}

void rend_Close() {
    printf("[CTR] rend_Close\n");
    // Evict texture cache
    for (int i = 0; i < TEX_CACHE_SIZE; i++) {
        if (s_tex_cache[i].bm_handle != -1) {
            C3D_TexDelete(&s_tex_cache[i].tex);
            s_tex_cache[i].bm_handle = -1;
        }
    }
    if (s_top)  { C3D_RenderTargetDelete(s_top); s_top = nullptr; }
    if (s_dvlb) { DVLB_Free(s_dvlb); s_dvlb = nullptr; }
    shaderProgramFree(&s_prog);
    C3D_Fini();
}

void rend_StartFrame(int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/,
                     int /*clear_flags*/) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C3D_RenderTargetClear(s_top, C3D_CLEAR_ALL, s_clear_color, 0);
    C3D_FrameDrawOn(s_top);

    // Re-upload shader + TEV each frame (citro3d state can drift)
    shaderProgramUse(&s_prog);
    setup_tev();
    s_tev_ready = true;
}

void rend_EndFrame() {
    // Flush any remaining quads (there's no bound texture here — this shouldn't
    // happen outside DrawChunkedBitmap, but guard anyway)
    // Actual flush happens inside rend_DrawChunkedBitmap after each tile.
    C3D_FrameEnd(0);
}

void rend_Flip() {
    // citro3d handles buffer swap in C3D_FrameEnd; nothing to do here.
}

void rend_ClearScreen(ddgr_color color) {
    // D3 ddgr_color is 0xRRGGBB (24-bit).  Convert to RGBA8 for PICA.
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >>  8) & 0xFF;
    uint8_t b =  color        & 0xFF;
    s_clear_color = ((uint32_t)r << 24) | ((uint32_t)g << 16) |
                    ((uint32_t)b << 8)  | 0xFF;
    // Immediate clear if a frame is in progress
    if (s_top)
        C3D_RenderTargetClear(s_top, C3D_CLEAR_COLOR, s_clear_color, 0);
}

// ---------------------------------------------------------------------------
// rend_DrawChunkedBitmap
// A chunked_bitmap is a grid of bm_handles each covering a power-of-two tile.
// We draw each tile as a textured quad, scaling from D3's 640x480 logical
// space to the 3DS top screen (400x240).
// ---------------------------------------------------------------------------
void rend_DrawChunkedBitmap(chunked_bitmap *chunk, int x, int y, uint8_t alpha) {
    if (!chunk || !s_top) return;
    printf("[CTR] DrawChunkedBitmap: %dx%d tiles, pw=%d ph=%d, at (%d,%d)\n",
           chunk->w, chunk->h, chunk->pw, chunk->ph, x, y);

    // --- Sanity test: draw tile 0 in top-left corner using push_quad ---
    {
        int bm0 = chunk->bm_array[0];
        C3D_Tex *tex0 = get_tex(bm0);
        if (tex0) {
            printf("[CTR] Sanity: drawing tile 0 (%dx%d) at (0,0)\n",
                   GameBitmaps[bm0].width, GameBitmaps[bm0].height);
            float tw = GameBitmaps[bm0].width;
            float th = GameBitmaps[bm0].height;
            float u1 = tw / tex0->width;
            float v1 = th / tex0->height;
            push_quad(tex0, 0, 0, tw, th, 0, 0, u1, v1, 1, 1, 1, 1);
            flush_quads(tex0);
        }
    }
    return; // skip normal drawing for now

    float a = alpha / 255.0f;
    // Scale factors from D3's fixed 640x480 to 400x240
    float sx = (float)CTR_TOP_W / 640.0f;
    float sy = (float)CTR_TOP_H / 480.0f;

    // chunk->w and chunk->h are the number of tiles in x and y
    // chunk->pw and chunk->ph are total pixel dimensions
    // Each tile bitmap is chunk->pw/chunk->w wide (approximately — bitmaps are
    // individually sized, so we read the actual dims from GameBitmaps).

    int tile_idx = 0;
    float cur_y = (float)y * sy;

    for (int row = 0; row < chunk->h; row++) {
        float cur_x = (float)x * sx;
        float row_h = 0;

        for (int col = 0; col < chunk->w; col++) {
            int bm = chunk->bm_array[tile_idx++];
            if (bm < 0 || bm >= MAX_BITMAPS || !GameBitmaps[bm].used) {
                cur_x += 64 * sx;  // skip
                continue;
            }
            float tw = GameBitmaps[bm].width  * sx;
            float th = GameBitmaps[bm].height * sy;

            C3D_Tex *tex = get_tex(bm);
            if (tex) {
                // UV: the bitmap may be smaller than the POT texture
                float u1 = (float)GameBitmaps[bm].width  / (float)tex->width;
                float v1 = (float)GameBitmaps[bm].height / (float)tex->height;
                push_quad(tex, cur_x, cur_y, tw, th,
                          0.0f, 0.0f, u1, v1,
                          1.0f, 1.0f, 1.0f, a);
                flush_quads(tex);  // flush immediately — one tex per tile
            }

            cur_x += tw;
            if (th > row_h) row_h = th;
        }
        cur_y += row_h;
    }
}

// ---------------------------------------------------------------------------
// Remaining renderer stubs — not needed for 2D path
// ---------------------------------------------------------------------------
void rend_SetZBufferState(int8_t)       {}
void rend_SetAlphaType(int8_t)          {}
void rend_SetAlphaValue(uint8_t)        {}
void rend_SetFlatColor(ddgr_color)      {}
void rend_SetTextureType(texture_type)  {}
void rend_SetColorModel(color_model)    {}
void rend_SetLighting(light_state)      {}
void rend_SetOverlayType(uint8_t)       {}
void rend_SetZBias(float)               {}
void rend_SetZBufferWriteMask(int)      {}
void rend_SetWrapType(wrap_type)        {}
void rend_SetFiltering(int8_t)          {}
void rend_SetMipState(int8_t)           {}
void rend_SetFogState(int8_t, float)    {}
void rend_SetSpecularColor(float, float, float) {}
void rend_SetRendererType(renderer_type t) { Renderer_type = t; }
void rend_SetGamma(float)               {}
void rend_SetOverlayMap(int)            {}
void rend_SetLightingState(light_state) {}

void rend_GetRenderState(rendering_state *s) { if (s) memset(s, 0, sizeof(*s)); }
bool rend_SetPreferredState(renderer_preferred_state *) { return true; }
bool rend_InitWindowMode() { return true; }
int  rend_InitOpenGLWindow(oeApplication *, renderer_preferred_state *) { return 1; }
void rend_CloseOpenGLWindow()           {}
bool rend_CheckTextureFormat(int, int)  { return true; }
void rend_SetMipBias(float)             {}
void rend_GetStatistics(tRendererStats *s) { if (s) memset(s, 0, sizeof(*s)); }

// 3D drawing — all no-ops until the 3D path is implemented
void rend_DrawPolygon3D(int, void *, int, int)  {}
void rend_DrawFlatPolygon3D(void *, int)        {}
void rend_DrawScaledBitmap(int, int, int, int, int, int, float, int) {}
void rend_DrawSimpleBitmap(int, int, int)       {}
void rend_DrawBitmap(int, int, int, int, int, float, int, float) {}
void rend_DrawRotatedBitmap(int, int, int, float, int, float)    {}
void rend_DrawSpecialBitmap(int, int, int, float, int)           {}
void rend_DrawLightningBolt(void *, int)        {}
void rend_DrawSphere(ddgr_color, int, int, int) {}
void rend_DrawLine(int, int, int, int)          {}
void rend_DrawCircle(int, int, int)             {}
void rend_DrawPixel(int, int, ddgr_color)       {}
void rend_FillRect(ddgr_color, int, int, int, int) {}

// Texture management stubs
void rend_PreUploadTextureToCard(int, int)      {}
void rend_FreePreUploadedTexture(int, int)      {}
void rend_FreeTexture(int)                      {}
void rend_FreeAllTextures()                     {}

// Misc
const char *rend_GetErrorMessage()  { return "CTR renderer error"; }
void rend_SetFrameBufferCopyState(bool) {}
// rend_GetHardwareInformation omitted — tHardwareInformation not available on 3DS
void rend_TransferBufferToScreen(int, int, int, int, int) {}
void rend_SetAnisotropicFilter(float)           {}

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
