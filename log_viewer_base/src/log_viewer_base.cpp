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

#include <log_viewer_base/log_viewer_base.hpp>

namespace log_viewer_base
{

LogViewerBase::LogViewerBase(const rclcpp::NodeOptions & options)
: rclcpp::Node("log_viewer_base", options),
  BUFF_SIZE(1000),
  pending_logs_(BUFF_SIZE)
{
  rclcpp::QoS qos(rclcpp::KeepLast(100));
  qos.reliable();
  sub_ = this->create_subscription<rcl_interfaces::msg::Log>(
    "/rosout", qos, std::bind(&LogViewerBase::log_callback, this, std::placeholders::_1));
  
  // Initialize graph inspector with this node and default update interval of 5 seconds
  graph_inspector_ = std::make_unique<GraphInspector>(
    std::shared_ptr<rclcpp::Node>(this, [](auto){}),
    std::chrono::seconds(5));
}

void LogViewerBase::log_callback(const rcl_interfaces::msg::Log::SharedPtr msg)
{
  if (is_paused()) {return;}
  if (excluded_names_.find(msg->name) != excluded_names_.end()) {return;}

  std::string msg_line = msg->msg;

  std::lock_guard<std::mutex> lock(pending_logs_mutex_);
  pending_logs_.push_back(*msg);
}

std::string LogViewerBase::convert_to_string(const rcl_interfaces::msg::Log & msg)
{
  std::string msg_line = msg.msg;
  if (msg_line.empty()) {
    return "";
  }

  std::string time_str = std::to_string(msg.stamp.sec) + "." +
    std::to_string(msg.stamp.nanosec);
  std::string name_str = msg.name;
  std::string level_str = level_to_string(static_cast<LogLevel>(msg.level));

  return "[" + name_str + "] [" + level_str + "] " + msg_line;
}

bool LogViewerBase::is_paused() const
{
  return now_pause_;
}

void LogViewerBase::set_paused_flag(bool paused)
{
  now_pause_ = paused;
}

void LogViewerBase::update_graph()
{
  if (graph_inspector_) {
    graph_inspector_->update();
  }
}

std::vector<GraphInspector::NodeInfo> LogViewerBase::get_nodes() const
{
  if (graph_inspector_) {
    return graph_inspector_->get_nodes();
  }
  return {};
}

std::map<std::string, std::vector<std::string>> LogViewerBase::get_topics() const
{
  if (graph_inspector_) {
    return graph_inspector_->get_topics();
  }
  return {};
}

std::map<std::string, std::vector<std::string>> LogViewerBase::get_services() const
{
  if (graph_inspector_) {
    return graph_inspector_->get_services();
  }
  return {};
}

boost::circular_buffer<rcl_interfaces::msg::Log> LogViewerBase::get_filtered_logs(
  const std::vector<std::string>& node_names)
{
  boost::circular_buffer<rcl_interfaces::msg::Log> filtered_logs(BUFF_SIZE);
  std::lock_guard<std::mutex> lock(pending_logs_mutex_);
  
  for (const auto & log : pending_logs_) {
    if (std::find(node_names.begin(), node_names.end(), log.name) != node_names.end()) {
      filtered_logs.push_back(log);
    }
  }
  return filtered_logs;
}

}  // namespace log_viewer_base
