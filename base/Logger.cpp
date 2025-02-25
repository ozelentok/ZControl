#include "Logger.hpp"
#include <iostream>
#include <format>
#include <ostream>

void Logger::log(LogLevel level, const std::string &message, const std::source_location &location) {
  // Not using location.file_name() as it is the full path
  auto line = std::format("[{}:{}:{}:{}]: {}", Logger::_level_letter(level), __FILE_NAME__, location.function_name(),
                          location.line(), message.c_str());
  syslog(level, "%s", line.c_str());
  std::clog << line << std::endl;
}
