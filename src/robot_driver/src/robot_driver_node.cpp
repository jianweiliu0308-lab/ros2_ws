#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "geometry_msgs/msg/pose.hpp"
#include "lifecycle_msgs/msg/state.hpp"
#include "robot_interfaces/msg/motion_command.hpp"
#include "robot_interfaces/msg/robot_state.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
using MotionCommand = robot_interfaces::msg::MotionCommand;
using RobotState = robot_interfaces::msg::RobotState;

/**
 * 驱动层：Lifecycle 节点，模拟 6 轴机械臂硬件。
 * - configure: 初始化关节
 * - activate: 开始发布 /robot/state，接收 /robot/motion_command
 * - deactivate: 停止运动并停止发布
 */
class RobotDriverNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit RobotDriverNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : LifecycleNode("robot_driver", options)
  {
    declare_parameter<int>("dof", 6);
    declare_parameter<double>("publish_rate_hz", 50.0);
    declare_parameter<double>("max_joint_velocity", 0.5);
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    dof_ = get_parameter("dof").as_int();
    publish_rate_hz_ = get_parameter("publish_rate_hz").as_double();
    max_joint_velocity_ = get_parameter("max_joint_velocity").as_double();
    joint_positions_.assign(dof_, 0.0);
    joint_velocities_.assign(dof_, 0.0);
    target_positions_ = joint_positions_;
    status_ = RobotState::STATUS_STOPPED;

    state_pub_ = create_publisher<RobotState>("robot/state", 10);
    cmd_sub_ = create_subscription<MotionCommand>(
      "robot/motion_command", 10,
      std::bind(&RobotDriverNode::on_motion_command, this, std::placeholders::_1));

    RCLCPP_INFO(get_logger(), "Configured: dof=%d", dof_);
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State & state) override
  {
    LifecycleNode::on_activate(state);
    const auto period = std::chrono::duration<double>(1.0 / publish_rate_hz_);
    control_timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::nanoseconds>(period),
      std::bind(&RobotDriverNode::control_loop, this));
    RCLCPP_INFO(get_logger(), "Activated: publishing /robot/state");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) override
  {
    control_timer_.reset();
    joint_velocities_.assign(dof_, 0.0);
    status_ = RobotState::STATUS_HOLDING;
    LifecycleNode::on_deactivate(state);
    RCLCPP_INFO(get_logger(), "Deactivated");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    control_timer_.reset();
    state_pub_.reset();
    cmd_sub_.reset();
    RCLCPP_INFO(get_logger(), "Cleaned up");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override
  {
    control_timer_.reset();
    state_pub_.reset();
    cmd_sub_.reset();
    RCLCPP_INFO(get_logger(), "Shutdown");
    return CallbackReturn::SUCCESS;
  }

private:
  void on_motion_command(const MotionCommand::SharedPtr msg)
  {
    if (get_current_state().id() != lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE) {
      RCLCPP_WARN(get_logger(), "Motion command ignored: driver not active");
      return;
    }
    if (msg->command_type == MotionCommand::CMD_STOP) {
      target_positions_ = joint_positions_;
      status_ = RobotState::STATUS_HOLDING;
      RCLCPP_INFO(get_logger(), "Stop command received");
      return;
    }
    if (msg->targets.size() != static_cast<size_t>(dof_)) {
      RCLCPP_ERROR(get_logger(), "Invalid target size: expected %d", dof_);
      status_ = RobotState::STATUS_FAULT;
      return;
    }
    target_positions_ = msg->targets;
    velocity_scale_ = std::min(1.0, std::max(0.05, msg->velocity_scale));
    status_ = RobotState::STATUS_MOVING;
  }

  void control_loop()
  {
    const double dt = 1.0 / publish_rate_hz_;
    const double max_step = max_joint_velocity_ * velocity_scale_ * dt;
    bool moving = false;

    for (int i = 0; i < dof_; ++i) {
      const double error = target_positions_[i] - joint_positions_[i];
      if (std::abs(error) > 1e-4) {
        moving = true;
        const double step = std::max(-max_step, std::min(max_step, error));
        joint_positions_[i] += step;
        joint_velocities_[i] = step / dt;
      } else {
        joint_velocities_[i] = 0.0;
      }
    }

    if (status_ == RobotState::STATUS_MOVING && !moving) {
      status_ = RobotState::STATUS_STOPPED;
    }

    RobotState state;
    state.stamp = now();
    state.joint_positions = joint_positions_;
    state.joint_velocities = joint_velocities_;
    state.tcp_pose = forward_kinematics(joint_positions_);
    state.status = status_;
    state_pub_->publish(state);
  }

  static geometry_msgs::msg::Pose forward_kinematics(const std::vector<double> & joints)
  {
    geometry_msgs::msg::Pose pose;
    pose.position.x = 0.5 * std::cos(joints.empty() ? 0.0 : joints[0]);
    pose.position.y = 0.5 * std::sin(joints.empty() ? 0.0 : joints[0]);
    pose.position.z = 0.3 + (joints.size() > 1 ? 0.1 * joints[1] : 0.0);
    pose.orientation.w = 1.0;
    return pose;
  }

  int dof_{6};
  double publish_rate_hz_{50.0};
  double max_joint_velocity_{0.5};
  double velocity_scale_{0.5};
  uint8_t status_{RobotState::STATUS_STOPPED};
  std::vector<double> joint_positions_;
  std::vector<double> joint_velocities_;
  std::vector<double> target_positions_;
  rclcpp_lifecycle::LifecyclePublisher<RobotState>::SharedPtr state_pub_;
  rclcpp::Subscription<MotionCommand>::SharedPtr cmd_sub_;
  rclcpp::TimerBase::SharedPtr control_timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<RobotDriverNode>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}
