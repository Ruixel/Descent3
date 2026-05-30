// 3DS stubs for ddebug platform-specific files (lnxdebug / windebug equivalents)
#include <3ds.h>
#include <stdio.h>
#include <cstdarg>

extern "C" {

void debug_break(void) { /* no-op on 3DS */ }
void debug_break_if(int cond) { (void)cond; }

// mono.h stubs (text-mode debug output)
void MonoPrint(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void MonoClear(void) {}
void MonoSetColor(int) {}

} // extern "C"
