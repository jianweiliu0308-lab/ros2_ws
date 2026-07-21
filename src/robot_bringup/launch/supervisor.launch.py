from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """监督层：整机状态机。"""
    params_file = PathJoinSubstitution([
        FindPackageShare('robot_bringup'), 'config', 'robot_params.yaml'])

    return LaunchDescription([
        Node(
            package='robot_supervisor',
            executable='supervisor_node',
            name='supervisor',
            output='screen',
            parameters=[params_file],
        ),
    ])
