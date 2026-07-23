#include <chrono>
#include <cmath>
#include <memory>
#include <string>

#include "geometry_msgs/msg/pose.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "robot_interfaces/msg/detected_object.hpp"

using DetectedObject = robot_interfaces::msg::DetectedObject;

namespace robot_perception
{

/**
 * 目标检测组件：可独立进程运行，也可与 safety_monitor 共进程。
 * 发布 /perception/objects。
 */
class ObjectDetectorNode : public rclcpp::Node
{
public:
  explicit ObjectDetectorNode(const rclcpp::NodeOptions & options)
  : Node("object_detector", options)
  {
    pub_ = create_publisher<DetectedObject>("perception/objects", 10);
    timer_ = create_wall_timer(
      std::chrono::seconds(1),
      std::bind(&ObjectDetectorNode::publish_objects, this));
    RCLCPP_INFO(get_logger(), "Object detector ready (component)");
  }

private:
  void publish_objects()
  {
    DetectedObject obj;
    obj.id = "part_" + std::to_string(seq_ % 3);
    obj.confidence = 0.85f + 0.05f * static_cast<float>(seq_ % 3);
    obj.pose.position.x = 0.4 + 0.05 * std::sin(seq_ * 0.3);
    obj.pose.position.y = 0.2 * std::cos(seq_ * 0.2);
    obj.pose.position.z = 0.1;
    obj.pose.orientation.w = 1.0;
    pub_->publish(obj);
    ++seq_;
  }

  rclcpp::Publisher<DetectedObject>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  int seq_{0};
};

}  // namespace robot_perception

RCLCPP_COMPONENTS_REGISTER_NODE(robot_perception::ObjectDetectorNode)
