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

#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include "log_viewer_base/log_viewer_base.hpp"

namespace log_viewer_stdout
{

class LogViewerStdout : public log_viewer_base::LogViewerBase
{
public:
  explicit LogViewerStdout(const rclcpp::NodeOptions & options)
  : LogViewerBase(options),
    last_graph_update_(std::chrono::steady_clock::now())
  {
    this->declare_parameter("show_node_name", "");
    this->get_parameter("show_node_name", show_node_name_);

    if (!show_node_name_.empty()) {
      filter_mode_ = true;
      std::stringstream ss(show_node_name_);
      std::string node_name;
      while (std::getline(ss, node_name, ',')) {
        if (!node_name.empty()) {
          filter_node_names_.push_back(node_name);
        }
      }
    }
    
    running_.store(true);
    std::signal(SIGINT, &LogViewerStdout::signal_handler);
    std::signal(SIGTERM, &LogViewerStdout::signal_handler);
    
    std::cout << "=== ROS2 Log Viewer (stdout) ===" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    if (filter_mode_) {
      std::cout << "Filtering logs for nodes: ";
      for (size_t i = 0; i < filter_node_names_.size(); ++i) {
        std::cout << filter_node_names_[i];
        if (i < filter_node_names_.size() - 1) std::cout << ", ";
      }
      std::cout << std::endl;
    }
    std::cout << "Monitoring /rosout topic..." << std::endl;
    std::cout << std::string(50, '=') << std::endl;
  }

