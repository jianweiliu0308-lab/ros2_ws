#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/** 最简节点：定时打印日志，对应教程「节点管理」 */
class HelloNode : public rclcpp::Node
{
public:
  HelloNode()
  : Node("hello_node")
  {
    timer_ = create_wall_timer(1s, [this]() {
      RCLCPP_INFO(get_logger(), "Hello ROS 2! count = %d", count_++);
    });
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  int count_{0};
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<HelloNode>());
  rclcpp::shutdown();
  return 0;
}
