#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "robot_interfaces/action/execute_motion.hpp"
#include "robot_interfaces/msg/robot_state.hpp"
#include "robot_interfaces/msg/safety_status.hpp"
#include "robot_interfaces/srv/get_robot_status.hpp"
#include "robot_interfaces/srv/set_mode.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using ExecuteMotion = robot_interfaces::action::ExecuteMotion;
using GoalHandleExecuteMotion = rclcpp_action::ClientGoalHandle<ExecuteMotion>;
using RobotState = robot_interfaces::msg::RobotState;
using SafetyStatus = robot_interfaces::msg::SafetyStatus;
using SetMode = robot_interfaces::srv::SetMode;
using GetRobotStatus = robot_interfaces::srv::GetRobotStatus;

/**
 * 监督层：整机状态机
 * - IDLE / MANUAL / AUTO / FAULT
 * - 提供 /supervisor/set_mode 与 /supervisor/get_status
 * - AUTO 模式下周期性下发演示运动任务
 */
class SupervisorNode : public rclcpp::Node
{
public:
  SupervisorNode()
  : Node("supervisor")
  {
    declare_parameter<std::vector<double>>(
      "demo_target_joints", std::vector<double>{0.5, 0.2, -0.1, 0.0, 0.3, 0.0});
    declare_parameter<double>("auto_task_period_sec", 8.0);

    demo_target_ = get_parameter("demo_target_joints").as_double_array();
    auto_task_period_sec_ = get_parameter("auto_task_period_sec").as_double();

    latest_state_ = std::make_shared<RobotState>();
    latest_safety_ = std::make_shared<SafetyStatus>();
    latest_safety_->safety_zone = SafetyStatus::ZONE_SAFE;

    set_mode_srv_ = create_service<SetMode>(
      "supervisor/set_mode",
      std::bind(&SupervisorNode::on_set_mode, this, std::placeholders::_1, std::placeholders::_2));
    get_status_srv_ = create_service<GetRobotStatus>(
      "supervisor/get_status",
      std::bind(&SupervisorNode::on_get_status, this, std::placeholders::_1, std::placeholders::_2));

    state_sub_ = create_subscription<RobotState>(
      "robot/state", 10,
      [this](const RobotState::SharedPtr msg) { latest_state_ = msg; });
    safety_sub_ = create_subscription<SafetyStatus>(
      "robot/safety", 10,
      [this](const SafetyStatus::SharedPtr msg) {
        latest_safety_ = msg;
        evaluate_fault();
      });

    motion_client_ = rclcpp_action::create_client<ExecuteMotion>(this, "motion/execute");

    auto_timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(auto_task_period_sec_)),
      std::bind(&SupervisorNode::auto_task_tick, this));

    RCLCPP_INFO(get_logger(), "Supervisor ready (mode=IDLE)");
  }

private:
  void on_set_mode(
    const std::shared_ptr<SetMode::Request> request,
    std::shared_ptr<SetMode::Response> response)
  {
    if (mode_ == MODE_FAULT && request->mode != SetMode::Request::MODE_IDLE) {
      response->success = false;
      response->message = "In FAULT mode, switch to IDLE first";
      return;
    }

    if (request->mode == SetMode::Request::MODE_AUTO && !is_safe()) {
      response->success = false;
      response->message = "Cannot enter AUTO: safety not OK";
      return;
    }

    mode_ = request->mode;
    response->success = true;
    response->message = "Mode set to " + mode_to_string(mode_);
    RCLCPP_INFO(get_logger(), "%s", response->message.c_str());
  }

  void on_get_status(
    const std::shared_ptr<GetRobotStatus::Request>,
    std::shared_ptr<GetRobotStatus::Response> response)
  {
    response->mode = mode_;
    response->robot_status = latest_state_ ? latest_state_->status : RobotState::STATUS_STOPPED;
    response->safety_ok = is_safe();
    response->message = "mode=" + mode_to_string(mode_) +
      ", safety_ok=" + (response->safety_ok ? "true" : "false");
  }

  void evaluate_fault()
  {
    if (!is_safe() && mode_ != MODE_FAULT) {
      mode_ = MODE_FAULT;
      RCLCPP_ERROR(get_logger(), "Entered FAULT mode due to safety violation");
    }
  }

  void auto_task_tick()
  {
    if (mode_ != SetMode::Request::MODE_AUTO || !is_safe() || task_in_flight_) {
      return;
    }
    if (!motion_client_->wait_for_action_server(std::chrono::seconds(0))) {
      return;
    }

    auto goal = ExecuteMotion::Goal();
    goal.target_joints = demo_target_;
    goal.velocity_scale = 0.4;

    auto options = rclcpp_action::Client<ExecuteMotion>::SendGoalOptions();
    options.result_callback = [this](const GoalHandleExecuteMotion::WrappedResult & result) {
      task_in_flight_ = false;
      if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
        RCLCPP_INFO(get_logger(), "AUTO task done: %s", result.result->message.c_str());
      } else {
        RCLCPP_WARN(get_logger(), "AUTO task failed or canceled");
      }
    };

    task_in_flight_ = true;
    motion_client_->async_send_goal(goal, options);
    RCLCPP_INFO(get_logger(), "AUTO task dispatched");
  }

  bool is_safe() const
  {
    return latest_safety_ &&
           !latest_safety_->e_stop &&
           !latest_safety_->protective_stop &&
           latest_safety_->safety_zone != SafetyStatus::ZONE_VIOLATION;
  }

  static std::string mode_to_string(uint8_t mode)
  {
    switch (mode) {
      case SetMode::Request::MODE_IDLE: return "IDLE";
      case SetMode::Request::MODE_MANUAL: return "MANUAL";
      case SetMode::Request::MODE_AUTO: return "AUTO";
      default: return "FAULT";
    }
  }

  static constexpr uint8_t MODE_FAULT = 99;

  uint8_t mode_{SetMode::Request::MODE_IDLE};
  bool task_in_flight_{false};
  double auto_task_period_sec_{8.0};
  std::vector<double> demo_target_;
  RobotState::SharedPtr latest_state_;
  SafetyStatus::SharedPtr latest_safety_;
  rclcpp::Service<SetMode>::SharedPtr set_mode_srv_;
  rclcpp::Service<GetRobotStatus>::SharedPtr get_status_srv_;
  rclcpp::Subscription<RobotState>::SharedPtr state_sub_;
  rclcpp::Subscription<SafetyStatus>::SharedPtr safety_sub_;
  rclcpp_action::Client<ExecuteMotion>::SharedPtr motion_client_;
  rclcpp::TimerBase::SharedPtr auto_timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SupervisorNode>());
  rclcpp::shutdown();
  return 0;
}
