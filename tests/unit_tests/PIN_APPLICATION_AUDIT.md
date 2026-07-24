# 新 PCB 外设引脚应用核对表

本表以 `TI2026电赛引脚说明.md` 为基准，核对 SysConfig、生成宏、驱动层和测试入口。所有业务代码均引用 `ti_msp_dl_config.h` 生成的语义宏，不在驱动中重复硬编码引脚号。

| 外设 | SysConfig / 引脚 | 驱动或应用 | 测试编号 |
| --- | --- | --- | ---: |
| 调试串口 | UART3，TX=PA14，RX=PA25 | `hal_uart.c`、`log.c`，日志固定使用 `DEBUG_UART_INST` | 14 |
| 灰度循迹 | CLK=PB27，DAT=PB23 | `gray_detection.c`，使用 `PORTB_TRACK_CLK/DAT` | 7、11 |
| OLED | 软件 I2C，SCL=PA17，SDA=PA16 | `oled_driver.c` | 1、10 |
| 摄像头 | UART2，TX=PA21，RX=PB18 | `maix_cam.c`，RX 中断由 `CAM_UART_INST_IRQHandler` 分发 | 9 |
| 云台 | UART0，TX=PA0，RX=PA1 | `gimbal_uart.c` 提供原始 UART 收发；设备协议需按实际云台型号继续封装 | 15（断开云台后短接 PA0→PA1） |
| 陀螺仪 | UART1，TX=PA8，RX=PA9 | `lengke_gyro.c`，RX 中断由 `GYRO_UART_INST_IRQHandler` 分发 | 8、12 |
| 电机 C / M1 | TIMG7 PWMA=PB15，IN1=PB7，IN2=PB6 | `motor_user.c`、`motor_tb6612.c`，索引 0 | 4、13 |
| 电机 D / M2 | TIMG8 PWMB=PB16，IN1=PA12，IN2=PA13 | `motor_user.c`、`motor_tb6612.c`，索引 1 | 4、13 |
| 编码器 C | A=PB13，B=PB12 | `encoder_user.c`，双相双边沿中断，索引 0 | 5、13 |
| 编码器 D | A=PA29，B=PB26 | `encoder_user.c`，双相双边沿中断，索引 1 | 5、13 |
| 蜂鸣器 | EN=PB3 | `beep.c`，按有源蜂鸣器高电平使能处理 | 3 |
| 按钮 1～4 | PA26、PA27、PB25、PB20 | `ui_button.c`，内部上拉、按下为低 | 6 |
| 天猛星板载 B22 灯 | PB22，高电平点亮 | `board_led.c`，单路状态指示 | 2 |

## 测试选择

烧录后由 OLED `TEST MENU` 选择测试：短按 K3 向下移动，短按 K4 确认；按开发板复位键可退出当前测试并回到菜单，无需为切换测试重新编译。云台测试 15 是纯 UART 环回测试，只验证 PA0/PA1 和 UART0，不假设云台设备协议；执行前必须先断开云台并短接 PA0 到 PA1。

XTARK MC520P30 的 13 线霍尔编码器版采用 `13 × 30 × 4 = 1560 counts/rev`。第一次运行电机或编码器测试时必须架空车轮，并避免堵转。
