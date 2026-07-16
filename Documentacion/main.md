# Archivo `main.c`

Este archivo constituye el punto de entrada del software del proyecto **SaxoTiles**, un saxofón electrónico construido sobre una placa **Allwinner T113 (ARM Cortex-A7)** ejecutando una distribución personalizada de Debian, basado en el hardware **Haxophone** desarrollado por cardonabits.

El archivo [`main.c`](CodigoSaxo/main.c) integra en un único proceso todos los subsistemas del instrumento:

- Lectura del teclado matricial físico.
- Lectura continua del sensor de presión (soplo).
- Comunicación con el sintetizador desarrollado en Faust mediante OSC.
- Transmisión del estado del instrumento en tiempo real mediante WebSocket.

---

# Dependencias

## Librerías externas

| Librería | Uso | Instalación |
|----------|-----|-------------|
| `liblo` | Comunicación OSC con Faust | `sudo apt install liblo-dev` |
| `libwebsockets` | Comunicación WebSocket con el servidor | `sudo apt install libwebsockets-dev` |
| `linux/input.h` | Lectura de eventos del teclado | Incluida en el kernel de Linux |
| `linux/i2c-dev.h` | Comunicación I²C con el sensor de presión | Incluida en el kernel de Linux |
| `pthread` | Ejecución concurrente del hilo del sensor | Incluida en glibc |

## Módulos del proyecto

Además de las librerías externas, `main.c` depende de los siguientes módulos propios del proyecto:

| Archivo | Función |
|---------|---------|
| [`teclado.h`](tecladoC.md) / `teclado.c` | Abstracción del teclado físico del Haxophone. |
| `cliente_ws.h` / `cliente_ws.c` | Cliente WebSocket encargado de comunicar el instrumento con el servidor. |

---

# Compilación

Para compilar el programa es necesario enlazar las librerías `liblo`, `pthread` y `libwebsockets`:

```bash
gcc main.c teclado.c cliente_ws.c \
    -o saxo \
    -llo \
    -lpthread \
    -lwebsockets
```

---

# Ejecución

Por defecto el programa utiliza el dispositivo de entrada `/dev/input/event0`:

```bash
./saxo
```

También es posible indicar manualmente otro dispositivo de entrada:

```bash
./saxo /dev/input/event1
```

---

# Arquitectura general

```text
                 Teclado físico (Haxophone)
                           │
                           ▼
                  /dev/input/event0
                           │
                           ▼
                      +-----------+
                      |  main.c   |
                      +-----------+
                     /      |      \
                    /       |       \
                   ▼        ▼        ▼
             Sensor I²C    OSC   WebSocket
               MCP3221      │         │
                            ▼         ▼
                      sax.dsp      Servidor
                       (Faust)     Node.js
```

El archivo `main.c` actúa como coordinador del sistema. Toda la información proveniente del teclado y del sensor de presión es procesada y distribuida hacia los distintos subsistemas (Faust y WebSocket).

---

# Subsistemas

## 1. Sensor de presión (`hilo_presion()`)

La lectura del sensor se ejecuta en un hilo independiente del programa principal utilizando `pthread`.

El hilo accede al ADC **MCP3221** mediante el bus **I²C** (`/dev/i2c-3`) y realiza una calibración automática durante el arranque promediando 50 muestras tomadas sin presión sobre el instrumento.

Posteriormente calcula la presión relativa:

```c
delta = raw - baseline;
presion = clamp(delta / DELTA_MAX, 0.0, 1.0);
```

Las pequeñas variaciones producidas por ruido electrónico son eliminadas mediante un umbral configurable.

### Parámetros ajustables

```c
#define UMBRAL_RUIDO 220
#define DELTA_MAX    850.0f
```

El valor final queda disponible en la variable global `presion_global`, utilizada por el programa principal.

---

## 2. Teclado matricial (`actualizar_bits_teclado()`)

El teclado del Haxophone es leído desde el dispositivo Linux `/dev/input/eventX` utilizando eventos de tipo `EV_KEY`.

Cada tecla corresponde a un bit dentro de un entero de 32 bits (`keys_state`), permitiendo representar cualquier combinación simultánea mediante operaciones binarias.

