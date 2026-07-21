#include <memory>

#include "my_interface/srv/add_two_ints.hpp"
#include "rclcpp/rclcpp.hpp"

/** 加法服务端，对应教程「服务 Service」 */
class AddTwoIntsServer : public rclcpp::Node
{
public:
  AddTwoIntsServer()
  : Node("add_two_ints_server")
  {
    service_ = create_service<my_interface::srv::AddTwoInts>(
      "add_two_ints",
      [this](
        const std::shared_ptr<my_interface::srv::AddTwoInts::Request> request,
        std::shared_ptr<my_interface::srv::AddTwoInts::Response> response)
      {
        response->sum = request->a + request->b;
        RCLCPP_INFO(
          get_logger(), "Request: a=%ld b=%ld -> sum=%ld",
          request->a, request->b, response->sum);
      });
    RCLCPP_INFO(get_logger(), "Ready: service /add_two_ints");
  }

private:
  rclcpp::Service<my_interface::srv::AddTwoInts>::SharedPtr service_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<AddTwoIntsServer>());
  rclcpp::shutdown();
  return 0;
}
