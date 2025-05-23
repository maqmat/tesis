#include <Servo.h>

// === SERVOS (360°) ===
Servo servo1;  // Pin 3
Servo servo2;  // Pin 2

// === MOTORES DC (L298N) ===
const int IN1 = 4;  // Motor izquierdo
const int IN2 = 5;
const int IN3 = 6;  // Motor derecho
const int IN4 = 7;

void setup() {
  Serial.begin(9600);

  // === Configura servos ===
  servo1.attach(3);
  servo2.attach(2);
  servo1.write(90);  // Detenido
  servo2.write(90);

  // === Configura pines de motores ===
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Serial.println("Sistema listo");
}

void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    Serial.println("Recibido: " + cmd);

    // === Control de servos ===
    if (cmd.startsWith("servo2,adelante")) {
      servo2.write(85);   // Giro continuo adelante
    } else if (cmd.startsWith("servo2,atras")) {
      servo2.write(95); // Giro atrás
    } else if (cmd.startsWith("servo3,adelante")) {
      servo1.write(85);
    } else if (cmd.startsWith("servo3,atras")) {
      servo1.write(97);
    } else if (cmd == "detener") {
      servo1.write(90);
      servo2.write(90);
      stopMotors();  // Detener ambos motores
    }

    // === Control de motores ===
    if (cmd == "adelante") {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      Serial.println("Motores: Adelante");
    } else if (cmd == "atras") {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      Serial.println("Motores: Atrás");
    } else if (cmd == "izquierda") {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);  // M1 atrás
      digitalWrite(IN3, HIGH);  // M2 adelante
      digitalWrite(IN4, LOW);
      Serial.println("Motores: Girando izquierda");
    } else if (cmd == "derecha") {
      digitalWrite(IN1, HIGH);  // M1 adelante
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);  // M2 atrás
      Serial.println("Motores: Girando derecha");
    } else if (cmd == "detenerm") {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      Serial.println("Motores: Detenidos");
    }
  }
}

// Función para detener motores
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}