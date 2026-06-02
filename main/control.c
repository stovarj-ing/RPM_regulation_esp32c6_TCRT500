#include "control.h"
#include "pid.h"
#include "motor.h"
#include "oled.h"
#include "rpm.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <math.h>

static portMUX_TYPE setpoint_lock = portMUX_INITIALIZER_UNLOCKED;
static float rpm_setpoint = DEFAULT_RPM_SETPOINT;

void control_set_setpoint(float rpm) {
    if (rpm < MIN_RPM_SETPOINT) {
        rpm = MIN_RPM_SETPOINT;
    }
    if (rpm > MAX_RPM_SETPOINT) {
        rpm = MAX_RPM_SETPOINT;
    }

    portENTER_CRITICAL(&setpoint_lock);
    rpm_setpoint = rpm;
    portEXIT_CRITICAL(&setpoint_lock);
}

float control_get_setpoint(void) {
    float rpm;
    portENTER_CRITICAL(&setpoint_lock);
    rpm = rpm_setpoint;
    portEXIT_CRITICAL(&setpoint_lock);
    return rpm;
}

void control_task(void *arg) {
    pid_t pid;
    pid_init(&pid, 0.5, 0.1, 0.01);

    static const char *TAG = "PID";
    while (1) {
        float current_rpm = rpm_get();
        float setpoint = control_get_setpoint();

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
        oled_show_tachometer(current_rpm, setpoint, duty_f);
        
        ESP_LOGI(TAG, "RPM: %.2f | REF: %.2f | OUT: %.2f | DUTY: %d",
         current_rpm,
         setpoint,
         output,
         (int)output);

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME_MS));
        //vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay adicional para evitar saturación del log
    }
}
