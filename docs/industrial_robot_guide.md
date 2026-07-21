# 工业机器人系统实操指南

配合 [industrial_robot_architecture.md](./industrial_robot_architecture.md) 使用。

---

## 0. 环境准备

```bash
source /opt/ros/foxy/setup.bash
cd /mnt/cfs-software/algorithm/jianwei.liu/git/ros2_ws

colcon build --cmake-args -DPYTHON_EXECUTABLE=/usr/bin/python3
source install/setup.bash
```

---

## 1. 一键启动整机

```bash
ros2 launch robot_bringup robot_bringup.launch.py
```

**做了什么**：按层启动 driver（自动 lifecycle activate）→ perception → motion → supervisor。

启动后应看到节点：

```bash
ros2 node list
# /robot_driver
# /safety_monitor
# /object_detector
# /trajectory_planner
# /motion_executor
# /supervisor
# /world_to_base
```

---

## 2. 观察系统状态

### 2.1 机器人状态

```bash
ros2 topic echo /robot/state
# 关节位置、TCP 位姿、运行状态 (STOPPED/MOVING/HOLDING/FAULT)
```

### 2.2 安全状态

```bash
ros2 topic echo /robot/safety
# e_stop, protective_stop, safety_zone (SAFE/WARNING/VIOLATION)
```

### 2.3 感知目标

```bash
ros2 topic echo /perception/objects
```

### 2.4 查询监督层状态

```bash
ros2 service call /supervisor/get_status robot_interfaces/srv/GetRobotStatus "{}"
```

---

## 3. 模式切换（核心操作）

### 3.1 进入手动模式

```bash
ros2 service call /supervisor/set_mode robot_interfaces/srv/SetMode "{mode: 1}"
# mode: 0=IDLE, 1=MANUAL, 2=AUTO
```

### 3.2 进入自动模式（会自动周期运动）

```bash
ros2 service call /supervisor/set_mode robot_interfaces/srv/SetMode "{mode: 2}"
```

**做了什么**：supervisor 每 8 秒（可配置）向 `/motion/execute` 发一次演示运动。

观察执行过程：

```bash
ros2 action list -t
ros2 topic echo /robot/state
```

### 3.3 回到空闲

```bash
ros2 service call /supervisor/set_mode robot_interfaces/srv/SetMode "{mode: 0}"
```

---

## 4. 手动触发一次运动（Action）

不依赖 AUTO 模式，直接调用执行层：

```bash
ros2 action send_goal /motion/execute robot_interfaces/action/ExecuteMotion \
  "{target_joints: [0.3, 0.1, 0.0, 0.0, 0.2, 0.0], velocity_scale: 0.5}" --feedback
```

**流程**：
1. motion_executor 检查 safety
2. 调用 `/motion/plan_trajectory` 规划
3. 逐点发布 `/robot/motion_command`
4. driver 执行插值运动
5. 返回 progress feedback 和最终结果

---

## 5. 分层启动（调试用）

只启动某一层，便于隔离问题：

```bash
# 仅驱动
ros2 launch robot_bringup driver.launch.py

# 仅感知
ros2 launch robot_bringup perception.launch.py

# 仅运动
ros2 launch robot_bringup motion.launch.py

# 仅监督
ros2 launch robot_bringup supervisor.launch.py
```

---

## 6. 安全场景演练

### 6.1 模拟安全区违规

编辑 `robot_bringup/config/robot_params.yaml`，把 `violation_radius_m` 改小（如 `0.3`），重新编译安装后启动。

大关节运动时 TCP 超出半径 → `safety_zone=VIOLATION` → supervisor 进入 FAULT → 运动被拒绝。

### 6.2 模拟急停

```yaml
safety_monitor:
  ros__parameters:
    simulate_e_stop: true
```

重启后 `safety_ok=false`，无法进入 AUTO，ExecuteMotion 会被拒绝。

### 6.3 从故障恢复

```bash
# 先排除故障（改回参数 / 清除 e_stop）
ros2 service call /supervisor/set_mode robot_interfaces/srv/SetMode "{mode: 0}"
# 再重新 set_mode
```

---

## 7. 参数调优

配置文件：`src/robot_bringup/config/robot_params.yaml`

| 参数 | 节点 | 含义 |
|------|------|------|
| `dof` | robot_driver | 关节数 |
| `max_joint_velocity` | robot_driver | 最大关节速度 |
| `warning_radius_m` | safety_monitor | 警告区半径 |
| `violation_radius_m` | safety_monitor | 禁止区半径 |
| `interpolation_steps` | trajectory_planner | 规划插值点数 |
| `demo_target_joints` | supervisor | AUTO 模式目标关节 |
| `auto_task_period_sec` | supervisor | AUTO 任务周期 |

改完后：

```bash
colcon build --packages-select robot_bringup
source install/setup.bash
```

---

## 8. 录制与回放（运维）

```bash
# 录制关键话题
ros2 bag record -o robot_run /robot/state /robot/safety /perception/objects

# 回放
ros2 bag play robot_run
```

---

## 9. 接口速查

```bash
ros2 interface show robot_interfaces/msg/RobotState
ros2 interface show robot_interfaces/msg/SafetyStatus
ros2 interface show robot_interfaces/srv/SetMode
ros2 interface show robot_interfaces/action/ExecuteMotion
```

---

## 10. 推荐学习路径

1. `ros2 launch robot_bringup robot_bringup.launch.py` — 看整机起来  
2. `topic echo` — 理解数据流  
3. `set_mode AUTO` — 看监督层调度  
4. `action send_goal` — 理解规划→执行链路  
5. 改 `robot_params.yaml` — 触发 FAULT，理解安全链  
6. 读 `industrial_robot_architecture.md` — 对照分层图  

---

## 11. 常见问题

| 现象 | 原因 | 处理 |
|------|------|------|
| 无 `/robot/state` | driver 未 activate | 检查 driver.launch lifecycle |
| AUTO 不运动 | safety 不 OK 或不在 AUTO | `get_status` 排查 |
| Action 被拒绝 | 安全违规或 driver 未 active | 先 bringup 整机 |
| 编译接口失败 | 未用系统 Python | `-DPYTHON_EXECUTABLE=/usr/bin/python3` |
