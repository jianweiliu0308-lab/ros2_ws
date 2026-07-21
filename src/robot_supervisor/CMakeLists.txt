cmake_minimum_required(VERSION 3.5)
project(robot_supervisor)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(robot_interfaces REQUIRED)

add_executable(supervisor_node src/supervisor_node.cpp)
ament_target_dependencies(supervisor_node rclcpp rclcpp_action robot_interfaces)

install(TARGETS supervisor_node DESTINATION lib/${PROJECT_NAME})
ament_package()
