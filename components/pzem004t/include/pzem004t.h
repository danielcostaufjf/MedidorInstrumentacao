#ifndef PZEM004T_H
#define PZEM004T_H

#include "driver/uart.h"
#include "esp_err.h"
#include <stdint.h>


// Default commands
#define PZEM_CMD_READ_MEASURE_RESULT 0x04

// Configuration defaults (can be overridden)
#define PZEM_DEFAULT_UART UART_NUM_2
#define PZEM_DEFAULT_TX_PIN 17
#define PZEM_DEFAULT_RX_PIN 16
#define PZEM_BAUD_RATE 9600

typedef struct {
  float voltage;   // V
  float current;   // A
  float power;     // W
  float energy;    // kWh
  float frequency; // Hz
  float pf;        // Power Factor
  uint16_t alarms; // Alarm status (optional)
} pzem_values_t;

typedef struct {
  uart_port_t uart_port;
  int tx_pin;
  int rx_pin;
  uint8_t addr; // PZEM address (usually 0x01)
} pzem_config_t;

/**
 * @brief Initialize the PZEM-004T UART connection
 *
 * @param config Configuration struct
 * @return esp_err_t ESP_OK on success
 */
esp_err_t pzem_init(const pzem_config_t *config);

/**
 * @brief Read all values from the PZEM module
 *
 * @param config Configuration struct (to know which UART/addr)
 * @param values Pointer to struct to store values
 * @return esp_err_t ESP_OK on success
 */
esp_err_t pzem_read_values(const pzem_config_t *config, pzem_values_t *values);

#endif // PZEM004T_H
