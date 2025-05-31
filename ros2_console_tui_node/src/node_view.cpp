// Node view related implementation for the LogViewerTui class

#include "ros2_console_tui_node/log_viewer.hpp"

namespace ros2_console_tui_node
{

void LogViewerTui::draw_node_window()
{
  auto & ctx = windows_.at(WindowType::Log);
  WINDOW * win = ctx.handle;
  werase(win);

  // Get node information from graph inspector
  auto nodes = get_nodes();
  auto topics = get_topics();
  auto services = get_services();

  int max_lines = ctx.height;
  int current_line = 0;
  int lines_displayed = 0;

  // Calculate total lines to display
  int total_lines = 0;
  
  // Active nodes section
  total_lines += 2; // Section header + blank line
  total_lines += nodes.size() > 0 ? nodes.size() : 1; // Nodes or "No active nodes" message
  total_lines += 1; // Blank line
  
  // Topics section
  total_lines += 2; // Section header + blank line
  for (const auto& pair : topics) {
    total_lines += 1; // Topic name
    total_lines += pair.second.size(); // Types
  }
  if (topics.empty()) {
    total_lines += 1; // "No active topics" message
  }
  total_lines += 1; // Blank line
  
  // Services section
  total_lines += 2; // Section header + blank line
  for (const auto& pair : services) {
    total_lines += 1; // Service name
    total_lines += pair.second.size(); // Types
  }
  if (services.empty()) {
    total_lines += 1; // "No active services" message
  }
  total_lines += 1; // Blank line
  
  // Topic connections section
  total_lines += 2; // Section header + blank line
  for (const auto& node : nodes) {
    total_lines += 2; // Node name + blank line
    
    // Publishers
    total_lines += 1; // Publishers header
    if (node.publishers.empty()) {
      total_lines += 1; // "No topics being published" message
    } else {
      for (const auto& pub : node.publishers) {
        total_lines += 1; // Topic name and types
      }
    }
    
    // Subscribers
    total_lines += 1; // Subscribers header
    if (node.subscribers.empty()) {
      total_lines += 1; // "No topics being subscribed to" message
    } else {
      for (const auto& sub : node.subscribers) {
        total_lines += 1; // Topic name and types
      }
    }
    
    total_lines += 1; // Blank line after each node
  }
  if (nodes.empty()) {
    total_lines += 1; // "No active nodes" message
  }

  // Handle scrolling
  int max_offset = std::max(0, total_lines - max_lines);
  node_scroll_offset_ = std::min(node_scroll_offset_, max_offset);
  int skip_lines = node_scroll_offset_;

  // Helper lambda to draw a line if it's in view
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

  // Display active nodes
  draw_line("=== Active Nodes ===", 6);
  draw_line("");
  
  if (nodes.empty()) {
    draw_line("  No active nodes found.");
  } else {
    for (const auto& node : nodes) {
      draw_line("  Node: " + node.name + ", Namespace: " + node.ns);
    }
  }
  draw_line("");
  
  // Display all topics and types
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
  
  // Display all services and types
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
  
  // Display topic connections by node
  draw_line("=== Topic Connections by Node ===", 6);
  draw_line("");
  
  if (nodes.empty()) {
    draw_line("  No active nodes to inspect connections.");
  } else {
    for (const auto& node : nodes) {
      draw_line("Node: " + node.full_name, 2);
      
      // Display publishers
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
      
      // Display subscribers
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
    node_scroll_offset_ = 0; // Reset scroll position
    update_graph(); // Update graph information immediately
    last_graph_update_ = std::chrono::steady_clock::now();
    windows_.at(WindowType::Header).needs_redraw = true;
    windows_.at(WindowType::Footer).needs_redraw = true;
    windows_.at(WindowType::Log).needs_redraw = true;
  }
}

} // namespace ros2_console_tui_node
