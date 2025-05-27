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

#ifndef ROS2_CONSOLE_TUI_NODE__LOG_VIEWER_HPP_
#define ROS2_CONSOLE_TUI_NODE__LOG_VIEWER_HPP_

#include <unordered_map>
#include <vector>

#include "log_viewer_base/log_viewer_base.hpp"
#include "ros2_console_tui_node/key_definitions.hpp"
#include "ros2_console_tui_node/window_context.hpp"

namespace ros2_console_tui_node
{

class LogViewerTui : public log_viewer_base::LogViewerBase
{
public:
  explicit LogViewerTui(const rclcpp::NodeOptions options);
  void spin();

private:
  boost::circular_buffer<rcl_interfaces::msg::Log> display_logs_;
  std::vector<rcl_interfaces::msg::Log> filtered_logs_;
  std::unordered_map<WindowType, WindowContext> windows_;
  log_viewer_base::LogLevel filter_level_;
  std::vector<KeyBindingEntry> keyBindings_;
  std::unordered_map<int, KeyBindingEntry> keyBindingMap_;
  int scroll_offset_ = 0;

  void init_screen();
  void init_key_bindings();

  void create_windows();
  void destroy_windows();
  void refresh_windows();
  void shutdown_screen();

  void load_config();

  bool should_display(log_viewer_base::LogLevel level);
  void set_filter_level(log_viewer_base::LogLevel level);

  void clear_logs();
  void pause_logs();
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
  static void handle_sigwinch(int) {
    resize_requested_.store(true, std::memory_order_relaxed);
  }
  void on_resize();
};

}  // namespace ros2_console_tui_node

#endif  // ROS2_CONSOLE_TUI_NODE__LOG_VIEWER_HPP_
