# Materiales

## 1. Núcleo del sistema
1) SIP Allwinner T-113  
2) Placa base del Haxophone de Javier Cardona  

---

## 2. Sistema de Audio
3) DAC para el sonido (incluido en la placa)  
4) Amplificador de Audio PAM8403  

<img width="381" height="492" alt="PAM8403" src="https://github.com/user-attachments/assets/fcf5f442-4eef-45f8-b6fd-644aaca3b528" />

5) Parlante de 8 Ω de impedancia y 2W de potencia  
6) Jack para introducir audífonos alámbricos  
7) Potenciómetro para variar el volumen  

---

## 3. Interacción con el Usuario
8) Sensor de presión MPXV7007 (incluido en la placa)  
9) LEDs RGB direccionables  
10) Motor vibrador redondo  

<img width="381" height="492" alt="Motor vibrador" src="https://github.com/user-attachments/assets/20b9b03c-cf69-44b0-8ee9-817ec5fcf288" />

---

## 5. Alimentación
11) Baterías y su respectivo módulo de carga

# Implementación de los Periféricos en Allwinner T-113

A continuación se presenta el pinout del Allwinner T-113:

<img width="1519" height="868" alt="image" src="https://github.com/user-attachments/assets/173c7dbb-0096-4f53-acd6-a006126bd6ce" />

De aquí, se seleccionó que:

1) El amplificador de audio se conectará en los pines HP OUT L y R, debido a que este estéreo. Existen algunos módulos de este amplificador que ya incluyen el potenciómetro, por lo cual no se requerirá de este elemento de manera externa para regular el volumen.
2) Para el caso de los audífonos, el jack va a los mismos pines de antes. Por lo tanto, mediante un switch se debe seleccionar el modo de salida de audio.
3) El sensor de presión se conectará al pin de convertidor análogo-digital GPADC.
4) El DAC se conectará al pin I2S.
5) La tira de LEDS se conectará como GPIO.
6) El motor vibrador se conectará como GPIO.
