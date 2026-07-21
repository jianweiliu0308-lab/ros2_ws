#include <memory>

#include "my_interface/action/count_down.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using CountDown = my_interface::action::CountDown;
using GoalHandleCountDown = rclcpp_action::ClientGoalHandle<CountDown>;

/** 倒计时 Action 客户端，命令行传入秒数 */
class CountDownClient : public rclcpp::Node
{
public:
  explicit CountDownClient(int seconds)
  : Node("countdown_client"), seconds_(seconds)
  {
    client_ = rclcpp_action::create_client<CountDown>(this, "countdown");
  }

  void send_goal()
  {
    if (!client_->wait_for_action_server(std::chrono::seconds(5))) {
      RCLCPP_ERROR(get_logger(), "Action server not available");
      rclcpp::shutdown();
      return;
    }

    auto goal_msg = CountDown::Goal();
    goal_msg.seconds = seconds_;

    RCLCPP_INFO(get_logger(), "Sending goal: seconds=%d", seconds_);

    auto options = rclcpp_action::Client<CountDown>::SendGoalOptions();
    options.feedback_callback =
      [this](GoalHandleCountDown::SharedPtr,
      const std::shared_ptr<const CountDown::Feedback> feedback)
      {
        RCLCPP_INFO(get_logger(), "Feedback remaining: %d", feedback->remaining);
      };
    options.result_callback =
      [this](const GoalHandleCountDown::WrappedResult & result)
      {
        switch (result.code) {
          case rclcpp_action::ResultCode::SUCCEEDED:
            RCLCPP_INFO(
              get_logger(), "Result: success=%s message=%s",
              result.result->success ? "true" : "false",
              result.result->message.c_str());
            break;
          case rclcpp_action::ResultCode::ABORTED:
            RCLCPP_ERROR(get_logger(), "Goal was aborted");
            break;
          case rclcpp_action::ResultCode::CANCELED:
            RCLCPP_WARN(get_logger(), "Goal was canceled");
            break;
          default:
            RCLCPP_ERROR(get_logger(), "Unknown result code");
            break;
        }
        rclcpp::shutdown();
      };

    client_->async_send_goal(goal_msg, options);
  }

private:
  rclcpp_action::Client<CountDown>::SharedPtr client_;
  int seconds_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  if (argc != 2) {
    RCLCPP_ERROR(
      rclcpp::get_logger("countdown_client"),
      "Usage: ros2 run my_cpp_pkg countdown_client <seconds>");
    return 1;
  }
  auto node = std::make_shared<CountDownClient>(std::atoi(argv[1]));
  node->send_goal();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
