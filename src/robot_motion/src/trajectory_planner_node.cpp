#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "robot_interfaces/srv/plan_trajectory.hpp"
#include "rclcpp/rclcpp.hpp"

using PlanTrajectory = robot_interfaces::srv::PlanTrajectory;

/** 规划层：提供 /motion/plan_trajectory 服务，线性插值生成轨迹点 */
class TrajectoryPlannerNode : public rclcpp::Node
{
public:
  TrajectoryPlannerNode()
  : Node("trajectory_planner")
  {
    declare_parameter<int>("interpolation_steps", 10);
    steps_ = get_parameter("interpolation_steps").as_int();

    service_ = create_service<PlanTrajectory>(
      "motion/plan_trajectory",
      std::bind(
        &TrajectoryPlannerNode::on_plan, this,
        std::placeholders::_1, std::placeholders::_2));
    RCLCPP_INFO(get_logger(), "Trajectory planner ready");
  }

private:
  void on_plan(
    const std::shared_ptr<PlanTrajectory::Request> request,
    std::shared_ptr<PlanTrajectory::Response> response)
  {
    if (request->start_joints.size() != request->goal_joints.size() ||
      request->start_joints.empty())
    {
      response->success = false;
      response->message = "Invalid joint dimensions";
      return;
    }

    const auto n = request->start_joints.size();
    const int total_points = std::max(2, steps_);
    response->trajectory_points.clear();
    response->trajectory_points.reserve(n * total_points);

    for (int s = 0; s < total_points; ++s) {
      const double alpha = static_cast<double>(s) / (total_points - 1);
      for (size_t i = 0; i < n; ++i) {
        const double value =
          request->start_joints[i] * (1.0 - alpha) + request->goal_joints[i] * alpha;
        response->trajectory_points.push_back(value);
      }
    }

    response->success = true;
    response->message = "Planned " + std::to_string(total_points) + " waypoints";
    RCLCPP_INFO(get_logger(), "%s", response->message.c_str());
  }

  int steps_{10};
  rclcpp::Service<PlanTrajectory>::SharedPtr service_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TrajectoryPlannerNode>());
  rclcpp::shutdown();
  return 0;
}
