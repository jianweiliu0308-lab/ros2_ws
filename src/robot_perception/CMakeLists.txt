cmake_minimum_required(VERSION 3.5)
project(robot_perception)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robot_interfaces REQUIRED)

# ---- Safety monitor component (+ standalone executable) ----
add_library(safety_monitor_component SHARED src/safety_monitor_component.cpp)
target_compile_definitions(safety_monitor_component PRIVATE "COMPOSITION_BUILDING_DLL")
ament_target_dependencies(safety_monitor_component rclcpp rclcpp_components robot_interfaces)
rclcpp_components_register_node(safety_monitor_component
  PLUGIN "robot_perception::SafetyMonitorNode"
  EXECUTABLE safety_monitor_node)

# ---- Object detector component (+ standalone executable) ----
add_library(object_detector_component SHARED src/object_detector_component.cpp)
target_compile_definitions(object_detector_component PRIVATE "COMPOSITION_BUILDING_DLL")
ament_target_dependencies(object_detector_component
  rclcpp rclcpp_components geometry_msgs robot_interfaces)
rclcpp_components_register_node(object_detector_component
  PLUGIN "robot_perception::ObjectDetectorNode"
  EXECUTABLE object_detector_node)

install(TARGETS
  safety_monitor_component
  object_detector_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

ament_package()
