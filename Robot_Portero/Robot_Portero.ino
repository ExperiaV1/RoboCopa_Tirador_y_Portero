#include <Servo.h>

// --- PINES DE PERIFÉRICOS ---
const int pinSensor = A0;      // Entrada analógica del micrófono (KY-038)
const int pinLed = 13;         // LED indicador de recepción de pito
const int pinServoHombro = 9;  // Servo 1: Base / Hombro
const int pinServoCodo = 10;   // Servo 2: Codo

// --- OBJETOS Y VARIABLES DE SERVOS ---
Servo servoHombro;
Servo servoCodo;

// --- VARIABLES DE CALIBRACIÓN Y FILTRADO ---
int nivelBaseAmbiente = 0;     
int umbralDeteccion = 0;       
const unsigned long ventanaTiempo = 250; // Tiempo de análisis (ms)
bool enCruce = false;

// Rango de validación del pito del árbitro / robot portero
const float frecMinimaPito = 2400.0; // Hz
const float frecMaximaPito = 2900.0; // Hz

void setup() {
  Serial.begin(9600);
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);

  // Inicialización y postura inicial de servos (90° reposo central)
  servoHombro.attach(pinServoHombro);
  servoCodo.attach(pinServoCodo);
  servoHombro.write(90);
  servoCodo.write(90);

  // --- CALIBRACIÓN AUTOMÁTICA DE RUIDO AMBIENTE ---
  Serial.println("Calibrando ambiente... ¡NO HAGAS RUIDO POR 3 SEGUNDOS!");
  delay(3000); 

  long sumaCalibracion = 0;
  int muestras = 200;
  
  for (int i = 0; i < muestras; i++) {
    sumaCalibracion += analogRead(pinSensor);
    delay(5); 
  }
  
  nivelBaseAmbiente = sumaCalibracion / muestras;
  umbralDeteccion = nivelBaseAmbiente + 80; // Umbral dinámico

  Serial.print("Nivel base ambiente: ");
  Serial.println(nivelBaseAmbiente);
  Serial.print("Umbral de disparo ajustado: ");
  Serial.println(umbralDeteccion);
  Serial.println("¡Sistema de detección y servos listo!");
}

void loop() {
  int lectura = analogRead(pinSensor);

  // Verificación de disparo por nivel de audio superior al umbral
  if (lectura > umbralDeteccion) {
    digitalWrite(pinLed, HIGH);

    unsigned long tiempoInicio = millis();
    int ciclos = 0;
    int lecturaMaxima = 0;

    // Ventana de conteo activo de ciclos de onda (250 ms)
    while (millis() - tiempoInicio < ventanaTiempo) {
      int lecturaActual = analogRead(pinSensor);

      if (lecturaActual > lecturaMaxima) {
        lecturaMaxima = lecturaActual;
      }

      // Conteo de cruzamiento por línea base para hallar la frecuencia
      if (lecturaActual > (nivelBaseAmbiente + 15) && !enCruce) {
        ciclos++;
        enCruce = true;
      } else if (lecturaActual <= (nivelBaseAmbiente + 5)) {
        enCruce = false;
      }
    }

    // Cálculo de la frecuencia en Hertz (Hz)
    float frecuenciaHz = (float)ciclos / (ventanaTiempo / 1000.0);

    Serial.print("¡PITO DETECTADO! | Ciclos: ");
    Serial.print(ciclos);
    Serial.print(" | Frecuencia: ");
    Serial.print(frecuenciaHz, 2);
    Serial.print(" Hz | Pico ADC: ");
    Serial.println(lecturaMaxima);

    digitalWrite(pinLed, LOW);

    // --- EVALUACIÓN Y MOVIMIENTO DEL BRAZO ROBÓTICO ---
    // Filtro de validación: Comprueba que sea una frecuencia real dentro del rango
    if (frecuenciaHz >= frecMinimaPito && frecuenciaHz <= frecMaximaPito) {
      
      // OPCIÓN 1: PORTERO A LA DERECHA (2400 Hz - 2599 Hz)
      if (frecuenciaHz >= 2400.0 && frecuenciaHz < 2600.0) {
        Serial.println(">> MENÚ: PORTERO A LA DERECHA <<");
        
        // Servo 1 (Base/Hombro): Estaba en 90° y se mueve a 0°
        servoHombro.write(0);
        // Servo 2 (Codo): Estaba en 90° y se mueve a 180°
        servoCodo.write(180);

        // Mantiene la postura inmóvil por 1 minuto (60,000 ms)
        Serial.println("Manteniendo postura por 1 minuto...");
        delay(60000);
      }
      
      // OPCIÓN 2: PORTERO AL CENTRO (2600 Hz - 2750 Hz)
      else if (frecuenciaHz >= 2600.0 && frecuenciaHz <= 2750.0) {
        Serial.println(">> MENÚ: PORTERO EN EL CENTRO <<");
        
        // Servo 2 (Codo): Se queda centrado en 90°
        servoCodo.write(90);

        // Servo 1 (Base/Hombro): Oscila ±15° sobre la base de 90° (75° a 105°)
        for (int i = 0; i < 3; i++) { // Realiza 3 ciclos de oscilación rápida
          servoHombro.write(105); // 15° a la derecha
          delay(200);
          servoHombro.write(75);  // 15° a la izquierda
          delay(200);
        }
        // Al final regresa y se queda fijo en 90°
        servoHombro.write(90);

        // Mantiene la postura inmóvil por 1 minuto (60,000 ms)
        Serial.println("Manteniendo postura por 1 minuto...");
        delay(60000);
      }
      
      // OPCIÓN 3: PORTERO A LA IZQUIERDA (2750 Hz - 2900 Hz)
      else if (frecuenciaHz > 2750.0 && frecuenciaHz <= 2900.0) {
        Serial.println(">> MENÚ: PORTERO A LA IZQUIERDA <<");
        
        // Servo 1 (Base/Hombro): Mueve a la izquierda a 180°
        servoHombro.write(180);
        // Servo 2 (Codo): Se repliega a 0°
        servoCodo.write(0);

        // Mantiene la postura inmóvil por 1 minuto (60,000 ms)
        Serial.println("Manteniendo postura por 1 minuto...");
        delay(60000);
      }

      // Regresar los servos a la postura base de reposo al terminar el minuto
      servoHombro.write(90);
      servoCodo.write(90);
      Serial.println("Reactivando monitoreo de audio...");
    } else {
      Serial.println("Frecuencia fuera del rango objetivo. Ruido descartado.");
    }
  }
}