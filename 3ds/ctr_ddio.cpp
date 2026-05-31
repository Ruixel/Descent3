// 3DS stubs for ddio (device-dependent I/O: input, timing, file helpers).
// Replace stub bodies with real libctru HID calls progressively.

#include <3ds.h>
#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <cstdarg>
#include <functional>
#include <regex>
#include <sys/stat.h>
#include <filesystem>
#include <vector>
#include "crossplat.h"

// ---------------------------------------------------------------------------
// System / init stubs
// ---------------------------------------------------------------------------
#include "ddio.h"  // path/mouse/keyboard API + ddio_init_info via ddio_common.h

bool ddio_Init(ddio_init_info *) { return true; }
void ddio_Close() {}
bool ddio_InternalInit(ddio_init_info *) { return true; }

// ---------------------------------------------------------------------------
// Keyboard / input — mapped from 3DS HID buttons
// ---------------------------------------------------------------------------
#include "ddio_common.h"  // KEY_UP, KEY_DOWN, KEY_ENTER, KEY_ESC, etc.

// Map a 3DS button mask to a D3 key scancode.
// D-pad is handled via mouse cursor movement (ctr_mouse_update), not keys,
// so that menu focus works without needing a pre-selected gadget.
static int ctr_buttons_to_key(u32 down) {
    if (down & KEY_A)      return KEY_ENTER;
    if (down & KEY_B)      return KEY_ESC;
    if (down & KEY_X)      return KEY_SPACEBAR;
    if (down & KEY_START)  return KEY_ESC;
    return 0;
}

// Persistent key state (held this frame)
static int  s_current_key   = 0;
static bool s_key_held      = false;

void ddio_KeyFlush() { s_current_key = 0; s_key_held = false; }

// Called once per UI frame by UISystem — returns the key pressed this frame
int ddio_KeyInKey() {
    hidScanInput();
    u32 down     = hidKeysDown();    // just-pressed
    u32 held     = hidKeysHeld();    // held (for repeat)
    int key = ctr_buttons_to_key(down);
    if (!key) key = 0;  // no repeat for now — add later if menus feel sluggish
    s_current_key = key;
    s_key_held    = (ctr_buttons_to_key(held) == key) && key != 0;
    return key;
}

bool ddio_KeyState(int scancode) {
    u32 held = hidKeysHeld();
    return ctr_buttons_to_key(held) == scancode;
}

bool ddio_GetAdjKeyState(int scancode) {
    return ddio_KeyState(scancode);
}

void ddio_InternalKeyClose() {}
bool ddio_InternalKeyInit(ddio_init_info *) { return true; }
void ddio_InternalKeyFrame() { hidScanInput(); }
void ddio_InternalKeySuspend() {}
void ddio_InternalKeyResume() {}
bool ddio_InternalKeyState(uint8_t) { return false; }
void ddio_InternalResetKey(uint8_t) {}

// ---------------------------------------------------------------------------
// Mouse — emulated via circle pad + touch screen
// UI coords are in D3's 640x480 logical space.
// ---------------------------------------------------------------------------

// Mouse coords stored in D3 logical space (0..639, 0..479).
// UISystem divides raw mouse coords by kDefaultMouseScale=20, so we multiply
// by 20 when returning from ddio_MouseGetState.
static const int MOUSE_SCALE = 20;

// Start the cursor at the first main-menu item position (MMITEM_X=384, MMITEM_Y=175)
// so the player can immediately click without having to move the cursor first.
static int  s_mouse_x  = 384, s_mouse_y  = 175;
static int  s_mouse_lx = 384, s_mouse_ly = 175;
static bool s_btn1_down = false;
static bool s_btn1_event = false;
static bool s_btn1_event_state = false;

// D-pad cursor step (matches mmItem spacing in mmItem.h: items 20px apart)
static const int DPAD_STEP = 20;

