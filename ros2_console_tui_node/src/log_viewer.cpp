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

#include <fstream>
#include "ros2_console_tui_node/log_viewer.hpp"

namespace ros2_console_tui_node
{
std::atomic<bool> LogViewerTui::resize_requested_{false};

LogViewerTui::LogViewerTui(const rclcpp::NodeOptions options)
: LogViewerBase(options),
  display_logs_(BUFF_SIZE)
{
  filtered_logs_.reserve(BUFF_SIZE);
  load_config();
}

void LogViewerTui::load_config()
{
  try {
    std::filesystem::path config_path =
      std::filesystem::path(std::getenv("HOME")) / ".ros2_console_tui/config.yaml";

    if (!std::filesystem::exists(config_path)) {
      std::filesystem::create_directories(config_path.parent_path());
      std::ofstream out(config_path);
      out << "excluded_names: []\n";
      out.close();
      std::cout << "Created default config at: " << config_path << std::endl;
      return;
    }

    YAML::Node config = YAML::LoadFile(config_path.string());
    if (config["excluded_names"]) {
      for (const auto & name : config["excluded_names"]) {
        excluded_names_.insert(name.as<std::string>());
      }
    }
  } catch (const std::exception & e) {
    std::cerr << "Failed to load config: " << e.what() << std::endl;
  }
}


void LogViewerTui::spin()
{
  init_screen();
  init_key_bindings();
  create_windows();

  std::signal(SIGWINCH, handle_sigwinch);

  while (rclcpp::ok()) {
    if (resize_requested_.load(std::memory_order_relaxed)) {
      resize_requested_.store(false, std::memory_order_relaxed);
      on_resize();
    }

    int ch = getch();
    if (ch != ERR) {handle_key(ch);}

    update_log_buffers();
    draw_header();
    draw_footer();
    draw_frame();
    draw_log_window();
    refresh_windows();
  }

  destroy_windows();
  shutdown_screen();
}

void LogViewerTui::init_screen()
{
  initscr();
  start_color();
  use_default_colors();
  curs_set(0);
  noecho();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);

  init_pair(1, COLOR_CYAN, -1);     // DEBUG
  init_pair(2, COLOR_WHITE, -1);    // INFO
  init_pair(3, COLOR_YELLOW, -1);   // WARN
  init_pair(4, COLOR_RED, -1);      // ERROR
  init_pair(5, COLOR_MAGENTA, -1);  // FATAL
  init_pair(6, COLOR_GREEN, -1);    // STATUS BAR
}

void LogViewerTui::create_windows()
{
  const auto [rows, cols] = std::pair{LINES, COLS};
  auto make_window_context = [](WINDOW * win, const std::string & label) {
      int y, x, h, w;
      getbegyx(win, y, x);
      getmaxyx(win, h, w);
      return WindowContext{
      .handle = win,
      .row = y,
      .col = x,
      .height = h,
      .width = w,
      .needs_redraw = true,
      .label = label};
    };

  windows_[WindowType::Header] = make_window_context(newwin(1, cols, 0, 0), "Header");
  windows_[WindowType::Footer] = make_window_context(newwin(1, cols, rows - 1, 0), "Footer");

  WINDOW * frame_win = newwin(rows - 2, cols, 1, 0);
  windows_[WindowType::Frame] = make_window_context(frame_win, "Frame");

  WINDOW * log_win = subwin(frame_win, rows - 4, cols - 2, 2, 1);
  windows_[WindowType::Log] = make_window_context(log_win, "Log_Viewer");
}

void LogViewerTui::destroy_windows()
{
  for (auto & [_, ctx] : windows_) {
    if (ctx.handle) {
      delwin(ctx.handle);
      ctx.handle = nullptr;
    }
  }
  windows_.clear();
}

void LogViewerTui::refresh_windows()
{
  for (auto & [_, ctx] : windows_) {
    if (ctx.needs_redraw) {
      wnoutrefresh(ctx.handle);
      ctx.needs_redraw = false;
    }
  }
  doupdate();
}

