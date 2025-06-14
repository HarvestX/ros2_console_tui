#include "ros2_console_tui_node/log_viewer.hpp"

namespace ros2_console_tui_node
{

void LogViewerTui::draw_node_window()
{
  auto & ctx = windows_.at(WindowType::Log);
  WINDOW * win = ctx.handle;
  werase(win);

  auto nodes = get_nodes();
  auto topics = get_topics();
  auto services = get_services();

  int max_lines = ctx.height;
  int current_line = 0;
  int lines_displayed = 0;

  int total_lines = 0;
  
  total_lines += 2;
  total_lines += nodes.size() > 0 ? nodes.size() : 1;
  total_lines += 1;
  
  total_lines += 2;
  for (const auto& pair : topics) {
    total_lines += 1;
    total_lines += pair.second.size();
  }
  if (topics.empty()) {
    total_lines += 1;
  }
  total_lines += 1;
  
  total_lines += 2;
  for (const auto& pair : services) {
    total_lines += 1;
    total_lines += pair.second.size();
  }
  if (services.empty()) {
    total_lines += 1;
  }
  total_lines += 1;
  
  total_lines += 2;
  for (const auto& node : nodes) {
    total_lines += 2;
    
    total_lines += 1;
    if (node.publishers.empty()) {
      total_lines += 1;
    } else {
      for (const auto& pub : node.publishers) {
        (void)pub;
        total_lines += 1;
      }
    }
    
    total_lines += 1;
    if (node.subscribers.empty()) {
      total_lines += 1;
    } else {
      for (const auto& sub : node.subscribers) {
        (void)sub;
        total_lines += 1;
      }
    }
    
    total_lines += 1;
  }
  if (nodes.empty()) {
    total_lines += 1;
  }

  int max_offset = std::max(0, total_lines - max_lines);
  node_scroll_offset_ = std::min(node_scroll_offset_, max_offset);
  int skip_lines = node_scroll_offset_;

  auto draw_line = [&](const std::string& line, int color = 0) {
    if (skip_lines > 0) {
      skip_lines--;
      return;
    }
    if (current_line >= max_lines) {
      return;
    }
    
    if (color > 0) {
      wattron(win, COLOR_PAIR(color));
    }
    mvwprintw(win, current_line, 0, "%s", line.c_str());
    if (color > 0) {
      wattroff(win, COLOR_PAIR(color));
    }
    
    current_line++;
    lines_displayed++;
  };

  draw_line("=== Active Nodes ===", 6);
  draw_line("");
  
  if (nodes.empty()) {
    draw_line("  No active nodes found.");
  } else {
    for (const auto& node : nodes) {
      draw_line("  Node: " + node.full_name, 2);
    }
  }
  draw_line("");
  
  draw_line("=== All Topics and Types ===", 6);
  draw_line("");
  
  if (topics.empty()) {
    draw_line("  No active topics found.");
  } else {
    for (const auto& pair : topics) {
      draw_line("  Topic: " + pair.first);
      for (const auto& type : pair.second) {
        draw_line("    Type: " + type);
      }
    }
  }
  draw_line("");
  
  draw_line("=== All Services and Types ===", 6);
  draw_line("");
  
  if (services.empty()) {
    draw_line("  No active services found.");
  } else {
    for (const auto& pair : services) {
      draw_line("  Service: " + pair.first);
      for (const auto& type : pair.second) {
        draw_line("    Type: " + type);
      }
    }
  }
  draw_line("");
  
  draw_line("=== Topic Connections by Node ===", 6);
  draw_line("");
  
  if (nodes.empty()) {
    draw_line("  No active nodes to inspect connections.");
  } else {
    for (const auto& node : nodes) {
      draw_line("Node: " + node.full_name, 2);
      
      if (node.publishers.empty()) {
        draw_line("  No topics being published.");
      } else {
        draw_line("  Published topics:");
        for (const auto& topic_pair : node.publishers) {
          std::string types_str = "";
          for (size_t i = 0; i < topic_pair.second.size(); ++i) {
            types_str += topic_pair.second[i] + (i == topic_pair.second.size() - 1 ? "" : ", ");
          }
          draw_line("    - " + topic_pair.first + " (Type: " + types_str + ")");
        }
      }
      
      if (node.subscribers.empty()) {
        draw_line("  No topics being subscribed to.");
      } else {
        draw_line("  Subscribed topics:");
        for (const auto& topic_pair : node.subscribers) {
          std::string types_str = "";
          for (size_t i = 0; i < topic_pair.second.size(); ++i) {
            types_str += topic_pair.second[i] + (i == topic_pair.second.size() - 1 ? "" : ", ");
          }
          draw_line("    - " + topic_pair.first + " (Type: " + types_str + ")");
        }
      }
      
      draw_line("");
    }
  }
}

void LogViewerTui::switch_to_log_mode()
{
  if (current_display_mode_ != DisplayMode::Log) {
    current_display_mode_ = DisplayMode::Log;
    windows_.at(WindowType::Header).needs_redraw = true;
    windows_.at(WindowType::Footer).needs_redraw = true;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::switch_to_node_mode()
{
  if (current_display_mode_ != DisplayMode::Node) {
    current_display_mode_ = DisplayMode::Node;
    node_scroll_offset_ = 0;
    update_graph();
    last_graph_update_ = std::chrono::steady_clock::now();
    windows_.at(WindowType::Header).needs_redraw = true;
    windows_.at(WindowType::Footer).needs_redraw = true;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::switch_to_node_select_mode()
{
  if (current_display_mode_ != DisplayMode::NodeSelect) {
    current_display_mode_ = DisplayMode::NodeSelect;
    node_scroll_offset_ = 0;
    selected_node_idx_ = 0;
    
    update_graph();
    
    auto nodes = get_nodes();
    if (selected_nodes_.size() != nodes.size()) {
      selected_nodes_.resize(nodes.size(), false);
    }
    
    windows_.at(WindowType::Header).needs_redraw = true;
    windows_.at(WindowType::Footer).needs_redraw = true;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::draw_node_select_window()
{
  auto & ctx = windows_.at(WindowType::Log);
  WINDOW * win = ctx.handle;
  werase(win);

  auto nodes = get_nodes();

  int max_lines = ctx.height;
  int current_line = 0;

  if (nodes.empty()) {
    selected_node_idx_ = 0;
  } else {
    selected_node_idx_ = std::min(selected_node_idx_, static_cast<int>(nodes.size()) - 1);
  }

  if (selected_node_idx_ < node_scroll_offset_) {
    node_scroll_offset_ = selected_node_idx_;
  } else if (selected_node_idx_ >= node_scroll_offset_ + max_lines) {
    node_scroll_offset_ = selected_node_idx_ - max_lines + 1;
  }

  wattron(win, A_BOLD);
  mvwprintw(win, current_line++, 0, "Node Selection (Use UP/DOWN to navigate, ENTER to toggle, L to return)");
  wattroff(win, A_BOLD);
  
  wattron(win, COLOR_PAIR(2));
  mvwprintw(win, current_line++, 0, "Selected nodes will be used to filter log messages.");
  wattroff(win, COLOR_PAIR(2));
  
  current_line++;

  if (selected_nodes_.size() != nodes.size()) {
    selected_nodes_.resize(nodes.size(), false);
  }

  for (size_t i = node_scroll_offset_; 
       i < nodes.size() && (current_line - 3) < max_lines; 
       ++i) {
    const auto & node = nodes[i];
    
    if (static_cast<int>(i) == selected_node_idx_) {
      wattron(win, A_REVERSE);
    }
    
    std::string prefix = selected_nodes_[i] ? "[x] " : "[ ] ";
    
    mvwprintw(win, current_line, 0, "%s%s", 
              prefix.c_str(), 
              node.full_name.c_str());
    
    if (static_cast<int>(i) == selected_node_idx_) {
      wattroff(win, A_REVERSE);
    }
    
    current_line++;
  }
  
  if (nodes.empty()) {
    wattron(win, COLOR_PAIR(3));
    mvwprintw(win, current_line++, 0, "No nodes found in the ROS graph.");
    wattroff(win, COLOR_PAIR(3));
  }
}

void LogViewerTui::select_next_node()
{
  auto nodes = get_nodes();
  if (!nodes.empty() && selected_node_idx_ < static_cast<int>(nodes.size()) - 1) {
    selected_node_idx_++;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::select_prev_node()
{
  if (selected_node_idx_ > 0) {
    selected_node_idx_--;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::toggle_selected_node()
{
  auto nodes = get_nodes();
  if (!nodes.empty() && selected_node_idx_ >= 0 && 
      selected_node_idx_ < static_cast<int>(nodes.size())) {
    selected_nodes_[selected_node_idx_] = !selected_nodes_[selected_node_idx_];
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

void LogViewerTui::apply_node_filter()
{
  auto nodes = get_nodes();
  
  bool any_selected = false;
  for (const auto& selected : selected_nodes_) {
    if (selected) {
      any_selected = true;
      break;
    }
  }
  
  node_filter_active_ = any_selected;
  
  filtered_logs_.clear();
  for (const auto & entry : display_logs_) {
    if (should_display(static_cast<log_viewer_base::LogLevel>(entry.level)) &&
        (!node_filter_active_ || should_display_from_node(entry.name))) {
      filtered_logs_.push_back(entry);
    }
  }
  
  windows_.at(WindowType::Header).needs_redraw = true;
  windows_.at(WindowType::Log).needs_redraw = true;
}

bool LogViewerTui::should_display_from_node(const std::string& node_name)
{
  if (!node_filter_active_) {
    return true;
  }
  
  auto nodes = get_nodes();
  for (size_t i = 0; i < nodes.size() && i < selected_nodes_.size(); ++i) {
    auto node_name_copy = "/" + node_name;
    std::replace(node_name_copy.begin(), node_name_copy.end(), '.', '/');

    if (selected_nodes_[i] && 
        (nodes[i].full_name == node_name_copy)) {
      return true;
    }
  }
  
  return false;
}

} // namespace ros2_console_tui_node
