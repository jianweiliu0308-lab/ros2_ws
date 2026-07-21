# 运动控制指令（控制层 -> 驱动层）
uint8 command_type
float64[] targets
float64 velocity_scale

uint8 CMD_JOINT=0
uint8 CMD_LINEAR=1
uint8 CMD_STOP=2
