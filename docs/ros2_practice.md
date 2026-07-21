# ROS 2 实操手册（配合 `ros2_cmd.md`）

本手册与 `docs/ros2_cmd.md` 命令分层指南一一对应，用本仓库 `src/` 下的示例包完成练习。  
**每个命令后面都标注「做了什么」**，方便边敲边理解。

| 教程章节 | 对应包 / 程序 |
|---------|----------------|
| 一、环境与工作空间 | `colcon` + 本工作空间 |
| 二、节点管理 | `my_cpp_pkg/hello_node`、`lifecycle_talker` |
| 三、话题 Topic | `talker` / `listener`、`student_pub` / `student_sub` |
| 三、服务 Service | `add_two_ints_*`、`student_server` |
| 三、动作 Action | `countdown_*` |
| 三、参数 Parameter | `param_node` + `config/param_node.yaml` |
| 四、Launch / Bag | `launch/*.launch.py` |
| 七、接口管理 | `my_interface`（msg/srv/action） |

发行版：**ROS 2 Foxy**（本机 `/opt/ros/foxy`）。

---

## 0. 一次性准备

### 0.1 加载环境（每个新终端都要做）

```bash
source /opt/ros/foxy/setup.bash
# 做了什么：把 ROS 2 Foxy 的命令、库路径、消息类型等写进当前 shell 环境，
#          这样后面才能用 ros2 / colcon 等命令。

cd /mnt/cfs-software/algorithm/jianwei.liu/git/ros2_ws
# 做了什么：进入工作空间根目录（编译、source install 都在这里做）。
```

### 0.2 编译

```bash
colcon build --packages-select my_interface my_cpp_pkg \
  --cmake-args -DPYTHON_EXECUTABLE=/usr/bin/python3
# 做了什么：
#   - 只编译 my_interface（接口）和 my_cpp_pkg（演示节点）两个包
#   - 把源码编成可执行文件，装到 install/ 目录
#   - 指定系统 Python，避免 conda Python 导致 ament 报错

source install/setup.bash
# 做了什么：把本工作空间里刚编译好的包加入环境，
#          之后 ros2 run my_cpp_pkg xxx 才能找到节点。
```

### 0.3 确认包与接口（对应教程「包管理 / 接口管理」）

```bash
ros2 pkg list | grep -E 'my_cpp_pkg|my_interface'
# 做了什么：检查环境里是否已经能看到这两个包。

ros2 pkg executables my_cpp_pkg
# 做了什么：列出 my_cpp_pkg 里有哪些可执行节点（talker、listener 等）。

ros2 pkg prefix my_cpp_pkg
# 做了什么：显示该包的安装路径（一般在 install/my_cpp_pkg）。

ros2 interface list | grep my_interface
# 做了什么：列出自定义接口：StudentInfo / AddTwoInts / Student / CountDown。

ros2 interface show my_interface/msg/StudentInfo
# 做了什么：查看消息字段定义（name / age / score）。

ros2 interface show my_interface/srv/AddTwoInts
# 做了什么：查看服务请求(a,b)和响应(sum)的结构。

ros2 interface show my_interface/action/CountDown
# 做了什么：查看动作的 Goal / Result / Feedback 三段结构。
```

---

## 实验 1：节点管理（对应教程第二节）

**这一节学什么**：节点是 ROS 2 里的基本运行单元；学会启动节点、查看节点信息、管理 daemon。

### 1.1 运行与查看节点

```bash
# ---- 终端 A ----
ros2 run my_cpp_pkg hello_node
# 做了什么：启动最简节点。它每秒打印一句日志，证明「节点在跑」。

# ---- 终端 B ----
ros2 node list
# 做了什么：列出当前系统中所有活跃节点，应能看到 /hello_node。

ros2 node info /hello_node
# 做了什么：查看该节点详情：它发布/订阅了哪些话题、有哪些服务等。
```

