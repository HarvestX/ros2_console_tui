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

#ifndef LOG_VIEWER_BASE__LOG_VIEWER_BASE_HPP_
#define LOG_VIEWER_BASE__LOG_VIEWER_BASE_HPP_

#include <ncurses.h>
#include <yaml-cpp/yaml.h>

#include <boost/circular_buffer.hpp>
#include <filesystem>
#include <mutex>
#include <rcl_interfaces/msg/log.hpp>
#include <rclcpp/rclcpp.hpp>
#include <set>
#include <string>
#include <utility>

#include "log_viewer_base/log_types.hpp"

namespace log_viewer_base
{

class LogViewerBase : public rclcpp::Node
{
public:
  explicit LogViewerBase(const rclcpp::NodeOptions & options);
  ~LogViewerBase() = default;
  void log_callback(const rcl_interfaces::msg::Log::SharedPtr msg);
  bool is_paused() const;
  void set_paused_flag(bool paused);
  std::string convert_to_string(const rcl_interfaces::msg::Log & msg);
  bool is_pending_logs_empty() const
  {
    return pending_logs_.empty();
  }
  void clear_pending_logs()
  {
    std::lock_guard<std::mutex> lock(pending_logs_mutex_);
    pending_logs_.clear();
  }
  boost::circular_buffer<rcl_interfaces::msg::Log> get_pending_logs()
  {
    return pending_logs_;
  }

public:
  const size_t BUFF_SIZE;

  std::set<std::string> excluded_names_;

private:
  std::mutex pending_logs_mutex_;
  rclcpp::Subscription<rcl_interfaces::msg::Log>::SharedPtr sub_;
  boost::circular_buffer<rcl_interfaces::msg::Log> pending_logs_;
  bool now_pause_ = false;
};

}  // namespace log_viewer_base

#endif  // LOG_VIEWER_BASE__LOG_VIEWER_BASE_HPP_
