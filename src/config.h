#pragma once

// ================= 硬件引脚配置（ESP32 DevKit / WROOM-32）=================
#define PIN_STATUS_LED   2      // 板载 LED
#define PIN_I2C_SDA      21     // OLED SSD1306
#define PIN_I2C_SCL      22
#define PIN_SERVO_PAN    25     // 头部水平舵机（左右）
#define PIN_SERVO_TILT   26     // 头部垂直舵机（上下）
#define PIN_BUZZER       27     // 无源蜂鸣器
#define BUZZER_LEDC_CH   7      // LEDC 通道（避开舵机占用的 0/1）
#define PIN_TOUCH        T0     // GPIO4 摸头触摸
#define PIN_LDR          34     // 光敏分压输出 (ADC1_CH6, 仅输入)

// ================= 舵机参数（角度） =================
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

// ================= 行为阈值 =================
#define TOUCH_RATIO          0.60f     // 触摸值低于基线 60% 判定按下
#define LDR_DARK_VALUE       800       // ADC 低于此值认为"天黑"（满量程 4095）
#define SLEEP_AFTER_DARK_MS  60000UL   // 暗光持续 60s → 睡觉
#define SLEEP_AFTER_IDLE_MS  600000UL  // 10 分钟没人理 → 睡觉
#define WAKE_AFTER_LIGHT_MS  5000UL    // 变亮 5s → 自动醒来
#define PET_DURATION_MS      3000UL    // 开心状态持续时间
#define BLINK_PERIOD_MIN_MS  2500UL
#define BLINK_PERIOD_MAX_MS  6000UL
