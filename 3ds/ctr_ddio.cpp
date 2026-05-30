// 3DS stubs for ddio (device-dependent I/O: input, timing, file helpers).
// Replace stub bodies with real libctru HID calls progressively.

#include <3ds.h>
#include <stdint.h>

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
