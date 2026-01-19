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

## Datalogger

O projeto inclui um script Python (`datalogger.py`) para capturar e salvar os dados via porta serial em um arquivo CSV.

### Pré-requisitos

É necessário ter o Python instalado e a biblioteca `pyserial`:

```bash
pip install pyserial
```

### Uso

Para iniciar o log, execute o script informando a porta COM correta do ESP32:

```bash
python datalogger.py --port COMx
```

Substitua `COMx` pela porta correspondente (ex: `COM3` no Windows ou `/dev/ttyUSB0` no Linux).

**Opções adicionais:**

- `--baud`: Define a taxa de transmissão (padrão: 115200).
- `--output`: Define o nome do arquivo de saída (padrão: `datalog.csv`).

Exemplo completo:

```bash
python datalogger.py --port COM3 --baud 115200 --output medicao.csv
```