**预期**：终端 A 每秒打印 `Hello ROS 2! count = ...`；终端 B 能列出并查看 `/hello_node`。

### 1.2 Daemon（对应教程 2.2）

```bash
ros2 daemon status
# 做了什么：查看 ROS 2 后台守护进程是否在跑（它缓存节点/话题信息，加速 list 等命令）。

ros2 daemon stop
# 做了什么：停止守护进程。之后第一次 ros2 node list 可能稍慢（会重建缓存）。

ros2 daemon start
# 做了什么：重新启动守护进程。

ros2 daemon restart
# 做了什么：一键重启。节点连接异常时常用。
```

---

## 实验 2：话题 Topic（对应教程 3.1）

**这一节学什么**：Topic 是「一对多、持续流式」通信。发布者往话题写消息，订阅者从话题读消息。

数据流：

```
talker  --publish-->  /chatter  --subscribe-->  listener
```

### 2.1 标准 String 发布/订阅

```bash
# ---- 终端 A ----
ros2 run my_cpp_pkg listener
# 做了什么：启动订阅者，监听 /chatter，收到消息就打印 "I heard: ..."。

# ---- 终端 B ----
ros2 run my_cpp_pkg talker
# 做了什么：启动发布者，每 0.5 秒向 /chatter 发一条 String。

# ---- 终端 C：命令练习 ----
ros2 topic list -t
# 做了什么：列出所有话题，并显示消息类型（-t）。应看到 /chatter [std_msgs/msg/String]。

ros2 topic info /chatter
# 做了什么：看这个话题有几个发布者、几个订阅者、消息类型是什么。

ros2 topic echo /chatter
# 做了什么：实时把 /chatter 上的消息内容打印到屏幕（调试最常用）。

ros2 topic hz /chatter
# 做了什么：统计话题发布频率（约 2 Hz）。

ros2 topic bw /chatter
# 做了什么：统计话题占用的带宽。
```

手动发一条（不启动 talker 时也可）：

```bash
ros2 topic pub /chatter std_msgs/msg/String "{data: 'hello from cli'}" --once
# 做了什么：用命令行当「临时发布者」，只发一条消息到 /chatter。
#          若 listener 在跑，终端 A 应打印 I heard: 'hello from cli'。
```

### 2.2 自定义消息 StudentInfo（对应教程「接口」）

```bash
# ---- 终端 A ----
ros2 run my_cpp_pkg student_sub
# 做了什么：订阅自定义话题 /student_info，打印 name/age/score。

# ---- 终端 B ----
ros2 run my_cpp_pkg student_pub
# 做了什么：按自定义消息类型 StudentInfo 周期性发布学生信息。

# ---- 终端 C ----
ros2 topic echo /student_info
# 做了什么：用 CLI 确认自定义字段确实在话题里传输。

ros2 interface show my_interface/msg/StudentInfo
# 做了什么：对照源文件，确认运行时接口与 .msg 定义一致。
```

### 2.3 一键 Launch（对应教程 4.1）

```bash
ros2 launch my_cpp_pkg talker_listener.launch.py
# 做了什么：按 launch 文件一次性启动 talker + listener，
#          不用开两个终端分别 ros2 run。这是多节点工程的标准启动方式。
```

---

## 实验 3：服务 Service（对应教程 3.2）

**这一节学什么**：Service 是「请求-响应」通信：客户端发一次请求，服务端算完返回一次结果。适合短时查询/控制，不适合持续流数据。

### 3.1 加法服务 + CLI 调用

