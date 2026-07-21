#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/**
 * 参数节点：演示 declare / get / 动态回调
 * 对应教程「参数 Parameter」：list / get / set / dump / load
 */
class ParamNode : public rclcpp::Node
{
public:
  ParamNode()
  : Node("param_node")
  {
    declare_parameter<std::string>("robot_name", "learner");
    declare_parameter<int>("publish_rate_hz", 1);
    declare_parameter<bool>("verbose", true);

    RCLCPP_INFO(
      get_logger(),
      "Init params: robot_name=%s publish_rate_hz=%ld verbose=%s",
      get_parameter("robot_name").as_string().c_str(),
      get_parameter("publish_rate_hz").as_int(),
      get_parameter("verbose").as_bool() ? "true" : "false");

    param_callback_handle_ = add_on_set_parameters_callback(
      [this](const std::vector<rclcpp::Parameter> & params) {
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;
        for (const auto & p : params) {
          RCLCPP_INFO(
            get_logger(), "Param changed: %s = %s",
            p.get_name().c_str(), p.value_to_string().c_str());
        }
        return result;
      });

    // 固定 1Hz 轮询；真正“频率感”用 verbose + robot_name 演示即可
    timer_ = create_wall_timer(1s, [this]() {
      if (!get_parameter("verbose").as_bool()) {
        return;
      }
      RCLCPP_INFO(
        get_logger(), "robot_name=%s rate=%ld tick=%d",
        get_parameter("robot_name").as_string().c_str(),
        get_parameter("publish_rate_hz").as_int(),
        tick_++);
    });
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;
  int tick_{0};
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ParamNode>());
  rclcpp::shutdown();
  return 0;
}
