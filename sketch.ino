
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP32Servo.h>

// =====================================
// PANTALLA OLED SSD1306 128x64
// =====================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE
);

// =====================================
// PINES DEL ESP32-S3
// =====================================
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;
const int BUTTON_PIN = 6;
const int SERVO_PIN = 7;

// =====================================
// CONFIGURACION DEL SERVO
// =====================================
Servo servoTapa;

const int TAPA_CERRADA = 0;
const int TAPA_ABIERTA = 90;

// =====================================
// VARIABLES DEL TACHO
// =====================================
float distancia = -1;
int nivelLlenado = 0;

String estadoTacho = "";
String colorEstado = "";

bool aperturaPermitida = false;
bool tapaAbierta = false;

// =====================================
// MEDIR DISTANCIA CON HC-SR04
// =====================================
float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracion == 0) {
    return -1;
  }

  return duracion * 0.0343 / 2.0;
}

// =====================================
// ACTUALIZAR ESTADO Y PORCENTAJE
// =====================================
void actualizarEstado() {
  if (distancia < 0) {
    estadoTacho = "Sin medicion";
    colorEstado = "GRIS";
    nivelLlenado = 0;
  }
  else if (distancia < 10) {
    estadoTacho = "LLENO";
    colorEstado = "ROJO";
    nivelLlenado = 100;
  }
  else if (distancia <= 40) {
    estadoTacho = "Nivel intermedio";
    colorEstado = "AMARILLO";

    nivelLlenado = (int)(
                     (40.0 - distancia) / 30.0 * 100.0
                   );

    nivelLlenado = constrain(nivelLlenado, 0, 100);
  }
  else {
    estadoTacho = "Con espacio";
    colorEstado = "VERDE";
    nivelLlenado = 0;
  }
}

// =====================================
// MOSTRAR INFORMACION EN LA OLED
// =====================================
void mostrarPantalla() {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 10, "TACHO INTELIGENTE");
  u8g2.drawHLine(0, 13, 128);

  if (distancia < 0) {
    u8g2.drawStr(0, 28, "Sin medicion valida");
    u8g2.drawStr(0, 43, "Revisar sensor");
    u8g2.sendBuffer();
    return;
  }

  char textoDistancia[24];

  snprintf(
    textoDistancia,
    sizeof(textoDistancia),
    "Distancia: %.1f cm",
    distancia
  );

  u8g2.drawStr(0, 25, textoDistancia);

  char textoNivel[24];

  snprintf(
    textoNivel,
    sizeof(textoNivel),
    "Capacidad: %d%%",
    nivelLlenado
  );

  u8g2.drawStr(0, 38, textoNivel);

  const int barraX = 3;
  const int barraY = 42;
  const int barraAncho = 122;
  const int barraAlto = 12;

  u8g2.drawFrame(
    barraX,
    barraY,
    barraAncho,
    barraAlto
  );

  int anchoRelleno =
    (barraAncho - 4) * nivelLlenado / 100;

  if (anchoRelleno > 0) {
    u8g2.drawBox(
      barraX + 2,
      barraY + 2,
      anchoRelleno,
      barraAlto - 4
    );
  }

  if (distancia < 10) {
    u8g2.drawStr(0, 63, "LLENO! VACIAR TACHO");
  }
  else if (tapaAbierta) {
    u8g2.drawStr(0, 63, "Tapa abierta");
  }
  else if (distancia <= 40) {
    u8g2.drawStr(0, 63, "Nivel intermedio");
  }
  else {
    u8g2.drawStr(0, 63, "Hay espacio disponible");
  }

  u8g2.sendBuffer();
}

// =====================================
// ABRIR Y CERRAR LA TAPA
// =====================================
void controlarTapa() {
  // Detectar una pulsacion nueva
  if (digitalRead(BUTTON_PIN) != LOW) {
    return;
  }

  // Esperar a que el boton se estabilice
  delay(30);

  if (digitalRead(BUTTON_PIN) != LOW) {
    return;
  }

  // Bloquear si esta lleno o no hay medicion
  if (distancia < 0 || distancia < 10) {
    aperturaPermitida = false;

    if (distancia >= 0) {
      Serial.println("TACHO LLENO: vaciar antes de abrir.");
    } else {
      Serial.println("Sin medicion valida. Tapa bloqueada.");
    }
  }
  else {
    aperturaPermitida = true;

    Serial.println("Abriendo tapa...");
    tapaAbierta = true;
    servoTapa.write(TAPA_ABIERTA);

    mostrarPantalla();

    // Mantener abierta durante 3 segundos
    unsigned long inicio = millis();

    while (millis() - inicio < 3000) {
      delay(10);
    }

    Serial.println("Cerrando tapa...");
    servoTapa.write(TAPA_CERRADA);
    tapaAbierta = false;

    mostrarPantalla();
  }

  // Esperar a que se suelte el boton
  while (digitalRead(BUTTON_PIN) == LOW) {
    delay(10);
  }

  delay(30);
}

// =====================================
// CONFIGURACION INICIAL
// =====================================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Inicializar servo
  servoTapa.setPeriodHertz(50);
  servoTapa.attach(SERVO_PIN, 500, 2400);
  servoTapa.write(TAPA_CERRADA);

  // Inicializar OLED
  Wire.begin(8, 9);
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(10, 28, "Iniciando tacho...");
  u8g2.sendBuffer();

  delay(1000);
}

// =====================================
// BUCLE PRINCIPAL
// =====================================
void loop() {
  distancia = medirDistancia();
  actualizarEstado();

  controlarTapa();

  mostrarPantalla();

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.print(" cm | Estado: ");
  Serial.print(estadoTacho);
  Serial.print(" | Capacidad: ");
  Serial.print(nivelLlenado);
  Serial.println("%");

  delay(100);
}