```bash
# ---- 终端 A：服务端 ----
ros2 run my_cpp_pkg add_two_ints_server
# 做了什么：启动加法服务，提供 /add_two_ints，等待别人来调用。

# 或
ros2 launch my_cpp_pkg service_demo.launch.py
# 做了什么：用 launch 启动同一个服务端，效果等价。

# ---- 终端 B：命令调用 ----
ros2 service list -t
# 做了什么：列出当前所有服务及类型，确认 /add_two_ints 存在。

ros2 service type /add_two_ints
# 做了什么：单独查询该服务的类型名。

ros2 service info /add_two_ints
# 做了什么：查看服务详情（类型、有哪些节点提供该服务等）。

ros2 service call /add_two_ints my_interface/srv/AddTwoInts "{a: 3, b: 5}"
# 做了什么：从命令行发起一次请求 a=3,b=5，服务端计算并返回 sum。
```

**预期**：返回 `sum: 8`；终端 A 打印收到请求的日志。

### 3.2 C++ 客户端

```bash
# 保持 server 运行
ros2 run my_cpp_pkg add_two_ints_client 10 20
# 做了什么：用 C++ 程序作为客户端，向 /add_two_ints 请求 10+20，
#          打印结果 Sum: 30。演示「代码里怎么调服务」，而不只是 CLI。
```

### 3.3 自定义 Student 查询服务

```bash
# ---- 终端 A ----
ros2 run my_cpp_pkg student_server
# 做了什么：启动学生查询服务 /query_student，内置 Alice/Bob/Carol 三条记录。

# ---- 终端 B ----
ros2 service call /query_student my_interface/srv/Student "{name: 'Alice'}"
# 做了什么：查询存在的学生，应返回 success: true 和 info 字符串。

ros2 service call /query_student my_interface/srv/Student "{name: 'Nobody'}"
# 做了什么：查询不存在的学生，应返回 success: false，体会「业务失败」与「调用失败」的区别。
```

**预期**：Alice 成功；Nobody 的 `success: false`。

---

## 实验 4：动作 Action（对应教程 3.3）

**这一节学什么**：Action 适合「耗时任务」：有目标(Goal)、过程反馈(Feedback)、最终结果(Result)，还可取消。比 Service 更适合导航、倒计时这类长时间过程。

```bash
# ---- 终端 A ----
ros2 run my_cpp_pkg countdown_server
# 做了什么：启动倒计时 Action 服务端 /countdown，等待接收目标。

# ---- 终端 B：CLI 发目标 ----
ros2 action list -t
# 做了什么：列出当前 Action 及类型，确认 /countdown 存在。

ros2 action info /countdown
# 做了什么：查看该 Action 的服务器/客户端数量等信息。

ros2 action send_goal /countdown my_interface/action/CountDown "{seconds: 5}" --feedback
# 做了什么：发送「倒计时 5 秒」目标；
#          --feedback 让终端持续打印剩余秒数（Feedback）；
#          结束后打印 Result（success / message）。
```

或用 C++ 客户端：

```bash
ros2 run my_cpp_pkg countdown_client 5
# 做了什么：用程序发同样的 Goal，并在回调里处理 Feedback / Result。
```

**预期**：每秒 feedback `remaining`，结束返回 `success: true`。

取消练习：

```bash
# 另开终端，在 send_goal 尚未结束时取消；或客户端运行中 Ctrl+C
# 做了什么：观察服务端走 cancel 分支，Result 变为 Canceled。
#          理解 Action「可中途取消」相对 Service 的优势。
```

---

## 实验 5：参数 Parameter（对应教程 3.4）

**这一节学什么**：参数是节点的可配置项，运行时可 get/set，也可 dump 成 YAML 再 load 回来。

