# Tacho inteligente (ESP32-S3)

Segundo parcial de Internet de las Cosas - ISEC.
Integrantes: Almirón, Amadio, Francisconi.

## Descripción
Tacho de basura inteligente que mide su nivel de llenado con un sensor ultrasónico, lo muestra en una pantalla OLED y abre la tapa con un servomotor al presionar un pulsador. Si el tacho está lleno o el sensor no mide, bloquea la apertura.

## Componentes
- ESP32-S3 DevKitC-1
- Pantalla OLED SSD1306 128x64 (I2C)
- Sensor ultrasónico HC-SR04
- Servo motor
- Pulsador
- Protoboard

## Conexiones
| Componente | Pin ESP32-S3 |
|---|---|
| OLED SDA | 8 |
| OLED SCL | 9 |
| HC-SR04 TRIG | 4 |
| HC-SR04 ECHO | 5 |
| Pulsador | 6 |
| Servo (PWM) | 7 |

## Funcionamiento
- Menos de 10 cm: tacho LLENO (100 %), la tapa se bloquea.
- Entre 10 y 40 cm: nivel intermedio, con porcentaje.
- Más de 40 cm: hay espacio.
- Sin lectura del sensor: sin medición, la tapa también se bloquea.
- Al presionar el pulsador, el servo abre la tapa (90°), espera 3 segundos y la cierra (0°).

## Librerías
U8g2 y ESP32Servo.

## Simulación
https://wokwi.com/projects/477807075508506625

## Archivos
- `sketch.ino`: código fuente
- `diagram.json`: circuito de Wokwi
- `libraries.txt`: librerías de Wokwi
