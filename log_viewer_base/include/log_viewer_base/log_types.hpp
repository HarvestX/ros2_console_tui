// Copyright 2025 HarvestX Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LOG_VIEWER_BASE__LOG_TYPES_HPP_
#define LOG_VIEWER_BASE__LOG_TYPES_HPP_

#include <cstdint>
#include <string>
#include <unordered_map>

namespace log_viewer_base
{

enum class LogLevel : uint8_t { ALL = 0, DEBUG = 10, INFO = 20, WARN = 30, ERROR = 40, FATAL = 50 };

enum ColorPair
{
  COLOR_DEBUG = 1,
  COLOR_INFO = 2,
  COLOR_WARN = 3,
  COLOR_ERROR = 4,
  COLOR_FATAL = 5,
};

const std::unordered_map<LogLevel, int> kLogLevelToColor = {
  {LogLevel::DEBUG, COLOR_DEBUG}, {LogLevel::INFO, COLOR_INFO}, {LogLevel::WARN, COLOR_WARN},
  {LogLevel::ERROR, COLOR_ERROR}, {LogLevel::FATAL, COLOR_FATAL},
};

const std::unordered_map<LogLevel, std::string> kLogLevelToString = {
  {LogLevel::ALL, "ALL"}, {LogLevel::DEBUG, "DEBUG"}, {LogLevel::INFO, "INFO"},
  {LogLevel::WARN, "WARN"}, {LogLevel::ERROR, "ERROR"}, {LogLevel::FATAL, "FATAL"}};

inline std::string level_to_string(LogLevel level)
{
  auto it = kLogLevelToString.find(level);
  return (it != kLogLevelToString.end()) ? it->second : "UNKNOWN";
}

inline int get_color_pair(LogLevel level)
{
  auto it = kLogLevelToColor.find(level);
  return (it != kLogLevelToColor.end()) ? it->second : COLOR_INFO;
}

}  // namespace log_viewer_base

#endif  // LOG_VIEWER_BASE__LOG_TYPES_HPP_
