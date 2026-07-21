cmake_minimum_required(VERSION 3.5)
project(robot_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(geometry_msgs REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/RobotState.msg"
  "msg/SafetyStatus.msg"
  "msg/DetectedObject.msg"
  "msg/MotionCommand.msg"
  "srv/SetMode.srv"
  "srv/GetRobotStatus.srv"
  "srv/PlanTrajectory.srv"
  "action/ExecuteMotion.action"
  DEPENDENCIES builtin_interfaces geometry_msgs
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
