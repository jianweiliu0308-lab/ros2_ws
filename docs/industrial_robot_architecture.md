# 工业机器人软件架构说明

本文档描述本工作空间 `src/` 下的分层机器人软件架构，面向**生产级 ROS 2 系统设计**。

---

## 1. 设计目标

| 目标 | 实现方式 |
|------|----------|
| 模块解耦 | 按职责分包，接口集中在 `robot_interfaces` |
| 可替换 | 驱动/规划/感知可独立升级，上层话题与服务不变 |
| 安全可控 | 安全监控 + Supervisor 状态机 + 运动前安全检查 |
| 可部署 | `robot_bringup` 分层 Launch，一键启动整机 |
| 可运维 | 统一参数 YAML、标准诊断话题、支持 bag 录制 |

---

## 2. 分层架构

```
┌─────────────────────────────────────────────────────────────┐
│                    robot_bringup (部署层)                    │
│         launch/*.launch.py  +  config/robot_params.yaml      │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                 robot_supervisor (监督层)                    │
│   状态机: IDLE / MANUAL / AUTO / FAULT                       │
│   服务: /supervisor/set_mode  /supervisor/get_status         │
└─────────────────────────────────────────────────────────────┘
          │                                    ▲
          ▼                                    │
┌──────────────────────┐            ┌─────────────────────────┐
│   robot_motion       │            │   robot_perception      │
│ 规划: plan_trajectory│            │ 安全: /robot/safety      │
│ 执行: execute (Action)│            │ 感知: /perception/objects│
└──────────────────────┘            └─────────────────────────┘
          │                                    ▲
          ▼                                    │
┌─────────────────────────────────────────────────────────────┐
│                   robot_driver (驱动层)                      │
│   Lifecycle: configure → activate                            │
│   发布: /robot/state    订阅: /robot/motion_command          │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│              robot_interfaces (接口契约层)                   │
│   msg / srv / action — 全系统 API 定义                       │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. 包职责

| 包名 | 层级 | 职责 |
|------|------|------|
| `robot_interfaces` | 接口 | 消息、服务、动作定义，无运行时代码 |
| `robot_driver` | 驱动 | 硬件抽象，Lifecycle 管理，发布关节/TCP 状态 |
| `robot_perception` | 感知 | 安全区域监控、目标检测（本仓库为仿真） |
| `robot_motion` | 运动 | 轨迹规划服务 + Action 执行器 |
| `robot_supervisor` | 监督 | 运行模式管理、自动任务调度、故障降级 |
| `robot_bringup` | 部署 | Launch 编排、参数配置、TF 静态树 |

---

## 4. 数据流

### 4.1 状态流（Topic）

```
robot_driver  ──/robot/state────────►  safety_monitor
                │                      motion_executor
                │                      supervisor
                │
robot_driver  ◄──/robot/motion_command──  motion_executor

safety_monitor ──/robot/safety──────►  motion_executor
                                       supervisor

object_detector ──/perception/objects──► (HMI / 上层应用)
```

### 4.2 控制流（Service / Action）

```
HMI / CLI
   │
   ├─ /supervisor/set_mode (Service)     → supervisor
   ├─ /supervisor/get_status (Service)   → supervisor
   │
supervisor ──/motion/execute (Action)──► motion_executor
motion_executor ──/motion/plan_trajectory (Service)──► trajectory_planner
```

### 4.3 自动模式时序

```
1. 用户: set_mode AUTO
2. supervisor 检查 safety_ok
3. supervisor 发送 ExecuteMotion Goal
4. motion_executor 调用 plan_trajectory
5. motion_executor 逐点发布 motion_command
6. robot_driver 插值运动并反馈 /robot/state
7. safety_monitor 持续评估安全区
8. 完成 / 取消 / 安全违规 → 返回 Result
```

---

## 5. 状态机（Supervisor）

```
                    ┌──────────┐
         ┌─────────│   IDLE   │◄────────┐
         │         └────┬─────┘         │
         │              │ set MANUAL    │ set IDLE (from FAULT)
         │              ▼               │
         │         ┌──────────┐         │
         │         │  MANUAL  │         │
         │         └──────────┘         │
         │              │ set AUTO       │
         │              ▼ (safety_ok)    │
         │         ┌──────────┐         │
         │         │   AUTO   │─────────┤ periodic ExecuteMotion
         │         └────┬─────┘         │
         │              │ safety fault  │
         │              ▼               │
         └─────────►┌──────────┐────────┘
                    │  FAULT   │
                    └──────────┘
```

**FAULT 触发条件**：急停、保护性停止、安全区 VIOLATION。

---

## 6. Lifecycle（驱动层）

`robot_driver` 使用 Lifecycle 节点，符合工业现场「先配置、再激活、可安全下线」的模式：

| 状态 | 行为 |
|------|------|
| unconfigured | 未初始化 |
| inactive (configured) | 已创建 publisher/subscriber，但不发布 |
| active | 50Hz 控制循环，接收运动指令 |
| deactivated | 停止运动，保持配置 |
| cleaned up | 释放资源 |

`driver.launch.py` 自动执行 configure → activate。

---

## 7. 接口清单

### Topic

| 话题 | 类型 | 发布者 | 订阅者 |
|------|------|--------|--------|
| `/robot/state` | `RobotState` | driver | safety, executor, supervisor |
| `/robot/safety` | `SafetyStatus` | safety_monitor | executor, supervisor |
| `/robot/motion_command` | `MotionCommand` | motion_executor | driver |
| `/perception/objects` | `DetectedObject` | object_detector | (应用层) |

### Service

| 服务 | 类型 | 提供者 |
|------|------|--------|
| `/supervisor/set_mode` | `SetMode` | supervisor |
| `/supervisor/get_status` | `GetRobotStatus` | supervisor |
| `/motion/plan_trajectory` | `PlanTrajectory` | trajectory_planner |

### Action

| 动作 | 类型 | 服务端 |
|------|------|--------|
| `/motion/execute` | `ExecuteMotion` | motion_executor |

---

## 8. 与真实生产的差异（本仓库定位）

本仓库是**可运行的架构骨架 + 仿真实现**，便于学习。真实产线还会增加：

- 真实 EtherCAT / CAN 驱动替代仿真 driver
- MoveIt / 专有运动学库替代简单插值规划
- 硬实时控制回路（独立 RTOS 或 ros2_control）
- 诊断上报（`diagnostic_updater`）、权限/HMI、OTA
- DDS QoS 调优、多机域隔离、网络安全

但**分层、接口、bringup、supervisor、lifecycle** 的组织方式与生产一致。

---

## 9. 扩展指南

| 需求 | 建议 |
|------|------|
| 换机械臂型号 | 只改 `robot_driver`，保持 `/robot/state` 不变 |
| 加视觉算法 | 新包发布到 `/perception/objects`，不改 supervisor |
| 加新任务类型 | 在 `robot_interfaces` 增 action，supervisor 调度 |
| 多工位 | 每工位独立 namespace + 独立 bringup |
| 仿真/实车切换 | launch 参数 `use_sim_time` + 不同 driver 插件 |

---

## 10. 源码目录

```
src/
├── robot_interfaces/     # API 契约
├── robot_driver/         # 驱动层 (Lifecycle)
├── robot_perception/     # 感知 + 安全
├── robot_motion/         # 规划 + 执行
├── robot_supervisor/     # 状态机
└── robot_bringup/        # Launch + 配置
```

详细操作见 [industrial_robot_guide.md](./industrial_robot_guide.md)。
