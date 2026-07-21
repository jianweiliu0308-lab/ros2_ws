#include <cmath>
#include <memory>

#include "robot_interfaces/msg/robot_state.hpp"
#include "robot_interfaces/msg/safety_status.hpp"
#include "rclcpp/rclcpp.hpp"

using RobotState = robot_interfaces::msg::RobotState;
using SafetyStatus = robot_interfaces::msg::SafetyStatus;

/** 安全监控：根据 TCP 位置判断安全区域，发布 /robot/safety */
class SafetyMonitorNode : public rclcpp::Node
{
public:
  SafetyMonitorNode()
  : Node("safety_monitor")
  {
    declare_parameter<double>("warning_radius_m", 0.8);
    declare_parameter<double>("violation_radius_m", 1.2);
    declare_parameter<bool>("simulate_e_stop", false);

    warning_radius_ = get_parameter("warning_radius_m").as_double();
    violation_radius_ = get_parameter("violation_radius_m").as_double();
    simulate_e_stop_ = get_parameter("simulate_e_stop").as_bool();

    safety_pub_ = create_publisher<SafetyStatus>("robot/safety", 10);
    state_sub_ = create_subscription<RobotState>(
      "robot/state", 10,
      std::bind(&SafetyMonitorNode::on_robot_state, this, std::placeholders::_1));

    RCLCPP_INFO(get_logger(), "Safety monitor ready");
  }

private:
  void on_robot_state(const RobotState::SharedPtr msg)
  {
    SafetyStatus status;
    status.stamp = now();
    status.e_stop = simulate_e_stop_;
    status.protective_stop = (msg->status == RobotState::STATUS_FAULT);
    status.collision_detected = false;

    const double radius = std::hypot(
      msg->tcp_pose.position.x, msg->tcp_pose.position.y);

    if (radius > violation_radius_) {
      status.safety_zone = SafetyStatus::ZONE_VIOLATION;
    } else if (radius > warning_radius_) {
      status.safety_zone = SafetyStatus::ZONE_WARNING;
    } else {
      status.safety_zone = SafetyStatus::ZONE_SAFE;
    }

    safety_pub_->publish(status);
  }

  double warning_radius_{0.8};
  double violation_radius_{1.2};
  bool simulate_e_stop_{false};
  rclcpp::Publisher<SafetyStatus>::SharedPtr safety_pub_;
  rclcpp::Subscription<RobotState>::SharedPtr state_sub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SafetyMonitorNode>());
  rclcpp::shutdown();
  return 0;
}
