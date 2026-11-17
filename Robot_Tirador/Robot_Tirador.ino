#include <Servo.h>

/*
  ROBOT FUTBOLISTA
  Arduino UNO
  Sensor de sonido HW-484
  Puente H L298N
  Motor DC
  Servomotor SG90
*/

// -------------------------------
// PINES DEL SENSOR
// -------------------------------

const int SENSOR = 2;

// -------------------------------
// PINES DEL PUENTE H
// -------------------------------

const int ENA = 5;
const int IN1 = 8;
const int IN2 = 9;

// -------------------------------
// PIN DEL SERVOMOTOR
// -------------------------------

const int PIN_SERVO = 6;

Servo servoMotor;

// -------------------------------
// CONFIGURACION
// -------------------------------

const int VELOCIDAD = 255;

const unsigned long TIEMPO_MOTOR = 1200;

const unsigned long TIEMPO_ESPERA = 500;

const int SONIDO_ACTIVO = LOW;

// Posiciones del servo
const int SERVO_INICIAL = 0;
const int SERVO_DERECHA = 90;

// -------------------------------
// VARIABLES
// -------------------------------

bool motorEncendido = false;
bool sistemaPreparado = true;

bool servoActivado = false;

unsigned long tiempoActivacion = 0;
unsigned long ultimoSonido = 0;

// -------------------------------
// CONFIGURACION INICIAL
// -------------------------------

void setup() {

  Serial.begin(9600);

  pinMode(SENSOR, INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Configurar servomotor
  servoMotor.attach(PIN_SERVO);

  // Posicion inicial
  servoMotor.write(SERVO_INICIAL);

  detenerMotor();

  Serial.println("ROBOT FUTBOLISTA");
  Serial.println("Esperando silbato...");

}

// -------------------------------
// PROGRAMA PRINCIPAL
// -------------------------------

void loop() {

  int estadoSensor = digitalRead(SENSOR);

  // Detectar el silbato
  if (estadoSensor == SONIDO_ACTIVO) {

    ultimoSonido = millis();

    if (sistemaPreparado && !motorEncendido) {

      Serial.println("SILBATO DETECTADO");

      activarMotor();

      // Mover servo 90 grados a la derecha
      if (!servoActivado) {

        servoMotor.write(SERVO_DERECHA);

        servoActivado = true;

        Serial.println("SERVO A 90 GRADOS");

      }

      sistemaPreparado = false;

    }

  }

  // Comprobar si termino el tiempo del motor
  if (motorEncendido) {

    if (millis() - tiempoActivacion >= TIEMPO_MOTOR) {

      detenerMotor();

      Serial.println("MOTOR DETENIDO");

    }

  }

  // Preparar para un nuevo silbato
  if (!sistemaPreparado && !motorEncendido) {

    if (millis() - ultimoSonido >= TIEMPO_ESPERA) {

      sistemaPreparado = true;

      Serial.println("LISTO PARA OTRO SILBATO");

    }

  }

}

// -------------------------------
// ACTIVAR MOTOR
// -------------------------------

void activarMotor() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  analogWrite(ENA, VELOCIDAD);

  tiempoActivacion = millis();

  motorEncendido = true;

}

// -------------------------------
// DETENER MOTOR
// -------------------------------

void detenerMotor() {

  analogWrite(ENA, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  motorEncendido = false;

}