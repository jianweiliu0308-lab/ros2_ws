# 机器人实时状态（驱动层发布）
builtin_interfaces/Time stamp
float64[] joint_positions
float64[] joint_velocities
geometry_msgs/Pose tcp_pose
uint8 status

# status constants
uint8 STATUS_STOPPED=0
uint8 STATUS_MOVING=1
uint8 STATUS_HOLDING=2
uint8 STATUS_FAULT=3
