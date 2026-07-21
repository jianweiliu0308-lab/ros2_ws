#include <chrono>
#include <memory>
#include <thread>

#include "my_interface/action/count_down.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using CountDown = my_interface::action::CountDown;
using GoalHandleCountDown = rclcpp_action::ServerGoalHandle<CountDown>;

/** 倒计时 Action 服务端，对应教程「动作 Action」 */
class CountDownServer : public rclcpp::Node
{
public:
  explicit CountDownServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("countdown_server", options)
  {
    using namespace std::placeholders;
    action_server_ = rclcpp_action::create_server<CountDown>(
      this,
      "countdown",
      std::bind(&CountDownServer::handle_goal, this, _1, _2),
      std::bind(&CountDownServer::handle_cancel, this, _1),
      std::bind(&CountDownServer::handle_accepted, this, _1));
    RCLCPP_INFO(get_logger(), "Ready: action /countdown");
  }

private:
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID &,
    std::shared_ptr<const CountDown::Goal> goal)
  {
    RCLCPP_INFO(get_logger(), "Received goal: seconds=%d", goal->seconds);
    if (goal->seconds <= 0) {
      return rclcpp_action::GoalResponse::REJECT;
    }
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandleCountDown>)
  {
    RCLCPP_INFO(get_logger(), "Cancel request received");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandleCountDown> goal_handle)
  {
    std::thread{std::bind(&CountDownServer::execute, this, goal_handle)}.detach();
  }

  void execute(const std::shared_ptr<GoalHandleCountDown> goal_handle)
  {
    RCLCPP_INFO(get_logger(), "Executing countdown...");
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<CountDown::Feedback>();
    auto result = std::make_shared<CountDown::Result>();

    for (int i = goal->seconds; i > 0; --i) {
      if (goal_handle->is_canceling()) {
        result->success = false;
        result->message = "Canceled";
        goal_handle->canceled(result);
        RCLCPP_INFO(get_logger(), "Goal canceled");
        return;
      }
      feedback->remaining = i;
      goal_handle->publish_feedback(feedback);
      RCLCPP_INFO(get_logger(), "Remaining: %d", i);
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    result->success = true;
    result->message = "Done";
    goal_handle->succeed(result);
    RCLCPP_INFO(get_logger(), "Goal succeeded");
  }

  rclcpp_action::Server<CountDown>::SharedPtr action_server_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<CountDownServer>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
