#include "header.h"

enum LogLevel global_log_level = LOG_LEVEL_ERROR;

void set_log_level(enum LogLevel log_level) {
  global_log_level = log_level;
}

static inline void vlog_msg(const char* fmt, va_list args) {
  vfprintf(stderr, fmt, args);
}

void log_trace(const char* fmt, ...) {
  if (global_log_level >= LOG_LEVEL_TRACE) {
    va_list args;
    va_start(args, fmt);
    vlog_msg(fmt, args);
    va_end(args);
  }
}

void log_info(const char* fmt, ...) {
  if (global_log_level >= LOG_LEVEL_INFO) {
    va_list args;
    va_start(args, fmt);
    vlog_msg(fmt, args);
    va_end(args);
  }
}

void log_error(const char* fmt, ...) {
  if (global_log_level >= LOG_LEVEL_ERROR) {
    va_list args;
    va_start(args, fmt);
    vlog_msg(fmt, args);
    va_end(args);
  }
}