void LogViewerTui::shutdown_screen()
{
  endwin();
  curs_set(1);
}

void LogViewerTui::draw_frame()
{
  auto & ctx = windows_.at(WindowType::Frame);
  WINDOW * win = ctx.handle;
  box(win, 0, 0);
}

void LogViewerTui::draw_log_window()
{
  auto & ctx = windows_.at(WindowType::Log);
  WINDOW * win = ctx.handle;
  werase(win);

  int row = 0;
  int lines_available = ctx.height;

  int max_offset = std::max(0, static_cast<int>(filtered_logs_.size()) - lines_available);
  scroll_offset_ = std::min(scroll_offset_, max_offset);
  int start_index =
    std::max(0, static_cast<int>(filtered_logs_.size()) - lines_available - scroll_offset_);

  for (size_t i = start_index; i < filtered_logs_.size() && row < ctx.height; ++i) {
    const auto & entry = filtered_logs_[i];
    const auto & line = convert_to_string(entry);
    if (line.empty()) {continue;}
    int color_pair = get_color_pair(static_cast<log_viewer_base::LogLevel>(entry.level));
    wattron(win, COLOR_PAIR(color_pair));
    mvwprintw(win, row++, 0, "%s", line.c_str());
    wattroff(win, COLOR_PAIR(color_pair));
  }
}

void LogViewerTui::draw_header()
{
  auto & ctx = windows_.at(WindowType::Header);
  WINDOW * win = ctx.handle;
  werase(win);
  wattron(win, A_BOLD);

  const std::string title = "ROS 2 Console TUI - Log Viewer";

  size_t total_logs = 0;
  for (const auto & entry : display_logs_) {
    if (should_display(static_cast<log_viewer_base::LogLevel>(entry.level))) {
      ++total_logs;
    }
  }

  std::string pause_label = "Pause | ";
  std::string filter_label = "Filter: ";
  if (is_paused()) {
    filter_label = pause_label + filter_label;
  }
  std::string filter_name = level_to_string(filter_level_);
  std::ostringstream scroll_stream;
  scroll_stream << " | Scroll: " << scroll_offset_ << "/" << total_logs;
  std::string scroll_info = scroll_stream.str();

  int state_x =
    ctx.width - static_cast<int>(filter_label.size() + filter_name.size() + scroll_info.size()) - 1;
  if (state_x > static_cast<int>(title.size()) + 2) {
    mvwprintw(win, 0, state_x, "%s", filter_label.c_str());

    int color_pair = log_viewer_base::get_color_pair(filter_level_);
    wattron(win, COLOR_PAIR(color_pair));
    wprintw(win, "%s", filter_name.c_str());
    wattroff(win, COLOR_PAIR(color_pair));

    wprintw(win, "%s", scroll_info.c_str());
  }

  mvwprintw(win, 0, 0, "%s", title.c_str());

  wattroff(win, A_BOLD);
}

void LogViewerTui::draw_footer()
{
  auto & ctx = windows_.at(WindowType::Footer);
  WINDOW * win = ctx.handle;
  werase(win);
  wattron(win, A_BOLD);

  int x = 0;

  for (const auto & entry : keyBindings_) {
    if (!entry.showInStatusBar || entry.label.empty()) {continue;}
    mvwprintw(win, 0, x, "%s", entry.label.c_str());
    x += entry.label.size() + 1;
  }

  wattroff(win, A_BOLD);
}

void LogViewerTui::update_log_buffers()
{
  if (is_pending_logs_empty()) {
    return;
  }
  windows_.at(WindowType::Header).needs_redraw = true;
  windows_.at(WindowType::Log).needs_redraw = true;

  auto logs = get_pending_logs();
  display_logs_.insert(
    display_logs_.end(), logs.begin(), logs.end());
  clear_pending_logs();

  filtered_logs_.clear();
  for (const auto & entry : display_logs_) {
    if (should_display(static_cast<log_viewer_base::LogLevel>(entry.level))) {
      filtered_logs_.push_back(entry);
    }
  }
}

