# Teclado Matricial

Este es un código en C para leer un teclado físico (`/dev/input/eventX`) y permite convertir las combinaciones de las teclas presionadas en notas musicales.

Primero definimos cada tecla como una posición distinta dentro de un entero de 32 bits, para eso utilizamos `<<`. En este caso llegamos hasta el bit 20.

Lo anterior nos permite guardar el estado de todas las teclas en una sola variable que se llama (`t->state`).

Ahora creamos la función `teclado_init()` que abre el device en modo solo lectura y no bloqueante (`O_RDONLY | O_NONBLOCK`).
`t->state` la arrancamos en 0 (que es el caso de ninguna tecla tocada).

Ahora detectamos eventos con el siguiente formato:
`update()` recibe cada evento de tecla y prende o apaga el bit correspondiente en `t->state`:
- Tecla presionada → OR (`|`) para prender el bit
- Tecla soltada → AND-NOT (`& ~`) para apagarlo

Nuestra función `map_note()` define la digitación, es decir nos muestra qué combinación exacta de bits es qué nota. `Z` funciona como modificador de octava. Si la combinación no coincide con nada, devuelve -1.


Por último usamos la función

`teclado_leer()` que nos permite:  
1. Leer un evento sin bloquear
3. Si hay evento de tecla, actualiza el estado con `update()`
4. Si fue una pulsación (no un soltar), intenta mapear la nota con `map_note()`
5. Si encontró nota, la guarda en `*nota` y devuelve 1
