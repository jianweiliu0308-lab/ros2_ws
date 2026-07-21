cmake_minimum_required(VERSION 3.5)
project(my_cpp_pkg)

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(std_msgs REQUIRED)
find_package(my_interface REQUIRED)
find_package(lifecycle_msgs REQUIRED)

# ---- Topic ----
add_executable(hello_node src/hello_node.cpp)
ament_target_dependencies(hello_node rclcpp)

add_executable(talker src/talker.cpp)
ament_target_dependencies(talker rclcpp std_msgs)

add_executable(listener src/listener.cpp)
ament_target_dependencies(listener rclcpp std_msgs)

add_executable(student_pub src/student_pub.cpp)
ament_target_dependencies(student_pub rclcpp my_interface)

add_executable(student_sub src/student_sub.cpp)
ament_target_dependencies(student_sub rclcpp my_interface)

# ---- Service ----
add_executable(add_two_ints_server src/add_two_ints_server.cpp)
ament_target_dependencies(add_two_ints_server rclcpp my_interface)

add_executable(add_two_ints_client src/add_two_ints_client.cpp)
ament_target_dependencies(add_two_ints_client rclcpp my_interface)

add_executable(student_server src/student_server.cpp)
ament_target_dependencies(student_server rclcpp my_interface)

# ---- Action ----
add_executable(countdown_server src/countdown_server.cpp)
ament_target_dependencies(countdown_server rclcpp rclcpp_action my_interface)

add_executable(countdown_client src/countdown_client.cpp)
ament_target_dependencies(countdown_client rclcpp rclcpp_action my_interface)

# ---- Parameter ----
add_executable(param_node src/param_node.cpp)
ament_target_dependencies(param_node rclcpp)

# ---- Lifecycle ----
add_executable(lifecycle_talker src/lifecycle_talker.cpp)
ament_target_dependencies(lifecycle_talker rclcpp rclcpp_lifecycle std_msgs lifecycle_msgs)

install(TARGETS
  hello_node
  talker
  listener
  student_pub
  student_sub
  add_two_ints_server
  add_two_ints_client
  student_server
  countdown_server
  countdown_client
  param_node
  lifecycle_talker
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME})

ament_package()
