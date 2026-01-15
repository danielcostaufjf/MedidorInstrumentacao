#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pzem004t.h"
#include <stdio.h>


static const char *TAG = "APP_MAIN";

void pzem_task(void *pvParameters) {
  pzem_config_t pzem_config = {.uart_port = PZEM_DEFAULT_UART,
                               .tx_pin = PZEM_DEFAULT_TX_PIN,
                               .rx_pin = PZEM_DEFAULT_RX_PIN,
                               .addr = 0x01};

  ESP_ERROR_CHECK(pzem_init(&pzem_config));

  pzem_values_t values;

  while (1) {
    esp_err_t err = pzem_read_values(&pzem_config, &values);
    if (err == ESP_OK) {
      ESP_LOGI(
          TAG,
          "V: %.1f V, I: %.3f A, P: %.1f W, E: %.3f kWh, F: %.1f Hz, PF: %.2f",
          values.voltage, values.current, values.power, values.energy,
          values.frequency, values.pf);
    } else {
      ESP_LOGE(TAG, "Error reading PZEM: %s", esp_err_to_name(err));
    }

    vTaskDelay(pdMS_TO_TICKS(2000)); // Update every 2 seconds
  }
}

void app_main(void) {
  ESP_LOGI(TAG, "Starting PZEM Energy Monitor...");
  xTaskCreate(pzem_task, "pzem_task", 4096, NULL, 5, NULL);
}
