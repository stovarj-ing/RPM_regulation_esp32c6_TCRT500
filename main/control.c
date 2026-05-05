#include "control.h"
#include "pid.h"
#include "motor.h"
#include "rpm.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <math.h>

void control_task(void *arg) {
    pid_t pid;
    pid_init(&pid, 0.5, 0.1, 0.01);

    float setpoint = 1000; // RPM objetivo

    static const char *TAG = "PID";
    while (1) {
        float current_rpm = rpm_get();

        float output = pid_compute(&pid, setpoint, current_rpm, 0.1);

        // Dirección
        int dir = (output >= 0) ? 1 : -1;

        // Magnitud
        float duty_f = fabs(output);

        // Saturación
        if (duty_f > 255) duty_f = 255;

        // Aplicar
        motor_set_direction(dir);
        motor_set_speed((uint8_t)duty_f);
        
        ESP_LOGI(TAG, "RPM: %.2f | OUT: %.2f | DUTY: %d",
         current_rpm,
         output,
         (int)output);

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME_MS));
        //vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay adicional para evitar saturación del log
    }
}