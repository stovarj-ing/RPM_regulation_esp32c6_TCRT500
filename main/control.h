#pragma once

void control_task(void *arg);
void control_set_setpoint(float rpm);
float control_get_setpoint(void);
