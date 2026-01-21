#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pzem004t.h"
#include <stdio.h>

static const char *TAG = "APP_MAIN";

#define SAMPLES_COUNT 50

void pzem_task(void *pvParameters) {
  pzem_config_t pzem_config = {.uart_port = PZEM_DEFAULT_UART,
                               .tx_pin = PZEM_DEFAULT_TX_PIN,
                               .rx_pin = PZEM_DEFAULT_RX_PIN,
                               .addr = 0xF8};

  ESP_ERROR_CHECK(pzem_init(&pzem_config));

  pzem_values_t values;

  // Buffer for moving average
  float voltage_buf[SAMPLES_COUNT] = {0};
  float current_buf[SAMPLES_COUNT] = {0};
  float power_buf[SAMPLES_COUNT] = {0};
  float energy_buf[SAMPLES_COUNT] = {0};
  float frequency_buf[SAMPLES_COUNT] = {0};
  float pf_buf[SAMPLES_COUNT] = {0};

  int sample_idx = 0;
  bool buffer_full = false;

  while (1) {
    esp_err_t err = pzem_read_values(&pzem_config, &values);
    if (err == ESP_OK) {
      // Update buffer
      voltage_buf[sample_idx] = values.voltage;
      current_buf[sample_idx] = values.current;
      power_buf[sample_idx] = values.power;
      energy_buf[sample_idx] = values.energy;
      frequency_buf[sample_idx] = values.frequency;
      pf_buf[sample_idx] = values.pf;

      sample_idx++;
      if (sample_idx >= SAMPLES_COUNT) {
        sample_idx = 0;
        buffer_full = true;
      }

      // Calculate Average if buffer is full (or sufficient samples if needed,
      // but simple valid flag is enough) If buffer not full, we could show
      // partial average, but waiting for 50 samples (5s) is fine to stabilize.
      // Actually, let's show partial average to be responsive at start.
      int count = buffer_full ? SAMPLES_COUNT : sample_idx;
      if (count == 0)
        count = 1; // avoid div/0

      float avg_voltage = 0;
      float avg_current = 0;
      float avg_power = 0;
      float avg_energy = 0;
      float avg_frequency = 0;
      float avg_pf = 0;

      for (int i = 0; i < count; i++) {
        avg_voltage += voltage_buf[i];
        avg_current += current_buf[i];
        avg_power += power_buf[i];
        avg_energy += energy_buf[i];
        avg_frequency += frequency_buf[i];
        avg_pf += pf_buf[i];
      }

      avg_voltage /= count;
      avg_current /= count;
      avg_power /= count;
      avg_energy /= count;
      avg_frequency /= count;
      avg_pf /= count;

      ESP_LOGI(TAG,
               "AVG50 - V: %.1f V, I: %.3f A, P: %.1f W, E: %.3f kWh, F: %.1f "
               "Hz, PF: %.2f",
               avg_voltage, avg_current, avg_power, avg_energy, avg_frequency,
               avg_pf);
    } else {
      ESP_LOGE(TAG, "Error reading PZEM: %s", esp_err_to_name(err));
    }

    vTaskDelay(pdMS_TO_TICKS(5)); // Update every 5ms -> 200Hz
  }
}

void app_main(void) {
  ESP_LOGI(TAG, "Starting PZEM Energy Monitor...");
  xTaskCreate(pzem_task, "pzem_task", 4096, NULL, 5, NULL);
}
