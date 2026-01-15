#include "pzem004t.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "PZEM004T";

// Modbus RTU CRC16 calculation
static uint16_t crc16(const uint8_t *data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

static inline uint16_t pzem_get_u16(const uint8_t *resp, int offset) {
  return (resp[offset] << 8) | resp[offset + 1];
}

static inline uint32_t pzem_get_u32(const uint8_t *resp, int offset) {
  return (resp[offset] << 24) | (resp[offset + 1] << 16) |
         (resp[offset + 2] << 8) | resp[offset + 3];
}

esp_err_t pzem_init(const pzem_config_t *config) {
  if (!config)
    return ESP_ERR_INVALID_ARG;

  uart_config_t uart_config = {
      .baud_rate = PZEM_BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_LOGI(TAG, "Initializing UART%d for PZEM...", config->uart_port);
  ESP_ERROR_CHECK(uart_driver_install(config->uart_port, 256, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(config->uart_port, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(config->uart_port, config->tx_pin,
                               config->rx_pin, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));

  return ESP_OK;
}

esp_err_t pzem_read_values(const pzem_config_t *config, pzem_values_t *values) {
  if (!config || !values)
    return ESP_ERR_INVALID_ARG;

  // Command: SlaveAddr, FuncCode, StartAddrHi, StartAddrLo, NumRegsHi,
  // NumRegsLo, CRCLo, CRCHi Read 10 registers starting from 0x0000
  uint8_t cmd[] = {config->addr, 0x04, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00};

  // Calculate CRC
  uint16_t crc = crc16(cmd, 6);
  cmd[6] = crc & 0xFF;        // Low byte
  cmd[7] = (crc >> 8) & 0xFF; // High byte

  // Flush buffer
  uart_flush_input(config->uart_port);

  // Send command
  int txBytes = uart_write_bytes(config->uart_port, cmd, sizeof(cmd));
  if (txBytes != sizeof(cmd)) {
    ESP_LOGE(TAG, "Failed to send command");
    return ESP_FAIL;
  }

  // Response structure (approx 25 bytes):
  // Addr (1) + Func (1) + ByteCount (1) + Data (20) + CRC (2)
  uint8_t resp[32];
  int len = uart_read_bytes(config->uart_port, resp, 25,
                            pdMS_TO_TICKS(500)); // 500ms timeout

  if (len < 25) {
    ESP_LOGW(TAG, "Response too short or timeout: %d bytes", len);
    return ESP_ERR_TIMEOUT;
  }

  // check CRC of response
  uint16_t resp_crc = crc16(resp, len - 2);
  uint16_t received_crc = resp[len - 2] | (resp[len - 1] << 8);

  if (resp_crc != received_crc) {
    ESP_LOGE(TAG, "CRC Check Failed: Calc 0x%04X, Recv 0x%04X", resp_crc,
             received_crc);
    return ESP_ERR_INVALID_CRC;
  }

  // Parse data (all are usually reported as 10x or 100x or 1000x scaled
  // integers, check datasheet) According to PZEM-004T v3.0 datasheets: Reg
  // 0x0000: Voltage (0.1V) - 16bit? Actually usually 16 bit is enough for
  // 240.0V -> 2400 Actually the response data block (20 bytes) is: Voltage (2B,
  // 0.1V), Current (4B, 0.001A), Power (4B, 0.1W), Energy (4B, 1Wh), Freq (2B,
  // 0.1Hz), PF (2B, 0.01), Alarm (2B)

  // Byte 0: Addr, Byte 1: Func, Byte 2: Count (should be 20 = 0x14)
  // Data starts at resp[3]

  values->voltage = pzem_get_u16(resp, 3) * 0.1f;
  values->current = pzem_get_u32(resp, 5) * 0.001f;
  values->power = pzem_get_u32(resp, 9) * 0.1f;
  values->energy = pzem_get_u32(resp, 13) *
                   0.001f; // Usually Wh, converting to kWh for consistency?
                           // Let's check user req. Datasheet says 1Wh
                           // resolution. Let's store as kWh if float, or just
                           // keep Wh. Let's stick to Wh in the comment, but
                           // float value. Code above was "energy" -> float.
                           // usually kWh is preferred. 1 Wh = 0.001 kWh

  values->energy =
      pzem_get_u32(resp, 13) *
      0.001f; // returning kWh directly might be better but let's stick to raw
              // physical units if possible or standard units. Let's output kWh
              // as it is standard. So 1Wh * 0.001 = kWh

  values->frequency = pzem_get_u16(resp, 17) * 0.1f;
  values->pf = pzem_get_u16(resp, 19) * 0.01f;
  values->alarms = pzem_get_u16(resp, 21);

  return ESP_OK;
}
