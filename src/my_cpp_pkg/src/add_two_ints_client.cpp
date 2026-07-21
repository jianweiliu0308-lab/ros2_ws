#include <chrono>
#include <memory>

#include "my_interface/srv/add_two_ints.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/** 加法客户端：命令行参数 a b，对应教程「服务 Service」 */
int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  if (argc != 3) {
    RCLCPP_ERROR(
      rclcpp::get_logger("add_two_ints_client"),
      "Usage: ros2 run my_cpp_pkg add_two_ints_client <a> <b>");
    return 1;
  }

  auto node = rclcpp::Node::make_shared("add_two_ints_client");
  auto client = node->create_client<my_interface::srv::AddTwoInts>("add_two_ints");

  while (!client->wait_for_service(1s)) {
    if (!rclcpp::ok()) {
      RCLCPP_ERROR(node->get_logger(), "Interrupted while waiting for service");
      return 1;
    }
    RCLCPP_INFO(node->get_logger(), "Waiting for /add_two_ints ...");
  }

  auto request = std::make_shared<my_interface::srv::AddTwoInts::Request>();
  request->a = std::atoll(argv[1]);
  request->b = std::atoll(argv[2]);

  auto future = client->async_send_request(request);
  if (rclcpp::spin_until_future_complete(node, future) ==
    rclcpp::FutureReturnCode::SUCCESS)
  {
    RCLCPP_INFO(node->get_logger(), "Sum: %ld", future.get()->sum);
  } else {
    RCLCPP_ERROR(node->get_logger(), "Failed to call service");
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}
