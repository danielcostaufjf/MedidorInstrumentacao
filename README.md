# Medidor de Instrumentação

Sistema de monitoramento de energia desenvolvido para a disciplina de Instrumentação Eletrônica, utilizando ESP32 e sensor PZEM-004T.

## Hardware Utilizado

- **Microcontrolador**: ESP32
- **Sensor**: PZEM-004T v3.0 (100A)

## Conexões

A comunicação com o sensor é feita via UART (Serial 2).

| Pino PZEM | Pino ESP32 | Função |
| :--- | :--- | :--- |
| TX | GPIO 16 | RX (Recebe dados do sensor) |
| RX | GPIO 17 | TX (Envia comandos para o sensor) |
| VCC | 5V | Alimentação |
| GND | GND | Terra |

## Funcionalidades

O firmware realiza a leitura periódica dos seguintes parâmetros elétricos:

- **Tensão** (V)
- **Corrente** (A)
- **Potência Ativa** (W)
- **Energia Acumulada** (kWh)
- **Frequência** (Hz)
- **Fator de Potência** (PF)

Os dados são exibidos no log do ESP32 via porta serial de debug.
