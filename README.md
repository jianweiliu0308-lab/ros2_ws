# Industrial Robot ROS 2 Workspace

本工作空间实现了一套**分层工业机器人软件架构**（仿真可运行），并配套文档。

## 文档

| 文档 | 内容 |
|------|------|
| [docs/industrial_robot_architecture.md](docs/industrial_robot_architecture.md) | 架构设计、分层、数据流、状态机 |
| [docs/industrial_robot_guide.md](docs/industrial_robot_guide.md) | 编译、启动、模式切换、安全演练 |
| [docs/ros2_cmd.md](docs/ros2_cmd.md) | ROS 2 命令参考 |
| [docs/ros2_practice.md](docs/ros2_practice.md) | ROS 2 基础实操 |

## 快速开始

```bash
source /opt/ros/foxy/setup.bash
cd /path/to/ros2_ws
colcon build --cmake-args -DPYTHON_EXECUTABLE=/usr/bin/python3
source install/setup.bash
ros2 launch robot_bringup robot_bringup.launch.py
```

## 包结构

```
src/
├── robot_interfaces/   # msg / srv / action
├── robot_driver/       # Lifecycle 驱动层
├── robot_perception/   # 安全 + 感知
├── robot_motion/       # 规划 + 执行
├── robot_supervisor/   # 状态机
└── robot_bringup/      # Launch + 配置
```

## 进入自动模式

```bash
ros2 service call /supervisor/set_mode robot_interfaces/srv/SetMode "{mode: 2}"
```
