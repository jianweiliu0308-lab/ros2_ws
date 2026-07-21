#include <chrono>
#include <memory>
#include <thread>

#include "robot_interfaces/action/execute_motion.hpp"
#include "robot_interfaces/msg/motion_command.hpp"
#include "robot_interfaces/msg/robot_state.hpp"
#include "robot_interfaces/msg/safety_status.hpp"
#include "robot_interfaces/srv/plan_trajectory.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using ExecuteMotion = robot_interfaces::action::ExecuteMotion;
using GoalHandleExecuteMotion = rclcpp_action::ServerGoalHandle<ExecuteMotion>;
using MotionCommand = robot_interfaces::msg::MotionCommand;
using RobotState = robot_interfaces::msg::RobotState;
using SafetyStatus = robot_interfaces::msg::SafetyStatus;
using PlanTrajectory = robot_interfaces::srv::PlanTrajectory;

/** 执行层：Action 服务端，规划 + 下发运动指令到驱动 */
class MotionExecutorNode : public rclcpp::Node
{
public:
  MotionExecutorNode()
  : Node("motion_executor")
  {
    latest_state_ = std::make_shared<RobotState>();
    latest_safety_ = std::make_shared<SafetyStatus>();
    latest_safety_->safety_zone = SafetyStatus::ZONE_SAFE;

    motion_pub_ = create_publisher<MotionCommand>("robot/motion_command", 10);
    state_sub_ = create_subscription<RobotState>(
      "robot/state", 10,
      [this](const RobotState::SharedPtr msg) { latest_state_ = msg; });
    safety_sub_ = create_subscription<SafetyStatus>(
      "robot/safety", 10,
      [this](const SafetyStatus::SharedPtr msg) { latest_safety_ = msg; });
    planner_client_ = create_client<PlanTrajectory>("motion/plan_trajectory");

    action_server_ = rclcpp_action::create_server<ExecuteMotion>(
      this, "motion/execute",
      std::bind(&MotionExecutorNode::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
      std::bind(&MotionExecutorNode::handle_cancel, this, std::placeholders::_1),
      std::bind(&MotionExecutorNode::handle_accepted, this, std::placeholders::_1));

    RCLCPP_INFO(get_logger(), "Motion executor ready");
  }

private:
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID &,
    std::shared_ptr<const ExecuteMotion::Goal> goal)
  {
    if (!is_safe()) {
      RCLCPP_WARN(get_logger(), "Goal rejected: safety not OK");
      return rclcpp_action::GoalResponse::REJECT;
    }
    if (goal->target_joints.empty()) {
      return rclcpp_action::GoalResponse::REJECT;
    }
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleExecuteMotion>)
  {
    publish_stop();
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandleExecuteMotion> goal_handle)
  {
    std::thread{std::bind(&MotionExecutorNode::execute, this, goal_handle)}.detach();
  }

  void execute(const std::shared_ptr<GoalHandleExecuteMotion> goal_handle)
  {
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<ExecuteMotion::Feedback>();
    auto result = std::make_shared<ExecuteMotion::Result>();

    feedback->phase = ExecuteMotion::Feedback::PHASE_PLANNING;
    feedback->progress = 0.0;
    goal_handle->publish_feedback(feedback);

    if (!planner_client_->wait_for_service(std::chrono::seconds(2))) {
      result->success = false;
      result->message = "Planner unavailable";
      goal_handle->abort(result);
      return;
    }

    auto plan_req = std::make_shared<PlanTrajectory::Request>();
    plan_req->start_joints = latest_state_->joint_positions;
    plan_req->goal_joints = goal->target_joints;
    plan_req->max_velocity = 0.5 * goal->velocity_scale;

    auto plan_future = planner_client_->async_send_request(plan_req);
    if (rclcpp::spin_until_future_complete(shared_from_this(), plan_future) !=
      rclcpp::FutureReturnCode::SUCCESS)
    {
      result->success = false;
      result->message = "Planning failed";
      goal_handle->abort(result);
      return;
    }

    const auto plan_resp = plan_future.get();
    if (!plan_resp->success) {
      result->success = false;
      result->message = plan_resp->message;
      goal_handle->abort(result);
      return;
    }

    const size_t dof = goal->target_joints.size();
    const size_t num_points = plan_resp->trajectory_points.size() / dof;

    feedback->phase = ExecuteMotion::Feedback::PHASE_EXECUTING;
    for (size_t p = 0; p < num_points; ++p) {
      if (goal_handle->is_canceling() || !is_safe()) {
        publish_stop();
        result->success = false;
        result->message = goal_handle->is_canceling() ? "Canceled" : "Safety violation";
        goal_handle->canceled(result);
        return;
      }

      MotionCommand cmd;
      cmd.command_type = MotionCommand::CMD_JOINT;
      cmd.velocity_scale = goal->velocity_scale;
      cmd.targets.assign(
        plan_resp->trajectory_points.begin() + p * dof,
        plan_resp->trajectory_points.begin() + (p + 1) * dof);
      motion_pub_->publish(cmd);

      feedback->progress = static_cast<double>(p + 1) / static_cast<double>(num_points);
      goal_handle->publish_feedback(feedback);
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    feedback->phase = ExecuteMotion::Feedback::PHASE_DONE;
    feedback->progress = 1.0;
    goal_handle->publish_feedback(feedback);

    result->success = true;
    result->message = "Motion completed";
    goal_handle->succeed(result);
  }

  bool is_safe() const
  {
    return latest_safety_ &&
           !latest_safety_->e_stop &&
           !latest_safety_->protective_stop &&
           latest_safety_->safety_zone != SafetyStatus::ZONE_VIOLATION;
  }

  void publish_stop()
  {
    MotionCommand cmd;
    cmd.command_type = MotionCommand::CMD_STOP;
    motion_pub_->publish(cmd);
  }

  RobotState::SharedPtr latest_state_;
  SafetyStatus::SharedPtr latest_safety_;
  rclcpp::Publisher<MotionCommand>::SharedPtr motion_pub_;
  rclcpp::Subscription<RobotState>::SharedPtr state_sub_;
  rclcpp::Subscription<SafetyStatus>::SharedPtr safety_sub_;
  rclcpp::Client<PlanTrajectory>::SharedPtr planner_client_;
  rclcpp_action::Server<ExecuteMotion>::SharedPtr action_server_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MotionExecutorNode>());
  rclcpp::shutdown();
  return 0;
}
