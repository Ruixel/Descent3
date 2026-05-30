// 3DS stubs for ddio (device-dependent I/O: input, timing, file helpers).
// Replace stub bodies with real libctru HID calls progressively.

#include <3ds.h>
#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <sys/stat.h>
#include <filesystem>
#include "crossplat.h"

// ---------------------------------------------------------------------------
// Keyboard / input stubs
// ---------------------------------------------------------------------------
extern "C" void ddio_KeyFlush() {}
extern "C" int  ddio_KeyInKey() { return 0; }
extern "C" bool ddio_KeyState(int) { return false; }

// ---------------------------------------------------------------------------
// Mouse stubs (no mouse on 3DS; touchscreen handled separately later)
// ---------------------------------------------------------------------------
extern "C" void ddio_MouseReset() {}
extern "C" void ddio_MouseGetState(int *, int *, int *, int *) {}

// ---------------------------------------------------------------------------
// Timer
// ---------------------------------------------------------------------------
extern "C" double timer_GetTime()
{
    // osGetTime() returns milliseconds since epoch as u64
    return (double)osGetTime() / 1000.0;
}

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
void ddio_SplitPath(const char *path, char *drive, char *dir, char *file, char *ext) {
    std::filesystem::path p(path);
    if (drive) drive[0] = '\0';
    if (dir)  strncpy(dir,  p.parent_path().c_str(), _MAX_PATH - 1);
    if (file) strncpy(file, p.stem().c_str(),        _MAX_FNAME - 1);
    if (ext)  strncpy(ext,  p.extension().c_str(),   _MAX_EXT - 1);
}
