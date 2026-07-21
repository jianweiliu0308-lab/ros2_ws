cmake_minimum_required(VERSION 3.5)
project(robot_motion)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(robot_interfaces REQUIRED)

add_executable(trajectory_planner_node src/trajectory_planner_node.cpp)
ament_target_dependencies(trajectory_planner_node rclcpp robot_interfaces)

add_executable(motion_executor_node src/motion_executor_node.cpp)
ament_target_dependencies(motion_executor_node rclcpp rclcpp_action robot_interfaces)

install(TARGETS trajectory_planner_node motion_executor_node DESTINATION lib/${PROJECT_NAME})
ament_package()
