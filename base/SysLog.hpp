#pragma once
#include <syslog.h>

#ifndef NDEBUG
#define SYSLOG_DEBUG(format, ...) do { syslog(LOG_DEBUG, format, ##__VA_ARGS__); } while(0)
#else
#define SYSLOG_DEBUG(format, ...)
#endif

#define SYSLOG_WARNING(format, ...) do { syslog(LOG_WARNING, format, ##__VA_ARGS__); } while(0)
#define SYSLOG_ERROR(format, ...) do { syslog(LOG_ERR, format, ##__VA_ARGS__); } while(0)