Ejemplo:

```c
#define W_BIT (1U << 1)
#define R_BIT (1U << 2)
```

La tecla **Z** actúa como modificador de octava, desplazando toda la digitación una octava por encima.

---

## 3. Mapeo de notas (`mapear_estado_a_nota()`)

Esta función recibe el estado completo del teclado (`keys_state`) y determina qué nota corresponde según la digitación del saxofón.

Cada combinación válida devuelve un índice comprendido entre **0 y 31**.

Ejemplo:

```c
case (W_BIT|R_BIT|T_BIT|U_BIT|I_BIT|O_BIT|M_BIT):
    return oct ? 12 : 0;

case (W_BIT|R_BIT):
    return oct ? 21 : 9;
```

Si la combinación no corresponde a ninguna digitación válida, la función devuelve `-1`, evitando generar sonido durante las transiciones entre teclas.

---

## 4. Comunicación con Faust (OSC)

La síntesis de audio se realiza externamente mediante el archivo `sax.dsp`, desarrollado en Faust.

La comunicación utiliza OSC mediante la librería `liblo`.

### Parámetros enviados

| Ruta OSC | Tipo | Descripción |
|----------|------|-------------|
| `/sax/note` | `float` | Índice de nota (0–31). |
| `/sax/presion` | `float` | Intensidad del soplo. |
| `/sax/activo` | `float` | Estado del instrumento (`1.0` = activo). |
| `/sax/error` | `float` | Indica una digitación inválida. |

En el sintetizador la presión controla la amplitud mediante una curva exponencial:

```faust
envPresion = pow(presion, 2.0);
```

---

## 5. Comunicación WebSocket

El programa mantiene una conexión WebSocket permanente con un servidor Node.js.

Su propósito es transmitir el estado del instrumento en tiempo real para aplicaciones de monitoreo, visualización o registro.

Ejemplo de mensaje enviado:

```json
{
  "type": "note",
  "value": "Sol4",
  "pressure": 0.782
}
```

Cuando no existe una nota activa:

```json
{
  "type": "note",
  "value": "none",
  "pressure": 0.000
}
```

Durante la conexión inicial el instrumento se registra mediante:

```json
{
  "type": "register",
  "role": "simulator"
}
```

---

# Lógica del programa principal

```text
Inicio
   │
   ▼
Inicializar OSC
Inicializar WebSocket
Inicializar teclado
Crear hilo del sensor
   │
   ▼
────────────── Bucle principal ──────────────
│                                            │
│ Leer presión actual                        │
│                                            │
│ ¿Cambió la presión?                        │
│      │                                     │
│      ├── Sí → enviar OSC                   │
│      │                                     │
│ Leer teclado                               │
│ Actualizar keys_state                      │
│                                            │
│ ¿Existe una nota válida?                   │
│      │                                     │
│      ├── Sí → enviar OSC + WebSocket       │
│      └── No → enviar estado "none"         │
│                                            │
│ Procesar WebSocket                         │
│ Esperar 15 ms                              │
└────────────────────────────────────────────┘
```

---

# Notas implementadas

| Índice | Nota | Índice | Nota |
|--------:|------|--------:|------|
| 0 | Do4 | 12 | Do5 |
| 1 | Do#4 | 13 | Do#5 |
| 2 | Re4 | 14 | Re5 |
| 3 | Mib4 | 15 | Mib5 |
| 4 | Mi4 | 16 | Mi5 |
| 5 | Fa4 | 17 | Fa5 |
| 6 | Fa#4 | 18 | Fa#5 |
| 7 | Sol4 | 19 | Sol5 |
| 8 | Sol#4 | 20 | Sol#5 |
| 9 | La4 | 21 | La5 |
| 10 | Sib4 | 22 | Sib5 |
| 11 | Si4 | 23 | Si5 |

---

# Resumen

El archivo `main.c` centraliza el funcionamiento del instrumento. Coordina la lectura del teclado y del sensor de presión, traduce la digitación a notas musicales, comunica dicha información al sintetizador desarrollado en Faust mediante OSC y mantiene sincronizado el estado del instrumento con un servidor externo utilizando WebSocket.