bool LogViewerTui::should_display(log_viewer_base::LogLevel level)
{
  if (filter_level_ == log_viewer_base::LogLevel::ALL) {return true;}
  return level == filter_level_;
}

void LogViewerTui::set_filter_level(log_viewer_base::LogLevel level)
{
  if (filter_level_ != level) {
    filter_level_ = level;
    windows_.at(WindowType::Header).needs_redraw = true;
    windows_.at(WindowType::Footer).needs_redraw = true;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::clear_logs()
{
  display_logs_.clear();
  windows_.at(WindowType::Header).needs_redraw = true;
  windows_.at(WindowType::Log).needs_redraw = true;
}

void LogViewerTui::pause_logs()
{
  set_paused_flag(!is_paused());
  windows_.at(WindowType::Header).needs_redraw = true;
  windows_.at(WindowType::Log).needs_redraw = true;
}

void LogViewerTui::scroll_up()
{
  const int max_offset = static_cast<int>(display_logs_.size());
  if (scroll_offset_ < max_offset) {
    scroll_offset_++;
    windows_.at(WindowType::Header).needs_redraw = true;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::scroll_down()
{
  if (scroll_offset_ > 0) {
    scroll_offset_--;
    windows_.at(WindowType::Header).needs_redraw = true;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::handle_key(int ch)
{
  auto it = keyBindingMap_.find(ch);
  if (it != keyBindingMap_.end()) {
    it->second.action();
  }
}

void LogViewerTui::init_key_bindings()
{
  keyBindings_ = {
    {Key::LowercaseA, "[a] All", [this]() {set_filter_level(log_viewer_base::LogLevel::ALL);}},
    {Key::LowercaseD, "[d] Debug", [this]() {set_filter_level(log_viewer_base::LogLevel::DEBUG);}},
    {Key::LowercaseI, "[i] Info", [this]() {set_filter_level(log_viewer_base::LogLevel::INFO);}},
    {Key::LowercaseW, "[w] Warn", [this]() {set_filter_level(log_viewer_base::LogLevel::WARN);}},
    {Key::LowercaseE, "[e] Error", [this]() {set_filter_level(log_viewer_base::LogLevel::ERROR);}},
    {Key::LowercaseF, "[f] Fatal", [this]() {set_filter_level(log_viewer_base::LogLevel::FATAL);}},
    {Key::LowercaseP, "[p] Pause", [this]() {pause_logs();}},
    {Key::LowercaseC, "[c] Clear", [this]() {clear_logs();}},
    {Key::LowercaseQ, "[q] Quit", [this]() {rclcpp::shutdown();}},
    {Key::LowercaseK, "Scroll Up", [this]() {scroll_up();}, false},
    {Key::LowercaseJ, "Scroll Down", [this]() {scroll_down();}, false},
    {Key::Up, "Scroll Up", [this]() {scroll_up();}, false},
    {Key::Down, "Scroll Down", [this]() {scroll_down();}, false}};

  keyBindingMap_.clear();
  for (const auto & entry : keyBindings_) {
    keyBindingMap_[to_keycode(entry.key)] = entry;
  }
}

// static void LogViewerTui::handle_sigwinch(int)
// {
//   resize_requested_.store(true, std::memory_order_relaxed);
// }

void LogViewerTui::on_resize()
{
  endwin();
  refresh();
  clear();

  destroy_windows();
  create_windows();
}

}  // namespace ros2_console_tui_node

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;

  auto node = std::make_shared<ros2_console_tui_node::LogViewerTui>(options);

  std::thread spin_thread([&]() {rclcpp::spin(node);});

  try {
    node->spin();
  } catch (const std::exception & e) {
    endwin();
    curs_set(1);
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  rclcpp::shutdown();
  return 0;
}