```bash
# ---- 终端 A：带 YAML 启动 ----
ros2 launch my_cpp_pkg param_demo.launch.py
# 做了什么：启动 param_node，并从 config/param_node.yaml 加载初始参数
#          （robot_name=learner_bot, publish_rate_hz=2, verbose=true）。

# 或默认参数（不读 YAML）
# ros2 run my_cpp_pkg param_node
# 做了什么：用代码里 declare_parameter 的默认值启动。

# ---- 终端 B ----
ros2 param list /param_node
# 做了什么：列出该节点声明了哪些参数。

ros2 param get /param_node robot_name
# 做了什么：读取单个参数当前值。

ros2 param set /param_node robot_name "demo_bot"
# 做了什么：运行时修改参数；节点回调打印 Param changed，后续日志用新名字。

ros2 param set /param_node verbose false
# 做了什么：关掉详细日志，节点几乎不再打印 tick。

ros2 param set /param_node verbose true
# 做了什么：重新打开详细日志。

# dump / load
ros2 param dump /param_node > /tmp/param_node_dump.yaml
# 做了什么：把当前全部参数导出成 YAML 文件，便于备份或分享配置。

ros2 param set /param_node robot_name "temp"
# 做了什么：故意改乱参数，为后面 load 做对比。

ros2 param load /param_node /tmp/param_node_dump.yaml
# 做了什么：从 YAML 恢复参数，robot_name 应回到 dump 时的值。
```

**预期**：`set` 后节点日志出现 `Param changed: ...`；`load` 后恢复 dump 的值。

---

## 实验 6：生命周期 Lifecycle（对应教程 2.1 / 5.2）

**这一节学什么**：Lifecycle 节点有明确状态机。资源在 configure 时创建，真正干活在 activate 后；可 deactivate / cleanup，便于安全启停传感器、驱动等。

```bash
# ---- 终端 A ----
ros2 run my_cpp_pkg lifecycle_talker
# 做了什么：启动生命周期节点，初始一般是 unconfigured，还不会发消息。

# ---- 终端 B：观察话题 ----
ros2 topic echo /lifecycle_chatter
# 做了什么：盯住话题。activate 之前这里应几乎没有数据。

# ---- 终端 C：切换状态 ----
ros2 lifecycle get /lifecycle_talker
# 做了什么：查询当前生命周期状态（unconfigured / inactive / active ...）。

ros2 lifecycle set /lifecycle_talker configure
# 做了什么：触发 on_configure：创建 publisher、timer 等资源，进入 inactive。

ros2 lifecycle set /lifecycle_talker activate
# 做了什么：触发 on_activate：节点变为 active，开始真正向 /lifecycle_chatter 发布。
#          此时终端 B 应开始收到消息。

ros2 lifecycle set /lifecycle_talker deactivate
# 做了什么：触发 on_deactivate：停止发布，但仍保留已配置资源。

ros2 lifecycle set /lifecycle_talker cleanup
# 做了什么：触发 on_cleanup：释放 publisher/timer，回到 unconfigured。
```

**预期**：仅在 `activate` 后发布；`deactivate` 后停止。

常用状态流转：

```
unconfigured --configure--> inactive --activate--> active
active --deactivate--> inactive --cleanup--> unconfigured
```

---

## 实验 7：Rosbag2（对应教程 4.2）

**这一节学什么**：把话题数据录成 bag，之后可离线回放，用于复现问题、算法离线调试。

```bash
# ---- 终端 A：产生数据 ----
ros2 run my_cpp_pkg talker
# 做了什么：产生持续的 /chatter 数据流，作为录制源。

# ---- 终端 B：录制 ----
cd /tmp
ros2 bag record -o my_chatter_bag /chatter
# 做了什么：只录制 /chatter，输出目录名为 my_chatter_bag。
# Ctrl+C
# 做了什么：结束录制，把已收到的消息落盘。

ros2 bag info my_chatter_bag
# 做了什么：查看 bag 里有哪些话题、时长、消息条数、文件大小等。

# 停掉 talker 后回放
ros2 bag play my_chatter_bag
# 做了什么：按录制时的时间轴，把消息重新发到 /chatter（无需 talker）。

# ---- 另开终端验证 ----
ros2 topic echo /chatter
# 做了什么：确认回放时话题内容与当初录制的一致。
```

可选：

```bash
ros2 bag play my_chatter_bag --loop
# 做了什么：循环回放，适合反复测试算法。

ros2 bag play my_chatter_bag --rate 0.5
# 做了什么：半速回放，方便慢慢观察。
```

