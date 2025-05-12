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

#ifndef __ROS2_CONSOLE_TUI_NODE_WINDOW_CONTEXT_HPP__
#define __ROS2_CONSOLE_TUI_NODE_WINDOW_CONTEXT_HPP__

#include <ncurses.h>

#include <string>

namespace ros2_console_tui_node
{

enum class WindowType { Log, Footer, Header, Frame };

struct WindowContext
{
  WINDOW * handle;
  int row, col, height, width;
  bool needs_redraw;
  std::string label;
};

}  // namespace ros2_console_tui_node

#endif  // __ROS2_CONSOLE_TUI_NODE_WINDOW_CONTEXT_HPP__
