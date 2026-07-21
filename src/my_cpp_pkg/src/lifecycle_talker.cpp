#include <chrono>
#include <memory>
#include <string>

#include "lifecycle_msgs/msg/state.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;
using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

/**
 * 生命周期发布节点
 * 对应教程「lifecycle set/get」：configure -> activate -> deactivate -> cleanup
 * 仅在 Active 状态才会真正发布话题
 */
class LifecycleTalker : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit LifecycleTalker(const std::string & node_name = "lifecycle_talker")
  : LifecycleNode(node_name)
  {
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &)
  {
    publisher_ = create_publisher<std_msgs::msg::String>("lifecycle_chatter", 10);
    timer_ = create_wall_timer(1s, [this]() {
      if (get_current_state().id() != lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE) {
        return;
      }
      auto msg = std_msgs::msg::String();
      msg.data = "lifecycle hello " + std::to_string(count_++);
      RCLCPP_INFO(get_logger(), "Publishing: '%s'", msg.data.c_str());
      publisher_->publish(msg);
    });
    RCLCPP_INFO(get_logger(), "on_configure()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State & state)
  {
    LifecycleNode::on_activate(state);
    RCLCPP_INFO(get_logger(), "on_activate() -> start publishing");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state)
  {
    LifecycleNode::on_deactivate(state);
    RCLCPP_INFO(get_logger(), "on_deactivate() -> stop publishing");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &)
  {
    timer_.reset();
    publisher_.reset();
    RCLCPP_INFO(get_logger(), "on_cleanup()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &)
  {
    timer_.reset();
    publisher_.reset();
    RCLCPP_INFO(get_logger(), "on_shutdown()");
    return CallbackReturn::SUCCESS;
  }

private:
  std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>> publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  size_t count_{0};
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<LifecycleTalker>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}
