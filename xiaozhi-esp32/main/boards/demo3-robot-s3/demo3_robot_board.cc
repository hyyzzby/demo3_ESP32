// =====================================================================
//  demo3_ESP32 桌面陪伴机器人 · ESP32-S3 语音版板级实现
//  基于 bread-compact-wifi 改造：
//   1. OLED 引脚 8/9, 128x64
//   2. 电容触摸 T14（自研 CapTouchButton，开机基线自校准）
//   3. 头部双舵机 MCP 工具（robot_motion.h）→ 语音指令可控制
//   4. 移除音量按键/灯控 GPIO（我们的接线没有这些）
// =====================================================================
#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "display/oled_display.h"
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "mcp_server.h"
#include "led/single_led.h"
#include "robot_motion.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/touch_pad.h>
#include <functional>

#define TAG "Demo3RobotBoard"

// ---------- 电容触摸按钮（T14，软件消抖 + 基线比率判定）----------
// 与 src/voice.cpp / robot.cpp 的判定逻辑同源：raw < baseline * 0.6 视为按下
class CapTouchButton {
private:
    touch_pad_t channel_;
    float baseline_ = 0;
    std::function<void()> on_press_, on_release_;
    bool pressed_ = false;
    int debounce_ = 0;

    static void PollTask(void* arg) {
        auto* self = static_cast<CapTouchButton*>(arg);
        uint32_t raw = 0;
        while (true) {
            touch_pad_read_raw(self->channel_, &raw);
            bool hit = raw < self->baseline_ * DEMO3_TOUCH_RATIO;
            self->debounce_ = hit ? (self->debounce_ < 3 ? self->debounce_ + 1 : 3)
                                  : (self->debounce_ > 0 ? self->debounce_ - 1 : 0);
            bool stable = self->debounce_ >= 2;
            if (stable != self->pressed_) {
                self->pressed_ = stable;
                if (stable && self->on_press_) self->on_press_();
                if (!stable && self->on_release_) self->on_release_();
            }
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

public:
    CapTouchButton(touch_pad_t channel) : channel_(channel) {
        ESP_ERROR_CHECK(touch_pad_init());
        ESP_ERROR_CHECK(touch_pad_config(channel_));
        // 基线自校准：开机时手不要碰
        uint64_t acc = 0;
        uint32_t v = 0;
        for (int i = 0; i < 32; i++) {
            touch_pad_read_raw(channel_, &v);
            acc += v;
            vTaskDelay(pdMS_TO_TICKS(2));
        }
        baseline_ = acc / 32.0f;
        ESP_LOGI(TAG, "cap touch ch=%d baseline=%.0f", channel_, baseline_);
        xTaskCreate(PollTask, "cap_touch", 2048, this, 3, nullptr);
    }
    void OnPressDown(std::function<void()> cb) { on_press_ = cb; }
    void OnPressUp(std::function<void()> cb) { on_release_ = cb; }
};

class Demo3RobotBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t display_i2c_bus_;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    Display* display_ = nullptr;
    Button boot_button_;
    CapTouchButton touch_button_;
    Demo3RobotMotion motion_;

    void InitializeDisplayI2c() {
        i2c_master_bus_config_t bus_config = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = DISPLAY_SDA_PIN,
            .scl_io_num = DISPLAY_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &display_i2c_bus_));
    }

    void InitializeSsd1306Display() {
        esp_lcd_panel_io_i2c_config_t io_config = {
            .dev_addr = 0x3C,
            .scl_speed_hz = 400 * 1000,
            .control_phase_bytes = 1,
            .dc_bit_offset = 6,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .on_color_trans_done = nullptr,
            .user_ctx = nullptr,
            .flags = {
                .dc_low_on_data = 0,
                .disable_control_phase = 0,
            },
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(display_i2c_bus_, &io_config, &panel_io_));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.bits_per_pixel = 1;
        esp_lcd_panel_ssd1306_config_t ssd1306_config = {
            .height = static_cast<uint8_t>(DISPLAY_HEIGHT),
        };
        panel_config.vendor_config = &ssd1306_config;

        ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(panel_io_, &panel_config, &panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
        if (esp_lcd_panel_init(panel_) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize display");
            display_ = new NoDisplay();
            return;
        }
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_, false));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));
        display_ = new OledDisplay(panel_io_, panel_, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                                   DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });
        // 摸头 → 按住说话（松手结束），与官方 press-to-talk 一致
        touch_button_.OnPressDown([this]() {
            Application::GetInstance().StartListening();
        });
        touch_button_.OnPressUp([this]() {
            Application::GetInstance().StopListening();
        });
    }

public:
    Demo3RobotBoard() :
        boot_button_(BOOT_BUTTON_GPIO),
        touch_button_(DEMO3_TOUCH_CHANNEL),
        motion_(DEMO3_SERVO_PAN_GPIO, DEMO3_SERVO_TILT_GPIO) {
        InitializeDisplayI2c();
        InitializeSsd1306Display();
        InitializeButtons();
        ESP_LOGI(TAG, "Demo3 robot board ready");
    }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
            AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }
};

DECLARE_BOARD(Demo3RobotBoard);
