#include "pid.h"

void pid_init(pid_t *pid, float Kp, float Ki, float Kd) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral = 0;
    pid->prev_error = 0;
}

float pid_compute(pid_t *pid, float setpoint, float measurement, float dt) {
    float error = setpoint - measurement;

    pid->integral += error * dt;
    float derivative = (error - pid->prev_error) / dt;

    float output = pid->Kp * error +
                   pid->Ki * pid->integral +
                   pid->Kd * derivative;

    pid->prev_error = error;

    // Saturación
    if (output > 255) output = 255;
    if (output < 0) output = 0;

    return output;
}