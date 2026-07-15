# Teclado Matricial → Notas Musicales

Código en C para leer un teclado físico (vía `/dev/input/eventX`) y convertir combinaciones de teclas presionadas al mismo tiempo en notas musicales, tipo digitación de flauta.

## Cómo funciona

**1) Cada tecla es un bit**

Definimos cada tecla como una posición distinta dentro de un entero de 32 bits, usando `<<`. Llegamos hasta el bit 20.

Esto nos permite guardar el estado de todas las teclas en una sola variable (`t->state`) y comparar combinaciones completas como si fueran un solo número, en vez de manejar 21 booleanos sueltos.

**2) Abrimos el dispositivo e iniciamos el estado**

`teclado_init()` abre el device en modo solo lectura y **no bloqueante** (`O_RDONLY | O_NONBLOCK`). Esto es clave: si no fuera no bloqueante, el programa se quedaría esperando a que alguien presione una tecla y todo lo demás se congelaría.

`t->state` arranca en 0 (ninguna tecla tocada). Si falla el `open()`, se imprime el error y se devuelve -1.

**3) Detectamos eventos y actualizamos el estado**

`update()` recibe cada evento de tecla y prende o apaga el bit correspondiente en `t->state`:
- Tecla presionada → OR (`|`) para prender el bit
- Tecla soltada → AND-NOT (`& ~`) para apagarlo

También ignoramos el `value == 2`, que es la autorepetición del sistema (mantener la tecla presionada) y no aporta nada nuevo.

Con esto, `t->state` siempre refleja exactamente qué teclas están sostenidas en ese momento.

**4) Mapeamos combinaciones a notas**

`map_note()` define la digitación: qué combinación exacta de bits es qué nota. `Z` funciona como modificador de octava. Si la combinación no coincide con nada, devuelve -1.

**5) Leemos el teclado**

`teclado_leer()` es la que se llama en el loop principal. Cada vez:
1. Lee un evento sin bloquear
2. Si no hay nada, sale altiro
3. Si hay evento de tecla, actualiza el estado con `update()`
4. Si fue una pulsación (no un soltar), intenta mapear la nota con `map_note()`
5. Si encontró nota, la guarda en `*nota` y devuelve 1

## Uso

```c
Teclado t;
int nota;

if (teclado_init(&t, "/dev/input/event3") == 0) {
    while (1) {
        if (teclado_leer(&t, &nota)) {
            // procesar la nota, ej: reproducirla
        }
    }
    teclado_close(&t);
}
```

## Pendientes / cosas raras que noté

- El `printf` de `[TECLADO RAW]` en `teclado_leer()` imprime literalmente todo lo que llega del driver. Sirve para debug pero hay que sacarlo (o meterlo detrás de un flag) antes de usar esto en serio.
- `C_BIT`, `E_BIT`, `J_BIT` y `Y_BIT` están definidos pero no se usan en `update()`. O es código pendiente o son restos de algo anterior, revisar.
- Como `map_note()` corre en cada pulsación individual, si al armar un acorde las teclas no caen exactamente al mismo tiempo, pueden salir notas intermedias no deseadas antes de llegar a la combinación final. Si suena mal, capaz valga la pena meter un pequeño debounce.
- `read()` trata cualquier valor negativo como "no hay datos", sin diferenciar `EAGAIN` (normal en modo no bloqueante) de un error real. Podría revisarse `errno` para distinguir.
