#include <Arduino.h>

// ===================== CONFIGURACIÓN ===================== //
// Periodo de muestreo (2 ms)
int dt_us = 2000;
float dt = dt_us * 0.000001;   // Conversión a segundos

unsigned long k = 0;          // Contador de ciclos

// Variables del encoder
int Np = 0;                   // Número de pulsos
const float R = 0.12587;       // Conversión pulsos → radianes o grados
float th = 0, thp = 0;        // Posición actual y anterior

// Filtro derivada
float dth_d = 0, dth_f = 0;
float alpha = 0.05;           // Filtro pasa-bajas

// Parámetros PID
float kp = 1.1 , kd = 0.5, ki = 0.05;
float e = 0, de = 0, inte = 0;
float u = 0, usat = 0;

// PWM aplicado
float PWM = 0;

// Posición deseada (SET POINT)
float th_des = 0;

// Pines del puente H
const int sen1 = 32;
const int sen2 = 33;

// Para leer desde Serial
String consigna;

// ===================== PROTOTIPOS DE TAREAS ===================== //
void TaskControl(void *pvParameters);
void TaskSerial(void *pvParameters);

// ===================== CONFIGURACIÓN INICIAL ===================== //
void setup() {
  Serial.begin(115200);

  // Interrupciones del encoder
  attachInterrupt(digitalPinToInterrupt(26), CH_A, RISING);
  attachInterrupt(digitalPinToInterrupt(27), CH_B, RISING);

  // Pines de control motor
  pinMode(sen1, OUTPUT);
  pinMode(sen2, OUTPUT);

  // ===== Crear tarea del CONTROL ===== //
  xTaskCreatePinnedToCore(
    TaskControl,
    "Control",
    4096,
    NULL,
    2,
    NULL,
    1
  );

  // ===== Crear tarea del SERIAL ===== //
  xTaskCreatePinnedToCore(
    TaskSerial,
    "Serial",
    4096,
    NULL,
    1,
    NULL,
    0
  );
}

void loop() {
  // FreeRTOS no usa loop
}



// ==========================================================
// TAREA DE CONTROL — SE EJECUTA CADA dt_us (2 ms)
// ==========================================================
void TaskControl(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1)
  {
    // ======== LECTURA DE SENSOR ======== //
    // Conversión de pulsos de encoder a posición
    th = R * Np;

    // Velocidad instantánea
    dth_d = (th - thp) / dt;

    // Filtro derivada
    dth_f = alpha * dth_d + (1 - alpha) * dth_f;

    // ======== CONTROL PID DE POSICIÓN ======== //
    e = th_des - th;      // Error de posición
    de = -dth_f;          // Derivada
    inte += e * dt;       // Integral

    // Ley de control
    u = kp*e + kd*de + ki*inte;

    // Saturación (motor a ±12V)
    usat = constrain(u, -12, 12);

    // Conversión a PWM (12 V → 255)
    PWM = usat * 21.25;

    // ======== APLICAR PWM ======== //
    if (PWM > 0) {
      analogWrite(sen1, PWM);
      analogWrite(sen2, 0);
    } else {
      analogWrite(sen1, 0);
      analogWrite(sen2, -PWM);
    }

    thp = th;

    // ======== IMPRIMIR DATOS ======== //
    Serial.print("th:"); Serial.print(th);
    Serial.print(", e:"); Serial.print(e);
    Serial.print(", PWM:"); Serial.println(PWM);

    // ======== ESPERA EXACTA ======== //
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(dt_us/1000));
  }
}



// ==========================================================
// TAREA SERIAL — LEE EL SETPOINT SIN BLOQUEAR EL CONTROL
// ==========================================================
void TaskSerial(void *pvParameters)
{
  while (1)
  {
    if (Serial.available() > 0)
    {
      consigna = Serial.readStringUntil('\n');
      th_des = consigna.toFloat();   // SET POINT
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}



// ==========================================================
// INTERRUPCIONES DEL ENCODER
// ==========================================================
void CH_A() {
  // Dirección depende del otro canal
  if (digitalRead(27) == LOW) Np++;
  else Np--;
}

void CH_B() {
  if (digitalRead(26) == HIGH) Np++;
  else Np--;
}

