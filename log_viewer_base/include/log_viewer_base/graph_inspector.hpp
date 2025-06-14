#ifndef LOG_VIEWER_BASE__GRAPH_INSPECTOR_HPP_
#define LOG_VIEWER_BASE__GRAPH_INSPECTOR_HPP_

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <thread>
#include <chrono>
#include <functional>
#include <optional>
#include <atomic>
#include <mutex>
#include <csignal>
#include <unordered_map>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/node_interfaces/node_graph_interface.hpp"

namespace log_viewer_base
{

class GraphInspector {
public:
  // Node data structure to store connection information
  struct NodeInfo {
    std::string full_name;
    std::map<std::string, std::vector<std::string>> publishers;    // topic -> message types
    std::map<std::string, std::vector<std::string>> subscribers;   // topic -> message types
  };

private:
  // Data containers for collected information
  std::vector<NodeInfo> nodes_;
  std::map<std::string, std::vector<std::string>> topics_;   // topic -> message types
  std::map<std::string, std::vector<std::string>> services_; // service -> service types
  
  rclcpp::Node::SharedPtr node_;
  std::string my_full_node_name_;
  std::chrono::seconds update_interval_;
  bool data_collected_ = false;
  mutable std::mutex data_mutex_;
  
  static bool should_exclude_type(const std::string& type) {
    static const std::vector<std::string> excluded_types = {
      "rcl_interfaces/srv/DescribeParameters",
      "rcl_interfaces/srv/GetParameterTypes", 
      "rcl_interfaces/srv/GetParameters", 
      "rcl_interfaces/srv/ListParameters", 
      "rcl_interfaces/srv/SetParameters", 
      "rcl_interfaces/srv/SetParametersAtomically",
      "rcl_interfaces/msg/ParameterEvent",
      "rcl_interfaces/msg/Log"
    };
    
    return std::find(excluded_types.begin(), excluded_types.end(), type) != excluded_types.end();
  }
  
  std::vector<std::string> filter_excluded_types(const std::vector<std::string>& types) {
    std::vector<std::string> filtered_types;
    std::copy_if(types.begin(), types.end(), std::back_inserter(filtered_types),
                [](const std::string& type) { return !should_exclude_type(type); });
    return filtered_types;
  }
  
  template<typename T>
  std::optional<T> execute_safely(const std::function<T()>& func, const std::string& error_message) {
    try {
      return func();
    } catch (const std::exception& e) {
      std::cerr << error_message << ": " << e.what() << std::endl;
      return std::nullopt;
    }
  }

  std::string get_full_node_name(const std::string& ns, const std::string& name) {
    if (ns == "/") {
      return "/" + name;
    }
    return ns + "/" + name;
  }
  
  void collect_data(const rclcpp::node_interfaces::NodeGraphInterface::SharedPtr& node_graph);
  
public:
  GraphInspector(rclcpp::Node::SharedPtr parent_node, 
                std::chrono::seconds update_interval = std::chrono::seconds(5));
  
  void update();
  
  // Accessor methods for the collected data
  std::vector<NodeInfo> get_nodes() const;
  std::map<std::string, std::vector<std::string>> get_topics() const;
  std::map<std::string, std::vector<std::string>> get_services() const;
  
  void set_update_interval(std::chrono::seconds interval);
  std::chrono::seconds get_update_interval() const { return update_interval_; }
};

}  // namespace log_viewer_base

#endif  // LOG_VIEWER_BASE__GRAPH_INSPECTOR_HPP_