---

## 实验 8：TF 快速体验（对应教程 4.3，系统自带工具）

**这一节学什么**：TF 描述坐标系之间的相对位姿（机器人 base、雷达、相机等）。

```bash
# ---- 终端 A：发布静态 TF ----
ros2 run tf2_ros static_transform_publisher 0 0 0.1 0 0 0 base_link laser_link
# 做了什么：发布静态变换：laser_link 相对 base_link 在 z 方向高 0.1 m，
#          姿态 yaw/pitch/roll 全 0。模拟「雷达装在车体上方」。

# ---- 终端 B ----
ros2 run tf2_ros tf2_echo base_link laser_link
# 做了什么：持续打印两个坐标系之间的平移和旋转，验证 TF 是否正确。

ros2 run tf2_tools view_frames
# 做了什么：生成 frames.pdf，把当前 TF 树画出来，方便看父子坐标系关系。
```

---

## 实验 9：诊断（对应教程第五节）

```bash
ros2 doctor
# 做了什么：检查 ROS 2 环境健康度（路径、DDS、常见配置问题等），给出告警。

ros2 doctor --report
# 做了什么：输出更详细的诊断报告（若当前版本支持该参数）。
```

---

## 源码地图（`src/`）

```
src/
├── my_interface/                 # 接口定义（先编译）
│   ├── msg/StudentInfo.msg
│   ├── srv/AddTwoInts.srv
│   ├── srv/Student.srv
│   └── action/CountDown.action
├── my_cpp_pkg/                   # C++ 学习节点
│   ├── src/
│   │   ├── hello_node.cpp
│   │   ├── talker.cpp / listener.cpp
│   │   ├── student_pub.cpp / student_sub.cpp
│   │   ├── add_two_ints_server.cpp / add_two_ints_client.cpp
│   │   ├── student_server.cpp
│   │   ├── countdown_server.cpp / countdown_client.cpp
│   │   ├── param_node.cpp
│   │   └── lifecycle_talker.cpp
│   ├── launch/
│   │   ├── talker_listener.launch.py
│   │   ├── param_demo.launch.py
│   │   └── service_demo.launch.py
│   └── config/param_node.yaml
└── my_first_pkg/                 # 已有 Python 示例（可选对照）
```

---

## 推荐练习顺序

1. 编译 + `pkg` / `interface` — 确认环境通了  
2. Topic（talker/listener → student_*）— 理解持续通信  
3. Service（CLI call → C++ client）— 理解请求响应  
4. Action（send_goal + feedback）— 理解长任务与反馈  
5. Param（get/set/dump/load）— 理解可配置行为  
6. Lifecycle — 理解安全启停  
7. Launch + Bag — 理解工程化启动与数据复现  
8. TF / doctor（可选）

每完成一节，回到 `ros2_cmd.md` 对应小节，把命令再默写一遍，效果最好。

---

## 常见问题

| 现象 | 处理 |
|------|------|
| `Package 'my_cpp_pkg' not found` | 未 `source install/setup.bash`（工作空间环境没加载） |
| 找不到 `my_interface/...` 类型 | 先编 `my_interface`，再编 `my_cpp_pkg`，再 source |
| zsh 下 `source setup.bash` 报错 | 用 `bash`，或 `source /opt/ros/foxy/setup.zsh` |
| lifecycle 话题无数据 | 忘记 `configure` + `activate`（节点还没进入 active） |
| service call 一直卡住 | 服务端未启动，或类型字符串写错 |

改代码后：

```bash
colcon build --packages-select my_cpp_pkg \
  --cmake-args -DPYTHON_EXECUTABLE=/usr/bin/python3
# 做了什么：重新编译改过的包。

source install/setup.bash
# 做了什么：刷新环境，让新二进制生效（有时还需要新开终端）。
```
