#ifndef __DEMO3_ROBOT_MOTION_H__
#define __DEMO3_ROBOT_MOTION_H__

#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>
#include <atomic>
#include "mcp_server.h"

// =====================================================================
//  头部双舵机（LEDC 50Hz · 14bit）+ MCP 工具
//  对小智说「抬头/低头/点头/看左边/回正」就会经 MCP 调用这些工具
// =====================================================================

class Demo3RobotMotion {
private:
    static constexpr int PAN_MIN = 45, PAN_MAX = 135;
    static constexpr int TILT_MIN = 60, TILT_MAX = 120;
    static constexpr int PAN_CENTER = 90, TILT_CENTER = 90;

    std::atomic<int> pan_cur_{90}, tilt_cur_{90};
    std::atomic<int> pan_tgt_{90}, tilt_tgt_{90};

    static int clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

    // 角度 → 14bit duty（500~2400us 脉宽 @50Hz）
    static uint32_t angle_to_duty(int angle) {
        int us = 500 + (int)((float)angle / 180.0f * 1900.0f);
        return (uint32_t)((float)us / 20000.0f * 16383.0f);
    }

    void attach(gpio_num_t gpio, ledc_channel_t ch) {
        ledc_channel_config_t ch_cfg = {};
        ch_cfg.gpio_num = gpio;
        ch_cfg.speed_mode = LEDC_LOW_SPEED_MODE;
        ch_cfg.channel = ch;
        ch_cfg.timer_sel = LEDC_TIMER_0;
        ch_cfg.duty = angle_to_duty(90);
        ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));
    }

    static void smooth_task(void* arg) {
        auto* self = static_cast<Demo3RobotMotion*>(arg);
        while (true) {
            int p = self->pan_cur_.load(), t = self->tilt_cur_.load();
            int dp = self->pan_tgt_.load() - p;
            int dt = self->tilt_tgt_.load() - t;
            if (dp) p += (abs(dp) < 2) ? dp : (dp > 0 ? 2 : -2);
            if (dt) t += (abs(dt) < 2) ? dt : (dt > 0 ? 2 : -2);
            self->pan_cur_.store(p);
            self->tilt_cur_.store(t);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, angle_to_duty(p));
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, angle_to_duty(t));
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    static void nod_task(void* arg) {
        auto* self = static_cast<Demo3RobotMotion*>(arg);
        int frames[] = {75, 100, 80, TILT_CENTER};
        for (int f : frames) {
            self->tilt_tgt_.store(f);
            vTaskDelay(pdMS_TO_TICKS(160));
        }
        vTaskDelete(nullptr);
    }

public:
    Demo3RobotMotion(gpio_num_t pan_gpio, gpio_num_t tilt_gpio) {
        ledc_timer_config_t timer = {};
        timer.speed_mode = LEDC_LOW_SPEED_MODE;
        timer.duty_resolution = LEDC_TIMER_14_BIT;
        timer.timer_num = LEDC_TIMER_0;
        timer.freq_hz = 50;
        timer.clk_cfg = LEDC_AUTO_CLK;
        ESP_ERROR_CHECK(ledc_timer_config(&timer));
        attach(pan_gpio, LEDC_CHANNEL_0);
        attach(tilt_gpio, LEDC_CHANNEL_1);
        xTaskCreate(smooth_task, "demo3_servo", 2048, this, 3, nullptr);
        RegisterMcpTools();
        ESP_LOGI("Demo3Motion", "robot head servos ready (pan=%d tilt=%d)",
                 (int)pan_gpio, (int)tilt_gpio);
    }

    void Nod() {
        xTaskCreate(nod_task, "demo3_nod", 2048, this, 4, nullptr);
    }

    void RegisterMcpTools() {
        auto& mcp = McpServer::GetInstance();

        mcp.AddTool("self.robot.set_head",
            "控制机器人头部角度。pan=水平角(45~135,90为正前), tilt=垂直角(60~120,90为平视)",
            PropertyList({
                Property("pan", kPropertyTypeInteger, PAN_CENTER, PAN_MIN, PAN_MAX),
                Property("tilt", kPropertyTypeInteger, TILT_CENTER, TILT_MIN, TILT_MAX),
            }),
            [this](const PropertyList& properties) -> ReturnValue {
                int pan = clamp(properties["pan"].value<int>(), PAN_MIN, PAN_MAX);
                int tilt = clamp(properties["tilt"].value<int>(), TILT_MIN, TILT_MAX);
                pan_tgt_.store(pan);
                tilt_tgt_.store(tilt);
                return "{\"pan\":" + std::to_string(pan) + ",\"tilt\":" + std::to_string(tilt) + "}";
            });

        mcp.AddTool("self.robot.nod", "让机器人开心地点点头",
            PropertyList(),
            [this](const PropertyList& properties) -> ReturnValue {
                Nod();
                return true;
            });

        mcp.AddTool("self.robot.center", "机器人头部回正",
            PropertyList(),
            [this](const PropertyList& properties) -> ReturnValue {
                pan_tgt_.store(PAN_CENTER);
                tilt_tgt_.store(TILT_CENTER);
                return true;
            });
    }
};

#endif // __DEMO3_ROBOT_MOTION_H__
