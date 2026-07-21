from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """运动层：轨迹规划 + 运动执行。"""
    params_file = PathJoinSubstitution([
        FindPackageShare('robot_bringup'), 'config', 'robot_params.yaml'])

    return LaunchDescription([
        Node(
            package='robot_motion',
            executable='trajectory_planner_node',
            name='trajectory_planner',
            output='screen',
            parameters=[params_file],
        ),
        Node(
            package='robot_motion',
            executable='motion_executor_node',
            name='motion_executor',
            output='screen',
        ),
    ])
