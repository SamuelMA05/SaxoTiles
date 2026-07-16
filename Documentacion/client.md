# Archivo `ws_client.c`

El módulo `cliente_ws` implementa el cliente **WebSocket** utilizado por SaxoTiles para comunicar el estado del instrumento con un servidor externo en tiempo real.

Su principal función es transmitir la nota ejecutada y la intensidad del soplo mediante mensajes JSON, permitiendo la visualización, monitoreo o registro de la ejecución del instrumento desde aplicaciones externas.

---

# Archivos del módulo

| Archivo | Función |
|---------|---------|
| `cliente_ws.h` | Declaración de la interfaz pública del módulo. |
| `cliente_ws.c` | Implementación del cliente WebSocket utilizando **libwebsockets**. |

---

# Dependencias

## Librerías externas

| Librería | Uso | Instalación |
|----------|-----|-------------|
| `libwebsockets` | Implementación del protocolo WebSocket | `sudo apt install libwebsockets-dev` |
| `stdio.h` | Entrada y salida estándar | Incluida en glibc |
| `stdlib.h` | Manejo de memoria y utilidades | Incluida en glibc |
| `string.h` | Manipulación de cadenas | Incluida en glibc |

---

# Compilación

El módulo se compila junto con el resto del proyecto:

```bash
gcc main.c teclado.c cliente_ws.c \
    -o saxo \
    -llo \
    -lpthread \
    -lwebsockets
```

---

# API pública

## `ws_init()`

```c
struct lws_context *ws_init(const char *ip, int port);
```

Inicializa el contexto WebSocket e intenta establecer una conexión con el servidor especificado mediante `ip` y `port`.

### Parámetros

| Parámetro | Descripción |
|-----------|-------------|
| `ip` | Dirección IP del servidor. |
| `port` | Puerto del servidor. |

### Retorno

Devuelve un puntero al contexto (`struct lws_context`) si la inicialización fue exitosa. En caso contrario retorna `NULL`.

Una vez establecida la conexión, el cliente envía automáticamente el siguiente mensaje de registro:

```json
{
    "type": "register",
    "role": "simulator"
}
```

---

## `ws_send()`

```c
void ws_send(const char *msg);
```

Encola un mensaje JSON para ser enviado posteriormente al servidor.

Esta función **no bloquea** la ejecución del programa. El mensaje únicamente se almacena en una cola FIFO y será enviado cuando el socket esté disponible.

Si la cola se encuentra llena, el mensaje es descartado mostrando un aviso en consola.

---

## `ws_service()`

```c
void ws_service(struct lws_context *context);
```

Procesa los eventos pendientes del cliente WebSocket.

Esta función debe ejecutarse continuamente dentro del bucle principal del programa para permitir:

- Envío de mensajes pendientes.
- Recepción de datos.
- Mantenimiento de la conexión.
- Procesamiento de callbacks de libwebsockets.

---

## `ws_cleanup()`

```c
void ws_cleanup(struct lws_context *context);
```

Libera el contexto WebSocket y todos los recursos asociados.

Debe ejecutarse antes de finalizar el programa.

---

# Cola de mensajes

Para evitar bloquear el hilo principal durante el envío de información, el módulo implementa una **cola circular (FIFO)**.

## Parámetros

| Parámetro | Valor | Descripción |
|-----------|------:|-------------|
| `QUEUE_SIZE` | 32 | Número máximo de mensajes pendientes. |
| `MSG_SIZE` | 128 | Tamaño máximo de cada mensaje. |

De esta manera, `ws_send()` únicamente almacena el mensaje en memoria y retorna inmediatamente. El envío real ocurre cuando **libwebsockets** indica que el socket está listo para escribir.

```text
main.c                    cliente_ws.c                 Servidor
  │                            │                           │
  │  ws_send("nota")           │                           │
  └──────────────────►  Cola FIFO                          │
                               │                           │
  │  ws_service()              │                           │
  └──────────────────►  lws_service()                      │
                               │                           │
                               │ LWS_CALLBACK_CLIENT_WRITEABLE
                               │                           │
                               └──────► lws_write() ─────► │
```

---

# Callbacks internos

El comportamiento del cliente depende de varios callbacks proporcionados por **libwebsockets**.

## `LWS_CALLBACK_CLIENT_ESTABLISHED`

Se ejecuta cuando la conexión se establece correctamente.

Acciones realizadas:

- Marca el cliente como conectado.
- Guarda el puntero `wsi_global`.
- Envía automáticamente el mensaje de registro.

---

## `LWS_CALLBACK_CLIENT_WRITEABLE`

Se ejecuta cuando el socket está listo para transmitir datos.

Acciones realizadas:

- Extrae el primer mensaje de la cola.
- Lo transmite mediante `lws_write()`.
- Si aún existen mensajes pendientes, solicita inmediatamente otro callback de escritura.

---

## `LWS_CALLBACK_CLIENT_RECEIVE`

Se ejecuta cuando el servidor envía información al cliente.

Actualmente el módulo únicamente imprime el contenido recibido por consola.

---

## `LWS_CALLBACK_CLIENT_CLOSED`

Se ejecuta cuando la conexión se pierde.

Acciones realizadas:

- Libera el puntero `wsi_global`.
- Marca el cliente como desconectado.

---

# Formato de los mensajes

Todos los mensajes enviados al servidor utilizan formato **JSON**.

## Mensaje de nota

```json
{
    "type": "note",
    "value": "Sol4",
    "pressure": 0.782
}
```

## Sin nota activa

```json
{
    "type": "note",
    "value": "none",
    "pressure": 0.000
}
```

## Registro del cliente

```json
{
    "type": "register",
    "role": "simulator"
}
```

### Campos del mensaje

| Campo | Descripción |
|--------|-------------|
| `type` | Tipo de mensaje enviado. |
| `value` | Nombre de la nota activa o `"none"`. |
| `pressure` | Intensidad del soplo normalizada entre **0.0** y **1.0**. |

---

# Integración con `main.c`

El módulo es utilizado desde `main.c` mediante la siguiente secuencia:

```c
/* Inicialización */
ctx_global = ws_init("10.52.223.50", 3000);

/* Envío de estado */
ws_send("{\"type\":\"note\",\"value\":\"Sol4\",\"pressure\":0.782}");

/* Procesamiento de eventos */
ws_service(ctx_global);

/* Finalización */
ws_cleanup(ctx_global);
```

---

# Resumen

El módulo `cliente_ws` encapsula completamente la comunicación WebSocket del proyecto SaxoTiles. Implementa una interfaz sencilla para el resto del programa, permitiendo enviar mensajes JSON sin bloquear el hilo principal gracias a una cola FIFO interna y al modelo de callbacks proporcionado por **libwebsockets**.
