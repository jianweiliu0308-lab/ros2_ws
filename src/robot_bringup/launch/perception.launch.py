from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import ComposableNodeContainer, Node
from launch_ros.descriptions import ComposableNode
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    """
    感知层启动。

    默认 use_composition:=true：
      safety_monitor + object_detector 共进程（Component Container）
    use_composition:=false：
      两个独立进程（便于对照 / 隔离调试）
    """
    params_file = PathJoinSubstitution([
        FindPackageShare('robot_bringup'), 'config', 'robot_params.yaml'])
    use_composition = LaunchConfiguration('use_composition')

    composition_group = GroupAction(
        condition=IfCondition(use_composition),
        actions=[
            ComposableNodeContainer(
                name='perception_container',
                namespace='',
                package='rclcpp_components',
                executable='component_container',
                output='screen',
                composable_node_descriptions=[
                    ComposableNode(
                        package='robot_perception',
                        plugin='robot_perception::SafetyMonitorNode',
                        name='safety_monitor',
                        parameters=[params_file],
                    ),
                    ComposableNode(
                        package='robot_perception',
                        plugin='robot_perception::ObjectDetectorNode',
                        name='object_detector',
                        parameters=[params_file],
                    ),
                ],
            ),
        ],
    )

    multiproc_group = GroupAction(
        condition=UnlessCondition(use_composition),
        actions=[
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
                parameters=[params_file],
            ),
        ],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'use_composition',
            default_value='true',
            description='Load perception nodes into one component_container process',
        ),
        composition_group,
        multiproc_group,
    ])
