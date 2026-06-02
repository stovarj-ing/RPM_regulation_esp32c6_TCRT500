#include "motor.h"
#include "rpm.h"
#include "control.h"
#include "oled.h"
#include "uart_cmd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "APP";

void app_main(void) {

    motor_init();
    motor_set_direction(1);

    rpm_init();

    if (oled_init() == ESP_OK) {
        oled_show_splash();
        vTaskDelay(pdMS_TO_TICKS(1500));
    } else {
        ESP_LOGW(TAG, "No se pudo inicializar la OLED SSD1306");
    }

    uart_cmd_init();

    xTaskCreate(control_task, "control_task", 4096, NULL, 5, NULL);
}
