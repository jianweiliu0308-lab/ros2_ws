cmake_minimum_required(VERSION 3.5)
project(robot_driver)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robot_interfaces REQUIRED)

add_executable(robot_driver_node src/robot_driver_node.cpp)
ament_target_dependencies(robot_driver_node
  rclcpp rclcpp_lifecycle lifecycle_msgs geometry_msgs robot_interfaces)

install(TARGETS robot_driver_node DESTINATION lib/${PROJECT_NAME})
ament_package()
