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
// Keyboard / input stubs
// ---------------------------------------------------------------------------
void ddio_KeyFlush() {}
int  ddio_KeyInKey() { return 0; }
bool ddio_KeyState(int) { return false; }
void ddio_InternalKeyClose() {}
bool ddio_InternalKeyInit(ddio_init_info *) { return true; }
void ddio_InternalKeyFrame() {}
void ddio_InternalKeySuspend() {}
void ddio_InternalKeyResume() {}
bool ddio_InternalKeyState(uint8_t) { return false; }
void ddio_InternalResetKey(uint8_t) {}

// ---------------------------------------------------------------------------
// Mouse stubs (no mouse on 3DS; touchscreen handled separately later)
// ---------------------------------------------------------------------------
bool ddio_MouseInit() { return true; }
void ddio_MouseClose() {}
void ddio_MouseReset() {}
void ddio_MouseMode(int) {}
void ddio_MouseQueueFlush() {}
void ddio_InternalMouseFrame() {}
void ddio_InternalMouseSuspend() {}
void ddio_InternalMouseResume() {}
bool ddio_MouseGetGrab() { return false; }
void ddio_MouseSetGrab(bool) {}
int  ddio_MouseGetCaps(int *btn, int *axis) {
    if (btn)  *btn  = 0;
    if (axis) *axis = 0;
    return 0;
}
int  ddio_MouseGetState(int *x, int *y, int *dx, int *dy, int *z, int *dz) {
    if (x)  *x  = 0; if (y)  *y  = 0;
    if (dx) *dx = 0; if (dy) *dy = 0;
    if (z)  *z  = 0; if (dz) *dz = 0;
    return 0;
}
bool ddio_MouseGetEvent(int *btn, bool *state) { return false; }
int  ddio_MouseBtnDownCount(int) { return 0; }
int  ddio_MouseBtnUpCount(int) { return 0; }
void ddio_MouseSetLimits(int,int,int,int,int,int) {}
void ddio_MouseGetLimits(int *l,int *t,int *r,int *b,int *zn,int *zx) {
    if(l)*l=0; if(t)*t=0; if(r)*r=400; if(b)*b=240;
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

void ddio_DoForeachFile(const std::filesystem::path &, const std::regex &,
                        const std::function<void(std::filesystem::path)> &) {}

std::filesystem::path ddio_GetTmpFileName(const std::filesystem::path &basedir, const char *prefix) {
    static int counter = 0;
    char name[64];
    snprintf(name, sizeof(name), "%s%d.tmp", prefix ? prefix : "tmp", counter++);
    return basedir / name;
}
