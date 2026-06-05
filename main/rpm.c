#include "rpm.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_attr.h"     
#include "config.h"
#include "freertos/FreeRTOS.h"
#define FILTER_ALPHA 0.2
static float rpm_filtered = 0;
static volatile int pulse_count = 0;
static portMUX_TYPE pulse_lock = portMUX_INITIALIZER_UNLOCKED;
static int64_t last_time = 0;
static float rpm = 0;

static int64_t last_pulse_time = 0;

static void IRAM_ATTR isr_handler(void* arg) {
    int64_t now = esp_timer_get_time(); // microsegundos

    // Ignorar pulsos menores a 2 ms (ajústalo luego)
    if ((now - last_pulse_time) > 2000) {
        portENTER_CRITICAL_ISR(&pulse_lock);
        pulse_count++;
        portEXIT_CRITICAL_ISR(&pulse_lock);
        last_pulse_time = now;
    }
}

void rpm_init(void) {
    gpio_set_direction(PIN_SENSOR, GPIO_MODE_INPUT);

    gpio_set_intr_type(PIN_SENSOR, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_SENSOR, isr_handler, NULL);

    last_time = esp_timer_get_time();
}

float rpm_get(void) {
    int64_t now = esp_timer_get_time();
    float dt = (now - last_time) / 1000000.0;

   if (dt >= 0.1) {
        int pulses;
        portENTER_CRITICAL(&pulse_lock);
        pulses = pulse_count;
        pulse_count = 0;
        portEXIT_CRITICAL(&pulse_lock);

        rpm = (pulses / dt) * (60.0 / PULSES_PER_REV);

        // Filtro exponencial
        rpm_filtered = FILTER_ALPHA * rpm + (1 - FILTER_ALPHA) * rpm_filtered;

        last_time = now;
    }

    return rpm_filtered;
}
