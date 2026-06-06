#pragma once

// Pines22
#define PIN_PWM     4
#define PIN_IN1     5
#define PIN_IN2     8
#define PIN_SENSOR  1

// OLED SSD1306 I2C
#define OLED_I2C_SDA      6
#define OLED_I2C_SCL      7
#define OLED_I2C_ADDR     0x3C
#define OLED_I2C_FREQ_HZ  100000

// PWM
#define PWM_FREQ    20000
#define PWM_RES     LEDC_TIMER_8_BIT

// Control
#define SAMPLE_TIME_MS 100
#define PULSES_PER_REV 1
#define DEFAULT_RPM_SETPOINT 1000.0f
#define MIN_RPM_SETPOINT 0.0f
#define MAX_RPM_SETPOINT 9000.0f