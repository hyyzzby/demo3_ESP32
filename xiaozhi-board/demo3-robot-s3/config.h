#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// =====================================================================
//  demo3_ESP32 桌面陪伴机器人 · ESP32-S3 语音版 板级配置
//  基于 bread-compact-wifi 修改（音频引脚与其完全一致）
//  差异：OLED 8/9 · 128x64 · 触摸 T14(电容) · 舵机 11/12 · 去掉音量键/灯控
// =====================================================================

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

#define AUDIO_I2S_METHOD_SIMPLEX

#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_4
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_5
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_6
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_7
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_15
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_16

// 板载 RGB WS2812（DevKitC-1 v1.0/v1.1 = GPIO48）
#define BUILTIN_LED_GPIO        GPIO_NUM_48
#define BOOT_BUTTON_GPIO        GPIO_NUM_0

// 电容触摸（摸头）——不是数字 GPIO 按钮，由 demo3_robot_board.cc 内
// CapTouch 类轮询 T14 实现（带开机基线自校准，阈值 60%）
#define DEMO3_TOUCH_CHANNEL     TOUCH_PAD_NUM14   // GPIO14
#define DEMO3_TOUCH_RATIO       0.60f

// ---- 头部双舵机（MCP 工具 self.robot.* 可语音控制）----
#define DEMO3_SERVO_PAN_GPIO    GPIO_NUM_11
#define DEMO3_SERVO_TILT_GPIO   GPIO_NUM_12
#define DEMO3_SERVO_PAN_MIN     45
#define DEMO3_SERVO_PAN_MAX     135
#define DEMO3_SERVO_TILT_MIN    60
#define DEMO3_SERVO_TILT_MAX    120
#define DEMO3_SERVO_PAN_CENTER  90
#define DEMO3_SERVO_TILT_CENTER 90

// ---- OLED（与官方差异点）----
#define DISPLAY_SDA_PIN GPIO_NUM_8
#define DISPLAY_SCL_PIN GPIO_NUM_9
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64        // 我们的 0.96" 是 128x64
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y true

#endif // _BOARD_CONFIG_H_
