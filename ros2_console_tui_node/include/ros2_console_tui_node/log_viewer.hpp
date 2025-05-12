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

#ifndef __ROS2_CONSOLE_TUI_NODE_LOG_VIEWER_HPP__
#define __ROS2_CONSOLE_TUI_NODE_LOG_VIEWER_HPP__

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

#include "ros2_console_tui_node/key_definitions.hpp"
#include "ros2_console_tui_node/log_types.hpp"
#include "ros2_console_tui_node/window_context.hpp"

namespace ros2_console_tui_node
{

class LogViewer : public rclcpp::Node
{
public:
  explicit LogViewer(const rclcpp::NodeOptions options);
  void spin();

private:
  const size_t BUFF_SIZE;
  boost::circular_buffer<std::pair<LogLevel, std::string>> display_logs_;
  boost::circular_buffer<std::pair<LogLevel, std::string>> pending_logs_;
  std::set<std::string> excluded_names_;

  std::mutex pending_logs_mutex_;

  rclcpp::Subscription<rcl_interfaces::msg::Log>::SharedPtr sub_;

  std::vector<std::pair<LogLevel, std::string>> filtered_logs_;
  std::unordered_map<WindowType, WindowContext> windows_;
  LogLevel filter_level_;
  std::vector<KeyBindingEntry> keyBindings_;
  std::unordered_map<int, KeyBindingEntry> keyBindingMap_;
  int scroll_offset_ = 0;

  void log_callback(const rcl_interfaces::msg::Log::SharedPtr msg);

  void init_screen();
  void init_key_bindings();

  void create_windows();
  void destroy_windows();
  void refresh_windows();
  void shutdown_screen();

  void load_config();

  bool should_display(LogLevel level);
  void set_filter_level(LogLevel level);

  void clear_logs();
  void scroll_up();
  void scroll_down();

  void handle_key(int ch);
  bool handle_status_bar_key(int ch);
  bool handle_scroll_key(int ch);

  void draw_log_window();
  void draw_header();
  void draw_footer();
  void draw_frame();

  void update_log_buffers();

  static std::atomic<bool> resize_requested_;
  static void handle_sigwinch(int);
  void on_resize();
};

}  // namespace ros2_console_tui_node

#endif  // __ROS2_CONSOLE_TUI_NODE_LOG_VIEWER_HPP__
