#include <Servo.h>

// === SERVOS ===
Servo servo1;  // Pin 3
Servo servo2;  // Pin 2
Servo servo3;  // Pin 1
#define pinS1 13
#define pinS2 2
#define pinS3 8
// === MOTORES DC (L298N) ===
const int IN1 = 4;   // Motor izquierdo
const int IN2 = 5;
const int ENA = 3;   // PWM M1

const int IN3 = 6;   // Motor derecho
const int IN4 = 7;
const int ENB = 11;  // PWM M2

// === SENSOR DE GAS ===
const int GAS_PIN = A1;

// === CONFIGURACIÓN RPM (solo referencia, sin sensor) ===
const int RPM_MAX = 120; // Ajusta a tu motor
const int RPM_BAJO = 40; // Valor de RPM para ruedas lentas en giro

// === VARIABLES DE VELOCIDAD ===
int rpm_izq = 0;
int rpm_der = 0;
int direccion = 0; // 0=detenido, 1=adelante, -1=atras, 2=derecha, 3=izquierda

void setup() {
  Serial.begin(9600);

  // Servos
  servo1.attach(pinS1);
  servo2.attach(pinS2);
  servo3.attach(pinS3);

  // Motores
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT); 
  pinMode(ENB, OUTPUT);

  // Sensor de gas
  pinMode(GAS_PIN, INPUT);

  // Motores apagados al inicio
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Serial.println("Sistema listo");
}

void loop() {
      // === LECTURA Y ENVÍO DEL SENSOR DE GAS ===
      int gas_value = analogRead(GAS_PIN);
      //Serial.print("gas");
      //Serial.println(gas_value);

      delay(100);

  // === LECTURA DE COMANDOS SERIAL ===
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    // === CONTROL DE SERVOS ===
    if (cmd.startsWith("servo1,adelante")) {
      servo1.write(85);
    } else if (cmd.startsWith("servo1,atras")) {
      servo1.write(90);
    } else if (cmd.startsWith("servo2,adelante")) {
      servo2.write(100);
    } else if (cmd.startsWith("servo2,atras")) {
      servo2.write(85);
    } else if (cmd.startsWith("servo3,adelante")) {
      servo3.write(85);
    } else if (cmd.startsWith("servo3,atras")) {
      servo3.write(100);
    } else if (cmd == "detener") {
      servo1.write(90);
      servo2.write(90);
      servo3.write(90);
    }

    // === DIRECCIÓN Y VELOCIDAD AUTOMÁTICA DE MOTORES ==
    if (cmd == "adelante") {
      direccion = 1;
      rpm_izq = RPM_MAX;
      rpm_der = RPM_MAX;
      Serial.println("Motores adelante");
    } else if (cmd == "atras") {
      direccion = -1;
      rpm_izq = RPM_MAX;
      rpm_der = RPM_MAX;
      Serial.println("Motores atras");
    } else if (cmd == "derecha") {
      direccion = 2;
      rpm_izq = RPM_MAX;
      rpm_der = RPM_BAJO;
      Serial.println("Girando derecha");
    } else if (cmd == "izquierda") {
      direccion = 3;
      rpm_izq = RPM_BAJO;
      rpm_der = RPM_MAX;
      Serial.println("Girando izquierda");
    } else if (cmd == "detenerm") {
      direccion = 0;
      rpm_izq = 0;
      rpm_der = 0;
      Serial.println("Motores detenidos");
    }
  }

  // Aplica velocidad y dirección siempre (fuera del if para mantener movimiento)
  int pwm_izq = map(rpm_izq, 0, RPM_MAX, 0, 255);
  int pwm_der = map(rpm_der, 0, RPM_MAX, 0, 255);
  // Motor izquierdo
  if (rpm_izq == 0) {
    analogWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  } else if (direccion == 1 || direccion == 2 || direccion == 3) { // adelante, derecha, izquierda
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, pwm_izq);
  } else if (direccion == -1) { // atras
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, pwm_izq);
  }

  // Motor derecho
  if (rpm_der == 0) {
    analogWrite(ENB, 0);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  } else if (direccion == 1 || direccion == 2) { // adelante, derecha, izquierda
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, pwm_der);

  }else if (direccion == 3){
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, pwm_der);
  } else if (direccion == -1) { // atras
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, pwm_der);
  }

  delay(30);
}