  void spin()
  {
    auto last_log_check = std::chrono::steady_clock::now();
    auto last_stats_print = std::chrono::steady_clock::now();
    const auto stats_interval = std::chrono::seconds(10);
    const auto log_check_interval = std::chrono::milliseconds(100);
    const auto graph_update_interval = std::chrono::seconds(30);

    while (rclcpp::ok() && running_) {
      auto now = std::chrono::steady_clock::now();
      
      if (now - last_log_check >= log_check_interval) {
        process_pending_logs();
        last_log_check = now;
      }
      
      if (now - last_stats_print >= stats_interval) {
        print_statistics();
        last_stats_print = now;
      }
      
      if (now - last_graph_update_ >= graph_update_interval) {
        print_graph_info();
        last_graph_update_ = now;
      }
      
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << std::endl << "Shutting down log viewer..." << std::endl;
  }

private:
  static std::atomic<bool> running_;
  std::chrono::steady_clock::time_point last_graph_update_;
  size_t total_logs_processed_ = 0;
  size_t logs_this_session_ = 0;
  bool filter_mode_ = false;
  std::vector<std::string> filter_node_names_;
  std::string show_node_name_;

  static void signal_handler(int signal)
  {
    (void)signal;
    running_.store(false);
  }

  void process_pending_logs()
  {
    if (is_pending_logs_empty()) {
      return;
    }

    if (filter_mode_ && !filter_node_names_.empty()) {
      auto filtered_logs = get_filtered_logs(filter_node_names_);
      for (const auto & log_entry : filtered_logs) {
        print_log_entry(log_entry);
        total_logs_processed_++;
        logs_this_session_++;
      }
    } else {
      auto logs = get_pending_logs();
      for (const auto & log_entry : logs) {
        print_log_entry(log_entry);
        total_logs_processed_++;
        logs_this_session_++;
      }
    }
    clear_pending_logs();
  }

  void print_log_entry(const rcl_interfaces::msg::Log & log_entry)
  {
    std::time_t t = static_cast<std::time_t>(log_entry.stamp.sec);
    std::tm * tm_info = std::localtime(&t);
    
    std::string level_str = log_viewer_base::level_to_string(
      static_cast<log_viewer_base::LogLevel>(log_entry.level));
    
    std::cout << "["
              << std::put_time(tm_info, "%H:%M:%S")
              << "." << std::setfill('0') << std::setw(3)
              << (log_entry.stamp.nanosec / 1000000)
              << "[" << std::setw(5) << level_str << "]"
              << "\033[0m"
              << " [" << log_entry.name << "] "
              << log_entry.msg << std::endl;
  }

  void print_statistics()
  {
    std::cout << std::string(50, '-') << std::endl;
    std::cout << "Statistics:" << std::endl;
    std::cout << "  Total logs processed: " << total_logs_processed_ << std::endl;
    std::cout << "  Logs this session: " << logs_this_session_ << std::endl;
    std::cout << "  Paused: " << (is_paused() ? "Yes" : "No") << std::endl;
    
    if (filter_mode_) {
      std::cout << "  Filtering logs for nodes: ";
      for (size_t i = 0; i < filter_node_names_.size(); ++i) {
        std::cout << filter_node_names_[i];
        if (i < filter_node_names_.size() - 1) std::cout << ", ";
      }
      std::cout << std::endl;
    }
    
    std::cout << "  Excluded nodes: " << excluded_names_.size() << std::endl;
    
    if (!excluded_names_.empty()) {
      std::cout << "    Excluded: ";
      bool first = true;
      for (const auto & name : excluded_names_) {
        if (!first) std::cout << ", ";
        std::cout << name;
        first = false;
      }
      std::cout << std::endl;
    }
    
    std::cout << std::string(50, '-') << std::endl;
    logs_this_session_ = 0;
  }

  void print_graph_info()
  {
    std::cout << std::string(50, '=') << std::endl;
    std::cout << "Graph Information Update:" << std::endl;
    
    update_graph();
    
    auto nodes = get_nodes();
    std::cout << "Active Nodes (" << nodes.size() << "):" << std::endl;
    
    if (nodes.empty()) {
      std::cout << "  No active nodes found." << std::endl;
    } else {
      for (const auto & node : nodes) {
        std::cout << "  - " << node.full_name << std::endl;
        
        if (!node.publishers.empty()) {
          std::cout << "    Publishers:" << std::endl;
          for (const auto & pub : node.publishers) {
            std::cout << "      " << pub.first << " (";
            bool first = true;
            for (const auto & type : pub.second) {
              if (!first) std::cout << ", ";
              std::cout << type;
              first = false;
            }
            std::cout << ")" << std::endl;
          }
        }
        
        if (!node.subscribers.empty()) {
          std::cout << "    Subscribers:" << std::endl;
          for (const auto & sub : node.subscribers) {
            std::cout << "      " << sub.first << " (";
            bool first = true;
            for (const auto & type : sub.second) {
              if (!first) std::cout << ", ";
              std::cout << type;
              first = false;
            }
            std::cout << ")" << std::endl;
          }
        }
      }
    }
    
    auto topics = get_topics();
    std::cout << std::endl << "All Topics (" << topics.size() << "):" << std::endl;
    
    if (topics.empty()) {
      std::cout << "  No active topics found." << std::endl;
    } else {
      for (const auto & topic : topics) {
        std::cout << "  - " << topic.first << " (";
        bool first = true;
        for (const auto & type : topic.second) {
          if (!first) std::cout << ", ";
          std::cout << type;
          first = false;
        }
        std::cout << ")" << std::endl;
      }
    }
    
    auto services = get_services();
    std::cout << std::endl << "All Services (" << services.size() << "):" << std::endl;
    
    if (services.empty()) {
      std::cout << "  No active services found." << std::endl;
    } else {
      for (const auto & service : services) {
        std::cout << "  - " << service.first << " (";
        bool first = true;
        for (const auto & type : service.second) {
          if (!first) std::cout << ", ";
          std::cout << type;
          first = false;
        }
        std::cout << ")" << std::endl;
      }
    }
    
    std::cout << std::string(50, '=') << std::endl;
  }

};

std::atomic<bool> LogViewerStdout::running_{true};

}  // namespace log_viewer_stdout

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  
  try {
    rclcpp::NodeOptions options;
    auto node = std::make_shared<log_viewer_stdout::LogViewerStdout>(options);
    
    std::thread spin_thread([&node]() {
      rclcpp::spin(node);
    });
    
    node->spin();
    
    rclcpp::shutdown();
    if (spin_thread.joinable()) {
      spin_thread.join();
    }
    
  } catch (const std::exception & e) {
    std::cerr << "Exception occurred: " << e.what() << std::endl;
    rclcpp::shutdown();
    return 1;
  }
  
  return 0;
}