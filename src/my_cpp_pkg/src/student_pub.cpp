#include <chrono>
#include <memory>

#include "my_interface/msg/student_info.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/** 自定义消息发布：StudentInfo，对应教程「接口管理」+ Topic */
class StudentPub : public rclcpp::Node
{
public:
  StudentPub()
  : Node("student_pub")
  {
    publisher_ = create_publisher<my_interface::msg::StudentInfo>("student_info", 10);
    timer_ = create_wall_timer(1s, [this]() {
      my_interface::msg::StudentInfo msg;
      msg.name = "Alice";
      msg.age = 20 + static_cast<int>(count_ % 5);
      msg.score = 80.0f + static_cast<float>(count_ % 20);
      RCLCPP_INFO(
        get_logger(), "Publish student: name=%s age=%d score=%.1f",
        msg.name.c_str(), msg.age, msg.score);
      publisher_->publish(msg);
      ++count_;
    });
  }

private:
  rclcpp::Publisher<my_interface::msg::StudentInfo>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  size_t count_{0};
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StudentPub>());
  rclcpp::shutdown();
  return 0;
}
