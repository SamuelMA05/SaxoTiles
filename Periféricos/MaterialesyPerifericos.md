# Materiales

## 1. Núcleo del sistema
1) SIP Allwinner T-113  
2) Placa base del Haxophone de Javier Cardona  

---

## 2. Sistema de Audio
3) DAC PCM5102A
<img width="960" height="960" alt="image" src="https://github.com/user-attachments/assets/17e47c55-e379-4267-90f4-10b50299d660" />

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

## 1. Salida de audio – Amplificador (PAM8403)
El módulo amplificador se conectará a los pines **HP OUT L** y **HP OUT R** del Allwinner, ya que estos corresponden a la salida de audio estéreo (proveniente del DAC interno del chip). Algunos módulos PAM8403 ya incluyen potenciómetro integrado, por lo que no será necesario añadir uno externo para el control de volumen.

---

## 2. Salida de audio – Jack de audífonos
El jack de audífonos también se conectará a los pines **HP OUT L** y **HP OUT R**.

Dado que tanto el amplificador como el jack comparten la misma fuente de señal, se implementará un **switch** para elegir el modo de salida:
- Modo parlant
- Modo audífonos
---

## 3. Sensor de presión (MPXV7007)
El sensor de presión entrega una señal **analógica**, por lo tanto debe conectarse al pin **GPADC**.

---

## 5. Tira de LEDs direccionables
La tira de LEDs se conectará a un **GPIO configurado como salida digital**. Desde ahí se enviará la señal de control de datos.

---

## 6. Motor vibrador
El motor vibrador se controlará mediante un **GPIO configurado como salida digital**.