// Call once per frame to update mouse state from HID
static void ctr_mouse_update() {
    hidScanInput();

    // Circle pad: smooth analog cursor movement
    circlePosition cp;
    hidCircleRead(&cp);
    const float DEAD = 20.0f, SCALE = 0.06f;
    if (cp.dx > DEAD || cp.dx < -DEAD)
        s_mouse_x += (int)(cp.dx * SCALE);
    if (cp.dy > DEAD || cp.dy < -DEAD)
        s_mouse_y -= (int)(cp.dy * SCALE);  // Y inverted

    // D-pad: snap cursor by one menu-item step per press.
    // This is the primary navigation method for menus.
    u32 down = hidKeysDown();
    if (down & KEY_DUP)    s_mouse_y -= DPAD_STEP;
    if (down & KEY_DDOWN)  s_mouse_y += DPAD_STEP;
    if (down & KEY_DLEFT)  s_mouse_x -= DPAD_STEP;
    if (down & KEY_DRIGHT) s_mouse_x += DPAD_STEP;

    // Clamp to 640x480 logical space
    if (s_mouse_x < 0)   s_mouse_x = 0;
    if (s_mouse_x > 639) s_mouse_x = 639;
    if (s_mouse_y < 0)   s_mouse_y = 0;
    if (s_mouse_y > 479) s_mouse_y = 479;

    // Touch screen → absolute cursor position (320x240 touch → 640x480)
    bool prev_down = s_btn1_down;
    if (hidKeysHeld() & KEY_TOUCH) {
        touchPosition tp;
        hidTouchRead(&tp);
        s_mouse_x   = tp.px * 2;
        s_mouse_y   = tp.py * 2;
        s_btn1_down = true;
    } else {
        s_btn1_down = false;
    }
    // A button acts as left click at current cursor position
    if (hidKeysHeld() & KEY_A) s_btn1_down = true;

    if (s_btn1_down != prev_down) {
        s_btn1_event       = true;
        s_btn1_event_state = s_btn1_down;
    }
}

bool ddio_MouseInit() { s_mouse_x = 384; s_mouse_y = 175; return true; }
void ddio_MouseClose() {}
void ddio_MouseReset() { s_mouse_x = 384; s_mouse_y = 175; s_btn1_down = false; }
void ddio_MouseMode(int) {}
void ddio_MouseQueueFlush() { s_btn1_event = false; }
void ddio_InternalMouseFrame() { ctr_mouse_update(); }
void ddio_InternalMouseSuspend() {}
void ddio_InternalMouseResume() {}
bool ddio_MouseGetGrab() { return false; }
void ddio_MouseSetGrab(bool) {}
int  ddio_MouseGetCaps(int *btn, int *axis) {
    if (btn)  *btn  = 1;
    if (axis) *axis = 2;
    return 1;
}
int  ddio_MouseGetState(int *x, int *y, int *dx, int *dy, int *z, int *dz) {
    ctr_mouse_update();
    // Scale logical coords by MOUSE_SCALE — UISystem divides by kDefaultMouseScale=20
    // to recover the logical position, so we must pre-multiply here.
    if (x)  *x  = s_mouse_x  * MOUSE_SCALE;
    if (y)  *y  = s_mouse_y  * MOUSE_SCALE;
    if (dx) *dx = (s_mouse_x - s_mouse_lx) * MOUSE_SCALE;
    if (dy) *dy = (s_mouse_y - s_mouse_ly) * MOUSE_SCALE;
    if (z)  *z  = 0;  if (dz) *dz = 0;
    s_mouse_lx = s_mouse_x;  s_mouse_ly = s_mouse_y;
    return s_btn1_down ? 1 : 0;
}
bool ddio_MouseGetEvent(int *btn, bool *state) {
    if (!s_btn1_event) return false;
    if (btn)   *btn   = 0;
    if (state) *state = s_btn1_event_state;
    s_btn1_event = false;
    return true;
}
int  ddio_MouseBtnDownCount(int) { return s_btn1_down ? 1 : 0; }
int  ddio_MouseBtnUpCount(int)   { return s_btn1_down ? 0 : 1; }
void ddio_MouseSetLimits(int,int,int,int,int,int) {}
void ddio_MouseGetLimits(int *l,int *t,int *r,int *b,int *zn,int *zx) {
    if(l)*l=0; if(t)*t=0; if(r)*r=640; if(b)*b=480;
    if(zn)*zn=0; if(zx)*zx=0;
}
void ddio_MouseSetVCoords(int, int) {}

