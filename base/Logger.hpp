#pragma once
#include <string>
#include <source_location>
#include <syslog.h>

enum LogLevel {
  Error = LOG_ERR,
  Warning = LOG_WARNING,
  Notice = LOG_NOTICE,
  Info = LOG_INFO,
  Debug = LOG_DEBUG
};

class Logger {
private:
  Logger() = delete;
  static constexpr char _level_letter(LogLevel level) {
    switch (level) {
    case Error:
      return 'E';
    case Warning:
      return 'W';
    case Notice:
      return 'N';
    case Info:
      return 'I';
    case Debug:
      return 'D';
    default:
      return '?';
    }
  }

public:
  static void log(LogLevel level, const std::string &message,
                  const std::source_location &location = std::source_location::current());
};

#ifndef NDEBUG
#define LOG_D(message)                                                                                                 \
  do {                                                                                                                 \
    Logger::log(LogLevel::Debug, message);                                                                             \
  } while (0)
#else
#define LOG_D(message)
#endif

#define LOG_W(message)                                                                                                 \
  do {                                                                                                                 \
    Logger::log(LogLevel::Warning, message);                                                                           \
  } while (0)
#define LOG_E(message)                                                                                                 \
  do {                                                                                                                 \
    Logger::log(LogLevel::Debug, message);                                                                             \
  } while (0)

#define DTOR_TRY try {
#define DTOR_CATCH                                                                                                     \
  }                                                                                                                    \
  catch (const std::exception &e) {                                                                                    \
    LOG_E(e.what());                                                                                                   \
  }                                                                                                                    \
  catch (...) {                                                                                                        \
    LOG_E("Unknown Error");                                                                                            \
  }
