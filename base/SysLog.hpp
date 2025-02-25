#pragma once
#include <syslog.h>

#ifndef NDEBUG
#define SYSLOG_DEBUG(format, ...)                                                                                      \
  do {                                                                                                                 \
    syslog(LOG_DEBUG, format, ##__VA_ARGS__);                                                                          \
  } while (0)
#else
#define SYSLOG_DEBUG(format, ...)
#endif

#define SYSLOG_WARNING(format, ...)                                                                                    \
  do {                                                                                                                 \
    syslog(LOG_WARNING, format, ##__VA_ARGS__);                                                                        \
  } while (0)
#define SYSLOG_ERROR(format, ...)                                                                                      \
  do {                                                                                                                 \
    syslog(LOG_ERR, format, ##__VA_ARGS__);                                                                            \
  } while (0)

#define CATCH_ALL_ERROR_HANDLER                                                                                        \
  catch (const std::exception &e) {                                                                                    \
    SYSLOG_ERROR("Error on %s:%d: %s", __func__, __LINE__, e.what());                                                  \
  }                                                                                                                    \
  catch (...) {                                                                                                        \
    SYSLOG_ERROR("Unknown Error on %s:%d", __func__, __LINE__);                                                        \
  }
