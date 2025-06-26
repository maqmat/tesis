#include <Servo.h>
#include <DHT.h> // Agrega la librería DHT

#define DHTPIN 8        // Pin digital donde está conectado el DHT11
#define DHTTYPE DHT11   // Modelo del sensor

DHT dht(DHTPIN, DHTTYPE);

// === SERVOS ===
int servo1_pos_counter = 0; // 0 es el centro, -10 máximo izquierda, +10 máximo derecha
const int SERVO1_MAX = 140;
const int SERVO1_MIN = -140;
const int LED=12;

int servo2_pos_counter = 0; // 0 es el centro, -5 máximo abajo, +5 máximo arriba
const int SERVO2_MAX = 5;
const int SERVO2_MIN = -5;

Servo servo1;  // Pin 3
Servo servo2;  // Pin 2
Servo servo3;  // Pin 1
#define pinS1 13
#define pinS2 2
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
const int RPM_BAJO = 25; // Valor de RPM para ruedas lentas en giro

// === VARIABLES DE VELOCIDAD ===
int rpm_izq = 0;
int rpm_der = 0;
int direccion = 0; // 0=detenido, 1=adelante, -1=atras, 2=derecha, 3=izquierda

// === CONTROL DE SERVOS POR TIEMPO ===
unsigned long servo1_timer = 0;
unsigned long servo2_timer = 0;
unsigned long servo3_timer = 0;
const unsigned long SERVO_MOVE_TIME = 200; // 0.2 segundos

// === CONTROL DE ENVÍO DE GAS POR TIEMPO ===
unsigned long last_gas_send = 0;
const unsigned long GAS_SEND_INTERVAL = 100; // 100 ms

// === CONTROL DE ENVÍO DE DHT11 POR TIEMPO ===
unsigned long last_dht_send = 0;
const unsigned long DHT_SEND_INTERVAL = 2000; // 2 segundos

void setup() {
  Serial.begin(9600);

  // Servos
  servo1.attach(pinS1);
  servo2.attach(pinS2);
  pinMode(LED,OUTPUT);

  // Motores
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT); 
  pinMode(ENB, OUTPUT);

  // Sensor de gas
  pinMode(GAS_PIN, INPUT);

  // DHT11
  dht.begin();

  // Motores apagados al inicio
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // Servos en posición neutra
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);

  Serial.println("Sistema listo");
}

void loop() {
  unsigned long now = millis();

  // === LECTURA Y ENVÍO DEL SENSOR DE GAS Y DHT11 SINCRONIZADOS ===
  if (now - last_gas_send >= GAS_SEND_INTERVAL) {
    int gas_value = analogRead(GAS_PIN);
    Serial.print("gas,");
    Serial.println(gas_value);

   

    last_gas_send = now;
  }
  

  // === LECTURA DE COMANDOS SERIAL ===
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    // === CONTROL DE SERVOS ===

    if (cmd.startsWith("servo1,adelante")) {
      if (servo1_pos_counter < SERVO1_MAX) {
        servo1.write(85);
        servo1_timer = now;
        servo1_pos_counter++;
      }
    }else if (cmd.startsWith("servo1,atras")) {
      if (servo1_pos_counter > SERVO1_MIN) {
        servo1.write(100);
        servo1_timer = now;
        servo1_pos_counter--;
      }
    }else if (cmd.startsWith("servo2,adelante")) {
      if (servo2_pos_counter < SERVO2_MAX) {
        servo2.write(98);
        servo3.write(70);
        servo2_timer = now;
        servo3_timer = now;  
        servo2_pos_counter++;
      }
    } else if (cmd.startsWith("servo2,atras")) {
      if (servo2_pos_counter > SERVO2_MIN) {
        servo2.write(85);
        servo3.write(100);
        servo2_timer = now;
        servo3_timer = now;  
        servo2_pos_counter--;
      }
    } else if (cmd.startsWith("servo3,adelante")) {
      servo3.write(85);
      servo3_timer = now;
    } else if (cmd.startsWith("servo3,atras")) {
      servo3.write(100);
      servo3_timer = now;
    } else if (cmd == "detener") {
      servo1.write(90);
      servo2.write(90);
      servo3.write(90);
      servo1_timer = 0;
      servo2_timer = 0;
      servo3_timer = 0;

    }

    // === DIRECCIÓN Y VELOCIDAD AUTOMÁTICA DE MOTORES ==
    if (cmd == "adelante") {
      direccion = 1;
      rpm_izq = -RPM_MAX;
      rpm_der = RPM_MAX;
      Serial.println("Motores adelante");
    } 
    else if (cmd == "atras") {
      direccion = -1;
      rpm_izq = RPM_MAX;
      rpm_der = -RPM_MAX;
      Serial.println("Motores atras");
    } 
    else if (cmd == "luzp") {
      digitalWrite(LED,HIGH);
    } 
    else if (cmd == "lusa") {
      digitalWrite(LED,LOW);

    } if (cmd == "derecha") {
      direccion = 2;
      rpm_izq = RPM_MAX;
      rpm_der = -RPM_BAJO; 
      Serial.println("Girando derecha");
    } else if (cmd == "izquierda") {
      direccion = 3;
      rpm_izq = -RPM_BAJO;
      rpm_der = RPM_MAX;
      Serial.println("Girando izquierda");
    }
    else if (cmd == "detenerm") {
      direccion = 0;
      rpm_izq = 0;
      rpm_der = 0;
      Serial.println("Motores detenidos");
    }
  }

  // === CONTROL DE TIEMPO DE MOVIMIENTO DE SERVOS ===
  if (servo1_timer && (now - servo1_timer >= SERVO_MOVE_TIME)) {
    servo1.write(90);
    servo1_timer = 0;
  }
  if (servo2_timer && (now - servo2_timer >= SERVO_MOVE_TIME)) {
    servo2.write(90);
    servo2_timer = 0;
  }
  if (servo3_timer && (now - servo3_timer >= SERVO_MOVE_TIME)) {
    servo3.write(90);
    servo3_timer = 0;
  }

  // Aplica velocidad y dirección siempre (fuera del if para mantener movimiento)
  int pwm_izq = map(rpm_izq, 0, RPM_MAX, 0, 255);
  int pwm_der = map(rpm_der, 0, RPM_MAX, 0, 255);
  // Motor izquierdo
  if (rpm_izq == 0) {
    analogWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  } else if (rpm_izq > 0) { // adelante
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, map(rpm_izq, 0, RPM_MAX, 0, 255));
  } else { // atrás
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, map(-rpm_izq, 0, RPM_MAX, 0, 255));
  }

  // Motor derecho
  if (rpm_der == 0) {
    analogWrite(ENB, 0);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  } else if (rpm_der > 0) { // adelante
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, map(rpm_der, 0, RPM_MAX, 0, 255));
  } else { // atrás
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, map(-rpm_der, 0, RPM_MAX, 0, 255));
  }

  delay(5); // Pequeño delay para estabilidad
}
