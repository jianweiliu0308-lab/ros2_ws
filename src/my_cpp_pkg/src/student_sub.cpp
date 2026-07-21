#include <memory>

#include "my_interface/msg/student_info.hpp"
#include "rclcpp/rclcpp.hpp"

/** 自定义消息订阅：StudentInfo */
class StudentSub : public rclcpp::Node
{
public:
  StudentSub()
  : Node("student_sub")
  {
    subscription_ = create_subscription<my_interface::msg::StudentInfo>(
      "student_info", 10,
      [this](const my_interface::msg::StudentInfo::SharedPtr msg) {
        RCLCPP_INFO(
          get_logger(), "Got student: name=%s age=%d score=%.1f",
          msg->name.c_str(), msg->age, msg->score);
      });
  }

private:
  rclcpp::Subscription<my_interface::msg::StudentInfo>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StudentSub>());
  rclcpp::shutdown();
  return 0;
}
