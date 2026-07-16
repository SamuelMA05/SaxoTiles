# Archivo main.c

Este archivo es la base del proyecto SaxoTiles, un saxofón electrónico construido sobre una placa Allwinner T113 (ARM Cortex-A7) corriendo Debian custom, basado en el hardware Haxophone de cardonabits. [main.c](CodigoSaxo/main.c) integra en un único proceso todos los subsistemas del instrumento:

- Lectura del teclado matricial físico
- Lectura continua del sensor de presión (soplo)
- Síntesis de audio vía OSC → Faust
- Transmisión del estado en tiempo real vía WebSocket a un servidor externo

## Dependencias

| Librería | Uso | Instalación |
|----------|-----|-------------|
| `liblo` | Comunicación OSC con Faust | `sudo apt install liblo-dev` |
| `libwebsockets` | WebSocket hacia el servidor | `sudo apt install libwebsockets-dev` |
| `linux/input.h` | Lectura de eventos del teclado | Incluida en el kernel de Linux |
| `linux/i2c-dev.h` | Comunicación I²C con el sensor | Incluida en el kernel de Linux |
| `pthread` | Hilo paralelo para el sensor | Incluida en glibc |

Además requiere los módulos propios del proyecto:

* [`teclado.h` / `teclado.c`](tecladoC.md) — abstracción del dispositivo de entrada.
* `cliente_ws.h` / `cliente_ws.c` — abstracción del cliente WebSocket.


