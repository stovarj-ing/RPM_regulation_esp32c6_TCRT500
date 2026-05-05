#pragma once

typedef struct {
    float Kp;
    float Ki;
    float Kd;

    float integral;
    float prev_error;
    float output;
} pid_t;

void pid_init(pid_t *pid, float Kp, float Ki, float Kd);
float pid_compute(pid_t *pid, float setpoint, float measurement, float dt);