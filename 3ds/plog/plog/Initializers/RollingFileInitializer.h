#pragma once
#include <plog/Log.h>
namespace plog {
template<typename Formatter>
struct RollingFileAppender {
    explicit RollingFileAppender(const char *) {}
};
} // namespace plog
