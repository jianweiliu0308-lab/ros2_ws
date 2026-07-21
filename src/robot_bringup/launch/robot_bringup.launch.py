from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """
    整机一键启动（生产 bringup 入口）。
    分层启动：driver -> perception -> motion -> supervisor
    """
    pkg_share = FindPackageShare('robot_bringup')

    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                PathJoinSubstitution([pkg_share, 'launch', 'driver.launch.py'])),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                PathJoinSubstitution([pkg_share, 'launch', 'perception.launch.py'])),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                PathJoinSubstitution([pkg_share, 'launch', 'motion.launch.py'])),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                PathJoinSubstitution([pkg_share, 'launch', 'supervisor.launch.py'])),
        ),
    ])