// Note: timer_GetTime is a macro in ddio.h that maps to D3::ChronoTimer::GetTime().
// The ChronoTimer implementation is in ddio/chrono_timer.cpp.
// Call D3::ChronoTimer::Initialize() once at startup (done in ctr_platform.cpp).

// ---------------------------------------------------------------------------
// File helpers (used by cfile)
// ---------------------------------------------------------------------------

int ddio_GetFileLength(FILE *filePtr) {
    if (!filePtr) return 0;
    long pos = ftell(filePtr);
    fseek(filePtr, 0, SEEK_END);
    long len = ftell(filePtr);
    fseek(filePtr, pos, SEEK_SET);
    return (int)len;
}

bool ddio_FileDiff(const std::filesystem::path &path1, const std::filesystem::path &path2) {
    struct stat s1{}, s2{};
    if (stat(path1.c_str(), &s1) != 0) return true;
    if (stat(path2.c_str(), &s2) != 0) return true;
    return s1.st_mtime != s2.st_mtime;
}

void ddio_CopyFileTime(const std::filesystem::path &dest, const std::filesystem::path &src) {
    // FAT32 on SD card doesn't support utimes — silently ignore
}

void ddio_InternalClose() {}

// ---------------------------------------------------------------------------
// Path helpers
// ---------------------------------------------------------------------------
void ddio_MakePath(char *newPath, const char *absolutePathHeader, const char *subDir, ...) {
    const char delimiter = '/';
    if (newPath != absolutePathHeader)
        strcpy(newPath, absolutePathHeader);
    int len = strlen(newPath);
    if (len > 0 && newPath[len-1] != delimiter) {
        newPath[len] = delimiter;
        newPath[len+1] = '\0';
    }
    strcat(newPath, subDir);

    va_list args;
    va_start(args, subDir);
    char *extra;
    while ((extra = va_arg(args, char*)) != nullptr) {
        len = strlen(newPath);
        if (newPath[len-1] != delimiter) {
            newPath[len] = delimiter;
            newPath[len+1] = '\0';
        }
        strcat(newPath, extra);
    }
    va_end(args);
}

void ddio_SplitPath(const char *srcPath, char *path, char *filename, char *ext) {
    std::filesystem::path p(srcPath);
    if (path)     strncpy(path,     p.parent_path().string().c_str(), _MAX_PATH - 1);
    if (filename) strncpy(filename, p.stem().string().c_str(),        _MAX_FNAME - 1);
    if (ext)      strncpy(ext,      p.extension().string().c_str(),   _MAX_EXT - 1);
}

// Working directory
void ddio_GetWorkingDir(char *path, int len) {
    if (path && len > 0) strncpy(path, "sdmc:/descent3", len-1);
}
bool ddio_SetWorkingDir(const char *) { return true; }

// Binary / pref paths
std::filesystem::path ddio_GetPrefPath(const char *, const char *) {
    return std::filesystem::path("sdmc:/descent3");
}
std::filesystem::path ddio_GetBasePath() {
    return std::filesystem::path("sdmc:/descent3");
}

bool ddio_GetBinaryPath(char *exec_path, size_t len) {
    if (exec_path && len > 0) strncpy(exec_path, "sdmc:/descent3/Descent3", len-1);
    return true;
}

// File ops
int ddio_DeleteFile(const char *name) { return remove(name); }

int ddio_GetFileSysRoots(char **roots, int max_roots) { return 0; }
std::vector<std::filesystem::path> ddio_GetSysRoots() {
    return { std::filesystem::path("sdmc:/") };
}

void ddio_DoForeachFile(const std::filesystem::path &dir, const std::regex &filter,
                        const std::function<void(std::filesystem::path)> &fn) {
    std::error_code ec;
    if (!std::filesystem::is_directory(dir, ec)) return;
    for (auto &entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec)) continue;
        std::string name = entry.path().filename().string();
        if (std::regex_match(name, filter))
            fn(entry.path());
    }
}

std::filesystem::path ddio_GetTmpFileName(const std::filesystem::path &basedir, const char *prefix) {
    static int counter = 0;
    char name[64];
    snprintf(name, sizeof(name), "%s%d.tmp", prefix ? prefix : "tmp", counter++);
    return basedir / name;
}
