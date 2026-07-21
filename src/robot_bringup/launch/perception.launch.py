from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """感知层：安全监控 + 目标检测。"""
    params_file = PathJoinSubstitution([
        FindPackageShare('robot_bringup'), 'config', 'robot_params.yaml'])

    return LaunchDescription([
        Node(
            package='robot_perception',
            executable='safety_monitor_node',
            name='safety_monitor',
            output='screen',
            parameters=[params_file],
        ),
        Node(
            package='robot_perception',
            executable='object_detector_node',
            name='object_detector',
            output='screen',
        ),
    ])
