from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, RegisterEventHandler
from launch.event_handlers import OnProcessStart
from launch.events import matches_action
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import LifecycleNode, Node
from launch_ros.event_handlers import OnStateTransition
from launch_ros.events.lifecycle import ChangeState
from launch_ros.substitutions import FindPackageShare
from lifecycle_msgs.msg import Transition


def generate_launch_description():
    """驱动层：Lifecycle 驱动 + 静态 TF。"""
    params_file = PathJoinSubstitution([
        FindPackageShare('robot_bringup'), 'config', 'robot_params.yaml'])

    driver_node = LifecycleNode(
        package='robot_driver',
        executable='robot_driver_node',
        name='robot_driver',
        namespace='',
        output='screen',
        parameters=[params_file],
    )

    # configure -> activate
    driver_configure = EmitEvent(
        event=ChangeState(
            lifecycle_node_matcher=matches_action(driver_node),
            transition_id=Transition.TRANSITION_CONFIGURE,
        )
    )
    driver_activate = RegisterEventHandler(
        OnProcessStart(
            target_action=driver_node,
            on_start=[driver_configure],
        )
    )
    driver_activate_handler = RegisterEventHandler(
        OnStateTransition(
            target_lifecycle_node=driver_node,
            goal_state='inactive',
            entities=[
                EmitEvent(
                    event=ChangeState(
                        lifecycle_node_matcher=matches_action(driver_node),
                        transition_id=Transition.TRANSITION_ACTIVATE,
                    )
                ),
            ],
        )
    )

    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='world_to_base',
        arguments=['0', '0', '0', '0', '0', '0', 'world', 'base_link'],
        output='screen',
    )

    return LaunchDescription([
        driver_node,
        driver_activate,
        driver_activate_handler,
        static_tf,
    ])
