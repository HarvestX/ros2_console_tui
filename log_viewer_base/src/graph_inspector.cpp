#include "log_viewer_base/graph_inspector.hpp"

namespace log_viewer_base
{

GraphInspector::GraphInspector(rclcpp::Node::SharedPtr parent_node, 
                               std::chrono::seconds update_interval)
  : node_(parent_node),
    update_interval_(update_interval) {
  const std::string my_node_namespace = node_->get_namespace();
  const std::string my_node_name = node_->get_name();
  my_full_node_name_ = get_full_node_name(my_node_namespace, my_node_name);
}

void GraphInspector::update() {
  auto node_graph = node_->get_node_graph_interface();
  collect_data(node_graph);
}

// Collect all data and store in member variables
void GraphInspector::collect_data(const rclcpp::node_interfaces::NodeGraphInterface::SharedPtr& node_graph) {
  std::lock_guard<std::mutex> lock(data_mutex_);
  
  // Clear previous data
  nodes_.clear();
  topics_.clear();
  services_.clear();
  
  // Collect nodes information
  auto node_names_result = execute_safely<std::vector<std::pair<std::string, std::string>>>(
    [&]() { return node_graph->get_node_names_and_namespaces(); },
    "Error getting node names"
  );
  
  // Collect topics information
  auto topics_result = execute_safely<std::map<std::string, std::vector<std::string>>>(
    [&]() { return node_graph->get_topic_names_and_types(false); },
    "Error getting topic names and types"
  );
  
  if (topics_result) {
    // Filter topics and store
    for (const auto& pair : *topics_result) {
      auto filtered_types = filter_excluded_types(pair.second);
      if (!filtered_types.empty()) {
        topics_[pair.first] = filtered_types;
      }
    }
  }
  
  // Collect services information
  auto services_result = execute_safely<std::map<std::string, std::vector<std::string>>>(
    [&]() { return node_graph->get_service_names_and_types(); },
    "Error getting service names and types"
  );
  
  if (services_result) {
    // Filter services and store
    for (const auto& pair : *services_result) {
      auto filtered_types = filter_excluded_types(pair.second);
      if (!filtered_types.empty()) {
        services_[pair.first] = filtered_types;
      }
    }
  }

  if (node_names_result) {
    for (const auto& node_pair : *node_names_result) {
      const std::string& name = node_pair.first;
      const std::string& ns = node_pair.second;
      std::string full_name = get_full_node_name(ns, name);
      
      if (full_name == my_full_node_name_) {
        continue;
      }
      
      NodeInfo node_info;
      node_info.full_name = full_name;
      
      // Get publishers for this node
      auto publishers_result = execute_safely<std::map<std::string, std::vector<std::string>>>(
        [&]() { return node_graph->get_publisher_names_and_types_by_node(name, ns, false); },
        "Error getting publishers for node " + name
      );
      
      if (publishers_result) {
        for (const auto& topic_pair : *publishers_result) {
          auto filtered_types = filter_excluded_types(topic_pair.second);
          if (!filtered_types.empty()) {
            node_info.publishers[topic_pair.first] = filtered_types;
          }
        }
      }
      
      // Get subscribers for this node
      auto subscribers_result = execute_safely<std::map<std::string, std::vector<std::string>>>(
        [&]() { return node_graph->get_subscriber_names_and_types_by_node(name, ns, false); },
        "Error getting subscribers for node " + name
      );
      
      if (subscribers_result) {
        for (const auto& topic_pair : *subscribers_result) {
          auto filtered_types = filter_excluded_types(topic_pair.second);
          if (!filtered_types.empty()) {
            node_info.subscribers[topic_pair.first] = filtered_types;
          }
        }
      }

      nodes_.push_back(node_info);
    }
  }
  
  data_collected_ = true;
}

std::vector<GraphInspector::NodeInfo> GraphInspector::get_nodes() const {
  std::lock_guard<std::mutex> lock(data_mutex_);
  return nodes_;
}

std::map<std::string, std::vector<std::string>> GraphInspector::get_topics() const {
  std::lock_guard<std::mutex> lock(data_mutex_);
  return topics_;
}

std::map<std::string, std::vector<std::string>> GraphInspector::get_services() const {
  std::lock_guard<std::mutex> lock(data_mutex_);
  return services_;
}

void GraphInspector::set_update_interval(std::chrono::seconds interval) {
  if (interval.count() > 0) {
    update_interval_ = interval;
  }
}

}  // namespace log_viewer_base
