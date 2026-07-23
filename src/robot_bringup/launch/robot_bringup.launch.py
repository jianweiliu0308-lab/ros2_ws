from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """
    整机一键启动（生产 bringup 入口）。
    分层启动：driver -> perception -> motion -> supervisor

    感知层默认 Composition（共进程）；可用 use_composition:=false 切回多进程。
    """
    pkg_share = FindPackageShare('robot_bringup')
    use_composition = LaunchConfiguration('use_composition')

    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument(
            'use_composition',
            default_value='true',
            description='Perception layer: true=one process (components), false=two processes',
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                PathJoinSubstitution([pkg_share, 'launch', 'driver.launch.py'])),
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                PathJoinSubstitution([pkg_share, 'launch', 'perception.launch.py'])),
            launch_arguments={'use_composition': use_composition}.items(),
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
