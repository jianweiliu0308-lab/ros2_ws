cmake_minimum_required(VERSION 3.5)
project(robot_perception)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robot_interfaces REQUIRED)

add_executable(safety_monitor_node src/safety_monitor_node.cpp)
ament_target_dependencies(safety_monitor_node rclcpp robot_interfaces)

add_executable(object_detector_node src/object_detector_node.cpp)
ament_target_dependencies(object_detector_node rclcpp geometry_msgs robot_interfaces)

install(TARGETS safety_monitor_node object_detector_node DESTINATION lib/${PROJECT_NAME})
ament_package()
