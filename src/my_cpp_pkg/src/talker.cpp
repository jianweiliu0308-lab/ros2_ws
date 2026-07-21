#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

/** 发布者：向 /chatter 发布 String，对应教程「话题 Topic」 */
class Talker : public rclcpp::Node
{
public:
  Talker()
  : Node("talker")
  {
    publisher_ = create_publisher<std_msgs::msg::String>("chatter", 10);
    timer_ = create_wall_timer(500ms, [this]() {
      auto msg = std_msgs::msg::String();
      msg.data = "Hello, count = " + std::to_string(count_++);
      RCLCPP_INFO(get_logger(), "Publishing: '%s'", msg.data.c_str());
      publisher_->publish(msg);
    });
  }

private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  size_t count_{0};
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Talker>());
  rclcpp::shutdown();
  return 0;
}
