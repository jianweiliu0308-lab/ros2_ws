from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.substitutions import PathJoinSubstitution


def generate_launch_description():
    """启动 param_node 并加载 YAML，练习 param dump/load。"""
    params_file = PathJoinSubstitution([
        FindPackageShare('my_cpp_pkg'),
        'config',
        'param_node.yaml',
    ])
    return LaunchDescription([
        Node(
            package='my_cpp_pkg',
            executable='param_node',
            name='param_node',
            output='screen',
            parameters=[params_file],
        ),
    ])
