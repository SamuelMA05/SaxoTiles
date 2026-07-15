## Sonido con Faust

EL sonido del saxofón se desarrolló utilizando **Faust**, tomando como base un código proporcionado por el profesor para extraer los armónicos característicos de cada nota del instrumento. 
A partir de este código se construyó un sintetizador que busca aproximar el timbre de un saxofón real.

Para esto se creó un **banco de armónicos** con hasta **12 armónicos** por nota. 
Además, se implementó un segundo banco de armónicos destinado a generar un sonido de ruido, el cual se utiliza para simular una ejecución incorrecta cuando el usuario toca una nota equivocada.

El sintetizador recibe tres controles principales:

- **Nota (`note`)**: selecciona la nota que se va a reproducir. Cada nota del registro del saxofón tiene asignado un identificador numérico.
- **Presión (`presion`)**: recibe un valor entre **0 y 1** que representa la intensidad con la que el usuario sopla el instrumento. Este valor controla el volumen del sonido, haciendo que la respuesta sea más natural.
- **Error (`error`)**: recibe un valor binario (0 o 1). Cuando está activo, mezcla el sonido de la nota con el banco de ruido para desafinar ligeramente la interpretación y representar una nota incorrecta.

Para finalizar, el sonido seleccionado se multiplica por la presión aplicada y por el estado de activación del instrumento, consiguiendo entonces la señal final que se envía al parlante.

import("stdfaust.lib");

harmonics_ruido = harmonicBank(333.0, 1.0, 0.8, 0.6, 0.4, 0.2, 0.1, 0.05, 0.03, 0.02, 0.01, 0.01, 0.01);
error = hslider("error", 0.0, 0, 1, 1);

harmonicBank(f, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12) =
    os.osc(1*f)  * g1  +
    os.osc(2*f)  * g2  +
    os.osc(3*f)  * g3  +
    os.osc(4*f)  * g4  +
    os.osc(5*f)  * g5  +
    os.osc(6*f)  * g6  +
    os.osc(7*f)  * g7  +
    os.osc(8*f)  * g8  +
    os.osc(9*f)  * g9  +
    os.osc(10*f) * g10 +
    os.osc(11*f) * g11 +
    os.osc(12*f) * g12;

selector = hslider("note", 0, 0, 31, 1) : int;
presion = hslider("presion", 0.0, 0, 1.0, 0.001);

harmonics_C3  = harmonicBank(130.81, 0.412, 0.662, 0.454, 0.635, 1.000, 0.425, 0.220, 0.065, 0.037, 0.115, 0.126, 0.129);
harmonics_D3  = harmonicBank(146.83, 0.412, 0.662, 0.454, 0.635, 1.000, 0.425, 0.220, 0.065, 0.037, 0.115, 0.126, 0.129);
harmonics_Eb3 = harmonicBank(155.56, 0.639, 1.000, 0.813, 0.633, 0.923, 0.179, 0.084, 0.223, 0.182, 0.150, 0.061, 0.054);
...
harmonics_Ab5 = harmonicBank(830.61, 1.000, 0.161, 0.207, 0.141, 0.032, 0.016, 0.006, 0.015, 0.016, 0.005, 0.009, 0.009);

envActivo = hslider("activo", 0.0, 0, 1, 1) : si.smoo;
envPresion = pow(presion : si.smoo, 2.0);

process =
(
    ba.selectn(
        32,
        selector,
        harmonics_C3, harmonics_D3, harmonics_Eb3, harmonics_E3,
        harmonics_F3, harmonics_Fs3, harmonics_G3, harmonics_Ab3,
        harmonics_A3, harmonics_Bb3, harmonics_B3, harmonics_C4,
        harmonics_Db4, harmonics_D4, harmonics_Eb4, harmonics_E4,
        harmonics_F4, harmonics_Fs4, harmonics_G4, harmonics_Ab4,
        harmonics_A4, harmonics_Bb4, harmonics_B4, harmonics_C5,
        harmonics_Db5, harmonics_D5, harmonics_Eb5, harmonics_E5,
        harmonics_F5, harmonics_Fs5, harmonics_G5, harmonics_Ab5
    ) * (1 - error)
    + harmonics_ruido * error
)
* envPresion
* envActivo <: _,_;
