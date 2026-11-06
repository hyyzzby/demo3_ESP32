#pragma once

// ============================================================
//  ESP32 桌面陪伴机器人 v0.5 — 语音版引脚配置
//  双板支持：ESP32-S3 (小智AI语音版) / ESP32-WROOM-32 (经典版)
//  S3 宏由框架自动定义 (CONFIG_IDF_TARGET_ESP32S3)，也可手动 -DBOARD_ESP32_S3
// ============================================================

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(BOARD_ESP32_S3)
#define ROBOT_IS_S3 1
#else
#define ROBOT_IS_S3 0
#endif

// ================= S3 语音版引脚（ESP32-S3-DevKitC-1）=================
#if ROBOT_IS_S3

#define PIN_STATUS_LED   47     // 状态 LED（DevKitC 板载 RGB 在 GPIO48，WS2812 需单独驱动）
#define PIN_I2C_SDA      8      // OLED SSD1306（S3 Wire 默认即 8/9）
#define PIN_I2C_SCL      9
#define PIN_SERVO_PAN    11     // 头部水平舵机
#define PIN_SERVO_TILT   12     // 头部垂直舵机
#define PIN_LDR          2      // 光敏分压 (ADC1_CH1)
#define PIN_TOUCH        T14    // GPIO14 摸头触摸（S3 支持 T1~T14）

// --- INMP441 全向MEMS麦克风 (I2S RX, 3.3V) ---
#define PIN_I2S_MIC_SCK  4      // SCK  → BCLK
#define PIN_I2S_MIC_WS   5      // WS   → LRCL
#define PIN_I2S_MIC_SD   6      // SD   → 数据输出
// L/R 引脚接 GND = 左声道

// --- MAX98357A I2S 功放 (接 8Ω 3W 小喇叭) ---
#define PIN_I2S_SPK_BCLK 15
#define PIN_I2S_SPK_LRC  16
#define PIN_I2S_SPK_DIN  7
// SD 引脚接 3.3V = 输出 (L+R)/2；GAIN 悬空 = 9dB

// ================= 经典版引脚（ESP32-WROOM-32，蜂鸣器方案）=================
#else

#define ROBOT_IS_S3 0
#define PIN_STATUS_LED   2      // 板载 LED
#define PIN_I2C_SDA      21     // OLED SSD1306
#define PIN_I2C_SCL      22
#define PIN_SERVO_PAN    25     // 头部水平舵机（左右）
#define PIN_SERVO_TILT   26     // 头部垂直舵机（上下）
#define PIN_BUZZER       27     // 无源蜂鸣器
#define BUZZER_LEDC_CH   7      // LEDC 通道（避开舵机占用的 0/1）
#define PIN_TOUCH        T0     // GPIO4 摸头触摸
#define PIN_LDR          34     // 光敏分压输出 (ADC1_CH6, 仅输入)

#endif // ROBOT_IS_S3

// ================= 公共参数 =================
#define SERVO_PAN_CENTER   90
#define SERVO_TILT_CENTER  90
#define SERVO_PAN_MIN      45
#define SERVO_PAN_MAX     135
#define SERVO_TILT_MIN     60
#define SERVO_TILT_MAX    120
#define SERVO_SPEED_DPS   160    // 平滑转头速度（度/秒）

// ================= WiFi / NTP =================
#define WIFI_SSID       "your_wifi_ssid"   // TODO: 改成你的 WiFi
#define WIFI_PASSWORD   "your_wifi_password"
#define TZ_INFO         "CST-8"            // 中国 UTC+8
#define NTP_SERVER_1    "ntp.aliyun.com"
#define NTP_SERVER_2    "pool.ntp.org"

// ================= 音频 / 语音（S3）=================
#define AUDIO_SAMPLE_RATE     16000     // 16kHz 单声道，够语音交互
#define AUDIO_RECORD_MS       3000      // 单次录音时长
#define AUDIO_MIC_GAIN_SHIFT  14        // 32bit→16bit 右移位数（越大音量越小）
#define AUDIO_SPK_VOLUME      0.8f      // 功放软件音量 0.0~1.0

// ================= 行为阈值 =================
#define TOUCH_RATIO          0.60f     // 触摸值低于基线 60% 判定按下
#define LDR_DARK_VALUE       800       // ADC 低于此值认为"天黑"（满量程 4095）
#define SLEEP_AFTER_DARK_MS  60000UL   // 暗光持续 60s → 睡觉
#define SLEEP_AFTER_IDLE_MS  600000UL  // 10 分钟没人理 → 睡觉
#define WAKE_AFTER_LIGHT_MS  5000UL    // 变亮 5s → 自动醒来
#define PET_DURATION_MS      3000UL    // 开心状态持续时间
#define BLINK_PERIOD_MIN_MS  2500UL
#define BLINK_PERIOD_MAX_MS  6000UL
