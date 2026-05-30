// Stub plog/Log.h for 3DS — all logging is a no-op.
#pragma once
#include <stdio.h>
#include <cstdarg>

namespace plog {

enum Severity { none = 0, fatal, error, warning, info, debug, verbose };

// Minimal record type — supports operator<< and .printf()
struct Record {
    Record& operator<<(const char *s)    { if (s) printf("%s", s); return *this; }
    Record& operator<<(int v)            { printf("%d", v); return *this; }
    Record& operator<<(unsigned v)       { printf("%u", v); return *this; }
    Record& operator<<(long v)           { printf("%ld", v); return *this; }
    Record& operator<<(unsigned long v)  { printf("%lu", v); return *this; }
    Record& operator<<(float v)          { printf("%f", v); return *this; }
    Record& operator<<(double v)         { printf("%f", v); return *this; }
    template<typename T>
    Record& operator<<(const T &)        { return *this; }

    void printf(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
};

// Stub logger — get() returns a static instance, addAppender() is a no-op.
struct Logger {
    void addAppender(void *) {}
    static Logger *instance;
};

inline Severity severityFromString(const char *) { return debug; }

template<int instance = 0>
inline Logger *get() {
    static Logger l;
    return &l;
}

template<int instance = 0>
inline Logger *init(Severity, void *) {
    return get<instance>();
}

struct TxtFormatter {};

} // namespace plog

// PLOG(severity) expands to a temporary Record that supports <<
#define PLOG(severity)          plog::Record()
#define PLOG_IF(sev, cond)      if(cond) plog::Record()

// Convenience macros used throughout the codebase
#define LOG_VERBOSE             PLOG(plog::verbose)
#define LOG_DEBUG               PLOG(plog::debug)
#define LOG_INFO                PLOG(plog::info)
#define LOG_WARNING             PLOG(plog::warning)
#define LOG_ERROR               PLOG(plog::error)
#define LOG_FATAL               PLOG(plog::fatal)

#define LOG_VERBOSE_IF(cond)    PLOG_IF(plog::verbose, cond)
#define LOG_DEBUG_IF(cond)      PLOG_IF(plog::debug,   cond)
#define LOG_INFO_IF(cond)       PLOG_IF(plog::info,    cond)
#define LOG_WARNING_IF(cond)    PLOG_IF(plog::warning, cond)
#define LOG_ERROR_IF(cond)      PLOG_IF(plog::error,   cond)
#define LOG_FATAL_IF(cond)      PLOG_IF(plog::fatal,   cond)
