# Industrial Robot ROS 2 Workspace

本工作空间实现了一套**分层工业机器人软件架构**（仿真可运行），并配套文档。

## 文档

| 文档 | 内容 |
|------|------|
| [docs/industrial_robot_architecture.md](docs/industrial_robot_architecture.md) | 架构设计、分层、Lifecycle、Composition、安全链 |
| [docs/industrial_robot_guide.md](docs/industrial_robot_guide.md) | 编译、启动、模式切换、Composition 实操、安全演练 |
| [docs/ros2_cmd.md](docs/ros2_cmd.md) | ROS 2 命令参考 |
| [docs/ros2_practice.md](docs/ros2_practice.md) | ROS 2 基础实操 |

## 快速开始

```bash
source /opt/ros/foxy/setup.bash
cd /path/to/ros2_ws
colcon build --cmake-args -DPYTHON_EXECUTABLE=/usr/bin/python3
source install/setup.bash

# 整机启动（感知层默认 Composition 共进程）
ros2 launch robot_bringup robot_bringup.launch.py

# 感知层改回双进程（对照学习）
ros2 launch robot_bringup robot_bringup.launch.py use_composition:=false
```

## 包结构

```
src/
├── robot_interfaces/   # msg / srv / action
├── robot_driver/       # Lifecycle 驱动层（独立进程）
├── robot_perception/   # 安全 + 感知（Component，默认同进程）
├── robot_motion/       # 规划 + 执行（独立进程）
├── robot_supervisor/   # 状态机（独立进程）
└── robot_bringup/      # Launch + 配置
```

## 进程模型（默认）

```
独立进程: robot_driver | motion_* | supervisor
共进程:   perception_container
            ├─ safety_monitor
            └─ object_detector
```

## 进入自动模式

```bash
ros2 service call /supervisor/set_mode robot_interfaces/srv/SetMode "{mode: 2}"
```
