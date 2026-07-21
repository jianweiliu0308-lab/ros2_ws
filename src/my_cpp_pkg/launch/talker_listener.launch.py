from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    """一次性启动 talker + listener，对应教程「Launch」。"""
    return LaunchDescription([
        Node(
            package='my_cpp_pkg',
            executable='talker',
            name='talker',
            output='screen',
        ),
        Node(
            package='my_cpp_pkg',
            executable='listener',
            name='listener',
            output='screen',
        ),
    ])
