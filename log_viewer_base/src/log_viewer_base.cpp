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
  qos.transient_local();
  qos.lifespan(std::chrono::nanoseconds(10000000000));
  
  std::cout << "LogViewerBase Node initialized with buffer size: " << BUFF_SIZE << std::endl;
  
  sub_ = this->create_subscription<rcl_interfaces::msg::Log>(
    "/rosout", qos, 
    [this](const rcl_interfaces::msg::Log::SharedPtr msg) {
      this->log_callback(msg);
    });
  
  graph_inspector_ = std::make_unique<GraphInspector>(
    std::shared_ptr<rclcpp::Node>(this, [](auto){}),
    std::chrono::seconds(5));
}

void LogViewerBase::log_callback(const rcl_interfaces::msg::Log::SharedPtr msg)
{
  if (is_paused()) {
    return;
  }
  
  if (excluded_names_.find(msg->name) != excluded_names_.end()) {
    return;
  }
  if (msg->msg.empty()) {
    return;
  }

  auto log_copy = std::make_shared<rcl_interfaces::msg::Log>(*msg);
  
  {
    std::lock_guard<std::mutex> lock(pending_logs_mutex_);
    pending_logs_.push_back(*log_copy);
  }
}

std::string LogViewerBase::convert_to_string(const rcl_interfaces::msg::Log & msg)
{
  if (msg.msg.empty()) {
    return "";
  }

  return "[" + msg.name + "] [" + level_to_string(static_cast<LogLevel>(msg.level)) + "] " +
         std::to_string(msg.stamp.sec) + "." + std::to_string(msg.stamp.nanosec);
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
  const std::vector<std::string>& full_names)
{
  boost::circular_buffer<rcl_interfaces::msg::Log> filtered_logs(BUFF_SIZE);
  boost::circular_buffer<rcl_interfaces::msg::Log> logs_copy;
  {
    std::lock_guard<std::mutex> lock(pending_logs_mutex_);
    logs_copy = boost::circular_buffer<rcl_interfaces::msg::Log>(pending_logs_);
  }

  if (graph_inspector_) {
    graph_inspector_->update();
  }

  for (const auto & log : logs_copy) {
    bool log_matches = false;
    std::string log_full_name = log.name;

    if (log_full_name.empty()) {
      continue;
    }

    std::replace(log_full_name.begin(), log_full_name.end(), '.', '/');

    if (full_names.empty()) {
      log_matches = true;
    } else {
      for (const auto& requested_full : full_names) {
        if (requested_full.empty()) {
          continue;
        }
        if (log_full_name == requested_full) {
          log_matches = true;
          break;
        }
      }
    }
    if (log_matches) {
      filtered_logs.push_back(log);
    }
  }
  
  return filtered_logs;
}

}  // namespace log_viewer_base
