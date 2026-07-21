from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    """启动加法服务端，便于配合 ros2 service call 练习。"""
    return LaunchDescription([
        Node(
            package='my_cpp_pkg',
            executable='add_two_ints_server',
            name='add_two_ints_server',
            output='screen',
        ),
    ])
