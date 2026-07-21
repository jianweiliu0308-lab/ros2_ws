#include <memory>
#include <string>
#include <unordered_map>

#include "my_interface/srv/student.hpp"
#include "rclcpp/rclcpp.hpp"

/** 学生查询服务端，练习自定义 srv Student */
class StudentServer : public rclcpp::Node
{
public:
  StudentServer()
  : Node("student_server")
  {
    db_["Alice"] = "Alice, age=20, score=95.0";
    db_["Bob"] = "Bob, age=21, score=88.5";
    db_["Carol"] = "Carol, age=19, score=91.0";

    service_ = create_service<my_interface::srv::Student>(
      "query_student",
      [this](
        const std::shared_ptr<my_interface::srv::Student::Request> request,
        std::shared_ptr<my_interface::srv::Student::Response> response)
      {
        auto it = db_.find(request->name);
        if (it != db_.end()) {
          response->success = true;
          response->info = it->second;
        } else {
          response->success = false;
          response->info = "Student not found: " + request->name;
        }
        RCLCPP_INFO(
          get_logger(), "Query '%s' -> success=%s info='%s'",
          request->name.c_str(),
          response->success ? "true" : "false",
          response->info.c_str());
      });
    RCLCPP_INFO(get_logger(), "Ready: service /query_student (Alice/Bob/Carol)");
  }

private:
  rclcpp::Service<my_interface::srv::Student>::SharedPtr service_;
  std::unordered_map<std::string, std::string> db_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StudentServer>());
  rclcpp::shutdown();
  return 0;
}
