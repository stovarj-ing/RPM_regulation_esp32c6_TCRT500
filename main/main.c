#include "motor.h"
#include "rpm.h"
#include "control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {

    motor_init();
    motor_set_direction(1);

    rpm_init();

    xTaskCreate(control_task, "control_task", 4096, NULL, 5, NULL);
}