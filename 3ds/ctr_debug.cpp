// 3DS stubs for ddebug platform-specific files (lnxdebug / windebug equivalents)
#include <3ds.h>
#include <stdio.h>
#include <cstdarg>

// C++ functions (must NOT be in extern "C")
int Debug_MessageBox(int type, const char *title, const char *str) {
  printf("[DBG] %s: %s\n", title ? title : "?", str ? str : "?");
  return 0;
}

int Debug_ErrorBox(int type, const char *topstring, const char *title, const char *bottomstring) {
  printf("[ERR] %s | %s | %s\n",
    topstring   ? topstring   : "",
    title       ? title       : "",
    bottomstring ? bottomstring : "");
  return 0;
}

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
